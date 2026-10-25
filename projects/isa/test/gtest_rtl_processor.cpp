#include <cstddef>
#include <cstdint>
#include <initializer_list>

#include <gtest/gtest.h>

#include <Vprocessor.h>
#include <Vprocessor___024root.h>
#include <isa/cpu.hpp>

namespace {

constexpr uint32_t kFlashBase = 0x00100000U;
constexpr uint32_t kFlashLimit = 0x0010FFFFU;
constexpr uint32_t kCodeAddress = 0x00100100U;
constexpr uint32_t kStackInitial = 0x0010F000U;
constexpr uint32_t kStackBoundary = 0x00100000U;
constexpr uint32_t kExceptionStackInitial = 0x0010E000U;
constexpr uint32_t kExceptionStackBoundary = 0x0010D000U;
constexpr uint32_t kRetiredSteps = 32U;

/// Encodes a typed instruction into the word both models fetch.
template <typename I>
uint32_t Encode(I const& instruction) {
    return isa::instructions::Instruction{instruction}.raw;
}

}  // namespace

/// Lockstep harness for M0: the C++ model is the oracle and the RTL is stepped one
/// retired instruction at a time. Both models share `oracle`'s attached flash, which
/// services the DUT bus exactly as `Processor::Peek` services the fast model.
class RtlProcessorTest : public testing::Test {
protected:
    isa::Processor oracle;
    isa::FlashMemory flash{isa::Range{kFlashBase, kFlashLimit}};
    Vprocessor dut;

    void SetUp() override {
        ASSERT_TRUE(oracle.AttachFlashMemory(flash));

        // Vector table, matching isa::VectorTable field order.
        ASSERT_TRUE(oracle.Poke(isa::Address{kFlashBase + 0U}, kStackInitial));
        ASSERT_TRUE(oracle.Poke(isa::Address{kFlashBase + 4U}, kStackBoundary));
        ASSERT_TRUE(oracle.Poke(isa::Address{kFlashBase + 8U}, kExceptionStackInitial));
        ASSERT_TRUE(oracle.Poke(isa::Address{kFlashBase + 12U}, kExceptionStackBoundary));
        ASSERT_TRUE(oracle.Poke(isa::Address{kFlashBase + 16U}, kCodeAddress));

        // An erased flash decodes as a stream of noops.
        for (uint32_t offset = 0U; offset < kRetiredSteps * 4U; offset += 4U) {
            ASSERT_TRUE(oracle.Poke(isa::Address{kCodeAddress + offset}, 0xFFFFFFFFU));
        }

        oracle.Reset();

        dut.reset = 1;
        Clock();
        dut.reset = 0;

        ASSERT_TRUE(RunUntilBooted());
    }

    /// Architectural register state lives in the register-file submodules.
    uint32_t Scratch(size_t i) {
        return dut.rootp->processor__DOT__u_scratch__DOT__scratch[i];
    }
    uint32_t Evaluation(size_t i) {
        return dut.rootp->processor__DOT__u_evaluation__DOT__evaluation[i];
    }

    /// Services one bus access from the DUT using the oracle's memory.
    void ServiceBus() {
        uint32_t value = 0U;
        dut.mem_rdata = 0U;
        if (dut.mem_req && oracle.Peek(isa::Address{static_cast<uint32_t>(dut.mem_addr)}, value)) {
            dut.mem_rdata = value;
            dut.mem_fault = 0;
        } else {
            dut.mem_fault = dut.mem_req ? 1 : 0;
        }
    }

    /// Drives one rising clock edge, with the bus serviced before the edge.
    void Clock() {
        dut.clk = 0;
        dut.eval();
        ServiceBus();
        dut.eval();
        dut.clk = 1;
        dut.eval();
    }

    bool RunUntilBooted() {
        for (int i = 0; i < 100 && !dut.booted; ++i) {
            Clock();
        }
        return dut.booted;
    }

    bool RunUntilRetire() {
        for (int i = 0; i < 1000; ++i) {
            Clock();
            if (dut.retire) {
                return true;
            }
            if (dut.halted) {
                return false;
            }
        }
        return false;
    }

