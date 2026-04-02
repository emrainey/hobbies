#include <cstddef>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <isa/cpu.hpp>
#include <isa/instructions.hpp>

class InstructionTest : public testing::Test {
protected:
    static constexpr uint32_t kMemoryBase = 0x10000000U;
    static constexpr uint32_t kMemoryEnd = 0x1000FFFFU;
    static constexpr uint32_t kCodeAddress = 0x10000100U;
    static constexpr uint32_t kInstructionFaultHandler = 0x10000200U;
    static constexpr uint32_t kTripHandler = 0x10000210U;
    static constexpr uint32_t kSystemCallHandler = 0x10000220U;
    static constexpr uint32_t kPrivilegeHandler = 0x10000228U;
    static constexpr uint32_t kSafeHandler = 0x10000230U;
    static constexpr uint32_t kDataAddress = 0x10000300U;

    isa::Processor cpu;
    isa::TightlyCoupledMemory memory{isa::Range{kMemoryBase, kMemoryEnd}};

    InstructionTest() {
        EXPECT_TRUE(cpu.AddTightlyCoupledMemory(memory));
    }

    void SetUp() override {
        cpu.Reset();

        auto& special = cpu.GetSpecial();
        special.vector_table_address_ = isa::Address{kMemoryBase};
        special.program_address_ = isa::Address{kCodeAddress};
        special.return_address_ = isa::Address{kCodeAddress + 0x40U};
        special.performance_counter_ = 0U;

        // Configure stack values so Grow/Undo can be checked deterministically.
        special.stack_.current = isa::Address{0x10001000U};
        special.stack_.base = isa::Address{0x10001000U};
        special.stack_.limit = isa::Address{0x10002000U};

        ASSERT_TRUE(cpu.Poke(kMemoryBase + offsetof(isa::VectorTable, reset_handler), kCodeAddress));
        ASSERT_TRUE(
            cpu.Poke(kMemoryBase + offsetof(isa::VectorTable, instruction_fault_handler), kInstructionFaultHandler));
        ASSERT_TRUE(cpu.Poke(kMemoryBase + offsetof(isa::VectorTable, trip_handler), kTripHandler));
        ASSERT_TRUE(cpu.Poke(kMemoryBase + offsetof(isa::VectorTable, system_call_handler), kSystemCallHandler));
        ASSERT_TRUE(cpu.Poke(kMemoryBase + offsetof(isa::VectorTable, privilege_handler), kPrivilegeHandler));
        ASSERT_TRUE(cpu.Poke(kMemoryBase + offsetof(isa::VectorTable, safe_handler), kSafeHandler));

        cpu.GetScratch()[0].as_u32[0] = 0U;
        cpu.GetScratch()[1].as_u32[0] = 0U;
        cpu.GetScratch()[2].as_u32[0] = 7U;
        cpu.GetScratch()[3].as_u32[0] = 3U;
        cpu.GetScratch()[4].as_u32[0] = 0xF0U;
        cpu.GetScratch()[5].as_u32[0] = 0x0FU;

        cpu.GetEvaluations()[0].value = 0U;
        cpu.GetEvaluations()[1].value = 0U;
    }

    void RunSingleInstruction(isa::instructions::Instruction const& instruction) {
        const uint32_t pc = cpu.ViewSpecial().program_address_.value;
        ASSERT_TRUE(cpu.Poke(pc, instruction.raw));
        cpu.Cycle();
    }
};

TEST_F(InstructionTest, NoOpAdvancesProgramCounter) {
    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::NoOp{}});

    EXPECT_EQ(kCodeAddress + 4U, cpu.ViewSpecial().program_address_.value);
    EXPECT_EQ(1U, cpu.ViewSpecial().performance_counter_);
}

TEST_F(InstructionTest, HaltSetsHaltedState) {
    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Halt{}});

    EXPECT_TRUE(cpu.IsHalted());
    EXPECT_EQ(kCodeAddress + 4U, cpu.ViewSpecial().program_address_.value);
}

TEST_F(InstructionTest, CopyCopiesValueWithoutClearingSource) {
    cpu.GetScratch()[1].as_u32[0] = 0x11111111U;
    cpu.GetScratch()[2].as_u32[0] = 0x22222222U;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Copy{
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2}}});

    EXPECT_EQ(0x22222222U, cpu.ViewScratch()[1].as_u32[0]);
    EXPECT_EQ(0x22222222U, cpu.ViewScratch()[2].as_u32[0]);
    EXPECT_FALSE(cpu.ViewSpecial().exception_.instruction_fault);
    EXPECT_EQ(kCodeAddress + 4U, cpu.ViewSpecial().program_address_.value);
}

