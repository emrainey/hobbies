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

/// If high precision is used, it's 1/(1024*1024) else only (1/1024)
constexpr static precision epsilon = use_high_precision ? 0x1.0p-20 : 0x1.0p-10;
// 1/(1024) == 0.000007875504032
// 1/(1024*1024) == 0.000000953674316

/// Holds a version of a Quiet (non signalling) NaN for comparisons
constexpr static precision nan = std::numeric_limits<precision>::quiet_NaN();

inline precision clamp(precision min, precision value, precision max) {
    return std::max(min, std::min(value, max));
}

/// Returns true if the two precisions are within the range of each other.
inline bool nearly_equals(precision a, precision b, precision range) {
    return (std::abs(a - b) <= std::abs(range));
}

/// Allows two precisions to be compared for equality.
inline bool nearly_equals(precision a, precision b) {
    return nearly_equals(a, b, basal::epsilon);
    // bool ngth_and_nlth = (not(a > b) and not(a < b));
}

/// Determines if a precision is very close to zero
inline bool nearly_zero(precision a) {
    using namespace basal::literals;
    return nearly_equals(a, 0.0_p, basal::epsilon);
}

/// @brief Returns true if the precision is either exactly +0.0 or -0.0
/// @param a The precision to check
inline bool is_exactly_zero(precision a) {
    return (FP_ZERO == std::fpclassify(a));
}

/// A comparison which neither value is larger than the other, effectively equivalent
inline bool equivalent(precision const a, precision const b) {
    return not(a > b) and not(a < b);
}

/// Returns a random value between -1.0 and 1.0
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