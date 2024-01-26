#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <cinttypes>
#include <type_traits>

namespace basal {

#if defined(USE_PRECISION_AS_FLOAT)
using precision = float;
#define PRIprecision "f"
#define SCNprecision "f"
constexpr bool use_high_precision = false;
#else
using precision = double;
#define PRIprecision "lf"
#define SCNprecision "lf"
constexpr bool use_high_precision = true;
#endif

static_assert(std::numeric_limits<precision>::is_iec559, "Must conform to IEEE754");

namespace literals {
/// When basal's type for precision is defined you can use 1.0_p and it will transparently go to the right type
constexpr inline precision operator""_p(long double v) { return precision(v); }
static_assert(std::is_same<decltype(1.0_p), basal::precision>(), "Must be a precision type");
static_assert(std::is_floating_point<decltype(1.0_p)>(), "Must be a floating value");
}

/** If high precision is used, it's 1/(1024*1024) else only (1/1024) */
constexpr static precision epsilon = use_high_precision ? 0x1.0p-20 : 0x1.0p-10;
// 1/(1024) == 0.000007875504032
// 1/(1024*1024) == 0.000000953674316

/// Holds a version of a Quiet (non signalling) NaN for comparisons
constexpr static precision nan = std::numeric_limits<precision>::quiet_NaN();

inline precision clamp(precision min, precision value, precision max) {
    return std::max(min, std::min(value, max));
}

/*! Allows a precision to be approximately equal to another precision within a range */
inline bool equals(precision a, precision b, precision range) {
    return (std::abs(a - b) <= std::abs(range));
}

/*! Allows two precisions to be compared for equality. */
inline bool equals(precision a, precision b) {
    return equals(a, b, basal::epsilon);
    // bool ngth_and_nlth = (not(a > b) and not(a < b));
}

/*! Allows a precision to be compared to zero */
inline bool equals_zero(precision a) {
    using namespace basal::literals;
    return equals(a, 0.0_p);
}

/*! Returns a random value between -1.0 and 1.0 */
inline precision rand_range(precision min_v, precision max_v) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<precision> dis(min_v, max_v);
    return dis(gen);
}

inline bool is_nan(precision a) {
    return (a != a); // Only NaN is not equal to itself
}

}  // namespace basal