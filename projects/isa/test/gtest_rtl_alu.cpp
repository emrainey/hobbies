#include <cstdint>
#include <cstring>
#include <random>
#include <type_traits>

#include <gtest/gtest.h>

#include <Valu.h>
#include <isa/arithmetic.hpp>
#include <isa/operator.hpp>
#include <isa/types.hpp>

namespace {

using isa::Operator;

enum class Size : uint8_t {
    Byte = 0,
    Half = 1,
    Word = 2,
    Double = 3
};

struct Expected {
    uint32_t result{0U};  ///< the low `size` bits, zero above (EXTEND: the whole word)
    uint8_t eva{0U};
    bool fault{false};
    bool compare_eva{true};  ///< false when the C++ model writes nothing for a faulting case
};

template <typename T>
uint32_t Truncate(T value) {
    return static_cast<uint32_t>(static_cast<std::make_unsigned_t<T>>(value));
}

/// Runs one operation through the C++ model's own helpers, as Processor::Cycle does.
template <typename T, typename I>
Expected Reference(Operator op, uint32_t a, uint32_t b, bool sat) {
    const T lhs = static_cast<T>(a);
    const T rhs = static_cast<T>(b);
    Expected expected;
    switch (op) {
        case Operator::Addition: {
            auto [result, eval] = isa::operations::Addition<T, I>(lhs, rhs, sat);
            expected.result = Truncate(result);
            expected.eva = eval.value;
            break;
        }
        case Operator::Subtract: {
            auto [result, eval] = isa::operations::Subtraction<T, I>(lhs, rhs, sat);
            expected.result = Truncate(result);
            expected.eva = eval.value;
            break;
        }
        case Operator::Multiply: {
            auto [result, eval] = isa::operations::Multiply<T, I>(lhs, rhs, sat);
            expected.result = Truncate(result);
            expected.eva = eval.value;
            break;
        }
        case Operator::Divide: {
            auto [result, eval] = isa::operations::Division<T>(lhs, rhs);
            expected.result = Truncate(result);
            expected.eva = eval.value;
            expected.fault = eval.arithmetic.undefined != 0U;
            break;
        }
        case Operator::Modulo: {
            auto [result, eval] = isa::operations::Modulo<T>(lhs, rhs);
            expected.result = Truncate(result);
            expected.eva = eval.value;
            expected.fault = eval.arithmetic.undefined != 0U;
            break;
        }
        default:
            break;
    }
    return expected;
}

/// Negate is `0 - src` with the signed subtraction, at the instruction's size.
template <typename T, typename I>
Expected ReferenceNegate(uint32_t a, bool sat) {
    auto [result, eval] = isa::operations::Subtraction<T, I>(T{0}, static_cast<T>(a), sat);
    Expected expected;
    expected.result = Truncate(result);
    expected.eva = eval.value;
    return expected;
}

/// Bitwise-style evaluation of an extended word, as BitwiseEvaluation() in cpu.cpp.
uint8_t ExtendEvaluation(uint32_t result) {
    isa::ArithmeticEvaluation eval;
    eval.positive = (result & 0x80000000U) == 0U ? 1U : 0U;
    eval.zero = result == 0U ? 1U : 0U;
    uint8_t byte = 0U;
    static_assert(sizeof(eval) == sizeof(byte));
    std::memcpy(&byte, &eval, sizeof(byte));
    return byte;
}

Expected ReferenceExtend(Size size, bool sign, uint32_t a) {
    Expected expected;
    if (size == Size::Byte) {
        expected.result = sign ? static_cast<uint32_t>(static_cast<int32_t>(static_cast<int8_t>(a)))
                               : static_cast<uint32_t>(static_cast<uint8_t>(a));
    } else if (size == Size::Half) {
        expected.result = sign ? static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(a)))
                               : static_cast<uint32_t>(static_cast<uint16_t>(a));
    } else {
        expected.fault = true;
        expected.compare_eva = false;
        return expected;
    }
    expected.eva = ExtendEvaluation(expected.result);
    return expected;
}

