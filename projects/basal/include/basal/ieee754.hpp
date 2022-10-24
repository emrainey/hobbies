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

static_assert(std::numeric_limits<double>::is_iec559, "Must conform to IEEE754");

namespace basal {

constexpr bool use_high_precision = true;

/** If high precision is used, it's 1/(1024*1024) else only (1/1024) */
constexpr double epsilon = use_high_precision ? 0x1.0p-20 : 0x1.0p-10;
// 1/(1024) == 0.000007875504032
// 1/(1024*1024) == 0.000000953674316

inline double clamp(double min, double value, double max) {
    return std::max(min, std::min(value, max));
}

/*! Allows a double to be approximately equal to another double within a range */
inline bool equals(double a, double b, double range) {
    return (std::abs(a - b) <= std::abs(range));
}

/*! Allows two doubles to be compared for equality. */
inline bool equals(double a, double b) {
    return equals(a, b, basal::epsilon);
    // bool ngth_and_nlth = (not(a > b) and not(a < b));
}

/*! Allows a double to be compared to zero */
inline bool equals_zero(double a) {
    return equals(a, 0.0);
}

/*! Returns a random value between -1.0 and 1.0 */
inline double rand_range(double min_v, double max_v) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(min_v, max_v);
    return dis(gen);
}
}  // namespace basal