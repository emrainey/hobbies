#include <cstddef>
#include <cstdint>
#include <optional>

#include <gtest/gtest.h>

#include <isa/cpu.hpp>

#include "../demo/program1.hpp"

namespace {

constexpr uint32_t kFlashBase = 0x00100000U;
constexpr uint32_t kFlashLimit = 0x001FFFFFU;
constexpr uint32_t kSramBase = 0x10000000U;
constexpr uint32_t kSramLimit = 0x1000FFFFU;
constexpr uint32_t kStackInitial = 0x10008000U;
constexpr uint32_t kStackBoundary = 0x10000000U;
constexpr uint32_t kExceptionStackInitial = 0x10010000U;
constexpr uint32_t kExceptionStackBoundary = 0x10008000U;

/// Rotates a word left, matching the `brot` the demo program rolls its patterns with.
uint32_t RotateLeft(uint32_t value, uint32_t bits) {
    bits &= 31U;
    return bits == 0U ? value : ((value << bits) | (value >> (32U - bits)));
}

/// The word the demo program should be holding at `index` of the pattern region before anything rolls it.
uint32_t ExpectedPattern(size_t index) {
    if (index < isa::kLiteralCount) {
        return isa::kLiterals[index];
    }
    return static_cast<uint32_t>(index - isa::kLiteralCount + 1U) * isa::kPatternStride;
}

}  // namespace

/// The demo program is a fixture of the demo rather than of the library, but it is program shaped logic: it has loops,
/// it fixes up its own addresses and it is supposed to leave a region of RAM in a known state. These tests load it the
/// way demo/main.cpp does, so a change to it that breaks the loops or the region is caught here rather than by eye.
class DemoProgramTest : public testing::Test {
protected:
    isa::Processor cpu;
    isa::FlashMemory flash{isa::Range{kFlashBase, kFlashLimit}};
    isa::TightlyCoupledMemory sram{isa::Range{kSramBase, kSramLimit}};

    void SetUp() override {
        ASSERT_TRUE(cpu.AttachFlashMemory(flash));
        ASSERT_TRUE(cpu.AddTightlyCoupledMemory(sram));
        cpu.Reset();

        // The same vector table demo/main.cpp installs.
        isa::VectorTable vector_table{
            .stack_initial = kStackInitial,
            .stack_boundary = kStackBoundary,
            .exception_stack_initial = kExceptionStackInitial,
            .exception_stack_boundary = kExceptionStackBoundary,
            .reset_handler = isa::kProgramEntry.value,
        };
        for (size_t i = 0U; i < isa::VectorTableCount; ++i) {
            ASSERT_TRUE(cpu.Poke(isa::Address{kFlashBase} + (i * sizeof(isa::Address)), vector_table[i].value));
        }
        ASSERT_TRUE(cpu.Load(isa::program1, isa::kProgramEntry));
        cpu.Reset();
    }

    /// Index of the instruction the program starts a pass at. It is the only one which clears just the register the
    /// generated words are built in.
    static size_t PassTopIndex() {
        size_t found = isa::program1.size();
        for (size_t i = 0U; i < isa::program1.size(); ++i) {
            const auto& instruction = isa::program1[i];
            if ((instruction.base() == isa::Operator::Zero) && (instruction.clear.type == isa::RegisterType::Scratch)
                && (instruction.clear.mask == (1U << isa::kValueRegister))) {
                EXPECT_EQ(isa::program1.size(), found) << "the program starts a pass in more than one place";
                found = i;
            }
        }
        EXPECT_LT(found, isa::program1.size()) << "the program has no pass to repeat";
        return found;
    }

    /// Decodes an address the setup block builds from a low and an upper 16 bit move into one scratch register.
    /// Evaluation registers are numbered from zero separately, so the evaluation bit has to be checked as well as the
    /// register number.
    static std::optional<uint32_t> SetupConstant(uint32_t reg) {
        uint32_t value = 0U;
        size_t moves = 0U;
        for (size_t i = 0U; i < PassTopIndex(); ++i) {
            const auto& instruction = isa::program1[i];
            if ((instruction.base() != isa::Operator::MoveImmediate) || (instruction.movi.eval != 0U)
                || (instruction.movi.dst != reg)) {
                continue;
            }
            value |= static_cast<uint32_t>(instruction.movi.imm) << (instruction.movi.upper ? 16U : 0U);
            ++moves;
        }
        return (moves == 2U) ? std::optional<uint32_t>{value} : std::nullopt;
    }

    static isa::Address AddressOf(size_t index) {
        return isa::kProgramEntry + (index * sizeof(isa::Instruction));
    }

    static isa::Address PassTopAddress() {
        return AddressOf(PassTopIndex());
    }

    static isa::Address RollLoopAddress() {
        const auto address = SetupConstant(isa::kRollAnchorRegister);
        EXPECT_TRUE(address.has_value()) << "the setup does not build the address of the roll loop";
        return isa::Address{address.value_or(0U)};
    }

    uint32_t WordAt(size_t index) const {
        uint32_t value = 0U;
        EXPECT_TRUE(cpu.Peek(isa::kPatternRamBase + (index * sizeof(uint32_t)), value));
        return value;
    }

