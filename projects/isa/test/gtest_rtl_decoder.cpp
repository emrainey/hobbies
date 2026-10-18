#include <gtest/gtest.h>

#include <set>

#include <Vdecoder.h>
#include <isa/instructions.hpp>

namespace {

using isa::Operator;

/// Every operator defined in the C++ model
std::set<uint32_t> DefinedOperators() {
    return {0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x20, 0x24, 0x28, 0x2C, 0x30, 0x34, 0x3C, 0x40, 0x44,
            0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60, 0x64, 0x68, 0x6C, 0x70, 0x74, 0x78, 0x7C, 0x80, 0x84, 0x88,
            0x8C, 0x90, 0x94, 0x98, 0x9C, 0xA0, 0xA4, 0xA8, 0xAC, 0xB0, 0xB4, 0xB8, 0xBC, 0xC0, 0xC4, 0xFC};
}

class RtlDecoderTest : public ::testing::Test {
protected:
    void Decode(uint32_t word) {
        dut_.instruction = word;
        dut_.eval();
    }
    Vdecoder dut_;
};

TEST_F(RtlDecoderTest, OperatorEnumMatchesCpp) {
    // Spot check the enum values against the C++ header
    struct {
        Operator op;
        uint32_t expected;
    } const checks[] = {
        {Operator::None, 0x00},
        {Operator::Copy, 0x04},
        {Operator::Leap, 0x18},
        {Operator::Fall, 0x24},
        {Operator::Load, 0x3C},
        {Operator::Compare, 0x44},
        {Operator::Reverse, 0x74},
        {Operator::Addition, 0x84},
        {Operator::Modulo, 0x94},
        {Operator::FloatingConvert, 0xAC},
        {Operator::FloatingDivision, 0xBC},
        {Operator::Negate, 0xC0},
        {Operator::Extend, 0xC4},
        {Operator::Halt, 0xFC},
    };
    for (auto const& c : checks) {
        EXPECT_EQ(static_cast<uint32_t>(c.op), c.expected);
        Decode(c.expected);
        EXPECT_TRUE(dut_.valid);
        EXPECT_EQ(dut_.op, c.expected);
    }
}

TEST_F(RtlDecoderTest, EveryOpcodeByteIsValidOnlyIfDefined) {
    auto const defined = DefinedOperators();
    for (uint32_t op = 0; op < 256U; op += 4U) {
        // The upper bits are zero so only the opcode and the form bits are exercised
        Decode(op | 0x0100U);
        EXPECT_EQ(dut_.valid != 0, defined.count(op) == 1U) << "opcode 0x" << std::hex << op;
    }
}

TEST_F(RtlDecoderTest, ErasedWordIsNoop) {
    Decode(0xFFFFFFFFU);
    EXPECT_TRUE(dut_.valid);
    EXPECT_EQ(dut_.op, 0x00U);
}

TEST_F(RtlDecoderTest, ImmediateLeapFormDecodesAsLeap) {
    isa::instructions::Instruction i{isa::instructions::LeapImmediate{isa::Address{0x1000U}, true}};
    Decode(i.raw);
    EXPECT_TRUE(dut_.valid);
    EXPECT_EQ(dut_.op, 0x18U);
}

TEST_F(RtlDecoderTest, CppInstructionsDecodeToTheirOperator) {
    using namespace isa::instructions;
    isa::Operand const s1{isa::Operand::Type::Scratch, 1};
    isa::Operand const s2{isa::Operand::Type::Scratch, 2};
    Instruction const list[] = {Instruction{Copy{s1, s2}}, Instruction{Halt{}}, Instruction{Swap{s1, s2}}};
    for (auto const& i : list) {
        Decode(i.raw);
        EXPECT_TRUE(dut_.valid);
        EXPECT_EQ(dut_.op, i.raw & 0xFFU);
    }
}

}  // namespace
