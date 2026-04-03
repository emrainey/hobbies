#include <basal/basal.hpp>
#include <basal/options.hpp>
#include <isa/isa.hpp>

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

#include "demo_memory_view.hpp"
#include "program1.hpp"
#include "demo_register_view.hpp"

using namespace ftxui;
using namespace isa;
using namespace instructions;
namespace isa_demo = isa::demo;

namespace {

constexpr size_t kMemoryPageRows = 8U;
constexpr size_t kConsoleMaxLines = 128U;
constexpr size_t kInstructionSize = sizeof(Instruction);

template <typename RegisterRows>
Elements BuildRegisterColumn(RegisterRows const& rows) {
    Elements column;
    column.reserve(rows.size());

    for (auto const& row : rows) {
        column.push_back(text(row));
    }

    return column;
}

std::string FormatAddressHex(Address address) {
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(8)
           << static_cast<unsigned int>(address.value);
    return stream.str();
}

bool TryAddAddress(Address address, size_t offset, Address& out) {
    constexpr Address kAddressMax{std::numeric_limits<Address::StorageType>::max()};
    const auto safe_offset = static_cast<Address::StorageType>(offset);
    if (Address{safe_offset} > kAddressMax - address) {
        return false;
    }
    out = address + safe_offset;
    return true;
}

Address SaturatingAddressAdd(Address address, size_t offset) {
    Address result{0};
    if (TryAddAddress(address, offset, result)) {
        return result;
    }
    return Address{std::numeric_limits<Address::StorageType>::max()};
}

Address SaturatingAddressSub(Address address, size_t offset) {
    const auto safe_offset = static_cast<Address::StorageType>(offset);
    if (Address{safe_offset} > address) {
        return Address{0};
    }
    return address - safe_offset;
}

std::string StripLineBreaks(std::string input) {
    input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
    input.erase(std::remove(input.begin(), input.end(), '\r'), input.end());
    return input;
}

bool TryParseAddress(std::string input, Address& out) {
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
        if (parsed > static_cast<unsigned long long>(std::numeric_limits<Address::StorageType>::max())) {
            return false;
        }
        out = Address{parsed};
        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    basal::options::config opts[] = {
        {"-d", "--dir", std::string("."), "Directory for saving/loading CPU state"},
    };

    basal::options::process(basal::dimof(opts), opts, argc, argv);
    const std::filesystem::path state_directory = std::get<std::string>(opts[0].value);

    Processor cpu;  // resets internally but nothing attached yet, so we'll need to reset again after attaching
                    // memory to get the correct entry point loaded.

    // Start "Config"

    FlashMemory flash0{Range{0x00100000, 0x001FFFFF}};  // Small 1MB Flash
    cpu.AttachFlashMemory(flash0);
    TightlyCoupledMemory sram0{Range{0x10000000, 0x1000FFFF}};  // 64KB SRAM
    cpu.AddTightlyCoupledMemory(sram0);

    // Start "Loader"

    cpu.Reset();  // reset after attaching memory to get the correct entry point loaded

    auto& scratch = cpu.GetScratch();
    auto& special = cpu.GetSpecial();
    auto& evaluation = cpu.GetEvaluations();

    program demo_program;

    // copy
    demo_program = program1;

    // copy the default vector table to ROM so that the CPU can fetch the entry point from it on reset
    uint32_t vector_table_offset = sizeof(VectorTable);
    VectorTable vector_table{
        .stack_initial = 0x10008000U,
        .stack_boundary = 0x10000000U,
        .exception_stack_initial = 0x10010000U,
        .exception_stack_boundary = 0x10008000U,
        .reset_handler
        = flash0.ViewRange().start + vector_table_offset,  // point the reset handler to the start of our demo program
    };
    Address tmp = flash0.ViewRange().start;
    for (size_t i = 0; i < VectorTableCount; ++i) {
        cpu.Poke(tmp + (i * sizeof(Address)), vector_table[i].value);
    }
    // load the program into flash at the appropriate offset so that the reset handler will point to it correctly.
    cpu.Load(demo_program, flash0.ViewRange().start + vector_table_offset);

    // Cause the cpu to reset back to the entry point of the demo program, which will allow us to test the persistence
    // of the CPU state after we load it later.
    cpu.Reset();

    // End "Loader"

    Address memory_base_address = DefaultVectorTableAddress;
    isa_demo::MemoryViewMode memory_view_mode = isa_demo::MemoryViewMode::Byte;
    std::string memory_base_input = FormatAddressHex(memory_base_address);
    std::string memory_view_status = "Enter base address (hex) and press Enter";
    std::vector<std::string> console_lines = {
        "Console ready",
        "State directory: " + state_directory.string(),
        "Press F1 for help",
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

    const auto set_memory_view_mode = [&](isa_demo::MemoryViewMode mode) {
        if (memory_view_mode == mode) {
            return;
        }

        memory_view_mode = mode;
        push_console_line("Memory view: " + std::string(isa_demo::MemoryViewModeLabel(memory_view_mode)));
    };

    const auto cycle_memory_view_mode = [&] {
        isa_demo::MemoryViewMode next_mode = isa_demo::MemoryViewMode::Byte;
        if (memory_view_mode == isa_demo::MemoryViewMode::Byte) {
            next_mode = isa_demo::MemoryViewMode::HalfWord;
        } else if (memory_view_mode == isa_demo::MemoryViewMode::HalfWord) {
            next_mode = isa_demo::MemoryViewMode::Word;
        }
        set_memory_view_mode(next_mode);
    };

    const auto update_persistence_status = [&](PersistenceReport const& report) {
        push_console_line(report.summary);
        for (auto const& file_name : report.files) {
            push_console_line("  " + file_name);
        }
    };

    const auto show_help = [&] {
        push_console_line("Keys:");
        push_console_line("  Enter apply base address");
        push_console_line("  Up/Down move by row");
        push_console_line("  PageUp/PageDown move by page");
        push_console_line("  Home jump to SRAM");
        push_console_line("  F6 cycle memory view (byte/short/word)");
        push_console_line("  F1 show help");
        push_console_line("  F2 save state");
        push_console_line("  F3 load state");
        push_console_line("  F4 execute cycle");
        push_console_line("  F5 reset CPU");
        push_console_line("  Esc quit");
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

        Address parsed{0};
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
        const Address pc = special.program_address_;
        // Align PC to instruction boundary
        const Address aligned_pc = pc & Address{~(static_cast<Address::StorageType>(kInstructionSize) - 1U)};

        // Calculate how many instruction rows fit in the panel (2 rows for the window border)
        const int visible_rows = std::max(1, asm_height - 2);

        // Show a couple of instructions before the PC for context
        constexpr size_t kLookBack = 2U;
        const Address asm_base = SaturatingAddressSub(aligned_pc, kLookBack * kInstructionSize);

        Elements rows;
        for (int i = 0; i < visible_rows; ++i) {
            Address addr{0};
            if (!TryAddAddress(asm_base, static_cast<size_t>(i) * kInstructionSize, addr)) {
                rows.push_back(text("  [end of address space]"));
                continue;
            }

            std::ostringstream line;
            line << FormatAddressHex(addr) << "  ";

            uint32_t raw_value = 0U;
            if (cpu.Peek(addr, raw_value)) {
                Instruction instr;
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
        const auto scratch_rows = isa_demo::FormatScratchRegisterRows(cpu);
        const auto special_rows = isa_demo::FormatSpecialRegisterRows(cpu);
        const auto eval_rows = isa_demo::FormatEvaluationRegisterRows(cpu);
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
        rows.push_back(text("Mode: " + std::string(isa_demo::MemoryViewModeLabel(memory_view_mode))));
        rows.push_back(text("Keys: Up/Down/PageUp/PageDown/Home move, F6 cycle view, F1 help"));
        rows.push_back(separator());

        const int visible_rows = std::max(1, memory_height - static_cast<int>(rows.size()) - 1);
        for (int row = 0; row < visible_rows; ++row) {
            Address row_address{0};
            const size_t row_offset = static_cast<size_t>(row) * isa_demo::MemoryViewBytesPerRow;
            if (TryAddAddress(memory_base_address, row_offset, row_address)) {
                rows.push_back(text(isa_demo::FormatMemoryRow(cpu, row_address, memory_view_mode)));
            } else {
                rows.push_back(text(isa_demo::FormatUnavailableMemoryRow(
                    Address{std::numeric_limits<Address::StorageType>::max()}, memory_view_mode)));
            }
        }

        return window(text(" Memory "), vbox(std::move(rows)) | flex);
    };

    const auto console_panel = [&](int panel_height) {
        Elements rows;
        rows.push_back(text("Keys: F2 save, F3 load, F4 execute cycle, F5 reset, Esc quit"));
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

            Address parsed{0};
            if (TryParseAddress(memory_base_input, parsed)) {
                memory_base_address = parsed;
                update_memory_base_input();
            } else {
                memory_view_status = "Invalid address, expected hexadecimal value";
            }
            return true;
        }

        if (event == Event::F2) {
            save_cpu_state();
            return true;
        }
        if (event == Event::F3) {
            load_cpu_state();
            return true;
        }
        if (event == Event::F4) {
            cpu.Cycle();
            push_console_line("Executed cycle");
            if (cpu.ViewSpecial().exception_.HasException()) {
                push_console_line(">>>Exception occurred!<<<");
            }
            if (cpu.IsHalted()) {
                push_console_line("CPU is halted. Press F5 to reset.");
            }
            return true;
        }
        if (event == Event::F5) {
            cpu.Reset();
            push_console_line("CPU Reset");
            return true;
        }
        if (event == Event::Escape) {
            screen.Exit();
            return true;
        }

        if (event == Event::F1) {
            show_help();
            return true;
        }

        if (event == Event::F6) {
            cycle_memory_view_mode();
            return true;
        }

        const size_t row_step = isa_demo::MemoryViewBytesPerRow;
        const size_t page_step = isa_demo::MemoryViewBytesPerRow * kMemoryPageRows;

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
            memory_base_address = DefaultSramStartAddress;
            update_memory_base_input();
            return true;
        }
        return false;
    });

    screen.Loop(app);
    return 0;
}