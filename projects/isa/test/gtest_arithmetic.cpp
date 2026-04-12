#include <cstdint>
#include <limits>
#include <type_traits>

#include <gtest/gtest.h>

#include <isa/arithmetic.hpp>

namespace {

using namespace isa::operations;

template <typename OPERAND_TYPE, typename INTERMEDIATE_TYPE>
void ExpectArithmeticResult(OPERAND_TYPE value, isa::Evaluation const& evaluation, OPERAND_TYPE expected_value,
                            bool overflow, bool positive, bool zero) {
    using ComparisonType = std::conditional_t<std::is_unsigned_v<OPERAND_TYPE>, uint64_t, int64_t>;
    EXPECT_EQ(static_cast<ComparisonType>(expected_value), static_cast<ComparisonType>(value));
    EXPECT_EQ(isa::EvaluationType::Arithmetic, evaluation.Type());
    EXPECT_EQ(static_cast<unsigned>(overflow), static_cast<unsigned>(evaluation.arithmetic.overflow));
    EXPECT_EQ(static_cast<unsigned>(positive), static_cast<unsigned>(evaluation.arithmetic.positive));
    EXPECT_EQ(static_cast<unsigned>(zero), static_cast<unsigned>(evaluation.arithmetic.zero));
}

template <typename OPERAND_TYPE, typename INTERMEDIATE_TYPE>
void ExpectAdditionEdgesForSize() {
    const auto max_value = std::numeric_limits<OPERAND_TYPE>::max();
    const auto min_value = std::numeric_limits<OPERAND_TYPE>::min();

    auto [sum_value, sum_evaluation] = Addition<OPERAND_TYPE, INTERMEDIATE_TYPE>(OPERAND_TYPE{7}, OPERAND_TYPE{5});
    ExpectArithmeticResult<OPERAND_TYPE, INTERMEDIATE_TYPE>(sum_value, sum_evaluation, OPERAND_TYPE{12}, false, true,
                                                            false);

    auto [zero_value, zero_evaluation] = Addition<OPERAND_TYPE, INTERMEDIATE_TYPE>(OPERAND_TYPE{0}, OPERAND_TYPE{0});
    ExpectArithmeticResult<OPERAND_TYPE, INTERMEDIATE_TYPE>(zero_value, zero_evaluation, OPERAND_TYPE{0}, false, false,
                                                            true);

    auto [positive_overflow_value, positive_overflow_evaluation]
        = Addition<OPERAND_TYPE, INTERMEDIATE_TYPE>(max_value, OPERAND_TYPE{1}, true);
    ExpectArithmeticResult<OPERAND_TYPE, INTERMEDIATE_TYPE>(positive_overflow_value, positive_overflow_evaluation,
                                                            max_value, true, true, false);

    if constexpr (std::is_signed_v<OPERAND_TYPE>) {
        auto [negative_overflow_value, negative_overflow_evaluation]
            = Addition<OPERAND_TYPE, INTERMEDIATE_TYPE>(min_value, OPERAND_TYPE{-1}, true);
        ExpectArithmeticResult<OPERAND_TYPE, INTERMEDIATE_TYPE>(negative_overflow_value, negative_overflow_evaluation,
                                                                min_value, true, false, false);
    }
}

template <typename OPERAND_TYPE, typename INTERMEDIATE_TYPE>
void ExpectSubtractionEdgesForSize() {
    const auto max_value = std::numeric_limits<OPERAND_TYPE>::max();
    const auto min_value = std::numeric_limits<OPERAND_TYPE>::min();

    auto [difference_value, difference_evaluation]
        = Subtraction<OPERAND_TYPE, INTERMEDIATE_TYPE>(OPERAND_TYPE{12}, OPERAND_TYPE{5});
    ExpectArithmeticResult<OPERAND_TYPE, INTERMEDIATE_TYPE>(difference_value, difference_evaluation, OPERAND_TYPE{7},
                                                            false, true, false);

    auto [zero_value, zero_evaluation] = Subtraction<OPERAND_TYPE, INTERMEDIATE_TYPE>(OPERAND_TYPE{9}, OPERAND_TYPE{9});
    ExpectArithmeticResult<OPERAND_TYPE, INTERMEDIATE_TYPE>(zero_value, zero_evaluation, OPERAND_TYPE{0}, false, false,
                                                            true);

    if constexpr (std::is_signed_v<OPERAND_TYPE>) {
        auto [positive_overflow_value, positive_overflow_evaluation]
            = Subtraction<OPERAND_TYPE, INTERMEDIATE_TYPE>(max_value, OPERAND_TYPE{-1}, true);
        ExpectArithmeticResult<OPERAND_TYPE, INTERMEDIATE_TYPE>(positive_overflow_value, positive_overflow_evaluation,
                                                                max_value, true, true, false);

        auto [negative_overflow_value, negative_overflow_evaluation]
            = Subtraction<OPERAND_TYPE, INTERMEDIATE_TYPE>(min_value, OPERAND_TYPE{1}, true);
        ExpectArithmeticResult<OPERAND_TYPE, INTERMEDIATE_TYPE>(negative_overflow_value, negative_overflow_evaluation,
                                                                min_value, true, false, false);
    } else {
        auto [underflow_value, underflow_evaluation]
            = Subtraction<OPERAND_TYPE, INTERMEDIATE_TYPE>(OPERAND_TYPE{0}, OPERAND_TYPE{1}, true);
        ExpectArithmeticResult<OPERAND_TYPE, INTERMEDIATE_TYPE>(underflow_value, underflow_evaluation, min_value, true,
                                                                false, false);
    }
}

template <typename OPERAND_TYPE, typename INTERMEDIATE_TYPE>
void ExpectMultiplyEdgesForSize() {
    const auto max_value = std::numeric_limits<OPERAND_TYPE>::max();
    const auto min_value = std::numeric_limits<OPERAND_TYPE>::min();

    auto [product_value, product_evaluation]
        = Multiply<OPERAND_TYPE, INTERMEDIATE_TYPE>(OPERAND_TYPE{6}, OPERAND_TYPE{7});
    ExpectArithmeticResult<OPERAND_TYPE, INTERMEDIATE_TYPE>(product_value, product_evaluation, OPERAND_TYPE{42}, false,
                                                            true, false);

    auto [zero_value, zero_evaluation] = Multiply<OPERAND_TYPE, INTERMEDIATE_TYPE>(OPERAND_TYPE{0}, OPERAND_TYPE{7});
    ExpectArithmeticResult<OPERAND_TYPE, INTERMEDIATE_TYPE>(zero_value, zero_evaluation, OPERAND_TYPE{0}, false, false,
                                                            true);

    auto [positive_overflow_value, positive_overflow_evaluation]
        = Multiply<OPERAND_TYPE, INTERMEDIATE_TYPE>(max_value, OPERAND_TYPE{2}, true);
    ExpectArithmeticResult<OPERAND_TYPE, INTERMEDIATE_TYPE>(positive_overflow_value, positive_overflow_evaluation,
                                                            max_value, true, true, false);

    if constexpr (std::is_signed_v<OPERAND_TYPE>) {
        auto [negative_overflow_value, negative_overflow_evaluation]
            = Multiply<OPERAND_TYPE, INTERMEDIATE_TYPE>(min_value, OPERAND_TYPE{2}, true);
        ExpectArithmeticResult<OPERAND_TYPE, INTERMEDIATE_TYPE>(negative_overflow_value, negative_overflow_evaluation,
                                                                min_value, true, false, false);
    }
}

TEST(ArithmeticTest, AdditionEdgeCasesCoverByteHalfWordAndWord) {
    ExpectAdditionEdgesForSize<int8_t, int16_t>();
    ExpectAdditionEdgesForSize<uint8_t, uint16_t>();
    ExpectAdditionEdgesForSize<int16_t, int32_t>();
    ExpectAdditionEdgesForSize<uint16_t, uint32_t>();
    ExpectAdditionEdgesForSize<int32_t, int64_t>();
    ExpectAdditionEdgesForSize<uint32_t, uint64_t>();
}

TEST(ArithmeticTest, SubtractionEdgeCasesCoverByteHalfWordAndWord) {
    ExpectSubtractionEdgesForSize<int8_t, int16_t>();
    ExpectSubtractionEdgesForSize<uint8_t, uint16_t>();
    ExpectSubtractionEdgesForSize<int16_t, int32_t>();
    ExpectSubtractionEdgesForSize<uint16_t, uint32_t>();
    ExpectSubtractionEdgesForSize<int32_t, int64_t>();
    ExpectSubtractionEdgesForSize<uint32_t, uint64_t>();
}

TEST(ArithmeticTest, MultiplyEdgeCasesCoverByteHalfWordAndWord) {
    ExpectMultiplyEdgesForSize<int8_t, int16_t>();
    ExpectMultiplyEdgesForSize<uint8_t, uint16_t>();
    ExpectMultiplyEdgesForSize<int16_t, int32_t>();
    ExpectMultiplyEdgesForSize<uint16_t, uint32_t>();
    ExpectMultiplyEdgesForSize<int32_t, int64_t>();
    ExpectMultiplyEdgesForSize<uint32_t, uint64_t>();
}

TEST(ArithmeticTest, MultiplyUnsignedWithoutSaturationWrapsAndFlagsOverflow) {
    auto [value, evaluation] = Multiply<uint8_t, uint16_t>(20U, 15U, false);

    ExpectArithmeticResult<uint8_t, uint16_t>(value, evaluation, static_cast<uint8_t>(300U), true, true, false);
}

// ===== Division =====

template <typename OPERAND_TYPE>
void ExpectDivisionEdgesForSize() {
    const auto max_value = std::numeric_limits<OPERAND_TYPE>::max();
    const auto min_value = std::numeric_limits<OPERAND_TYPE>::min();

    // Normal quotient
    {
        auto [result, eval] = Division<OPERAND_TYPE>(OPERAND_TYPE{42}, OPERAND_TYPE{7});
        EXPECT_EQ(isa::EvaluationType::Arithmetic, eval.Type());
        EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.undefined));
        EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.overflow));
        EXPECT_EQ(OPERAND_TYPE{6}, result);
        EXPECT_EQ(1U, static_cast<unsigned>(eval.arithmetic.positive));
        EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.zero));
    }

    // Result is zero
    {
        auto [result, eval] = Division<OPERAND_TYPE>(OPERAND_TYPE{0}, OPERAND_TYPE{7});
        EXPECT_EQ(isa::EvaluationType::Arithmetic, eval.Type());
        EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.undefined));
        EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.overflow));
        EXPECT_EQ(OPERAND_TYPE{0}, result);
        EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.positive));
        EXPECT_EQ(1U, static_cast<unsigned>(eval.arithmetic.zero));
    }

    // Divide by zero → undefined flag, result 0
    {
        auto [result, eval] = Division<OPERAND_TYPE>(OPERAND_TYPE{10}, OPERAND_TYPE{0});
        EXPECT_EQ(isa::EvaluationType::Arithmetic, eval.Type());
        EXPECT_EQ(1U, static_cast<unsigned>(eval.arithmetic.undefined));
        EXPECT_EQ(OPERAND_TYPE{0}, result);
    }

    if constexpr (std::is_signed_v<OPERAND_TYPE>) {
        // Negative dividend
        {
            auto [result, eval] = Division<OPERAND_TYPE>(OPERAND_TYPE{-14}, OPERAND_TYPE{7});
            EXPECT_EQ(isa::EvaluationType::Arithmetic, eval.Type());
            EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.undefined));
            EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.overflow));
            EXPECT_EQ(OPERAND_TYPE{-2}, result);
            EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.positive));
            EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.zero));
        }

        // Signed overflow: min / -1 saturates to max
        {
            auto [result, eval] = Division<OPERAND_TYPE>(min_value, OPERAND_TYPE{-1});
            EXPECT_EQ(isa::EvaluationType::Arithmetic, eval.Type());
            EXPECT_EQ(1U, static_cast<unsigned>(eval.arithmetic.overflow));
            EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.undefined));
            EXPECT_EQ(max_value, result);
            EXPECT_EQ(1U, static_cast<unsigned>(eval.arithmetic.positive));
        }
    }
}