TEST_F(InstructionTest, MoveCopiesScratchRegisterAndClearsSource) {
    cpu.GetScratch()[2].as_u32[0] = 0x12345678U;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Move{
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2}}});

    EXPECT_EQ(0x12345678U, cpu.ViewScratch()[1].as_u32[0]);
    EXPECT_EQ(0U, cpu.ViewScratch()[2].as_u32[0]);
}

TEST_F(InstructionTest, MoveImmediateToScratchWritesDestination) {
    RunSingleInstruction(isa::instructions::Instruction{
        isa::instructions::MoveImmediate{isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Immediate<16>{0xABCDU}}});

    EXPECT_EQ(0xABCDU, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, MoveImmediateToScratchWritesDestination2) {
    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::MoveImmediate{
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Immediate<16>{0xABCDU}, true}});

    EXPECT_EQ(0xABCD0000U, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, MoveImmediateToScratchWritesDestination3) {
    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::MoveImmediate{
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Immediate<16>{0xABCDU}, true}});
    RunSingleInstruction(isa::instructions::Instruction{
        isa::instructions::MoveImmediate{isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Immediate<16>{0xEF89U}}});

    EXPECT_EQ(0xABCDEF89U, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, MoveImmediateToEvaluationWritesLowByte) {
    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::MoveImmediate{
        isa::Operand{isa::Operand::Type::Evaluation, 1}, isa::instructions::MoveImmediate::ImmediateType{0xA5U}}});

    EXPECT_EQ(0xA5U, cpu.ViewEvaluations()[1].value);
}

TEST_F(InstructionTest, SwapExchangesScratchValues) {
    cpu.GetScratch()[1].as_u32[0] = 0xAAAAAAAAU;
    cpu.GetScratch()[2].as_u32[0] = 0x55555555U;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Swap{
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2}}});

    EXPECT_EQ(0x55555555U, cpu.ViewScratch()[1].as_u32[0]);
    EXPECT_EQ(0xAAAAAAAAU, cpu.ViewScratch()[2].as_u32[0]);
}

TEST_F(InstructionTest, ZeroClearsMaskedScratchRegisters) {
    cpu.GetScratch()[0].as_u32[0] = 10U;
    cpu.GetScratch()[1].as_u32[0] = 11U;
    cpu.GetScratch()[2].as_u32[0] = 12U;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Zero{
        isa::Operand{isa::Operand::Type::Mask, isa::Immediate<16>{0b0111U}}, isa::RegisterType::Scratch}});

    EXPECT_EQ(0U, cpu.ViewScratch()[0].as_u32[0]);
    EXPECT_EQ(0U, cpu.ViewScratch()[1].as_u32[0]);
    EXPECT_EQ(0U, cpu.ViewScratch()[2].as_u32[0]);
}

TEST_F(InstructionTest, LoadReadsMemoryIntoScratch) {
    ASSERT_TRUE(cpu.Poke(kDataAddress, 0xCAFEBABEU));
    cpu.GetScratch()[1].as_address = isa::Address{kDataAddress};

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Load::Word(
        isa::Operand{isa::Operand::Type::Scratch, 2}, isa::Operand{isa::Operand::Type::Scratch, 1})});

    EXPECT_EQ(0xCAFEBABEU, cpu.ViewScratch()[2].as_u32[0]);
}

TEST_F(InstructionTest, SaveWritesScratchToMemory) {
    cpu.GetScratch()[1].as_address = isa::Address{kDataAddress};
    cpu.GetScratch()[2].as_u32[0] = 0xDEADBEEFU;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Save::Word(
        isa::Operand{isa::Operand::Type::Scratch, 2}, isa::Operand{isa::Operand::Type::Scratch, 1})});

    uint32_t saved = 0U;
    ASSERT_TRUE(cpu.Peek(kDataAddress, saved));
    EXPECT_EQ(0xDEADBEEFU, saved);
}

TEST_F(InstructionTest, LoadByteReadsSelectedByteLane) {
    ASSERT_TRUE(cpu.Poke(kDataAddress, 0xAABBCCDDU));
    cpu.GetScratch()[1].as_address = isa::Address{kDataAddress};

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Load::Byte(
        isa::Operand{isa::Operand::Type::Scratch, 2}, isa::Operand{isa::Operand::Type::Scratch, 1}, 2U)});

    EXPECT_EQ(0xBBU, cpu.ViewScratch()[2].as_u32[0]);
}

