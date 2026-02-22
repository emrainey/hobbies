#pragma once

/// @file
/// Definitions for gamma correction and color space conversions.

#include <fourcc/types.hpp>

namespace fourcc {

namespace gamma {

/// Enables using the full equation for the corrections
constexpr bool use_full{true};
/// Enables contract enforcement on the parameters to the use of gama related functions.
constexpr bool enforce_contract{false};

/// Interpolates between a and b using the gamma correction scheme. The ratio is between 0 and 1, where 0 is all a and 1 is all b.
/// @param a The first value to interpolate between.
/// @param b The second value to interpolate between.
/// @param s The ratio between a and b to use for the interpolation. Should be between 0 and 1, where 0 is all a and 1 is all b.
precision interpolate(precision a, precision b, precision s);

/// Converts a Gamma Corrected value to a Linear value. This is used for converting from sRGB space to linear space for calculations.
/// @param value The value to convert from gamma corrected to linear. Should be between 0 and 1.
constexpr precision remove_correction(precision value) {
    using namespace basal::literals;
    if constexpr (use_full) {
        if (value <= 0.04045_p) {
            return value / 12.92_p;
        } else {
            return std::pow((value + 0.055_p) / 1.055_p, 2.4_p);
        }
    } else {
        return std::pow(value, 2.2_p);
    }
}

/// Converts a Linear value to a Gamma Corrected value. This is used for converting from linear space to sRGB space for output.
/// @param value The value to convert from linear to gamma corrected. Should be between 0 and 1.
constexpr precision apply_correction(precision value) {
    using namespace basal::literals;
    if constexpr (use_full) {
        if (value <= 0.0031308_p) {
            return 12.92_p * value;
        } else {
            return (1.055_p * std::pow(value, 0.41666_p)) - 0.055_p;
        }
    } else {
        return std::pow(value, 1.0_p / 2.2_p);
    }
}

/// Interpolates between a and b using the gamma correction scheme. The ratio is between 0 and 1, where 0 is all a and 1 is all b.
/// @param a The first value to interpolate between.
/// @param b The second value to interpolate between.
/// @param s The ratio between a and b to use for the interpolation. Should be between 0 and 1, where 0 is all a and 1 is all b.
precision interpolate(precision const a, precision const b, precision const s);

}  // namespace gamma

namespace linear {

/// Interpolates between a and b using the linear scheme. The ratio is between 0 and 1, where 0 is all a and 1 is all b.
/// @param a The first value to interpolate between.
/// @param b The second value to interpolate between.
/// @param s The ratio between a and b to use for the interpolation. Should be between 0 and 1, where 0 is all a and 1 is all b.
constexpr precision interpolate(precision a, precision b, precision s) {
    return (a * s) + (b * (1.0_p - s));
}

}  // namespace linear

}  // namespace fourcc