Expected ReferenceFor(Operator op, Size size, bool sign, bool sat, uint32_t a, uint32_t b) {
    if (op == Operator::Extend) {
        return ReferenceExtend(size, sign, a);
    }
    if (size == Size::Double) {
        Expected expected;
        expected.fault = true;
        expected.compare_eva = false;
        return expected;
    }
    if (op == Operator::Negate) {
        switch (size) {
            case Size::Byte:
                return ReferenceNegate<int8_t, int16_t>(a, sat);
            case Size::Half:
                return ReferenceNegate<int16_t, int32_t>(a, sat);
            default:
                return ReferenceNegate<int32_t, int64_t>(a, sat);
        }
    }
    switch (size) {
        case Size::Byte:
            return sign ? Reference<int8_t, int16_t>(op, a, b, sat) : Reference<uint8_t, uint16_t>(op, a, b, sat);
        case Size::Half:
            return sign ? Reference<int16_t, int32_t>(op, a, b, sat) : Reference<uint16_t, uint32_t>(op, a, b, sat);
        default:
            return sign ? Reference<int32_t, int64_t>(op, a, b, sat) : Reference<uint32_t, uint64_t>(op, a, b, sat);
    }
}

constexpr Operator kOperations[] = {Operator::Addition, Operator::Subtract, Operator::Multiply, Operator::Divide,
                                    Operator::Modulo,   Operator::Negate,   Operator::Extend};
constexpr Size kSizes[] = {Size::Byte, Size::Half, Size::Word};

class RtlAluTest : public ::testing::Test {
protected:
    void Apply(Operator op, Size size, bool sign, bool sat, uint32_t a, uint32_t b) {
        dut_.op = static_cast<uint8_t>(op);
        dut_.size = static_cast<uint8_t>(size);
        dut_.sign = sign ? 1 : 0;
        dut_.sat = sat ? 1 : 0;
        dut_.a = a;
        dut_.b = b;
        dut_.eval();
    }

    void ExpectMatchesReference(Operator op, Size size, bool sign, bool sat, uint32_t a, uint32_t b) {
        SCOPED_TRACE(::testing::Message()
                     << "op 0x" << std::hex << static_cast<unsigned>(op) << " size " << static_cast<unsigned>(size)
                     << " sign " << sign << " sat " << sat << " a 0x" << a << " b 0x" << b);
        const Expected expected = ReferenceFor(op, size, sign, sat, a, b);
        Apply(op, size, sign, sat, a, b);
        EXPECT_TRUE(dut_.valid);
        EXPECT_EQ(expected.fault, static_cast<bool>(dut_.fault));
        if (!expected.fault) {
            EXPECT_EQ(expected.result, dut_.result);
        }
        if (expected.compare_eva) {
            EXPECT_EQ(expected.eva, dut_.eva);
        }
    }

    Valu dut_;
};

TEST_F(RtlAluTest, DirectedWordExamples) {
    Apply(Operator::Addition, Size::Word, true, false, 40U, 2U);
    EXPECT_EQ(42U, dut_.result);
    EXPECT_FALSE(dut_.fault);

    Apply(Operator::Subtract, Size::Word, true, false, 2U, 5U);
    EXPECT_EQ(0xFFFFFFFDU, dut_.result);

    Apply(Operator::Multiply, Size::Word, true, false, 6U, 7U);
    EXPECT_EQ(42U, dut_.result);

    Apply(Operator::Divide, Size::Word, true, false, static_cast<uint32_t>(-7), 2U);
    EXPECT_EQ(static_cast<uint32_t>(-3), dut_.result);  // truncates toward zero

    Apply(Operator::Modulo, Size::Word, true, false, static_cast<uint32_t>(-7), 2U);
    EXPECT_EQ(static_cast<uint32_t>(-1), dut_.result);

    Apply(Operator::Negate, Size::Word, true, false, 5U, 0U);
    EXPECT_EQ(static_cast<uint32_t>(-5), dut_.result);
}

TEST_F(RtlAluTest, OverflowWrapsOrSaturates) {
    Apply(Operator::Addition, Size::Word, true, false, 0x7FFFFFFFU, 1U);
    EXPECT_EQ(0x80000000U, dut_.result);
    EXPECT_EQ((isa::operations::Addition<int32_t, int64_t>(INT32_MAX, 1).second.value), dut_.eva);

    Apply(Operator::Addition, Size::Word, true, true, 0x7FFFFFFFU, 1U);
    EXPECT_EQ(0x7FFFFFFFU, dut_.result);

    Apply(Operator::Addition, Size::Byte, false, true, 0xF0U, 0x20U);
    EXPECT_EQ(0xFFU, dut_.result);
}