    void ExpectStateMatches() {
        for (size_t i = 0; i < isa::CountOfScratchRegisters; ++i) {
            EXPECT_EQ(oracle.ViewScratch()[i].as_u32[0], Scratch(i)) << "scratch[" << i << "]";
        }
        for (size_t i = 0; i < isa::CountOfEvaluationRegisters; ++i) {
            EXPECT_EQ(oracle.ViewEvaluations()[i].value, Evaluation(i)) << "evaluation[" << i << "]";
        }
        for (size_t i = 0; i < isa::CountOfSpecialRegisters; ++i) {
            EXPECT_EQ(oracle.ViewSpecial().as_word(i).as_u32[0], dut.special[i]) << "special[" << i << "]";
        }
        EXPECT_EQ(oracle.IsHalted(), static_cast<bool>(dut.halted));
    }

    /// Pokes `program` at the code address, then steps both models one retired
    /// instruction at a time and diffs the whole architectural state after each.
    /// Every instruction must be one this milestone implements, so `unsupported`
    /// staying low is part of the assertion.
    void RunProgram(std::initializer_list<uint32_t> program) {
        uint32_t address = kCodeAddress;
        for (uint32_t const word : program) {
            ASSERT_TRUE(oracle.Poke(isa::Address{address}, word));
            address += sizeof(uint32_t);
        }

        for (size_t step = 0; step < program.size(); ++step) {
            SCOPED_TRACE(::testing::Message() << "step " << step);
            oracle.Cycle();
            ASSERT_TRUE(RunUntilRetire()) << "the RTL did not retire";
            EXPECT_FALSE(dut.unsupported) << "instruction not implemented by this milestone";
            ExpectStateMatches();
        }
    }
};

TEST_F(RtlProcessorTest, ResetStateMatchesTheFastModel) {
    ExpectStateMatches();
    for (size_t i = 0; i < isa::CountOfScratchRegisters; ++i) {
        EXPECT_EQ(0U, Scratch(i)) << "scratch[" << i << "]";
    }
    for (size_t i = 0; i < isa::CountOfEvaluationRegisters; ++i) {
        EXPECT_EQ(0U, Evaluation(i)) << "evaluation[" << i << "]";
    }
}

TEST_F(RtlProcessorTest, ExposedStateWidthsMatchTheFastModel) {
    EXPECT_EQ(isa::CountOfScratchRegisters, std::size(dut.rootp->processor__DOT__u_scratch__DOT__scratch));
    EXPECT_EQ(isa::CountOfEvaluationRegisters, std::size(dut.rootp->processor__DOT__u_evaluation__DOT__evaluation));
    EXPECT_EQ(isa::CountOfSpecialRegisters, std::size(dut.special));
}

TEST_F(RtlProcessorTest, ErasedProgramRetiresNoOpsInLockstep) {
    for (uint32_t step = 0U; step < kRetiredSteps; ++step) {
        SCOPED_TRACE(::testing::Message() << "step " << step);
        oracle.Cycle();
        ASSERT_TRUE(RunUntilRetire());
        EXPECT_FALSE(dut.unsupported) << "the erased word is a noop, not an unimplemented opcode";
        ExpectStateMatches();
    }
}

// === M1: the register file and the no-memory instructions ===
//
// Registers reset to zero; programs seed nonzero values when needed to make
// copy, shift, and conditional behavior observable.

