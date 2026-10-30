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
constexpr uint32_t kFaultHandler = 0x00100200U;
constexpr uint32_t kCodeAddress = 0x00100100U;
constexpr uint32_t kStackInitial = 0x0010F000U;
constexpr uint32_t kStackBoundary = 0x00100000U;
constexpr uint32_t kExceptionStackInitial = 0x0010E000U;
constexpr uint32_t kExceptionStackBoundary = 0x0010D000U;
constexpr uint32_t kRetiredSteps = 32U;
constexpr int kMaxWaitCycles = 10000;

/// Encodes a typed instruction into the word both models fetch.
template <typename I>
uint32_t Encode(I const& instruction) {
    return isa::instructions::Instruction{instruction}.raw;
}

}  // namespace

/// Lockstep harness for the RTL processor. The C++ model is the oracle and both
/// models share its attached flash through a variable-latency bus responder.
class RtlProcessorTest : public testing::Test {
protected:
    isa::Processor oracle;
    isa::FlashMemory flash{isa::Range{kFlashBase, kFlashLimit}};
    Vprocessor dut;
    uint32_t request_delay_cycles = 0;
    uint32_t request_cycles_remaining = 0;
    uint32_t response_delay_cycles = 0;
    uint32_t bus_request_count = 0;
    bool bus_pending = false;
    uint32_t response_cycles_remaining = 0;
    uint32_t response_data = 0;
    bool response_fault = false;

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

    /// Drives one clock edge and advances the one-outstanding-transaction responder.
    void Clock() {
        dut.clk = 0;
        dut.mem_req_ready = !bus_pending && request_cycles_remaining == 0;
        dut.mem_rsp_valid = bus_pending && response_cycles_remaining == 0;
        dut.mem_rsp_data = response_data;
        dut.mem_rsp_fault = response_fault;
        dut.eval();

        const bool request_accepted = dut.mem_req_valid && dut.mem_req_ready;
        const bool response_accepted = dut.mem_rsp_valid && dut.mem_rsp_ready;
        const uint32_t request_address = static_cast<uint32_t>(dut.mem_addr);

        dut.clk = 1;
        dut.eval();

        if (response_accepted) {
            bus_pending = false;
        } else if (bus_pending && response_cycles_remaining > 0) {
            --response_cycles_remaining;
        }

        if (request_accepted) {
            ASSERT_FALSE(bus_pending);
            ++bus_request_count;
            response_fault = !oracle.Peek(isa::Address{request_address}, response_data);
            response_cycles_remaining = response_delay_cycles;
            bus_pending = true;
            request_cycles_remaining = request_delay_cycles;
        } else if (dut.mem_req_valid && request_cycles_remaining > 0) {
            --request_cycles_remaining;
        }
    }

    bool RunUntilBooted() {
        for (int i = 0; i < kMaxWaitCycles && !dut.booted; ++i) {
            Clock();
        }
        return dut.booted;
    }