TEST_F(InstructionTest, LoadHalfWordReadsSelectedHalfWordLane) {
    ASSERT_TRUE(cpu.Poke(kDataAddress, 0xAABBCCDDU));
    cpu.GetScratch()[1].as_address = isa::Address{kDataAddress};

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Load::HalfWord(
        isa::Operand{isa::Operand::Type::Scratch, 2}, isa::Operand{isa::Operand::Type::Scratch, 1}, 1U)});

    EXPECT_EQ(0xAABBU, cpu.ViewScratch()[2].as_u32[0]);
}

TEST_F(InstructionTest, LoadWordIncrementUpdatesBaseRegister) {
    ASSERT_TRUE(cpu.Poke(kDataAddress, 0x11223344U));
    cpu.GetScratch()[1].as_address = isa::Address{kDataAddress};

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Load::Word(
        isa::Operand{isa::Operand::Type::Scratch, 2}, isa::Operand{isa::Operand::Type::Scratch, 1},
        isa::instructions::Load::ImmediateType{4U}, true)});

    EXPECT_EQ(0x11223344U, cpu.ViewScratch()[2].as_u32[0]);
    EXPECT_EQ(kDataAddress + 4U, cpu.ViewScratch()[1].as_address.value);
}

TEST_F(InstructionTest, SaveByteWritesSelectedByteLane) {
    ASSERT_TRUE(cpu.Poke(kDataAddress, 0x11223344U));
    cpu.GetScratch()[1].as_address = isa::Address{kDataAddress};
    cpu.GetScratch()[2].as_u32[0] = 0xAAU;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Save::Byte(
        isa::Operand{isa::Operand::Type::Scratch, 2}, isa::Operand{isa::Operand::Type::Scratch, 1}, 2U)});

    uint32_t saved = 0U;
    ASSERT_TRUE(cpu.Peek(kDataAddress, saved));
    EXPECT_EQ(0x11AA3344U, saved);
}

TEST_F(InstructionTest, SaveHalfWordWritesSelectedHalfWordLane) {
    ASSERT_TRUE(cpu.Poke(kDataAddress, 0x11223344U));
    cpu.GetScratch()[1].as_address = isa::Address{kDataAddress};
    cpu.GetScratch()[2].as_u32[0] = 0xBEEFU;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Save::HalfWord(
        isa::Operand{isa::Operand::Type::Scratch, 2}, isa::Operand{isa::Operand::Type::Scratch, 1}, 1U)});

    uint32_t saved = 0U;
    ASSERT_TRUE(cpu.Peek(kDataAddress, saved));
    EXPECT_EQ(0xBEEF3344U, saved);
}

TEST_F(InstructionTest, SaveWordOffsetWritesAtBasePlusImmediateWithoutIncrementingBase) {
    ASSERT_TRUE(cpu.Poke(kDataAddress, 0x11223344U));
    ASSERT_TRUE(cpu.Poke(kDataAddress + 4U, 0x55667788U));
    cpu.GetScratch()[1].as_address = isa::Address{kDataAddress};
    cpu.GetScratch()[2].as_u32[0] = 0xDEADBEEFU;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Save::Word(
        isa::Operand{isa::Operand::Type::Scratch, 2}, isa::Operand{isa::Operand::Type::Scratch, 1},
        isa::instructions::Save::ImmediateType{4U}, false)});

    uint32_t saved_base = 0U;
    uint32_t saved_offset = 0U;
    ASSERT_TRUE(cpu.Peek(kDataAddress, saved_base));
    ASSERT_TRUE(cpu.Peek(kDataAddress + 4U, saved_offset));
    EXPECT_EQ(0x11223344U, saved_base);
    EXPECT_EQ(0xDEADBEEFU, saved_offset);
    EXPECT_EQ(kDataAddress, cpu.ViewScratch()[1].as_address.value);
}

TEST_F(InstructionTest, LeapJumpsToTargetAddress) {
    cpu.GetScratch()[1].as_address = isa::Address{0x10000180U};

    RunSingleInstruction(isa::instructions::Instruction{
        isa::instructions::Leap{isa::Operand{isa::Operand::Type::Scratch, 1}, isa::SignedImmediate<10>{0}}});

    EXPECT_EQ(0x10000184U, cpu.ViewSpecial().program_address_.value);
}