TEST_F(RtlProcessorTest, MoveShiftsScratchAndClearsTheSource) {
    using isa::Operand;
    using namespace isa::instructions;
    RunProgram({
        // S2 = 0x1234_5678, built one half at a time.
        Encode(MoveImmediate{Operand{Operand::Type::Scratch, 2}, isa::Immediate<16>{0x5678U}}),
        Encode(MoveImmediate{Operand{Operand::Type::Scratch, 2}, isa::Immediate<16>{0x1234U}, true}),
        // S1 = S2 << 3, then S2 is cleared because it is not the destination.
        Encode(Move{Operand{Operand::Type::Scratch, 1}, Operand{Operand::Type::Scratch, 2}, false,
                    Move::ImmediateType{3}}),
        // S4 = 0x8000_0000 ...
        Encode(MoveImmediate{Operand{Operand::Type::Scratch, 4}, isa::Immediate<16>{0x0000U}}),
        Encode(MoveImmediate{Operand{Operand::Type::Scratch, 4}, isa::Immediate<16>{0x8000U}, true}),
        Encode(MoveImmediate{Operand{Operand::Type::Scratch, 5}, isa::Immediate<16>{0xFFFFU}}),
        Encode(MoveImmediate{Operand{Operand::Type::Scratch, 5}, isa::Immediate<16>{0xFFFFU}, true}),
        // ... then S3 = S4 >> 4 arithmetically.
        Encode(
            Move{Operand{Operand::Type::Scratch, 3}, Operand{Operand::Type::Scratch, 4}, true, Move::ImmediateType{4}}),
        // Same register on both sides: shifted in place, nothing cleared.
        Encode(Move{Operand{Operand::Type::Scratch, 5}, Operand{Operand::Type::Scratch, 5}, false,
                    Move::ImmediateType{2}}),
    });

    EXPECT_EQ(0x91A2B3C0U, oracle.ViewScratch()[1].as_u32[0]);  // 0x12345678 << 3
    EXPECT_EQ(0U, oracle.ViewScratch()[2].as_u32[0]);           // source cleared
    EXPECT_EQ(0xF8000000U, oracle.ViewScratch()[3].as_u32[0]);  // arithmetic, not logical, shift
    EXPECT_EQ(0U, oracle.ViewScratch()[4].as_u32[0]);
    EXPECT_EQ(0xFFFFFFFCU, oracle.ViewScratch()[5].as_u32[0]);  // in-place, so nothing cleared
}

TEST_F(RtlProcessorTest, MoveShiftsEvaluationWithinEightBits) {
    using isa::Operand;
    using namespace isa::instructions;
    RunProgram({
        Encode(MoveImmediate{Operand{Operand::Type::Evaluation, 2}, isa::Immediate<16>{0x00FFU}}),
        // E1 = E2, E2 cleared.
        Encode(Move{Operand{Operand::Type::Evaluation, 1}, Operand{Operand::Type::Evaluation, 2}}),
        Encode(MoveImmediate{Operand{Operand::Type::Evaluation, 4}, isa::Immediate<16>{0x00FFU}}),
        // E3 = E4 >> 4 == 0x0F, E4 cleared.
        Encode(Move{Operand{Operand::Type::Evaluation, 3}, Operand{Operand::Type::Evaluation, 4}, true,
                    Move::ImmediateType{4}}),
        Encode(MoveImmediate{Operand{Operand::Type::Evaluation, 6}, isa::Immediate<16>{0x00FFU}}),
        // E5 = E6 << 4 == 0xF0: the shift truncates at the byte, not the word.
        Encode(Move{Operand{Operand::Type::Evaluation, 5}, Operand{Operand::Type::Evaluation, 6}, false,
                    Move::ImmediateType{4}}),
        Encode(MoveImmediate{Operand{Operand::Type::Evaluation, 7}, isa::Immediate<16>{0x00FFU}}),
        // E7 = E7 << 8 == 0: every bit shifted out.
        Encode(Move{Operand{Operand::Type::Evaluation, 7}, Operand{Operand::Type::Evaluation, 7}, false,
                    Move::ImmediateType{8}}),
    });

    EXPECT_EQ(0xFFU, oracle.ViewEvaluations()[1].value);
    EXPECT_EQ(0x00U, oracle.ViewEvaluations()[2].value);
    EXPECT_EQ(0x0FU, oracle.ViewEvaluations()[3].value);
    EXPECT_EQ(0x00U, oracle.ViewEvaluations()[4].value);
    EXPECT_EQ(0xF0U, oracle.ViewEvaluations()[5].value);
    EXPECT_EQ(0x00U, oracle.ViewEvaluations()[6].value);
    EXPECT_EQ(0x00U, oracle.ViewEvaluations()[7].value);
}

