#include <cstdint>
#include <cstring>
#include <random>

#include <gtest/gtest.h>

#include <Vbitwise_logic_unit.h>
#include <isa/operator.hpp>
#include <isa/types.hpp>

namespace {

using isa::Operator;

uint32_t ReverseUnits(uint32_t value, uint32_t shift) {
    const uint32_t unit_bits = 8U << shift;
    uint32_t result = 0U;
    for (uint32_t base = 0U; base < 32U; base += unit_bits) {
        for (uint32_t bit = 0U; bit < unit_bits; ++bit) {
            if ((value & (1U << (base + bit))) != 0U) {
                result |= 1U << (base + unit_bits - 1U - bit);
            }
        }
    }
    return result;
}

/// The evaluation byte `cond` writes, built from the C++ model's own type.
uint8_t BitwiseEvaluationByte(uint32_t result) {
    isa::ArithmeticEvaluation eval;
    eval.positive = (result & 0x80000000U) == 0U ? 1U : 0U;
    eval.zero = result == 0U ? 1U : 0U;
    uint8_t byte = 0U;
    static_assert(sizeof(eval) == sizeof(byte));
    std::memcpy(&byte, &eval, sizeof(byte));
    return byte;
}

/// Mirrors the bitwise cases of Processor::Cycle (source/cpu.cpp). `b` is the second
/// source for the two-source operations and the shift field for the one-source ones.
bool Reference(Operator op, uint32_t a, uint32_t b, uint32_t& result) {
    const uint32_t shift = b & 0x1FU;
    result = 0U;
    switch (op) {
        case Operator::Complement:
            result = ~a;
            break;
        case Operator::Rsh:
            result = a >> shift;
            break;
        case Operator::Asr:
            result = static_cast<uint32_t>(static_cast<int32_t>(a) >> shift);
            break;
        case Operator::Lsh:
            result = a << shift;
            break;
        case Operator::Rotate:
            result = (shift == 0U) ? a : ((a << shift) | (a >> (32U - shift)));
            break;
        case Operator::Count1s:
            for (uint32_t v = a; v != 0U; v >>= 1) {
                result += v & 1U;
            }
            break;
        case Operator::CountL0s:
            while (result < 32U && (a & (0x80000000U >> result)) == 0U) {
                result++;
            }
            break;
        case Operator::Not:
            result = (a == 0U) ? 0xFFFFFFFFU : 0U;
            break;
        case Operator::SetBit:
            result = a | (1U << shift);
            break;
        case Operator::ClearBit:
            result = a & ~(1U << shift);
            break;
        case Operator::ToggleBit:
            result = a ^ (1U << shift);
            break;
        case Operator::Reverse:
            if (shift > 2U) {
                return false;
            }
            result = ReverseUnits(a, shift);
            break;
        case Operator::And:
            result = a & b;
            break;
        case Operator::Or:
            result = a | b;
            break;
        case Operator::Xor:
            result = a ^ b;
            break;
        default:
            return false;
    }
    return true;
}

constexpr Operator kOperations[] = {
    Operator::Complement, Operator::Rsh,      Operator::Asr, Operator::Lsh,    Operator::Rotate,
    Operator::Count1s,    Operator::CountL0s, Operator::Not, Operator::SetBit, Operator::ClearBit,
    Operator::ToggleBit,  Operator::Reverse,  Operator::And, Operator::Or,     Operator::Xor,
};

class RtlBitwiseTest : public ::testing::Test {
protected:
    void Apply(Operator op, uint32_t a, uint32_t b) {
        dut_.opcode = static_cast<uint8_t>(op);
        dut_.a = a;
        dut_.b = b;
        dut_.eval();
    }

    void ExpectMatchesReference(Operator op, uint32_t a, uint32_t b) {
        SCOPED_TRACE(::testing::Message()
                     << "op 0x" << std::hex << static_cast<unsigned>(op) << " a 0x" << a << " b 0x" << b);
        uint32_t expected = 0U;
        const bool expected_valid = Reference(op, a, b, expected);
        Apply(op, a, b);
        EXPECT_EQ(expected_valid, static_cast<bool>(dut_.valid));
        if (expected_valid) {
            EXPECT_EQ(expected, dut_.result);
            EXPECT_EQ(BitwiseEvaluationByte(expected), dut_.evaluation);
        }
    }

