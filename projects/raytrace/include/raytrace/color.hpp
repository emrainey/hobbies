#pragma once

#include <cmath>
#include <cstdint>
#include <fourcc/image.hpp>

#include "raytrace/types.hpp"

namespace raytrace {

/// The Gamma Correction Namespace
namespace gamma {

constexpr static bool use_full = true;

///
/// Removes Gamma Correction
/// @see https://ixora.io/projects/colorblindness/color-blindness-simulation-research/
/// @param value The value to correct (must be nominal (0-1))
/// @return The corrected value
///
constexpr precision remove_correction(precision value) {
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

///
/// Applies Gamma Correction
/// @see https://ixora.io/projects/colorblindness/color-blindness-simulation-research/
/// @param value The value to correct (must be nominal (0-1))
/// @return The corrected value
///
constexpr precision apply_correction(precision value) {
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

}  // namespace gamma

/// Adds two channels together after gamma correction by the scalar given.
/// @note the scalar must be 0.0_p <= s <= 1.0_p.
/// @retval precision will be clamped from 0.0_p to 1.0_p
precision gamma_interpolate(precision const a, precision const b, precision const s);

#if defined(USE_XMMT)
#if defined(USE_PRECISION_AS_FLOAT)
using precision4 = intel::float4;
#else
using precision4 = intel::double4;
#endif
#endif

/// @brief sRGB linear color space value
class color {
public:
    constexpr static size_t NUM_CHANNELS = 4;
    precision channels[NUM_CHANNELS];

    /// Indicates how the values are stored
    enum class space : int {
        linear,
        logarithmic,
    } representation;

    constexpr explicit color() : color(0.0_p, 0.0_p, 0.0_p) {
    }

    constexpr explicit color(precision _r, precision _g, precision _b, precision _i = 1.0_p)
        : channels{_r, _g, _b, _i}, representation{space::linear} {
    }

    // Copy Constructor
    color(color const& other);
    // Move constructor
    color(color&& other);
    // Copy Assign
    color& operator=(color const&);
    // Move Assign
    color& operator=(color&&);  // this is used in the get_color() assignment
    // Destructor
    ~color() = default;

    constexpr precision const volatile& red() const volatile {
        return channels[0];
    }
    constexpr precision const volatile& green() const volatile {
        return channels[1];
    }
    constexpr precision const volatile& blue() const volatile {
        return channels[2];
    }
    constexpr precision const volatile& intensity() const volatile {
        return channels[3];
    }

    /// Converts the color from one space to another.
    void to_space(space desired);

    /// Scaling operator
    inline void scale(precision const a, bool with_clamp = true) {
        for (auto& c : channels) {
            c *= a;
        }
        if (with_clamp) {
            clamp();
        }
    }

    inline void clamp() {
        for (auto& c : channels) {
            c = std::clamp(c, 0.0_p, 1.0_p);
        }
    }

    /// Scale Wrapper
    inline color& operator*=(precision const a) {
        scale(a, true);
        return (*this);
    }

    fourcc::rgba to_rgba() const;
    fourcc::abgr to_abgr() const;
    fourcc::rgb8 to_rgb8() const;
    fourcc::bgr8 to_bgr8() const;

    /// Allows a per channel operation
    void per_channel(std::function<precision(precision c)> iter);

    /// Accumulates colors together (does not gamma correct)
    color& operator+=(color const& other);

    /// The comparison precision for equal colors (to a millionth)
    constexpr static precision equality_limit = 1.0E-6;

    /// Blends all colors together in a equal weighted average with gamma
    /// correction.
    /// @param subsamples The vector of color samples
    static color blend_samples(std::vector<color> const& subsamples);

    /// Accumulates all the samples together into a summed color in the linear space
    static color accumulate_samples(std::vector<color> const& samples);

    /// Map numbers near the range -1.0_p to 1.0_p to a color in the spectrum.
    /// Bluer for negatives, Reds for Positives.
    static color jet(precision d);

    /// Takes a number between min and max and returns an RGB from 0.0_p to 1.0_p.
    /// If the input is out of range, returns magenta.
    static color greyscale(precision d, precision min, precision max);

    /// Generates a random color.
    static color random();

    // Don't add interfaces as color needs to be constexpr for colors below
};

namespace colors {
constexpr color white(1.0_p, 1.0_p, 1.0_p);
constexpr color grey(0.73536062_p, 0.73536062_p, 0.73536062_p);
constexpr color black(0.0_p, 0.0_p, 0.0_p);
// primaries
constexpr color red(1.0_p, 0.0_p, 0.0_p);
constexpr color green(0.0_p, 1.0_p, 0.0_p);
constexpr color blue(0.0_p, 0.0_p, 1.0_p);
// secondaries
constexpr color yellow(1.0_p, 1.0_p, 0.0_p);
constexpr color magenta(1.0_p, 0.0_p, 1.0_p);
constexpr color cyan(0.0_p, 1.0_p, 1.0_p);
// nice others
constexpr color orange(1.0_p, 0.84_p, 0);
constexpr color brown(0.588235294_p, 0.294117647_p, 0);
constexpr color plum(0.760784314_p, 0.576470588_p, 0.552941176_p);
constexpr color forest_green(0.133333333_p, 0.545098039_p, 0.133333333_p);
constexpr color silver(0.752941176_p, 0.752941176_p, 0.752941176_p);

constexpr color aquamarine(0.439216_p, 0.858824_p, 0.576471_p);
constexpr color blue_violet(0.623529_p, 0.372549_p, 0.623529_p);
constexpr color brilliant_red(0.68_p, 0.405_p, 0.405_p);
// constexpr color brown(0.647059_p, 0.164706_p, 0.164706_p);
constexpr color cadet_blue(0.372549_p, 0.623529_p, 0.623529_p);
constexpr color coral(1, 0.498039_p, 0);
constexpr color cornflower_blue(0.258824_p, 0.258824_p, 0.435294_p);
constexpr color dark_brown(0.1941_p, 0.0492_p, 0.0492_p);
constexpr color dark_gray(0.329412_p, 0.329412_p, 0.329412_p);
constexpr color dark_grey(0.329412_p, 0.329412_p, 0.329412_p);
constexpr color dark_green(0.184314_p, 0.309804_p, 0.184314_p);
constexpr color dark_olive_green(0.309804_p, 0.309804_p, 0.184314_p);
constexpr color dark_orchid(0.6_p, 0.196078_p, 0.8_p);
constexpr color dark_plum(0.45_p, 0.34_p, 0.45_p);
constexpr color dark_slate_blue(0.419608_p, 0.137255_p, 0.556863_p);
constexpr color dark_slate_gray(0.184314_p, 0.309804_p, 0.309804_p);
constexpr color dark_slate_grey(0.184314_p, 0.309804_p, 0.309804_p);
constexpr color dark_tan(0.51_p, 0.35_p, 0.264_p);
constexpr color dark_turquoise(0.439216_p, 0.576471_p, 0.858824_p);
constexpr color dim_brown(0.384_p, 0.096_p, 0.096_p);
constexpr color dim_gray(0.329412_p, 0.329412_p, 0.329412_p);
constexpr color dim_grey(0.329412_p, 0.329412_p, 0.329412_p);
constexpr color firebrick(0.556863_p, 0.137255_p, 0.137255_p);
// constexpr color forest_green(0.137255_p, 0.556863_p, 0.137255_p);
constexpr color gold(0.8_p, 0.498039_p, 0.196078_p);
constexpr color goldenrod(0.858824_p, 0.858824_p, 0.439216_p);
constexpr color gray(0.752941_p, 0.752941_p, 0.752941_p);
constexpr color green_yellow(0.576471_p, 0.858824_p, 0.439216_p);

constexpr color indian_red(0.309804_p, 0.184314_p, 0.184314_p);
constexpr color khaki(0.623529_p, 0.623529_p, 0.372549_p);
constexpr color light_blue(0.74902_p, 0.847059_p, 0.847059_p);
constexpr color light_gray(0.658824_p, 0.658824_p, 0.658824_p);
constexpr color light_grey(0.658824_p, 0.658824_p, 0.658824_p);
constexpr color light_sky_blue(135.0_p / 255, 206.0_p / 255, 235.0_p / 255);
constexpr color light_steel_blue(0.560784_p, 0.560784_p, 0.737255_p);
constexpr color lime_green(0.196078_p, 0.8_p, 0.196078_p);

constexpr color maroon(0.556863_p, 0.137255_p, 0.419608_p);
constexpr color medium_aquamarine(0.196078_p, 0.8_p, 0.6_p);
constexpr color medium_blue(0.196078_p, 0.196078_p, 0.8_p);
constexpr color medium_brown(0.455_p, 0.112_p, 0.115_p);
constexpr color medium_forest_green(0.419608_p, 0.556863_p, 0.137255_p);
constexpr color medium_goldenrod(0.917647_p, 0.917647_p, 0.678431_p);
constexpr color medium_gray(0.5_p, 0.5_p, 0.5_p);
constexpr color medium_grey(0.5_p, 0.5_p, 0.5_p);
constexpr color medium_orchid(0.576471_p, 0.439216_p, 0.858824_p);
constexpr color medium_plum(0.55_p, 0.40_p, 0.55_p);
constexpr color medium_sea_green(0.258824_p, 0.435294_p, 0.258824_p);
constexpr color medium_slate_blue(0.498039_p, 0, 1);
constexpr color medium_spring_green(0.498039_p, 1, 0);
constexpr color medium_turquoise(0.439216_p, 0.858824_p, 0.858824_p);
constexpr color medium_violet_red(0.858824_p, 0.439216_p, 0.576471_p);
constexpr color midnight_blue(0.184314_p, 0.184314_p, 0.309804_p);
constexpr color navy(0.137255_p, 0.137255_p, 0.556863_p);
constexpr color navy_blue(0.137255_p, 0.137255_p, 0.556863_p);
// constexpr color orange(0.8_p, 0.196078_p, 0.196078_p);
constexpr color orange_red(1, 0, 0.498039_p);
constexpr color orchid(0.858824_p, 0.439216_p, 0.858824_p);
constexpr color pale_green(0.560784_p, 0.737255_p, 0.560784_p);
constexpr color pink(0.737255_p, 0.560784_p, 0.560784_p);
// constexpr color plum (0.917647_p, 0.678431_p, 0.917647_p);

constexpr color salmon(0.435294_p, 0.258824_p, 0.258824_p);
constexpr color sea_green(0.137255_p, 0.556863_p, 0.419608_p);
constexpr color sienna(0.556863_p, 0.419608_p, 0.137255_p);
// constexpr color silver(0.6_p, 0.6_p, 0.7_p);
constexpr color sky_blue(0.196078_p, 0.6_p, 0.8_p);
constexpr color slate_blue(0, 0.498039_p, 1);
constexpr color spring_green(0, 1, 0.498039_p);
constexpr color steel_blue(0.137255_p, 0.419608_p, 0.556863_p);
//      this was just "tan" but now tan is a keyword
constexpr color medium_tan(0.858824_p, 0.576471_p, 0.439216_p);
constexpr color thistle(0.847059_p, 0.74902_p, 0.847059_p);
constexpr color turquoise(0.678431_p, 0.917647_p, 0.917647_p);
constexpr color violet(0.309804_p, 0.184314_p, 0.309804_p);
constexpr color violet_red(0.8_p, 0.196078_p, 0.6_p);
constexpr color wheat(0.847059_p, 0.847059_p, 0.74902_p);

constexpr color yellow_green(0.6_p, 0.8_p, 0.196078_p);
constexpr color lamp_blue(0.046875_p, 0.0625_p, 0.69921875_p);
constexpr color coffee_brown(0.15234375_p, 0.2578125_p, 0.11328125_p);
constexpr color beige_peach(0.80078_p, 0.80078_p, 0.33203_p);

// metallic colors
constexpr color aluminum(0.91_p, 0.91_p, 0.91_p);
constexpr color brass(10.0_p, 0.85_p, 0.37_p);
constexpr color bronze(0.47_p, 0.34_p, 0.18_p);
constexpr color chrome(0.8_p, 0.93_p, 1.0_p);
constexpr color copper(0.7784315_p, 0.48681375_p, 0.1336275_p);
constexpr color stainless(0.97_p, 1.0_p, 1.0_p);
constexpr color steel(0.62_p, 0.62_p, 0.51_p);
constexpr color tin(0.72_p, 0.71_p, 0.61_p);
}  // namespace colors

/// Assumes a 50% blend (uses gamma correct)
color blend(color const& x, color const& y);

/// Allows for user blend ratio (uses gamma correct)
color interpolate(color const& x, color const& y, precision a);

namespace operators {
/// Blend the colors together (uses gamma correct)
inline color operator+(color const& x, color const& y) {
    return blend(x, y);
}

/// Scale all the channels together
inline color operator*(color const& x, precision a) {
    color y = x;
    y.scale(a);
    return y;
}

/// Scale all the channels together
inline color operator*(precision a, color const& x) {
    return operator*(x, a);
}

/// Pairwise Color Mixing (when a light and a surface color self select the output color)
color operator*(color const& a, color const& b);
}  // namespace operators

inline std::ostream& operator<<(std::ostream& os, color const& c) {
    os << "color(" << c.red() << ", " << c.green() << ", " << c.blue() << ")";
    return os;
}

/// Compares two colors within the epsilon
bool operator==(color const& a, color const& b);

///
/// Converts a specific wavelength of light into an LMS color.
/// @see https://en.wikipedia.org/wiki/LMS_color_space
/// @param lambda The wavelength of the photon. Must be between 380nm and 780nm.
/// @note This uses a simplified gaussian response for each LMS peak.
///
color wavelength_to_color(iso::meters lambda) noexcept(false);

}  // namespace raytrace