TEST_F(RtlProcessorTest, ConditionalMoveIsGatedByTheEvaluationAndMask) {
    using isa::Operand;
    using namespace isa::instructions;
    RunProgram({
        Encode(MoveImmediate{Operand{Operand::Type::Evaluation, 0}, isa::Immediate<16>{1U}}),
        Encode(MoveImmediate{Operand{Operand::Type::Scratch, 2}, isa::Immediate<16>{0x2222U}}),
        Encode(MoveImmediate{Operand{Operand::Type::Scratch, 4}, isa::Immediate<16>{0x4444U}}),
        // zero.e 0x0002: E1 = 0.
        Encode(Zero{Operand{Operand::Type::Mask, isa::Immediate<16>{0x0002U}}, isa::RegisterType::Evaluation}),
        // E0 & E1 == 0, so the move is blocked and S2 stays unchanged.
        Encode(Move{Operand{Operand::Type::Evaluation, 0}, Operand{Operand::Type::Evaluation, 1},
                    Operand{Operand::Type::Scratch, 1}, Operand{Operand::Type::Scratch, 2}}),
        // E1 = 1, so E0 & E1 != 0 and the same move is now taken.
        Encode(MoveImmediate{Operand{Operand::Type::Evaluation, 1}, isa::Immediate<16>{1U}}),
        Encode(Move{Operand{Operand::Type::Evaluation, 0}, Operand{Operand::Type::Evaluation, 1},
                    Operand{Operand::Type::Scratch, 3}, Operand{Operand::Type::Scratch, 4}}),
    });

    EXPECT_EQ(0U, oracle.ViewScratch()[1].as_u32[0]);
    EXPECT_EQ(0x2222U, oracle.ViewScratch()[2].as_u32[0]);  // blocked, so not cleared
    EXPECT_EQ(0x4444U, oracle.ViewScratch()[3].as_u32[0]);
    EXPECT_EQ(0x00000000U, oracle.ViewScratch()[4].as_u32[0]);  // taken, so cleared
    EXPECT_EQ(0x01U, oracle.ViewEvaluations()[1].value);        // masked to 8 bits
}

TEST_F(RtlProcessorTest, MoveImmediateWritesHalvesAndMasksEvaluationDestination) {
    using isa::Operand;
    using namespace isa::instructions;
    RunProgram({
        // The low half only; the reset high half remains zero.
        Encode(MoveImmediate{Operand{Operand::Type::Scratch, 5}, isa::Immediate<16>{0xABCDU}}),
        Encode(MoveImmediate{Operand{Operand::Type::Scratch, 5}, isa::Immediate<16>{0x1234U}, true}),
        // An evaluation destination takes the low 8 bits of the immediate.
        Encode(MoveImmediate{Operand{Operand::Type::Evaluation, 9}, isa::Immediate<16>{0x01FFU}}),
    });

    EXPECT_EQ(0x1234ABCDU, oracle.ViewScratch()[5].as_u32[0]);
    EXPECT_EQ(0xFFU, oracle.ViewEvaluations()[9].value);
}

TEST_F(RtlProcessorTest, CopyWritesTheComparisonEvaluationOnlyWhenConditional) {
    using isa::Operand;
    using namespace isa::instructions;
    RunProgram({
        Encode(MoveImmediate{Operand{Operand::Type::Scratch, 2}, isa::Immediate<16>{0x1234U}}),
        Encode(Copy{Operand{Operand::Type::Scratch, 1}, Operand{Operand::Type::Scratch, 2}}),
        Encode(Copy{Operand{Operand::Type::Scratch, 3}, Operand{Operand::Type::Scratch, 4},
                    Operand{Operand::Type::Evaluation, 2}}),
    });

    EXPECT_EQ(0x1234U, oracle.ViewScratch()[1].as_u32[0]);
    EXPECT_EQ(0x1234U, oracle.ViewScratch()[2].as_u32[0]);  // copy leaves the source alone
    EXPECT_EQ(0U, oracle.ViewEvaluations()[0].value);       // unconditional: untouched
    EXPECT_EQ(0x09U, oracle.ViewEvaluations()[2].value);    // type = comparison, equal
}

