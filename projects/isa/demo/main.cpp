#include <basal/basal.hpp>
#include <basal/options.hpp>
#include <isa/isa.hpp>
#include <isa/register_view.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>
#include <ftxui/screen/string.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace ftxui;

namespace {

constexpr size_t kMemoryBytesPerRow = 16U;
constexpr size_t kMemoryPageRows = 8U;
constexpr size_t kConsoleMaxLines = 128U;
constexpr size_t kInstructionSize = sizeof(isa::instructions::Instruction);
constexpr char kUnavailableMemoryCells[] = "-- -- -- --  -- -- -- --  -- -- -- --  -- -- -- --";

template <typename RegisterRows>
Elements BuildRegisterColumn(RegisterRows const& rows) {
    Elements column;
    column.reserve(rows.size());

    for (auto const& row : rows) {
        column.push_back(text(row));
    }

    return column;
}

std::string FormatAddressHex(isa::Address address) {
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(8)
           << static_cast<unsigned int>(address.value);
    return stream.str();
}

bool TryAddAddress(isa::Address address, size_t offset, isa::Address& out) {
    constexpr isa::Address kAddressMax{std::numeric_limits<isa::Address::StorageType>::max()};
    const auto safe_offset = static_cast<isa::Address::StorageType>(offset);
    if (isa::Address{safe_offset} > kAddressMax - address) {
        return false;
    }
    out = address + safe_offset;
    return true;
}

isa::Address SaturatingAddressAdd(isa::Address address, size_t offset) {
    isa::Address result{0};
    if (TryAddAddress(address, offset, result)) {
        return result;
    }
    return isa::Address{std::numeric_limits<isa::Address::StorageType>::max()};
}

isa::Address SaturatingAddressSub(isa::Address address, size_t offset) {
    const auto safe_offset = static_cast<isa::Address::StorageType>(offset);
    if (isa::Address{safe_offset} > address) {
        return isa::Address{0};
    }
    return address - safe_offset;
}

std::string StripLineBreaks(std::string input) {
    input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
    input.erase(std::remove(input.begin(), input.end(), '\r'), input.end());
    return input;
}

bool TryParseAddress(std::string input, isa::Address& out) {
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
        if (parsed > static_cast<unsigned long long>(std::numeric_limits<isa::Address::StorageType>::max())) {
            return false;
        }
        out = isa::Address{parsed};
        return true;
    } catch (...) {
        return false;
    }
}

std::string FormatMemoryByte(isa::Processor const& cpu, isa::Address address) {
    uint32_t value = 0U;
    if (not cpu.Peek(address, value)) {
        return "--";
    }

    std::ostringstream stream;
    stream << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << (value & 0xFFU);
    return stream.str();
}