    /// Executes until the next instruction to run is the one at `target`.
    void RunUntil(isa::Address target) {
        constexpr size_t kCycleLimit = 100000U;
        size_t cycles = 0U;
        while ((cpu.ViewSpecial().program_address_ != target) && (cycles < kCycleLimit)) {
            if (cpu.IsHalted() || cpu.ViewSpecial().exception_.HasException()) {
                break;
            }
            cpu.Cycle();
            ++cycles;
        }
        EXPECT_LT(cycles, kCycleLimit) << "the program never reached the instruction at " << target;
    }

    /// Executes until the program is about to begin a pass again.
    void RunToNextPass() {
        RunUntil(PassTopAddress());
        EXPECT_FALSE(cpu.IsHalted());
        EXPECT_FALSE(cpu.ViewSpecial().exception_.HasException());
    }
};

/// The loader in demo/main.cpp places the program directly after the vector table, and the entry point in that vector
/// table has to agree with where the program expects to be.
TEST_F(DemoProgramTest, ResetLandsOnTheFirstInstruction) {
    EXPECT_EQ(isa::kProgramEntry, cpu.ViewSpecial().program_address_);
}

/// The two loops and the repeating pass jump to absolute addresses, which the program has to work out for itself
/// because they are inside the block that names them. If that fixup is wrong the program runs off into the flash.
TEST_F(DemoProgramTest, SetupNamesTheLoopsInsideItself) {
    const auto in_program = [](uint32_t address) {
        const uint32_t end = isa::kProgramEntry.value + (isa::program1.size() * sizeof(isa::Instruction));
        return (address >= isa::kProgramEntry.value) && (address < end) && ((address % sizeof(isa::Instruction)) == 0U);
    };
    const auto instruction_at = [](uint32_t address) {
        return isa::program1[(address - isa::kProgramEntry.value) / sizeof(isa::Instruction)];
    };

    // The addresses the program is built around.
    EXPECT_EQ(std::optional<uint32_t>{isa::kPatternRamBase.value}, SetupConstant(isa::kBaseRegister));
    EXPECT_EQ(std::optional<uint32_t>{isa::kPatternRamEnd.value}, SetupConstant(isa::kEndRegister));
    EXPECT_EQ(std::optional<uint32_t>{isa::kPatternSeriesStart.value}, SetupConstant(isa::kSeriesRegister));
    EXPECT_EQ(std::optional<uint32_t>{isa::kPatternStride}, SetupConstant(isa::kStrideRegister));
    EXPECT_EQ(static_cast<uint32_t>(isa::kPatternCount * sizeof(uint32_t)),
              (isa::kPatternRamEnd - isa::kPatternRamBase).value);

    // Each anchor points at the first instruction of the loop or pass it names.
    const auto fill = SetupConstant(isa::kFillAnchorRegister);
    const auto roll = SetupConstant(isa::kRollAnchorRegister);
    const auto pass = SetupConstant(isa::kOuterAnchorRegister);
    ASSERT_TRUE(fill.has_value());
    ASSERT_TRUE(roll.has_value());
    ASSERT_TRUE(pass.has_value());
    EXPECT_TRUE(in_program(*fill));
    EXPECT_TRUE(in_program(*roll));
    EXPECT_TRUE(in_program(*pass));
    EXPECT_EQ(isa::Operator::Addition, instruction_at(*fill).base());
    EXPECT_EQ(isa::Operator::Load, instruction_at(*roll).base());
    EXPECT_EQ(isa::Operator::Zero, instruction_at(*pass).base());
    EXPECT_EQ(PassTopAddress().value, *pass);

    // The pass ends by jumping back to its own top, unconditionally, so the program never stops.
    const auto& repeat = isa::program1.back();
    EXPECT_EQ(isa::Operator::Leap, repeat.base());
    EXPECT_EQ(0U, repeat.leap.cond);
    EXPECT_EQ(isa::kOuterAnchorRegister, repeat.leap.dst);
}

/// The words the program is supposed to leave in RAM. Checked in the middle of the first pass, before anything has
/// been rolled, and again at the end of the pass once every word has.
TEST_F(DemoProgramTest, FillsTheRegionThenRollsItOnEveryPass) {
    // Step to the top of the roll loop: the whole region has been written and nothing has been rolled yet.
    RunUntil(RollLoopAddress());
    for (size_t i = 0U; i < isa::kPatternCount; ++i) {
        EXPECT_EQ(ExpectedPattern(i), WordAt(i)) << "word " << i << " before any rolling";
    }

    // Each pass writes the patterns again and rolls each of them once more. Because the patterns are rewritten, the
    // region returns to the same unrolled words at the start of every pass, so what the memory window shows is the
    // legible patterns and then the same patterns rolled by 8, over and over.
    for (size_t pass = 1U; pass <= 4U; ++pass) {
        RunToNextPass();
        for (size_t i = 0U; i < isa::kPatternCount; ++i) {
            EXPECT_EQ(RotateLeft(ExpectedPattern(i), 8U), WordAt(i)) << "word " << i << " at the end of pass " << pass;
        }
    }

    // And the program is still running, not halted and not faulted.
    EXPECT_FALSE(cpu.IsHalted());
    EXPECT_FALSE(cpu.ViewSpecial().exception_.HasException());
}
