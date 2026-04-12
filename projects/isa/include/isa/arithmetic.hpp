#pragma once

#include <algorithm>
#include <isa/types.hpp>

namespace isa {

/// A collection of arithmetic operations that can be used by the CPU to perform arithmetic calculations
namespace operations {
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

}  // namespace operations

}  // namespace isa