TEST_F(InstructionTest, StandardInstructionEncodingsReserveBottomTwoBits) {
    const isa::instructions::Instruction instr{isa::instructions::Move{isa::Operand{isa::Operand::Type::Scratch, 1},
                                                                       isa::Operand{isa::Operand::Type::Scratch, 2}}};
    EXPECT_EQ(0U, instr.raw & 0x3U);
}

TEST_F(InstructionTest, ImmediateLeapJumpsToAbsoluteAddress) {
    RunSingleInstruction(
        isa::instructions::Instruction{isa::instructions::LeapImmediate{isa::Address{0x10000180U}, false}});

    EXPECT_EQ(0x10000184U, cpu.ViewSpecial().program_address_.value);
    EXPECT_EQ(kCodeAddress + 0x40U, cpu.ViewSpecial().return_address_.value);
}

TEST_F(InstructionTest, ImmediateLeapWithSaveStoresReturnAddress) {
    RunSingleInstruction(
        isa::instructions::Instruction{isa::instructions::LeapImmediate{isa::Address{0x100001C0U}, true}});

    EXPECT_EQ(kCodeAddress + 4U, cpu.ViewSpecial().return_address_.value);
    EXPECT_EQ(0x100001C4U, cpu.ViewSpecial().program_address_.value);
}

TEST_F(InstructionTest, BackJumpsToReturnAddress) {
    cpu.GetSpecial().return_address_ = isa::Address{0x100001C0U};

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Back{false}});

    EXPECT_EQ(0x100001C4U, cpu.ViewSpecial().program_address_.value);
    EXPECT_EQ(0x100001C0U, cpu.ViewSpecial().return_address_.value);
}

TEST_F(InstructionTest, BackWithZeroSetsReturnAddressToSafeHandler) {
    cpu.GetSpecial().return_address_ = isa::Address{0x100001C0U};

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Back{true}});

    EXPECT_EQ(0x100001C4U, cpu.ViewSpecial().program_address_.value);
    EXPECT_EQ(kSafeHandler, cpu.ViewSpecial().return_address_.value);
}

TEST_F(InstructionTest, SafeExceptionRoutesToSafeHandlerAboveExternalPriority) {
    cpu.GetSpecial().exception_.safe = 1;
    cpu.GetSpecial().exception_.external = 1;
    cpu.GetSpecial().exception_.type = isa::ExceptionType::Safe;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::NoOp{}});

    EXPECT_EQ(kSafeHandler, cpu.ViewSpecial().program_address_.value);
}

TEST_F(InstructionTest, PrivilegeExceptionRoutesAboveSafeAndExternalPriority) {
    cpu.GetSpecial().exception_.privilege_fault = 1;
    cpu.GetSpecial().exception_.safe = 1;
    cpu.GetSpecial().exception_.external = 1;
    cpu.GetSpecial().exception_.type = isa::ExceptionType::Privilege;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::NoOp{}});

    EXPECT_EQ(kPrivilegeHandler, cpu.ViewSpecial().program_address_.value);
}

TEST_F(InstructionTest, GrowMovesStackPointerDownByImmediateWords) {
    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Grow{isa::Immediate<10>{1U}}});

    EXPECT_EQ(0x10000FFCU, cpu.ViewSpecial().stack_.current.value);
}

TEST_F(InstructionTest, UndoMovesStackPointerUpByImmediateWords) {
    cpu.GetSpecial().stack_.current = isa::Address{0x10000FFCU};

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Undo{isa::Immediate<10>{1U}}});

    EXPECT_EQ(0x10001000U, cpu.ViewSpecial().stack_.current.value);
}

TEST_F(InstructionTest, CallJumpsToSystemCallHandlerAndStoresCallNumber) {
    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Call{isa::Immediate<10>{7U}}});

    EXPECT_EQ(7U, cpu.ViewSpecial().call_number_);
    EXPECT_EQ(kCodeAddress + 4U, cpu.ViewSpecial().return_address_.value);
    EXPECT_EQ(kSystemCallHandler + 4U, cpu.ViewSpecial().program_address_.value);
}