    bool RunUntilRetire() {
        for (int i = 0; i < kMaxWaitCycles; ++i) {
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

    /// Pokes `program` after whatever was run before, then steps both models one
    /// retired instruction at a time and diffs the whole architectural state after each.
    /// Every instruction must be one this milestone implements, so `unsupported`
    /// staying low is part of the assertion.
    uint32_t next_program_address = kCodeAddress;

    void RunProgram(std::initializer_list<uint32_t> program) {
        RunProgram_Vector(std::vector<uint32_t>{program});
    }

    void RunProgram_Vector(std::vector<uint32_t> const& program) {
        for (uint32_t const word : program) {
            ASSERT_TRUE(oracle.Poke(isa::Address{next_program_address}, word));
            next_program_address += sizeof(uint32_t);
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

TEST_F(RtlProcessorTest, FetchWaitsForAnArbitrarilyDelayedMemoryResponse) {
    constexpr uint32_t kRequestDelayCycles = 17;
    constexpr uint32_t kResponseDelayCycles = 257;
    request_delay_cycles = kRequestDelayCycles;
    request_cycles_remaining = kRequestDelayCycles;
    response_delay_cycles = kResponseDelayCycles;

    uint32_t const requests_before_fetch = bus_request_count;
    oracle.Cycle();
    uint32_t const stalled_fetch_address = kCodeAddress;
    uint32_t const total_delay = kRequestDelayCycles + kResponseDelayCycles + 2;
    for (uint32_t cycle = 0; cycle <= total_delay; ++cycle) {
        Clock();
        EXPECT_FALSE(dut.retire) << "retired before the fetch response, cycle " << cycle;
        EXPECT_LE(bus_request_count, requests_before_fetch + 1U)
            << "the outstanding fetch must not be issued more than once";
        if (cycle < kRequestDelayCycles) {
            EXPECT_TRUE(dut.mem_req_valid);
            EXPECT_EQ(stalled_fetch_address, dut.mem_addr) << "request fields changed while backpressured";
            EXPECT_EQ(requests_before_fetch, bus_request_count) << "request accepted before ready";
        } else {
            EXPECT_EQ(requests_before_fetch + 1U, bus_request_count) << "request was not accepted after ready";
        }
        EXPECT_EQ(kCodeAddress, dut.special[isa::special::ProgramAddress]);
    }

    ASSERT_TRUE(RunUntilRetire()) << "the processor did not resume after the response";
    EXPECT_EQ(requests_before_fetch + 1U, bus_request_count);
    ExpectStateMatches();
}

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

namespace {

/// Loads a full 32-bit value into a scratch register with two `movei` halves.
void LoadWord(std::vector<uint32_t>& program, uint8_t index, uint32_t value) {
    using isa::Operand;
    using namespace isa::instructions;
    Operand const dst{Operand::Type::Scratch, index};
    program.push_back(Encode(MoveImmediate{dst, isa::Immediate<16>{value & 0xFFFFU}}));
    program.push_back(Encode(MoveImmediate{dst, isa::Immediate<16>{value >> 16U}, true}));
}

}  // namespace

TEST_F(RtlProcessorTest, LoadReadsWordAndSelectedByteAndHalfWordLanes) {
    using isa::Operand;
    using namespace isa::instructions;
    constexpr uint32_t kDataAddress = kFlashBase + 0x800U;
    ASSERT_TRUE(oracle.Poke(isa::Address{kDataAddress}, 0xD4C3B2A1U));
    ASSERT_TRUE(oracle.Poke(isa::Address{kDataAddress + 4U}, 0x44332211U));

    Operand const base{Operand::Type::Scratch, 1};
    std::vector<uint32_t> program;
    LoadWord(program, 1, kDataAddress);
    program.push_back(Encode(Load::Byte(Operand{Operand::Type::Scratch, 2}, base, 0)));
    program.push_back(Encode(Load::Byte(Operand{Operand::Type::Scratch, 3}, base, 2)));
    program.push_back(Encode(Load::HalfWord(Operand{Operand::Type::Scratch, 4}, base, 1)));
    program.push_back(Encode(Load::Word(Operand{Operand::Type::Scratch, 5}, base)));
    program.push_back(Encode(Load::Word(Operand{Operand::Type::Scratch, 7}, base, Load::ImmediateType{4U}, false)));
    program.push_back(Encode(Load::Word(Operand{Operand::Type::Scratch, 6}, base, Load::ImmediateType{4U}, true)));
    request_delay_cycles = 3;
    response_delay_cycles = 5;

    RunProgram_Vector(program);

    EXPECT_EQ(0xA1U, Scratch(2));
    EXPECT_EQ(0xC3U, Scratch(3));
    EXPECT_EQ(0xD4C3U, Scratch(4));
    EXPECT_EQ(0xD4C3B2A1U, Scratch(5));
    EXPECT_EQ(0xD4C3B2A1U, Scratch(6));
    EXPECT_EQ(0x44332211U, Scratch(7));
    EXPECT_EQ(kDataAddress + 4U, Scratch(1)) << "post-index load increments the base";
}

TEST_F(RtlProcessorTest, LoadCanUseStackAndProgramAddressBases) {
    using isa::Operand;
    using namespace isa::instructions;
    constexpr uint32_t kStackData = 0x76543210U;
    ASSERT_TRUE(oracle.Poke(isa::Address{kStackInitial}, kStackData));

    Instruction const from_program_address{Load::FromProgramAddress(Operand{Operand::Type::Scratch, 3})};
    RunProgram({Encode(Load::FromStack(Operand{Operand::Type::Scratch, 2})), from_program_address.raw});

    EXPECT_EQ(kStackData, Scratch(2));
    EXPECT_EQ(from_program_address.raw, Scratch(3));
}

TEST_F(RtlProcessorTest, OneSourceBitwiseOperationsMatchTheFastModel) {
    using isa::Operand;
    using namespace isa::instructions;
    using Shift = Bitwise1::ImmediateType;
    Operand const s1{Operand::Type::Scratch, 1};
    Operand const dst{Operand::Type::Scratch, 2};
    Operand const e3{Operand::Type::Evaluation, 3};

    std::vector<uint32_t> program;
    LoadWord(program, 1, 0x80F01234U);
    std::vector<Bitwise1> ops = {
        Bitwise1::Complement(dst, s1),
        Bitwise1::Rsh(dst, s1, Shift{5}),
        Bitwise1::Asr(dst, s1, Shift{7}),
        Bitwise1::Lsh(dst, s1, Shift{9}),
        Bitwise1::Rotate(dst, s1, Shift{0}),
        Bitwise1::Rotate(dst, s1, Shift{13}),
        Bitwise1::Count(dst, s1),
        Bitwise1::LeadingZeros(dst, s1),
        Bitwise1::Not(dst, s1),
        Bitwise1::Set(dst, s1, Shift{31}),
        Bitwise1::Clear(dst, s1, Shift{4}),
        Bitwise1::Toggle(dst, s1, Shift{0}),
        Bitwise1::ReverseBytes(dst, s1),
        Bitwise1::ReverseHalf(dst, s1),
        Bitwise1::ReverseWord(dst, s1),
        Bitwise1::Lsh(dst, s1, Shift{31}).Evaluate(e3),
        Bitwise1::Lsh(dst, s1, Shift{31}).Evaluate(e3),
    };
    // A zero result sets the zero flag and a negative one clears `positive`.
    ops.push_back(Bitwise1::Rsh(dst, s1, Shift{31}).Evaluate(e3));
    ops.push_back(Bitwise1::Clear(dst, dst, Shift{0}).Evaluate(e3));
    for (auto const& op : ops) {
        program.push_back(Encode(op));
    }
    RunProgram_Vector(program);
}

TEST_F(RtlProcessorTest, TwoSourceBitwiseOperationsMatchTheFastModel) {
    using isa::Operand;
    using namespace isa::instructions;
    Operand const s1{Operand::Type::Scratch, 1};
    Operand const s2{Operand::Type::Scratch, 2};
    Operand const dst{Operand::Type::Scratch, 3};
    Operand const e4{Operand::Type::Evaluation, 4};

    std::vector<uint32_t> program;
    LoadWord(program, 1, 0xF0F0AAAAU);
    LoadWord(program, 2, 0x0FF05555U);
    program.push_back(Encode(Bitwise2::And(dst, s1, s2)));
    program.push_back(Encode(Bitwise2::Or(dst, s1, s2).Evaluate(e4)));
    program.push_back(Encode(Bitwise2::Xor(dst, s1, s2).Evaluate(e4)));
    program.push_back(Encode(Bitwise2::And(dst, s1, Operand{Operand::Type::Scratch, 5}).Evaluate(e4)));  // zero result
    program.push_back(Encode(Bitwise2::Xor(s1, s1, s1)));  // dst aliases both sources
    RunProgram_Vector(program);
}

TEST_F(RtlProcessorTest, ReverseWiderThanAWordWritesNothingAndIsFlagged) {
    using isa::Operand;
    using namespace isa::instructions;
    Operand const s1{Operand::Type::Scratch, 1};
    Operand const dst{Operand::Type::Scratch, 2};

    std::vector<uint32_t> program;
    LoadWord(program, 1, 0x12345678U);
    RunProgram_Vector(program);

    // `shift` = 3 is not a unit width, so both models raise an instruction fault, write
    // nothing and continue at the instruction-fault handler.
    ASSERT_TRUE(oracle.Poke(isa::Address{kFlashBase + 32U}, kFaultHandler));
    Instruction bad{Bitwise1::ReverseBytes(dst, s1)};
    bad.bitwise1.shift = 3U;
    ASSERT_TRUE(oracle.Poke(isa::Address{next_program_address}, bad.raw));
    oracle.Cycle();
    ASSERT_TRUE(RunUntilRetire());
    EXPECT_FALSE(dut.unsupported) << "a fault is not an unimplemented opcode";
    EXPECT_EQ(0U, Scratch(2));
    EXPECT_EQ(kFaultHandler, dut.special[isa::special::ProgramAddress]);
    ExpectStateMatches();
}

TEST_F(RtlProcessorTest, ArithmeticOperationsMatchTheFastModel) {
    using isa::Operand;
    using namespace isa::instructions;
    using Size = Arithmetic::Size;
    using Factory = Arithmetic (*)(Operand, Operand, Operand, Size);

    Operand const s1{Operand::Type::Scratch, 1};
    Operand const s2{Operand::Type::Scratch, 2};
    Operand const dst{Operand::Type::Scratch, 3};
    Operand const e5{Operand::Type::Evaluation, 5};

    // A divisor whose low byte and low half-word are never zero, so nothing here faults.
    constexpr uint32_t kLhs[]
        = {0U, 1U, 0x80U, 0x7FU, 0x8000U, 0x7FFFU, 0x80000000U, 0x7FFFFFFFU, 0xFFFFFFFFU, 0x12345678U};
    constexpr uint32_t kRhs[]
        = {1U, 0x7FU, 0xFFU, 0x7F7FU, 0xFFFFU, 3U, 0x7FFFFFFFU, 0xFFFFFFFFU, 0x80000001U, 0x00018001U};
    constexpr Factory kFactories[] = {
        Arithmetic::Addu, Arithmetic::Adds, Arithmetic::AdduSat, Arithmetic::AddsSat,
        Arithmetic::Subu, Arithmetic::Subs, Arithmetic::SubuSat, Arithmetic::SubsSat,
        Arithmetic::Mulu, Arithmetic::Muls, Arithmetic::MuluSat, Arithmetic::MulsSat,
        Arithmetic::Divu, Arithmetic::Divs, Arithmetic::Modu,    Arithmetic::Mods,
    };
    constexpr Size kSizes[] = {Size::Byte, Size::HalfWord, Size::Word};

    std::vector<uint32_t> program;
    LoadWord(program, 3, 0xA5A5A5A5U);  // the upper bits a sized result must leave alone
    for (uint32_t const lhs : kLhs) {
        for (uint32_t const rhs : kRhs) {
            LoadWord(program, 1, lhs);
            LoadWord(program, 2, rhs);
            for (auto const factory : kFactories) {
                for (Size const size : kSizes) {
                    Arithmetic instruction = factory(dst, s1, s2, size);
                    instruction.cond = 1U;
                    instruction.eval = e5.index;
                    program.push_back(Encode(instruction));
                }
            }
        }
    }
    RunProgram_Vector(program);
}

TEST_F(RtlProcessorTest, NegateAndExtendMatchTheFastModel) {
    using isa::Operand;
    using isa::Size;
    using namespace isa::instructions;

    Operand const s1{Operand::Type::Scratch, 1};
    Operand const dst{Operand::Type::Scratch, 2};
    Operand const e6{Operand::Type::Evaluation, 6};

    std::vector<uint32_t> program;
    LoadWord(program, 2, 0xA5A5A5A5U);
    for (uint32_t const value : {0U, 1U, 0x80U, 0x8000U, 0x80000000U, 0x7FFFFFFFU, 0xFFFFFFFFU, 0x1234ABCDU}) {
        LoadWord(program, 1, value);
        for (Size const size : {Size::Byte, Size::HalfWord, Size::Word}) {
            program.push_back(Encode(Negate::Make(dst, s1, size, false).Evaluate(e6)));
            program.push_back(Encode(Negate::Make(dst, s1, size, true).Evaluate(e6)));
        }
        for (Size const size : {Size::Byte, Size::HalfWord}) {
            program.push_back(Encode(Extend::Sign(dst, s1, size).Evaluate(e6)));
            program.push_back(Encode(Extend::Zero(dst, s1, size)));
        }
    }
    RunProgram_Vector(program);
}

/// Runs `word` after loading scratch 1 and 3, expecting a fault: nothing is written and
/// both models continue at the instruction-fault handler.
#define EXPECT_FAULTS_LIKE_THE_FAST_MODEL(word_expr)                                   \
    do {                                                                               \
        std::vector<uint32_t> setup;                                                   \
        LoadWord(setup, 1, 0x1234U);                                                   \
        LoadWord(setup, 3, 0xCAFEF00DU);                                               \
        RunProgram_Vector(setup);                                                      \
        ASSERT_TRUE(oracle.Poke(isa::Address{kFlashBase + 32U}, kFaultHandler));       \
        ASSERT_TRUE(oracle.Poke(isa::Address{next_program_address}, (word_expr)));     \
        oracle.Cycle();                                                                \
        ASSERT_TRUE(RunUntilRetire());                                                 \
        EXPECT_FALSE(dut.unsupported);                                                 \
        EXPECT_EQ(0xCAFEF00DU, Scratch(3)) << "a faulting instruction writes nothing"; \
        EXPECT_EQ(kFaultHandler, dut.special[isa::special::ProgramAddress]);           \
        ExpectStateMatches();                                                          \
    } while (false)

TEST_F(RtlProcessorTest, DivideByZeroFaultsInLockstep) {
    using isa::Operand;
    using namespace isa::instructions;
    EXPECT_FAULTS_LIKE_THE_FAST_MODEL(Encode(Arithmetic::Divu(
        Operand{Operand::Type::Scratch, 3}, Operand{Operand::Type::Scratch, 1}, Operand{Operand::Type::Scratch, 2})));
}

TEST_F(RtlProcessorTest, ModuloByZeroFaultsInLockstep) {
    using isa::Operand;
    using namespace isa::instructions;
    EXPECT_FAULTS_LIKE_THE_FAST_MODEL(
        Encode(Arithmetic::Mods(Operand{Operand::Type::Scratch, 3}, Operand{Operand::Type::Scratch, 1},
                                Operand{Operand::Type::Scratch, 2}, Arithmetic::Size::HalfWord)));
}

TEST_F(RtlProcessorTest, NegateWithAnInvalidSizeFaultsInLockstep) {
    using isa::Operand;
    using namespace isa::instructions;
    EXPECT_FAULTS_LIKE_THE_FAST_MODEL(Encode(Negate::Make(
        Operand{Operand::Type::Scratch, 3}, Operand{Operand::Type::Scratch, 1}, isa::Size::DoubleWord, false)));
}

TEST_F(RtlProcessorTest, ExtendToTheSameSizeFaultsInLockstep) {
    using isa::Operand;
    using namespace isa::instructions;
    // The encoder refuses a word-sized extend, so the size field is patched in raw.
    Instruction instruction{
        Extend::Sign(Operand{Operand::Type::Scratch, 3}, Operand{Operand::Type::Scratch, 1}, isa::Size::Byte)};
    instruction.extend.size = static_cast<uint32_t>(isa::Size::Word);
    EXPECT_FAULTS_LIKE_THE_FAST_MODEL(instruction.raw);
}

TEST_F(RtlProcessorTest, FaultingLoadPreservesRegistersAndVectorsToBusFaultHandler) {
    using isa::Operand;
    using namespace isa::instructions;
    constexpr uint32_t kUnmappedAddress = 0x00200000U;
    ASSERT_TRUE(oracle.Poke(isa::Address{kFlashBase + 28U}, kFaultHandler));

    std::vector<uint32_t> program;
    LoadWord(program, 1, kUnmappedAddress);
    LoadWord(program, 3, 0xCAFEF00DU);
    program.push_back(Encode(Load::Word(Operand{Operand::Type::Scratch, 3}, Operand{Operand::Type::Scratch, 1},
                                        Load::ImmediateType{4U}, true)));
    request_delay_cycles = 2;
    response_delay_cycles = 9;

    RunProgram_Vector(program);

    EXPECT_EQ(kUnmappedAddress, Scratch(1)) << "failed load must not update its base";
    EXPECT_EQ(0xCAFEF00DU, Scratch(3)) << "failed load must not write its destination";
    EXPECT_EQ(kFaultHandler, dut.special[isa::special::ProgramAddress]);
    ExpectStateMatches();
}

TEST_F(RtlProcessorTest, InvalidLoadSizeFaultsBeforeIssuingMemoryRequest) {
    using isa::Operand;
    using namespace isa::instructions;
    ASSERT_TRUE(oracle.Poke(isa::Address{kFlashBase + 32U}, kFaultHandler));

    Instruction invalid{Load::Word(Operand{Operand::Type::Scratch, 3}, Operand{Operand::Type::Scratch, 1})};
    invalid.loads.size = isa::Size::DoubleWord;

    std::vector<uint32_t> program;
    LoadWord(program, 1, kFlashBase + 0x800U);
    LoadWord(program, 3, 0xCAFEF00DU);
    RunProgram_Vector(program);

    uint32_t const requests_before_invalid_load = bus_request_count;
    ASSERT_TRUE(oracle.Poke(isa::Address{next_program_address}, invalid.raw));
    oracle.Cycle();
    ASSERT_TRUE(RunUntilRetire());

    EXPECT_EQ(requests_before_invalid_load + 2U, bus_request_count)
        << "only the instruction fetch and instruction-fault vector read should issue";
    EXPECT_EQ(0xCAFEF00DU, Scratch(3)) << "faulting load must not write its destination";
    EXPECT_EQ(kFaultHandler, dut.special[isa::special::ProgramAddress]);
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

TEST_F(RtlProcessorTest, UndefinedOpcodeRaisesAnInstructionFaultInLockstep) {
    // 0x38 lies between the defined opcodes 0x34 (`trip`) and 0x3C (`load`).
    ASSERT_TRUE(oracle.Poke(isa::Address{kFlashBase + 32U}, kFaultHandler));
    ASSERT_TRUE(oracle.Poke(isa::Address{kCodeAddress}, 0x00000038U));
    oracle.Cycle();
    ASSERT_TRUE(RunUntilRetire());
    EXPECT_FALSE(dut.unsupported) << "undefined, not unimplemented";
    EXPECT_EQ(1U, dut.special[isa::special::PerformanceCounter]) << "still retires exactly once";
    EXPECT_EQ(kFaultHandler, dut.special[isa::special::ProgramAddress]);
    ExpectStateMatches();
}

TEST_F(RtlProcessorTest, FaultThroughAnErasedHandlerSlotMatchesTheFastModel) {
    // The handler slot is erased flash, which reads back as 0xFFFFFFFF; both models
    // take that as the next program address.
    ASSERT_TRUE(oracle.Poke(isa::Address{kCodeAddress}, 0x00000038U));
    oracle.Cycle();
    ASSERT_TRUE(RunUntilRetire());
    ExpectStateMatches();
}