TEST_F(RtlProcessorTest, ZeroClearsOnlyTheMaskedRegisters) {
    using isa::Operand;
    using namespace isa::instructions;
    RunProgram({
        Encode(MoveImmediate{Operand{Operand::Type::Scratch, 1}, isa::Immediate<16>{0x1111U}}),
        Encode(MoveImmediate{Operand{Operand::Type::Scratch, 3}, isa::Immediate<16>{0x3333U}}),
        Encode(MoveImmediate{Operand{Operand::Type::Evaluation, 0}, isa::Immediate<16>{0x00FFU}}),
        Encode(MoveImmediate{Operand{Operand::Type::Evaluation, 8}, isa::Immediate<16>{0x00FFU}}),
        Encode(MoveImmediate{Operand{Operand::Type::Evaluation, 9}, isa::Immediate<16>{0x00FFU}}),
        Encode(Zero{Operand{Operand::Type::Mask, isa::Immediate<16>{0x0005U}}}),
        Encode(Zero{Operand{Operand::Type::Mask, isa::Immediate<16>{0x0300U}}, isa::RegisterType::Evaluation}),
    });

    EXPECT_EQ(0x00000000U, oracle.ViewScratch()[0].as_u32[0]);
    EXPECT_EQ(0x1111U, oracle.ViewScratch()[1].as_u32[0]);
    EXPECT_EQ(0x00000000U, oracle.ViewScratch()[2].as_u32[0]);
    EXPECT_EQ(0x3333U, oracle.ViewScratch()[3].as_u32[0]);
    for (size_t i = 0; i < isa::CountOfEvaluationRegisters; ++i) {
        uint8_t const expected = i == 0U ? 0xFFU : 0U;
        EXPECT_EQ(expected, oracle.ViewEvaluations()[i].value) << "evaluation[" << i << "]";
    }
}

TEST_F(RtlProcessorTest, HaltStopsBothModelsAndHolds) {
    using isa::Operand;
    using namespace isa::instructions;
    RunProgram({
        Encode(MoveImmediate{Operand{Operand::Type::Scratch, 1}, isa::Immediate<16>{0x1234U}}),
        Encode(Halt{}),
    });

    EXPECT_TRUE(oracle.IsHalted());
    EXPECT_TRUE(dut.halted);

    // Both models hold: the C++ model returns immediately from Cycle(), and the RTL
    // stops fetching and stops pulsing retire.
    uint32_t const program_address = dut.special[isa::special::ProgramAddress];
    uint32_t const counter = dut.special[isa::special::PerformanceCounter];
    for (int i = 0; i < 4; ++i) {
        Clock();
        EXPECT_FALSE(dut.retire) << "retire pulsed after halt";
    }
    oracle.Cycle();
    EXPECT_EQ(program_address, dut.special[isa::special::ProgramAddress]);
    EXPECT_EQ(counter, dut.special[isa::special::PerformanceCounter]);
    ExpectStateMatches();
}

TEST_F(RtlProcessorTest, OpcodesBeyondThisMilestoneAreFlagged) {
    // 0x18 is a defined opcode (`leap`) that M1 does not execute. The RTL retires it
    // as a noop and says so; the C++ model performs the leap, so the state is
    // expected to differ and is not compared.
    ASSERT_TRUE(oracle.Poke(isa::Address{kCodeAddress}, 0x00000018U));
    oracle.Cycle();
    ASSERT_TRUE(RunUntilRetire());
    EXPECT_TRUE(dut.unsupported) << "defined but unimplemented opcode";
    EXPECT_EQ(kCodeAddress + 4U, dut.special[isa::special::ProgramAddress]);
}

TEST_F(RtlProcessorTest, UndefinedOpcodeIsFlagged) {
    // 0x38 lies between the defined opcodes 0x34 (`trip`) and 0x3C (`load`). The C++
    // model raises an instruction fault and redirects through the vector table, which
    // is M8; the RTL only reports that it could not execute it.
    ASSERT_TRUE(oracle.Poke(isa::Address{kCodeAddress}, 0x00000038U));
    oracle.Cycle();
    ASSERT_TRUE(RunUntilRetire());
    EXPECT_TRUE(dut.unsupported) << "undefined opcode";
    EXPECT_EQ(1U, dut.special[isa::special::PerformanceCounter]) << "still retires exactly once";
}
