#include <isa/isa.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>
#include <ftxui/screen/string.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

using namespace ftxui;

namespace {

constexpr size_t kMemoryBytesPerRow = 16U;
constexpr size_t kMemoryPageRows = 8U;
constexpr char kUnavailableMemoryCells[] = "-- -- -- --  -- -- -- --  -- -- -- --  -- -- -- --";
using Address32 = std::uint32_t;

template <typename RegisterRows>
Elements BuildRegisterColumn(RegisterRows const& rows) {
    Elements column;
    column.reserve(rows.size());

    for (auto const& row : rows) {
        column.push_back(text(row));
    }

    return column;
}

std::string FormatAddressHex(Address32 address) {
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(8)
           << static_cast<unsigned int>(address);
    return stream.str();
}

bool TryAddAddress(Address32 address, size_t offset, Address32& out) {
    constexpr Address32 kAddressMax = std::numeric_limits<Address32>::max();
    const auto safe_offset = static_cast<Address32>(offset);
    if (safe_offset > kAddressMax - address) {
        return false;
    }
    out = static_cast<Address32>(address + safe_offset);
    return true;
}

Address32 SaturatingAddressAdd(Address32 address, size_t offset) {
    Address32 result = 0;
    if (TryAddAddress(address, offset, result)) {
        return result;
    }
    return std::numeric_limits<Address32>::max();
}

Address32 SaturatingAddressSub(Address32 address, size_t offset) {
    const auto safe_offset = static_cast<Address32>(offset);
    if (safe_offset > address) {
        return 0;
    }
    return static_cast<Address32>(address - safe_offset);
}

std::string StripLineBreaks(std::string input) {
    input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
    input.erase(std::remove(input.begin(), input.end(), '\r'), input.end());
    return input;
}

bool TryParseAddress(std::string input, Address32& out) {
    input = StripLineBreaks(std::move(input));

    auto trim = [](std::string& value) {
        while (not value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
            value.erase(value.begin());
        }
        while (not value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
            value.pop_back();
        }
    };

    trim(input);
    if (input.empty()) {
        return false;
    }

    if ((input.size() > 2U) && input[0] == '0' && (input[1] == 'x' || input[1] == 'X')) {
        input.erase(0, 2U);
    }

    if (input.empty()) {
        return false;
    }

    for (char c : input) {
        if (not std::isxdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }

    try {
        const auto parsed = std::stoull(input, nullptr, 16);
        if (parsed > static_cast<unsigned long long>(std::numeric_limits<Address32>::max())) {
            return false;
        }
        out = static_cast<Address32>(parsed);
        return true;
    } catch (...) {
        return false;
    }
}

std::string FormatMemoryByte(isa::Processor const& cpu, Address32 address) {
    uint32_t value = 0U;
    if (not cpu.Peek(static_cast<isa::Address>(address), value)) {
        return "--";
    }

    std::ostringstream stream;
    stream << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << (value & 0xFFU);
    return stream.str();
}

std::string FormatMemoryRow(isa::Processor const& cpu, Address32 row_base_address) {
    std::ostringstream stream;
    stream << FormatAddressHex(row_base_address) << ": ";

    for (size_t i = 0; i < kMemoryBytesPerRow; ++i) {
        Address32 address = 0;
        if (TryAddAddress(row_base_address, i, address)) {
            stream << FormatMemoryByte(cpu, address);
        } else {
            stream << "--";
        }

        if (i + 1U < kMemoryBytesPerRow) {
            stream << ((i % 4U == 3U) ? "  " : " ");
        }
    }

    return stream.str();
}

}  // namespace

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    isa::Processor cpu;  // resets internally

    // Start "Config"

    isa::TightlyCoupledMemory sram0{isa::Range{0x10000000, 0x1000FFFF}};  // 64KB SRAM
    cpu.AddTightlyCoupledMemory(sram0);

    // Start "Loader"

    auto& scratch = cpu.GetScratch();
    scratch[0] = isa::word<32>{0x00001000U};
    scratch[1] = isa::word<32>{0x00000010U};
    scratch[2] = isa::word<32>{0x00000011U};
    scratch[3] = isa::word<32>{0x00012345U};
    scratch[4] = isa::word<32>{0xABCDEF01U};

    auto& special = cpu.GetSpecial();
    special.program_address_ = static_cast<isa::Address>(0x0000000000000008UL);
    special.return_address_ = static_cast<isa::Address>(0x0000000000000010UL);
    special.stack_.limit = static_cast<isa::Address>(0x0000000010000000UL);
    special.stack_.current = static_cast<isa::Address>(0x000000000FFFFFF0UL);
    special.stack_.base = static_cast<isa::Address>(0x000000000FFF0000UL);
    special.exception_stack_.limit = static_cast<isa::Address>(0x0000000011000000UL);
    special.exception_stack_.current = static_cast<isa::Address>(0x0000000010FFFF00UL);
    special.exception_stack_.base = static_cast<isa::Address>(0x0000000010FF0000UL);

    isa::program demo_program = {
        isa::instructions::Instruction{isa::instructions::NoOp{}},
        isa::instructions::Instruction{isa::instructions::MoveScratchToScratch{
            isa::Operand{isa::OperandType::Scratch, 1}, isa::Operand{isa::OperandType::Scratch, 2}}},
        isa::instructions::Instruction{isa::instructions::MoveImmediateToScratch{
            isa::Operand{isa::OperandType::Scratch, 3}, isa::Immediate<20>{0x12345}}},
    };