TEST_F(RtlAluTest, DivideAndModuloByZeroFault) {
    for (Operator op : {Operator::Divide, Operator::Modulo}) {
        Apply(op, Size::Word, true, false, 5U, 0U);
        EXPECT_TRUE(dut_.fault);
        isa::ArithmeticEvaluation undefined;
        undefined.undefined = 1U;
        uint8_t byte = 0U;
        std::memcpy(&byte, &undefined, sizeof(byte));
        EXPECT_EQ(byte, dut_.eva);
    }
}

TEST_F(RtlAluTest, MinimumDividedByMinusOneSaturatesWithOverflow) {
    Apply(Operator::Divide, Size::Word, true, false, 0x80000000U, 0xFFFFFFFFU);
    EXPECT_FALSE(dut_.fault);
    EXPECT_EQ(0x7FFFFFFFU, dut_.result);
    EXPECT_EQ((isa::operations::Division<int32_t>(INT32_MIN, -1).second.value), dut_.eva);

    Apply(Operator::Modulo, Size::Word, true, false, 0x80000000U, 0xFFFFFFFFU);
    EXPECT_FALSE(dut_.fault);
    EXPECT_EQ(0U, dut_.result);
}

TEST_F(RtlAluTest, ResultIsZeroAboveTheOperandSize) {
    Apply(Operator::Addition, Size::Byte, false, false, 0xFFFFFF80U, 0xFFFFFF80U);
    EXPECT_EQ(0x00U, dut_.result);
    Apply(Operator::Addition, Size::Half, false, false, 0xFFFFFFFFU, 0x00000001U);
    EXPECT_EQ(0x0000U, dut_.result);
}

TEST_F(RtlAluTest, ExtendWidensByteAndHalfOnly) {
    Apply(Operator::Extend, Size::Byte, true, false, 0x000000FEU, 0U);
    EXPECT_EQ(0xFFFFFFFEU, dut_.result);
    Apply(Operator::Extend, Size::Byte, false, false, 0xFFFFFFFEU, 0U);
    EXPECT_EQ(0x000000FEU, dut_.result);
    Apply(Operator::Extend, Size::Half, true, false, 0x00008000U, 0U);
    EXPECT_EQ(0xFFFF8000U, dut_.result);
    Apply(Operator::Extend, Size::Word, true, false, 0x00008000U, 0U);
    EXPECT_TRUE(dut_.fault);
}

TEST_F(RtlAluTest, InvalidSizeFaults) {
    for (Operator op : {Operator::Addition, Operator::Subtract, Operator::Divide, Operator::Modulo, Operator::Negate}) {
        Apply(op, Size::Double, true, false, 6U, 3U);
        EXPECT_TRUE(dut_.fault);
    }
}

TEST_F(RtlAluTest, UndefinedOperationIsInvalid) {
    Apply(Operator::Complement, Size::Word, true, false, 1U, 2U);
    EXPECT_FALSE(dut_.valid);
    EXPECT_FALSE(dut_.fault);
}

TEST_F(RtlAluTest, MatchesReferenceOnCornerValues) {
    constexpr uint32_t kValues[] = {0U,      1U,      2U,          0x7FU,       0x80U,       0xFFU,       0x7FFFU,
                                    0x8000U, 0xFFFFU, 0x7FFFFFFFU, 0x80000000U, 0xFFFFFFFFU, 0xA5A5A5A5U, 0x12345678U};
    for (Operator op : kOperations) {
        for (Size size : kSizes) {
            for (bool sign : {false, true}) {
                for (bool sat : {false, true}) {
                    for (uint32_t a : kValues) {
                        for (uint32_t b : kValues) {
                            ExpectMatchesReference(op, size, sign, sat, a, b);
                        }
                    }
                }
            }
        }
    }
}

TEST_F(RtlAluTest, MatchesReferenceOnRandomVectors) {
    std::mt19937 generator{0xA10U};
    for (int i = 0; i < 2000; ++i) {
        for (Operator op : kOperations) {
            for (Size size : kSizes) {
                for (bool sign : {false, true}) {
                    const bool sat = (generator() & 1U) != 0U;
                    ExpectMatchesReference(op, size, sign, sat, generator(), generator());
                }
            }
        }
    }
}

}  // namespace