    Vbitwise_logic_unit dut_;
};

TEST_F(RtlBitwiseTest, DirectedExamplesFromTheInstructionTests) {
    Apply(Operator::Complement, 0U, 0U);
    EXPECT_EQ(0xFFFFFFFFU, dut_.result);

    Apply(Operator::Rsh, 0x80U, 2U);
    EXPECT_EQ(0x20U, dut_.result);

    Apply(Operator::Lsh, 0x11U, 3U);
    EXPECT_EQ(0x88U, dut_.result);

    Apply(Operator::Asr, 0x80000000U, 4U);
    EXPECT_EQ(0xF8000000U, dut_.result);

    Apply(Operator::Rotate, 0x80000001U, 1U);
    EXPECT_EQ(0x00000003U, dut_.result);

    Apply(Operator::Count1s, 0b10110100U, 0U);
    EXPECT_EQ(4U, dut_.result);

    Apply(Operator::CountL0s, 0x00010000U, 0U);
    EXPECT_EQ(15U, dut_.result);
}

TEST_F(RtlBitwiseTest, CountsAreNotMasked) {
    Apply(Operator::Count1s, 0xFFFFFFFFU, 0U);
    EXPECT_EQ(32U, dut_.result);
    Apply(Operator::CountL0s, 0U, 0U);
    EXPECT_EQ(32U, dut_.result);
    Apply(Operator::CountL0s, 0xFFFFFFFFU, 0U);
    EXPECT_EQ(0U, dut_.result);
}

TEST_F(RtlBitwiseTest, RotateByZeroLeavesValueAlone) {
    Apply(Operator::Rotate, 0xA5A5A5A5U, 0U);
    EXPECT_EQ(0xA5A5A5A5U, dut_.result);
}

TEST_F(RtlBitwiseTest, ShiftUsesOnlyTheLowFiveBits) {
    Apply(Operator::Lsh, 1U, 0x21U);
    EXPECT_EQ(2U, dut_.result);
}

TEST_F(RtlBitwiseTest, ReverseSelectsByteHalfAndWordUnits) {
    Apply(Operator::Reverse, 0x01020304U, 0U);
    EXPECT_EQ(0x8040C020U, dut_.result);

    Apply(Operator::Reverse, 0x00000001U, 1U);
    EXPECT_EQ(0x00008000U, dut_.result);

    Apply(Operator::Reverse, 0x00000001U, 2U);
    EXPECT_EQ(0x80000000U, dut_.result);
}

TEST_F(RtlBitwiseTest, ReverseAboveWordIsInvalid) {
    for (uint32_t shift = 3U; shift < 32U; ++shift) {
        Apply(Operator::Reverse, 0x12345678U, shift);
        EXPECT_FALSE(dut_.valid) << "shift " << shift;
    }
}

TEST_F(RtlBitwiseTest, UndefinedOperationIsInvalid) {
    Apply(Operator::Addition, 1U, 2U);
    EXPECT_FALSE(dut_.valid);
}

TEST_F(RtlBitwiseTest, EvaluationReportsSignAndZero) {
    Apply(Operator::Or, 0U, 0U);
    EXPECT_EQ(BitwiseEvaluationByte(0U), dut_.evaluation);
    Apply(Operator::Or, 0x80000000U, 0U);
    EXPECT_EQ(BitwiseEvaluationByte(0x80000000U), dut_.evaluation);
    Apply(Operator::Or, 1U, 0U);
    EXPECT_EQ(BitwiseEvaluationByte(1U), dut_.evaluation);
}

TEST_F(RtlBitwiseTest, MatchesReferenceOnCornerValues) {
    constexpr uint32_t kValues[] = {0U, 1U, 2U, 0x7FFFFFFFU, 0x80000000U, 0xFFFFFFFFU, 0xA5A5A5A5U, 0x12345678U};
    for (Operator op : kOperations) {
        for (uint32_t a : kValues) {
            for (uint32_t b : kValues) {
                ExpectMatchesReference(op, a, b);
            }
        }
    }
}

TEST_F(RtlBitwiseTest, MatchesReferenceOnEveryShiftAmount) {
    for (Operator op : kOperations) {
        for (uint32_t shift = 0U; shift < 32U; ++shift) {
            ExpectMatchesReference(op, 0x91A2B3C4U, shift);
            ExpectMatchesReference(op, 0x00000001U, shift);
        }
    }
}

TEST_F(RtlBitwiseTest, MatchesReferenceOnRandomVectors) {
    std::mt19937 generator{0x5EEDU};
    for (int i = 0; i < 2000; ++i) {
        for (Operator op : kOperations) {
            ExpectMatchesReference(op, generator(), generator());
        }
    }
}

}  // namespace
