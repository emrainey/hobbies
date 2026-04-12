#pragma once

#include <algorithm>
#include <isa/types.hpp>

namespace isa {

/// A collection of arithmetic operations that can be used by the CPU to perform arithmetic calculations
namespace operations {

/// Adds two values of OPERAND_TYPE using INTERMEDIATE_TYPE as the wider scratch type for overflow detection.
/// The overflow flag is set when the true mathematical result does not fit in OPERAND_TYPE.
/// When @p saturating is true the returned value is clamped to [min, max] instead of wrapping.
/// @tparam OPERAND_TYPE  The type of both operands and the returned value (e.g. int8_t, uint32_t).
/// @tparam INTERMEDIATE_TYPE  A wider type used for the intermediate calculation (e.g. int16_t for int8_t operands).
/// @param a         Left operand.
/// @param b         Right operand.
/// @param saturating  When true, clamp the result to the representable range rather than wrapping.
/// @return A pair of {result, Evaluation} where Evaluation carries overflow/positive/zero flags.
template <typename OPERAND_TYPE, typename INTERMEDIATE_TYPE>
constexpr std::pair<OPERAND_TYPE, Evaluation> Addition(OPERAND_TYPE a, OPERAND_TYPE b, bool saturating = false) {
    using CalculationType = std::conditional_t<std::is_unsigned_v<INTERMEDIATE_TYPE>,
                                               std::make_signed_t<INTERMEDIATE_TYPE>, INTERMEDIATE_TYPE>;
    constexpr auto max_value = static_cast<CalculationType>(std::numeric_limits<OPERAND_TYPE>::max());
    constexpr auto min_value = static_cast<CalculationType>(std::numeric_limits<OPERAND_TYPE>::min());
    CalculationType result = static_cast<CalculationType>(a) + static_cast<CalculationType>(b);
    Evaluation evaluation = ArithmeticEvaluation{};
    evaluation.arithmetic.overflow = (result < min_value or result > max_value) ? 1U : 0U;
    evaluation.arithmetic.positive = (result > 0) ? 1U : 0U;
    evaluation.arithmetic.zero = (result == 0) ? 1U : 0U;
    if (saturating) {
        result = std::min(std::max(result, min_value), max_value);
    }
    return std::make_pair(static_cast<OPERAND_TYPE>(result), evaluation);
}

/// Subtracts @p b from @p a using INTERMEDIATE_TYPE as the wider scratch type for overflow detection.
/// The overflow flag is set when the true mathematical result does not fit in OPERAND_TYPE.
/// When @p saturating is true the returned value is clamped to [min, max] instead of wrapping.
/// @tparam OPERAND_TYPE  The type of both operands and the returned value.
/// @tparam INTERMEDIATE_TYPE  A wider type used for the intermediate calculation.
/// @param a         Left (minuend) operand.
/// @param b         Right (subtrahend) operand.
/// @param saturating  When true, clamp the result to the representable range rather than wrapping.
/// @return A pair of {result, Evaluation} where Evaluation carries overflow/positive/zero flags.
template <typename OPERAND_TYPE, typename INTERMEDIATE_TYPE>
constexpr std::pair<OPERAND_TYPE, Evaluation> Subtraction(OPERAND_TYPE a, OPERAND_TYPE b, bool saturating = false) {
    using CalculationType = std::conditional_t<std::is_unsigned_v<INTERMEDIATE_TYPE>,
                                               std::make_signed_t<INTERMEDIATE_TYPE>, INTERMEDIATE_TYPE>;
    constexpr auto max_value = static_cast<CalculationType>(std::numeric_limits<OPERAND_TYPE>::max());
    constexpr auto min_value = static_cast<CalculationType>(std::numeric_limits<OPERAND_TYPE>::min());
    CalculationType result = static_cast<CalculationType>(a) - static_cast<CalculationType>(b);
    Evaluation evaluation = ArithmeticEvaluation{};
    evaluation.arithmetic.overflow = (result < min_value or result > max_value) ? 1U : 0U;
    evaluation.arithmetic.positive = (result > 0) ? 1U : 0U;
    evaluation.arithmetic.zero = (result == 0) ? 1U : 0U;
    if (saturating) {
        result = std::min(std::max(result, min_value), max_value);
    }
    return std::make_pair(static_cast<OPERAND_TYPE>(result), evaluation);
}

/// Multiplies two values of OPERAND_TYPE using INTERMEDIATE_TYPE to hold the full-width product.
/// The overflow flag is set when the product does not fit back into OPERAND_TYPE.
/// The positive and zero flags reflect the full-width product before any saturation is applied.
/// When @p saturating is true the returned value is clamped to [min, max] instead of wrapping.
/// @tparam OPERAND_TYPE  The type of both operands and the returned value.
/// @tparam INTERMEDIATE_TYPE  A wider type that can hold the full product (e.g. int16_t for int8_t operands).
/// @param a         Left (multiplicand) operand.
/// @param b         Right (multiplier) operand.
/// @param saturating  When true, clamp the product to the representable range rather than wrapping.
/// @return A pair of {result, Evaluation} where Evaluation carries overflow/positive/zero flags.
template <typename OPERAND_TYPE, typename INTERMEDIATE_TYPE>
constexpr std::pair<OPERAND_TYPE, Evaluation> Multiply(OPERAND_TYPE a, OPERAND_TYPE b, bool saturating = false) {
    constexpr auto max_value = static_cast<INTERMEDIATE_TYPE>(std::numeric_limits<OPERAND_TYPE>::max());
    constexpr auto min_value = static_cast<INTERMEDIATE_TYPE>(std::numeric_limits<OPERAND_TYPE>::min());
    INTERMEDIATE_TYPE result = static_cast<INTERMEDIATE_TYPE>(a) * static_cast<INTERMEDIATE_TYPE>(b);
    Evaluation evaluation = ArithmeticEvaluation{};
    if constexpr (std::is_unsigned<OPERAND_TYPE>::value) {
        evaluation.arithmetic.overflow = result > max_value;
    } else {
        evaluation.arithmetic.overflow = result < min_value or result > max_value;
    }
    evaluation.arithmetic.positive = (result > 0) ? 1 : 0;
    evaluation.arithmetic.zero = (result == 0) ? 1 : 0;
    if (saturating) {
        result = std::min(std::max(result, min_value), max_value);
    }
    return std::make_pair(static_cast<OPERAND_TYPE>(result), evaluation);
}

/// Division with divide-by-zero detection and signed overflow (INT_MIN / -1) handling.
/// @note Because integer division always produces a result that fits in the operand type (except
/// the special case above), no INTERMEDIATE_TYPE is required.
template <typename OPERAND_TYPE>
constexpr std::pair<OPERAND_TYPE, Evaluation> Division(OPERAND_TYPE a, OPERAND_TYPE b) {
    Evaluation evaluation = ArithmeticEvaluation{};
    if (b == OPERAND_TYPE{0}) {
        evaluation.arithmetic.undefined = 1U;
        return std::make_pair(OPERAND_TYPE{0}, evaluation);
    }
    if constexpr (std::is_signed_v<OPERAND_TYPE>) {
        // Signed overflow: minval / -1 is UB in C++; detect, saturate, and flag it.
        if (a == std::numeric_limits<OPERAND_TYPE>::min() and b == OPERAND_TYPE{-1}) {
            evaluation.arithmetic.overflow = 1U;
            evaluation.arithmetic.positive = 1U;
            return std::make_pair(std::numeric_limits<OPERAND_TYPE>::max(), evaluation);
        }
    }
    OPERAND_TYPE result = a / b;
    evaluation.arithmetic.positive = (result > OPERAND_TYPE{0}) ? 1U : 0U;
    evaluation.arithmetic.zero = (result == OPERAND_TYPE{0}) ? 1U : 0U;
    return std::make_pair(result, evaluation);
}

/// Computes the remainder of @p a divided by @p b (a % b).
/// Divide-by-zero sets the undefined flag and returns 0.
/// For signed types, modulo by -1 always yields 0 (not UB — the signed UB case is minval/%−1 which
/// the C++ standard defines as 0 for `%`, unlike `/`).
/// The positive and zero flags reflect the remainder value.
/// @tparam OPERAND_TYPE  The type of both operands and the returned value.
/// @param a  Dividend.
/// @param b  Divisor.
/// @return A pair of {remainder, Evaluation} where Evaluation carries positive/zero/undefined flags.
template <typename OPERAND_TYPE>
constexpr std::pair<OPERAND_TYPE, Evaluation> Modulo(OPERAND_TYPE a, OPERAND_TYPE b) {
    Evaluation evaluation = ArithmeticEvaluation{};
    if (b == OPERAND_TYPE{0}) {
        evaluation.arithmetic.undefined = 1U;
        return std::make_pair(OPERAND_TYPE{0}, evaluation);
    }
    // For signed types, any_value % -1 == 0 by definition (a = q*b + r, b=-1 means q=-a, r=0).
    // Avoid letting the compiler treat minval % -1 as UB via the related minval / -1 overflow path.
    if constexpr (std::is_signed_v<OPERAND_TYPE>) {
        if (b == OPERAND_TYPE{-1}) {
            evaluation.arithmetic.zero = 1U;
            return std::make_pair(OPERAND_TYPE{0}, evaluation);
        }
    }
    OPERAND_TYPE result = a % b;
    evaluation.arithmetic.positive = (result > OPERAND_TYPE{0}) ? 1U : 0U;
    evaluation.arithmetic.zero = (result == OPERAND_TYPE{0}) ? 1U : 0U;
    return std::make_pair(result, evaluation);
}

}  // namespace operations

}  // namespace isa