TEST_F(InstructionTest, TripSetsExceptionAndJumpsToTripHandler) {
    RunSingleInstruction(isa::instructions::Instruction{
        isa::instructions::Trip{isa::Immediate<10>{static_cast<uint32_t>(isa::ExceptionType::Triggered)}}});

    EXPECT_TRUE(cpu.ViewSpecial().exception_.tripped);
    EXPECT_EQ(isa::ExceptionType::Triggered, cpu.ViewSpecial().exception_.type);
    EXPECT_EQ(kTripHandler, cpu.ViewSpecial().program_address_.value);
}

TEST_F(InstructionTest, CompareUpdatesEvaluationFlags) {
    cpu.GetScratch()[2].as_u32[0] = 5U;
    cpu.GetScratch()[3].as_u32[0] = 9U;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Compare{
        isa::Operand{isa::Operand::Type::Evaluation, 0}, isa::Operand{isa::Operand::Type::Scratch, 2},
        isa::Operand{isa::Operand::Type::Scratch, 3}, false}});

    auto const cmp = cpu.ViewEvaluations()[0].comparison;
    EXPECT_EQ(1U, cmp.less_than);
    EXPECT_EQ(0U, cmp.equal);
    EXPECT_EQ(0U, cmp.greater_than);
    EXPECT_EQ(1U, cmp.not_equal);
}

TEST_F(InstructionTest, BitwiseAndWritesResult) {
    cpu.GetScratch()[2].as_u32[0] = 0xF0U;
    cpu.GetScratch()[3].as_u32[0] = 0xAAU;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Bitwise2::And(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2},
        isa::Operand{isa::Operand::Type::Scratch, 3})});

    EXPECT_EQ(0xA0U, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, BitwiseOrWritesResult) {
    cpu.GetScratch()[2].as_u32[0] = 0xF0U;
    cpu.GetScratch()[3].as_u32[0] = 0x0FU;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Bitwise2::Or(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2},
        isa::Operand{isa::Operand::Type::Scratch, 3})});

    EXPECT_EQ(0xFFU, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, BitwiseXorWritesResult) {
    cpu.GetScratch()[2].as_u32[0] = 0xF0U;
    cpu.GetScratch()[3].as_u32[0] = 0xAAU;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Bitwise2::Xor(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2},
        isa::Operand{isa::Operand::Type::Scratch, 3})});

    EXPECT_EQ(0x5AU, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, BitwiseComplementWritesResult) {
    cpu.GetScratch()[2].as_u32[0] = 0x00000000U;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Bitwise1::Complement(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2})});

    EXPECT_EQ(0xFFFFFFFFU, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, BitwiseRightShiftWritesResult) {
    cpu.GetScratch()[2].as_u32[0] = 0x80U;

    RunSingleInstruction(isa::instructions::Instruction{
        isa::instructions::Bitwise1::Rsh(isa::Operand{isa::Operand::Type::Scratch, 1},
                                         isa::Operand{isa::Operand::Type::Scratch, 2}, isa::Immediate<5>{2U})});

    EXPECT_EQ(0x20U, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, BitwiseLeftShiftWritesResult) {
    cpu.GetScratch()[2].as_u32[0] = 0x11U;

    RunSingleInstruction(isa::instructions::Instruction{
        isa::instructions::Bitwise1::Lsh(isa::Operand{isa::Operand::Type::Scratch, 1},
                                         isa::Operand{isa::Operand::Type::Scratch, 2}, isa::Immediate<5>{3U})});

    EXPECT_EQ(0x88U, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, BitwiseCountOnesWritesResult) {
    cpu.GetScratch()[2].as_u32[0] = 0b10110100U;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Bitwise1::Count(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2})});

    EXPECT_EQ(4U, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, BitwiseCountLeadingZerosWritesResult) {
    cpu.GetScratch()[2].as_u32[0] = 0x00FF0000U;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Bitwise1::LeadingZeros(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2})});

    EXPECT_EQ(8U, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, ArithmeticAdditionWritesResult) {
    cpu.GetScratch()[2].as_u32[0] = 12U;
    cpu.GetScratch()[3].as_u32[0] = 30U;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Arithmetic::Addu(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2},
        isa::Operand{isa::Operand::Type::Scratch, 3})});

    EXPECT_EQ(42U, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, ArithmeticSubtractionWritesResult) {
    cpu.GetScratch()[2].as_u32[0] = 40U;
    cpu.GetScratch()[3].as_u32[0] = 2U;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Arithmetic::Subu(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2},
        isa::Operand{isa::Operand::Type::Scratch, 3})});

    EXPECT_EQ(38U, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, ArithmeticMultiplyWritesResult) {
    cpu.GetScratch()[2].as_u32[0] = 6U;
    cpu.GetScratch()[3].as_u32[0] = 7U;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Arithmetic::Mulu(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2},
        isa::Operand{isa::Operand::Type::Scratch, 3})});

    EXPECT_EQ(42U, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, ArithmeticDivideWritesResult) {
    cpu.GetScratch()[2].as_u32[0] = 84U;
    cpu.GetScratch()[3].as_u32[0] = 2U;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Arithmetic::Divu(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2},
        isa::Operand{isa::Operand::Type::Scratch, 3})});

    EXPECT_EQ(42U, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, ArithmeticModuloWritesResult) {
    cpu.GetScratch()[2].as_u32[0] = 85U;
    cpu.GetScratch()[3].as_u32[0] = 43U;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Arithmetic::Modu(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2},
        isa::Operand{isa::Operand::Type::Scratch, 3})});

    EXPECT_EQ(42U, cpu.ViewScratch()[1].as_u32[0]);
}