std::string FormatMemoryRow(isa::Processor const& cpu, isa::Address row_base_address) {
    std::ostringstream stream;
    stream << FormatAddressHex(row_base_address) << ": ";

    for (size_t i = 0; i < kMemoryBytesPerRow; ++i) {
        isa::Address address{0};
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
    basal::options::config opts[] = {
        {"-d", "--dir", std::string("."), "Directory for saving/loading CPU state"},
    };

    basal::options::process(basal::dimof(opts), opts, argc, argv);
    const std::filesystem::path state_directory = std::get<std::string>(opts[0].value);

    isa::Processor cpu;  // resets internally

    // Start "Config"

    isa::TightlyCoupledMemory sram0{isa::Range{0x10000000, 0x1000FFFF}};  // 64KB SRAM
    cpu.AddTightlyCoupledMemory(sram0);

    // Start "Loader"

    auto& scratch = cpu.GetScratch();
    scratch[0] = isa::word<32>{0x00001000U};
    scratch[1] = isa::word<32>{0x00000010U};
    scratch[2] = isa::word<32>{0x00000011U};
    scratch[3] = isa::word<32>{0x5555AAAAU};
    scratch[4] = isa::word<32>{0xABCDEF01U};
    scratch[5] = isa::word<32>{0x12345678U};

    auto& special = cpu.GetSpecial();
    special.program_address_ = isa::Address{0x10000000UL};
    special.return_address_ = isa::Address{0x10000000UL};
    special.stack_.limit = isa::Address{0x10000000UL};
    special.stack_.current = isa::Address{0x0FFFFFF0UL};
    special.stack_.base = isa::Address{0x0FFF0000UL};
    special.exception_stack_.limit = isa::Address{0x11000000UL};
    special.exception_stack_.current = isa::Address{0x10FFFF00UL};
    special.exception_stack_.base = isa::Address{0x10FF0000UL};

    auto& evaluation = cpu.GetEvaluations();
    evaluation[0].comparison = 1U;
    evaluation[0].greater_than = 1U;
    evaluation[0].non_zero = 1U;

    isa::program demo_program = {
        isa::instructions::Instruction{isa::instructions::ClearScratch{
            isa::Operand{isa::OperandType::Mask, 0xFFFF}}},
        isa::instructions::Instruction{isa::instructions::NoOp{}},
        isa::instructions::Instruction{isa::instructions::MoveImmediateToScratch{
            isa::Operand{isa::OperandType::Scratch, 0}, isa::Immediate<20>{0x12345}}},
        isa::instructions::Instruction{isa::instructions::MoveScratchToScratch{
            isa::Operand{isa::OperandType::Scratch, 1}, isa::Operand{isa::OperandType::Scratch, 0}}},
        isa::instructions::Instruction{isa::instructions::SwapScratch{
            isa::Operand{isa::OperandType::Scratch, 0}, isa::Operand{isa::OperandType::Scratch, 4}}},
        isa::instructions::Instruction{isa::instructions::SwapEvaluation{
            isa::Operand{isa::OperandType::Evaluation, 0}, isa::Operand{isa::OperandType::Evaluation, 1}}},
    };
    // copy the instruction to SRAM
    for (size_t i = 0; i < basal::dimof(demo_program); ++i) {
        cpu.Poke(isa::Address{0x10000000UL + i * sizeof(isa::instructions::Instruction)}, demo_program[i].raw);
    }

    // End "Loader"

    constexpr isa::Address kSramStartAddress = isa::memory::Map[2].range.start;
    isa::Address memory_base_address = kSramStartAddress;
    std::string memory_base_input = FormatAddressHex(memory_base_address);
    std::string memory_view_status = "Enter base address (hex) and press Enter";
    std::vector<std::string> console_lines = {
        "Console ready",
        "State directory: " + state_directory.string(),
        "Press s to save, l to load",
    };

    const auto push_console_line = [&](std::string const& line) {
        console_lines.push_back(line);
        if (console_lines.size() > kConsoleMaxLines) {
            console_lines.erase(
                console_lines.begin(),
                console_lines.begin() + static_cast<std::ptrdiff_t>(console_lines.size() - kConsoleMaxLines));
        }
    };

    const auto update_memory_base_input = [&] {
        memory_base_input = FormatAddressHex(memory_base_address);
        memory_view_status = "Viewing from " + memory_base_input;
    };

    const auto update_persistence_status = [&](isa::PersistenceReport const& report) {
        push_console_line(report.summary);
        for (auto const& file_name : report.files) {
            push_console_line("  " + file_name);
        }
    };

    const auto save_cpu_state = [&] {
        std::error_code error;
        std::filesystem::create_directories(state_directory, error);
        if (error) {
            push_console_line("Save failed: could not create " + state_directory.string());
            return;
        }

        update_persistence_status(cpu.Save(state_directory.string()));
    };

    const auto load_cpu_state = [&] {
        auto const report = cpu.Load(state_directory.string());
        update_persistence_status(report);
        if (report.success) {
            update_memory_base_input();
            memory_view_status = "Loaded state; viewing from " + memory_base_input;
        }
    };

    InputOption memory_base_input_options;
    memory_base_input_options.multiline = false;
    memory_base_input_options.on_enter = [&] {
        memory_base_input = StripLineBreaks(memory_base_input);

        isa::Address parsed{0};
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

    const auto asm_panel = [&cpu](int asm_height) {
        const auto& special = cpu.ViewSpecial();
        const isa::Address pc = special.program_address_;
        // Align PC to instruction boundary
        const isa::Address aligned_pc
            = pc & isa::Address{~(static_cast<isa::Address::StorageType>(kInstructionSize) - 1U)};

        // Calculate how many instruction rows fit in the panel (2 rows for the window border)
        const int visible_rows = std::max(1, asm_height - 2);

        // Show a couple of instructions before the PC for context
        constexpr size_t kLookBack = 2U;
        const isa::Address asm_base = SaturatingAddressSub(aligned_pc, kLookBack * kInstructionSize);

        Elements rows;
        for (int i = 0; i < visible_rows; ++i) {
            isa::Address addr{0};
            if (!TryAddAddress(asm_base, static_cast<size_t>(i) * kInstructionSize, addr)) {
                rows.push_back(text("  [end of address space]"));
                continue;
            }

            std::ostringstream line;
            line << FormatAddressHex(addr) << "  ";

            uint32_t raw_value = 0U;
            if (cpu.Peek(addr, raw_value)) {
                isa::instructions::Instruction instr;
                instr.raw = raw_value;
                line << instr;
            } else {
                line << "";
            }

            const bool is_pc = (addr == aligned_pc);
            if (is_pc) {
                rows.push_back(text("> " + line.str()) | bold | inverted);
            } else {
                rows.push_back(text("  " + line.str()));
            }
        }

        return window(text(" Assembly "), vbox(std::move(rows)) | flex);
    };

    const auto registers_panel = [&cpu] {
        const auto scratch_rows = isa::FormatScratchRegisterRows(cpu);
        const auto special_rows = isa::FormatSpecialRegisterRows(cpu);
        const auto eval_rows = isa::FormatEvaluationRegisterRows(cpu);
        return window(text(" Registers "), hbox({
                                               vbox({
                                                   text("Scratch"),
                                                   separator(),
                                                   vbox(BuildRegisterColumn(scratch_rows)) | flex,
                                               }) | flex,
                                               separator(),
                                               vbox({
                                                   text("Evaluation"),
                                                   separator(),
                                                   vbox(BuildRegisterColumn(eval_rows)) | flex,
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
        rows.push_back(text("Keys: Up/Down/PageUp/PageDown/Home move view"));
        rows.push_back(separator());

        const int visible_rows = std::max(1, memory_height - static_cast<int>(rows.size()) - 1);
        for (int row = 0; row < visible_rows; ++row) {
            isa::Address row_address{0};
            const size_t row_offset = static_cast<size_t>(row) * kMemoryBytesPerRow;
            if (TryAddAddress(memory_base_address, row_offset, row_address)) {
                rows.push_back(text(FormatMemoryRow(cpu, row_address)));
            } else {
                rows.push_back(
                    text(FormatAddressHex(isa::Address{std::numeric_limits<isa::Address::StorageType>::max()}) + ": "
                         + kUnavailableMemoryCells));
            }
        }

        return window(text(" Memory "), vbox(std::move(rows)) | flex);
    };

    const auto console_panel = [&](int panel_height) {
        Elements rows;
        rows.push_back(text("Keys: s save, l load, e execute cycle, q quit"));
        rows.push_back(separator());

        const int visible_rows = std::max(1, panel_height - static_cast<int>(rows.size()) - 1);
        const int start = std::max(0, static_cast<int>(console_lines.size()) - visible_rows);
        for (size_t i = static_cast<size_t>(start); i < console_lines.size(); ++i) {
            rows.push_back(text(console_lines[i]));
        }

        return window(text(" Console "), vbox(std::move(rows)) | flex);
    };

    auto app = Renderer(controls, [&] {
        const auto dimensions = Terminal::Size();
        const int container_height = std::max(8, dimensions.dimy - 2);
        const int top_height = std::max(3, (container_height * 2) / 3);
        const int memory_height = std::max(3, container_height - top_height - 1);

        const auto top_row = hbox({
                                 asm_panel(top_height) | flex,
                                 separator(),
                                 registers_panel() | flex,
                             })
                             | flex;

        const auto bottom_row = hbox({
                                    memory_panel(memory_height) | flex,
                                    separator(),
                                    console_panel(memory_height) | size(WIDTH, GREATER_THAN, 28) | flex,
                                })
                                | flex;

        return vbox({
                   top_row | size(HEIGHT, EQUAL, top_height),
                   separator(),
                   bottom_row | size(HEIGHT, EQUAL, memory_height),
               })
               | border | flex;
    });

    app = CatchEvent(app, [&](Event event) {
        if (event == Event::Return) {
            memory_base_input = StripLineBreaks(memory_base_input);

            isa::Address parsed{0};
            if (TryParseAddress(memory_base_input, parsed)) {
                memory_base_address = parsed;
                update_memory_base_input();
            } else {
                memory_view_status = "Invalid address, expected hexadecimal value";
            }
            return true;
        }

        if (event == Event::Character("s") || event == Event::Character("S")) {
            save_cpu_state();
            return true;
        }
        if (event == Event::Character("l") || event == Event::Character("L")) {
            load_cpu_state();
            return true;
        }
        if (event == Event::Character("e") || event == Event::Character("E")) {
            cpu.Cycle();
            push_console_line("Executed cycle");
            return true;
        }
        if (event == Event::Character("q") || event == Event::Character("Q")) {
            screen.Exit();
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