    // End "Loader"

    (void)demo_program;

    constexpr Address32 kSramStartAddress = static_cast<Address32>(isa::memory::Map[2].range.start);
    Address32 memory_base_address = kSramStartAddress;
    std::string memory_base_input = FormatAddressHex(memory_base_address);
    std::string memory_view_status = "Enter base address (hex) and press Enter";

    const auto update_memory_base_input = [&] {
        memory_base_input = FormatAddressHex(memory_base_address);
        memory_view_status = "Viewing from " + memory_base_input;
    };

    InputOption memory_base_input_options;
    memory_base_input_options.multiline = false;
    memory_base_input_options.on_enter = [&] {
        memory_base_input = StripLineBreaks(memory_base_input);

        Address32 parsed = 0;
        if (TryParseAddress(memory_base_input, parsed)) {
            memory_base_address = parsed;
            update_memory_base_input();
        } else {
            memory_view_status = "Invalid address, expected hexadecimal value";
        }
    };

    Component memory_base_input_component = Input(&memory_base_input, "0x10000000", memory_base_input_options);
    Component controls = Container::Vertical({memory_base_input_component});

    auto screen = ScreenInteractive::TerminalOutput();

    const auto asm_panel = [] {
        return window(text(" Assembly "), vbox({
                                              text("0x0000  MOV R1, #0x10"),
                                              text("0x0004  ADD R2, R1, #0x01"),
                                              text("0x0008  STR R2, [R0]"),
                                              text("0x000C  B   0x0018"),
                                          }) | flex);
    };

    const auto registers_panel = [&cpu] {
        const auto scratch_rows = isa::FormatScratchRegisterRows(cpu);
        const auto special_rows = isa::FormatSpecialRegisterRows(cpu);
        return window(text(" Registers "), hbox({
                                               vbox({
                                                   text("Scratch"),
                                                   separator(),
                                                   vbox(BuildRegisterColumn(scratch_rows)) | flex,
                                               }) | flex,
                                               separator(),
                                               vbox({
                                                   text("Special"),
                                                   separator(),
                                                   vbox(BuildRegisterColumn(special_rows)) | flex,
                                               }) | flex,
                                           }) | flex);
    };

    const auto memory_panel = [&](int memory_height) {
        Elements rows;
        rows.push_back(hbox({text("Base: "), memory_base_input_component->Render() | flex}));
        rows.push_back(text(memory_view_status));
        rows.push_back(text("Use Up/Down/PageUp/PageDown/Home to move view"));
        rows.push_back(separator());

        const int visible_rows = std::max(1, memory_height - 5);
        for (int row = 0; row < visible_rows; ++row) {
            Address32 row_address = 0;
            const size_t row_offset = static_cast<size_t>(row) * kMemoryBytesPerRow;
            if (TryAddAddress(memory_base_address, row_offset, row_address)) {
                rows.push_back(text(FormatMemoryRow(cpu, row_address)));
            } else {
                rows.push_back(
                    text(FormatAddressHex(std::numeric_limits<Address32>::max()) + ": " + kUnavailableMemoryCells));
            }
        }

        return window(text(" Memory "), vbox(std::move(rows)) | flex);
    };

    auto app = Renderer(controls, [&] {
        const auto dimensions = Terminal::Size();
        const int container_height = std::max(8, dimensions.dimy - 2);
        const int top_height = std::max(3, (container_height * 2) / 3);
        const int memory_height = std::max(3, container_height - top_height - 1);

        const auto top_row = hbox({
                                 asm_panel() | flex,
                                 separator(),
                                 registers_panel() | flex,
                             })
                             | flex;

        return vbox({
                   top_row | size(HEIGHT, EQUAL, top_height),
                   separator(),
                   memory_panel(memory_height) | size(HEIGHT, EQUAL, memory_height),
               })
               | border | flex;
    });

    app = CatchEvent(app, [&](Event event) {
        if (event == Event::Return) {
            memory_base_input = StripLineBreaks(memory_base_input);

            Address32 parsed = 0;
            if (TryParseAddress(memory_base_input, parsed)) {
                memory_base_address = parsed;
                update_memory_base_input();
            } else {
                memory_view_status = "Invalid address, expected hexadecimal value";
            }
            return true;
        }

        const size_t row_step = kMemoryBytesPerRow;
        const size_t page_step = kMemoryBytesPerRow * kMemoryPageRows;

        if (event == Event::ArrowUp) {
            memory_base_address = SaturatingAddressSub(memory_base_address, row_step);
            update_memory_base_input();
            return true;
        }
        if (event == Event::ArrowDown) {
            memory_base_address = SaturatingAddressAdd(memory_base_address, row_step);
            update_memory_base_input();
            return true;
        }
        if (event == Event::PageUp) {
            memory_base_address = SaturatingAddressSub(memory_base_address, page_step);
            update_memory_base_input();
            return true;
        }
        if (event == Event::PageDown) {
            memory_base_address = SaturatingAddressAdd(memory_base_address, page_step);
            update_memory_base_input();
            return true;
        }
        if (event == Event::Home) {
            memory_base_address = kSramStartAddress;
            update_memory_base_input();
            return true;
        }
        return false;
    });

    screen.Loop(app);
    return 0;
}