TEST(ArithmeticTest, DivisionEdgeCasesCoverByteHalfWordAndWord) {
    ExpectDivisionEdgesForSize<int8_t>();
    ExpectDivisionEdgesForSize<uint8_t>();
    ExpectDivisionEdgesForSize<int16_t>();
    ExpectDivisionEdgesForSize<uint16_t>();
    ExpectDivisionEdgesForSize<int32_t>();
    ExpectDivisionEdgesForSize<uint32_t>();
}

// ===== Modulo =====

template <typename OPERAND_TYPE>
void ExpectModuloEdgesForSize() {
    // Normal remainder
    {
        auto [result, eval] = Modulo<OPERAND_TYPE>(OPERAND_TYPE{17}, OPERAND_TYPE{5});
        EXPECT_EQ(isa::EvaluationType::Arithmetic, eval.Type());
        EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.undefined));
        EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.overflow));
        EXPECT_EQ(OPERAND_TYPE{2}, result);
        EXPECT_EQ(1U, static_cast<unsigned>(eval.arithmetic.positive));
        EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.zero));
    }

    // Remainder is zero (exact multiple)
    {
        auto [result, eval] = Modulo<OPERAND_TYPE>(OPERAND_TYPE{12}, OPERAND_TYPE{4});
        EXPECT_EQ(isa::EvaluationType::Arithmetic, eval.Type());
        EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.undefined));
        EXPECT_EQ(OPERAND_TYPE{0}, result);
        EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.positive));
        EXPECT_EQ(1U, static_cast<unsigned>(eval.arithmetic.zero));
    }

    // Divide by zero -> undefined flag, result 0
    {
        auto [result, eval] = Modulo<OPERAND_TYPE>(OPERAND_TYPE{10}, OPERAND_TYPE{0});
        EXPECT_EQ(isa::EvaluationType::Arithmetic, eval.Type());
        EXPECT_EQ(1U, static_cast<unsigned>(eval.arithmetic.undefined));
        EXPECT_EQ(OPERAND_TYPE{0}, result);
    }

    if constexpr (std::is_signed_v<OPERAND_TYPE>) {
        // Negative dividend: remainder retains dividend sign in C++
        {
            auto [result, eval] = Modulo<OPERAND_TYPE>(OPERAND_TYPE{-17}, OPERAND_TYPE{5});
            EXPECT_EQ(isa::EvaluationType::Arithmetic, eval.Type());
            EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.undefined));
            EXPECT_EQ(OPERAND_TYPE{-2}, result);
            EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.positive));
            EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.zero));
        }

        // Modulo by -1: always 0, no overflow (unlike division)
        {
            auto [result, eval] = Modulo<OPERAND_TYPE>(std::numeric_limits<OPERAND_TYPE>::min(), OPERAND_TYPE{-1});
            EXPECT_EQ(isa::EvaluationType::Arithmetic, eval.Type());
            EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.undefined));
            EXPECT_EQ(0U, static_cast<unsigned>(eval.arithmetic.overflow));
            EXPECT_EQ(OPERAND_TYPE{0}, result);
            EXPECT_EQ(1U, static_cast<unsigned>(eval.arithmetic.zero));
        }
    }
}

TEST(ArithmeticTest, ModuloEdgeCasesCoverByteHalfWordAndWord) {
    ExpectModuloEdgesForSize<int8_t>();
    ExpectModuloEdgesForSize<uint8_t>();
    ExpectModuloEdgesForSize<int16_t>();
    ExpectModuloEdgesForSize<uint16_t>();
    ExpectModuloEdgesForSize<int32_t>();
    ExpectModuloEdgesForSize<uint32_t>();
}

}  // namespace
