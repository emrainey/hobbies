
#pragma once

#include <bitset>
#include <cstdlib>
#include <cstddef>
#include "basal/exception.hpp"

namespace htm {

/**
 * Returns the closest odd number (at least 1) that is approximately 2% of the input value.
 *
 * @param input The input value to calculate percent of.
 * @return The closest odd number >= 1.
 */
constexpr size_t closest_odd_approx_for_percent(size_t input, double percent) {
    // Handle the edge case for 0 immediately.
    if (input == 0) {
        return 1;
    }

    // Calculate 2% of the input.
    // We use double for the calculation as the requirement is "approximately 2%".
    double target = static_cast<double>(input) * percent;

    // The result must be at least 1.
    if (target < 1.0) {
        return 1;
    }

    // Round to the nearest integer.
    // Since target >= 1.0, target + 0.5 will always be >= 1.5.
    // We cast to long long to perform the rounding.
    long long rounded = static_cast<long long>(target + 0.5);

    // If the rounded value is odd, return it.
    if (rounded % 2 != 0) {
        return static_cast<size_t>(rounded);
    }

    // If the rounded value is even, we check which neighbor (rounded-1 or rounded+1)
    // is closer to the original target value.
    double lower = static_cast<double>(rounded - 1);
    double upper = static_cast<double>(rounded + 1);

    // If r is 0, lower would be -1, but target >= 1.0 ensures r >= 1.
    if ((target - lower) < (upper - target)) {
        return static_cast<size_t>(lower);
    } else {
        // In case of a tie (e.g., target is exactly 2.0),
        // we default to the larger odd number.
        return static_cast<size_t>(upper);
    }
}

/// @brief A dense word is a bitset that is exactly the size of the number of bits
/// @tparam BITS
template <size_t BITS>
class dense_word {
public:
    /// This would require up to a 65536 + Width bit field size.
    using Type = std::uint16_t;

    constexpr static std::size_t Bits = BITS;
    /// Width should be an odd value >= 1 and approximately 2% of the number of BITS.
    constexpr static std::size_t Width = closest_odd_approx_for_percent(BITS, 0.02);
    /// The under-half is the integer rounded half value of the Width
    constexpr static std::size_t Underhalf = (Width / 2U);
    /// The value which indicates that there is no value set
    constexpr static Type None = static_cast<Type>(BITS);

    /// Default constructor
    dense_word() : bits_{} {
    }

    /// Copy
    dense_word(dense_word const& other) : bits_{other.bits_} {
    }

    /// Assign
    dense_word& operator=(dense_word const& other) {
        bits_ = other.bits_;
        return *this;
    }

    /// Assign via integer
    dense_word& operator=(size_t value) {
        basal::exception::throw_if(value > (Bits - (2U * Underhalf)), __FILE__, __LINE__,
                                   "Can't represent %z in %z bits\r\n", value, BITS);
        for (size_t i = (value - Underhalf); i <= (value + Underhalf); i++) {
            bits_.set(i);
        }
        return *this;
    }

    /// Parameter Constructor
    dense_word(size_t value) : bits_{} {
        basal::exception::throw_if(value > (Bits - (2U * Underhalf)), __FILE__, __LINE__,
                                   "Can't represent %zu in %zu bits\r\n", value, BITS);
        for (size_t i = (value - Underhalf); i <= (value + Underhalf); i++) {
            bits_.set(i);
        }
    }

    /// Directly tests ech particular bit
    bool test(std::size_t value) const {
        return bits_.test(value);
    }

    /// This reduces the value back from a sparse or dense bitfield to a singular value
    explicit operator Type() const {
        // we have to find the center of the Width of bits
        std::size_t start = BITS;
        std::size_t count = 0;
        for (std::size_t i = 0; i < BITS; i++) {
            if (bits_.test(i)) {
                if (start == BITS) {
                    start = i;
                }
                if (count < Width) {
                    count++;
                }
                if (count == Width) {
                    return static_cast<Type>(i - Underhalf);
                }
            } else {
                start = BITS;
                count = 0;
            }
        }
        // nothing found, but it can't be "nul", will return unpresentable number
        return BITS;
    }

    constexpr std::size_t size() const {
        return BITS;
    }

    bool any() const {
        return bits_.any();
    }

    std::size_t count() const {
        return bits_.count();
    }

protected:
    std::bitset<BITS> bits_;
};

}  // namespace htm