TEST_F(InstructionTest, FloatingAbsWritesAbsoluteValue) {
    cpu.GetScratch()[2].as_float[0] = -3.25F;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Precision1::FAbs(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2})});

    EXPECT_FLOAT_EQ(3.25F, cpu.ViewScratch()[1].as_float[0]);
}

TEST_F(InstructionTest, FloatingNegateWritesNegatedValue) {
    cpu.GetScratch()[2].as_float[0] = 6.5F;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Precision1::FNeg(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2})});

    EXPECT_FLOAT_EQ(-6.5F, cpu.ViewScratch()[1].as_float[0]);
}

TEST_F(InstructionTest, FloatingFloorWritesFlooredValue) {
    cpu.GetScratch()[2].as_float[0] = 3.75F;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Precision1::Floor(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2})});

    EXPECT_FLOAT_EQ(3.0F, cpu.ViewScratch()[1].as_float[0]);
}

TEST_F(InstructionTest, FloatingCeilWritesCeiledValue) {
    cpu.GetScratch()[2].as_float[0] = 3.25F;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Precision1::Ceil(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2})});

    EXPECT_FLOAT_EQ(4.0F, cpu.ViewScratch()[1].as_float[0]);
}

TEST_F(InstructionTest, FloatingFractionalWritesFractionalPart) {
    cpu.GetScratch()[2].as_float[0] = 3.75F;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Precision1::Fractional(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2})});

    EXPECT_FLOAT_EQ(0.75F, cpu.ViewScratch()[1].as_float[0]);
}

TEST_F(InstructionTest, FloatingAdditionWritesSum) {
    cpu.GetScratch()[2].as_float[0] = 1.5F;
    cpu.GetScratch()[3].as_float[0] = 2.25F;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Precision2::FAdd(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2},
        isa::Operand{isa::Operand::Type::Scratch, 3})});

    EXPECT_FLOAT_EQ(3.75F, cpu.ViewScratch()[1].as_float[0]);
}

TEST_F(InstructionTest, FloatingSubtractionWritesDifference) {
    cpu.GetScratch()[2].as_float[0] = 5.0F;
    cpu.GetScratch()[3].as_float[0] = 1.5F;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Precision2::FSub(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2},
        isa::Operand{isa::Operand::Type::Scratch, 3})});

    EXPECT_FLOAT_EQ(3.5F, cpu.ViewScratch()[1].as_float[0]);
}

TEST_F(InstructionTest, FloatingMultiplicationWritesProduct) {
    cpu.GetScratch()[2].as_float[0] = 2.5F;
    cpu.GetScratch()[3].as_float[0] = 3.0F;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Precision2::FMul(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2},
        isa::Operand{isa::Operand::Type::Scratch, 3})});

    EXPECT_FLOAT_EQ(7.5F, cpu.ViewScratch()[1].as_float[0]);
}

TEST_F(InstructionTest, FloatingDivisionWritesQuotient) {
    cpu.GetScratch()[2].as_float[0] = 7.5F;
    cpu.GetScratch()[3].as_float[0] = 2.5F;

    RunSingleInstruction(isa::instructions::Instruction{isa::instructions::Precision2::FDiv(
        isa::Operand{isa::Operand::Type::Scratch, 1}, isa::Operand{isa::Operand::Type::Scratch, 2},
        isa::Operand{isa::Operand::Type::Scratch, 3})});

    EXPECT_FLOAT_EQ(3.0F, cpu.ViewScratch()[1].as_float[0]);
}
