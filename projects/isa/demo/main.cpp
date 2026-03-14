#include <isa/isa.hpp>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>
#include <ftxui/screen/string.hpp>

#include <algorithm>

using namespace ftxui;

namespace {

template <typename RegisterRows>
Elements BuildRegisterColumn(RegisterRows const& rows) {
    Elements column;
    column.reserve(rows.size());

    for (auto const& row : rows) {
        column.push_back(text(row));
    }

    return column;
}

}  // namespace

int main(int argc, char* argv[]) {
    isa::MemoryBus bus0{0U, isa::Range{0x00000000, 0x0000FFFF}};
    isa::MemoryBus bus1{1U, isa::Range{0x00010000, 0x0001FFFF}};
    isa::Processor cpu; // resets internally

    // Start "Config"

    isa::TightlyCoupledMemory sram0{isa::Range{0x10000000, 0x1000FFFF}}; // 64KB SRAM
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
        return window(text(" Registers "),
                      hbox({
                          vbox({
                              text("Scratch"),
                              separator(),
                              vbox(BuildRegisterColumn(scratch_rows)) | flex,
                          })
                              | flex,
                          separator(),
                          vbox({
                              text("Special"),
                              separator(),
                              vbox(BuildRegisterColumn(special_rows)) | flex,
                          })
                              | flex,
                      }) | flex);
    };

    const auto memory_panel = [] {
        return window(text(" Memory "), vbox({
                                            text("0x1000: 11 00 00 00  2A 00 00 00  FF 7F 00 00  00 00 00 00"),
                                            text("0x1010: 08 00 10 00  0C 00 10 00  18 00 00 00  00 00 00 00"),
                                            text("0x1020: 4D 4F 56 20  52 31 2C 20  23 31 30 00  00 00 00 00"),
                                        }) | flex);
    };

    auto app = Renderer([&] {
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
                   memory_panel() | size(HEIGHT, EQUAL, memory_height),
               })
               | border | flex;
    });
    screen.Loop(app);
    return 0;
}