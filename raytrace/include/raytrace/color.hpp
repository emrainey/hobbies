#pragma once

#include <cmath>
#include <cstdint>
#include <fourcc/image.hpp>

#include "raytrace/types.hpp"

#if defined(__x86_64__) and defined(__AVX__)
#include <immintrin.h>
#define USE_SIMD_INSTRUCTIONS
#endif

namespace raytrace {

/** The Gamma Correction Namespace */
namespace gamma {

constexpr static bool use_full = true;

/**
 * Removes Gamma Correction
 * @see https://ixora.io/projects/colorblindness/color-blindness-simulation-research/
 * @param value The value to correct (must be nominal (0-1))
 * @return The corrected value
 */
constexpr element_type remove_correction(element_type value) {
    if constexpr (use_full) {
        if (value <= 0.04045) {
            return value / 12.92;
        } else {
            return std::pow((value + 0.055) / 1.055, 2.4);
        }
    } else {
        return std::pow(value, 2.2);
    }
}

/**
 * Applies Gamma Correction
 * @see https://ixora.io/projects/colorblindness/color-blindness-simulation-research/
 * @param value The value to correct (must be nominal (0-1))
 * @return The corrected value
 */
constexpr element_type apply_correction(element_type value) {
    if constexpr (use_full) {
        if (value <= 0.0031308) {
            return 12.92 * value;
        } else {
            return (1.055 * std::pow(value, 0.41666)) - 0.055;
        }
    } else {
        return std::pow(value, 1.0 / 2.2);
    }
}

}  // namespace gamma

/**
 * Adds two channels together after gamma correction by the scalar given.
 * @note the scalar must be 0.0 <= s <= 1.0.
 * @retval element_type will be clamped from 0.0 to 1.0
 */
element_type gamma_interpolate(const element_type a, const element_type b, const element_type s);

/**
 * @brief sRGB linear color space value
 */
class color {
public:
    constexpr static size_t NUM_CHANNELS = 4;

#if defined(USE_SIMD_INSTRUCTIONS)
    union {
        alignas(16) double channels[NUM_CHANNELS];
        __m256d simd_channels;
    };
#else
    double channels[NUM_CHANNELS];
#endif

    /** Indicates hwo the values are stored */
    enum class space : int
    {
        linear,
        logarithmic,
    } representation;

    constexpr explicit color() : color(0.0, 0.0, 0.0) {
    }

    constexpr explicit color(element_type _r, element_type _g, element_type _b, element_type _i = 1.0)
        : channels(), representation(space::linear) {
        channels[0] = _r;
        channels[1] = _g;
        channels[2] = _b;
        channels[3] = _i;
    }

    // Copy Constructor
    color(const color& other);
    // Move constructor
    color(color&& other);
    // Copy Assign
    color& operator=(const color&);
    // Move Assign
    color& operator=(color&&);  // this is used in the get_color() assignment
    // Destructor
    ~color() = default;

    constexpr const element_type& red() const {
        return channels[0];
    }
    constexpr const element_type& green() const {
        return channels[1];
    }
    constexpr const element_type& blue() const {
        return channels[2];
    }
    constexpr const element_type& intensity() const {
        return channels[3];
    }

    /** Converts the color from one space to another. */
    void to_space(space desired);

    /** Scaling operator */
    inline void scale(const element_type a) {
#if defined(USE_SIMD_INSTRUCTIONS)
        __m256d imm = _mm256_set1_pd(a);
        simd_channels = _mm256_mul_pd(simd_channels, imm);
        imm = _mm256_set1_pd(1.0);
        simd_channels = _mm256_min_pd(imm, simd_channels);
        imm = _mm256_setzero_pd();
        simd_channels = _mm256_max_pd(imm, simd_channels);
#else
        for (auto& c : channels) {
            c = std::clamp(a * c, 0.0, 1.0);
        }
#endif
    }
    /** Scale Wrapper */
    inline color& operator*=(const element_type a) {
        scale(a);
        return (*this);
    }

    fourcc::rgba to_rgba() const;
    fourcc::abgr to_abgr() const;
    fourcc::rgb8 to_rgb8() const;
    fourcc::bgr8 to_bgr8() const;

    /** Allows a per channel operation */
    void per_channel(std::function<element_type(element_type c)> iter);

    /** Accumulates colors together (does not gamma correct) */
    color& operator+=(const color& other);

    /** The comparison precision for equal colors (to a millionth) */
    constexpr static element_type precision = 1.0E-6;

    /**
     * Blends all colors together in a equal weighted average with gamma
     * correction.
     * @param subsamples The vector of color samples
     */
    static color blend_samples(const std::vector<color>& subsamples);

    /** Accumulates all the samples together into a summed color in the linear space*/
    static color accumulate_samples(const std::vector<color>& samples);

    /**
     * Map numbers near the range -1.0 to 1.0 to a color in the spectrum.
     * Bluer for negatives, Reds for Positives.
     */
    static color jet(element_type d);

    /**
     * Takes a number between min and max and returns an RGB from 0.0 to 1.0.
     * If the input is out of range, returns magenta.
     */
    static color greyscale(element_type d, element_type min, element_type max);

    /**
     * Generates a random color.
     */
    static color random();

    // Don't add interfaces as color needs to be constexpr for colors below
};

namespace colors {
constexpr color white(1.0, 1.0, 1.0);
constexpr color grey(0.73536062, 0.73536062, 0.73536062);
constexpr color black(0.0, 0.0, 0.0);
// primaries
constexpr color red(1.0, 0.0, 0.0);
constexpr color green(0.0, 1.0, 0.0);
constexpr color blue(0.0, 0.0, 1.0);
// secondaries
constexpr color yellow(1.0, 1.0, 0.0);
constexpr color magenta(1.0, 0.0, 1.0);
constexpr color cyan(0.0, 1.0, 1.0);
// nice others
constexpr color orange(1.0, 0.84, 0);
constexpr color brown(0.588235294, 0.294117647, 0);
constexpr color plum(0.760784314, 0.576470588, 0.552941176);
constexpr color forest_green(0.133333333, 0.545098039, 0.133333333);
constexpr color silver(0.752941176, 0.752941176, 0.752941176);

constexpr color aquamarine(.439216, .858824, .576471);
constexpr color blue_violet(.623529, .372549, .623529);
constexpr color brilliant_red(.68, .405, .405);
// constexpr color brown(.647059, .164706, .164706);
constexpr color cadet_blue(.372549, .623529, .623529);
constexpr color coral(1, .498039, 0);
constexpr color cornflower_blue(.258824, .258824, .435294);
constexpr color dark_brown(.1941, .0492, .0492);
constexpr color dark_gray(.329412, .329412, .329412);
constexpr color dark_grey(.329412, .329412, .329412);
constexpr color dark_green(.184314, .309804, .184314);
constexpr color dark_olive_green(.309804, .309804, .184314);
constexpr color dark_orchid(.6, .196078, .8);
constexpr color dark_plum(.45, .34, .45);
constexpr color dark_slate_blue(.419608, .137255, .556863);
constexpr color dark_slate_gray(.184314, .309804, .309804);
constexpr color dark_slate_grey(.184314, .309804, .309804);
constexpr color dark_tan(.51, .35, .264);
constexpr color dark_turquoise(.439216, .576471, .858824);
constexpr color dim_brown(.384, .096, .096);
constexpr color dim_gray(.329412, .329412, .329412);
constexpr color dim_grey(.329412, .329412, .329412);
constexpr color firebrick(.556863, .137255, .137255);
// constexpr color forest_green(.137255, .556863, .137255);
constexpr color gold(.8, .498039, .196078);
constexpr color goldenrod(.858824, .858824, .439216);
constexpr color gray(.752941, .752941, .752941);
constexpr color green_yellow(.576471, .858824, .439216);

constexpr color indian_red(.309804, .184314, .184314);
constexpr color khaki(.623529, .623529, .372549);
constexpr color light_blue(.74902, .847059, .847059);
constexpr color light_gray(.658824, .658824, .658824);
constexpr color light_grey(.658824, .658824, .658824);
constexpr color light_sky_blue(135.0 / 255, 206.0 / 255, 235.0 / 255);
constexpr color light_steel_blue(.560784, .560784, .737255);
constexpr color lime_green(.196078, .8, .196078);

constexpr color maroon(.556863, .137255, .419608);
constexpr color medium_aquamarine(.196078, .8, .6);
constexpr color medium_blue(.196078, .196078, .8);
constexpr color medium_brown(.455, .112, .115);
constexpr color medium_forest_green(.419608, .556863, .137255);
constexpr color medium_goldenrod(.917647, .917647, .678431);
constexpr color medium_gray(.5, .5, .5);
constexpr color medium_grey(.5, .5, .5);
constexpr color medium_orchid(.576471, .439216, .858824);
constexpr color medium_plum(.55, .40, .55);
constexpr color medium_sea_green(.258824, .435294, .258824);
constexpr color medium_slate_blue(.498039, 0, 1);
constexpr color medium_spring_green(.498039, 1, 0);
constexpr color medium_turquoise(.439216, .858824, .858824);
constexpr color medium_violet_red(.858824, .439216, .576471);
constexpr color midnight_blue(.184314, .184314, .309804);
constexpr color navy(.137255, .137255, .556863);
constexpr color navy_blue(.137255, .137255, .556863);
// constexpr color orange(.8, .196078, .196078);
constexpr color orange_red(1, 0, .498039);
constexpr color orchid(.858824, .439216, .858824);
constexpr color pale_green(.560784, .737255, .560784);
constexpr color pink(.737255, .560784, .560784);
// constexpr color plum (.917647, .678431, .917647);

constexpr color salmon(.435294, .258824, .258824);
constexpr color sea_green(.137255, .556863, .419608);
constexpr color sienna(.556863, .419608, .137255);
// constexpr color silver(.6, .6, .7);
constexpr color sky_blue(.196078, .6, .8);
constexpr color slate_blue(0, .498039, 1);
constexpr color spring_green(0, 1, .498039);
constexpr color steel_blue(.137255, .419608, .556863);
//      this was just "tan" but now tan is a keyword
constexpr color medium_tan(.858824, .576471, .439216);
constexpr color thistle(.847059, .74902, .847059);
constexpr color turquoise(.678431, .917647, .917647);
constexpr color violet(.309804, .184314, .309804);
constexpr color violet_red(.8, .196078, .6);
constexpr color wheat(.847059, .847059, .74902);

constexpr color yellow_green(.6, .8, .196078);
constexpr color lamp_blue(.046875, .0625, .69921875);
constexpr color coffee_brown(.15234375, .2578125, .11328125);
constexpr color beige_peach(.80078, .80078, .33203);

// metallic colors
constexpr color aluminum(0.91, 0.91, 0.91);
constexpr color brass(1.0, 0.85, 0.37);
constexpr color bronze(0.47, 0.34, 0.18);
constexpr color chrome(0.8, 0.93, 1.0);
constexpr color copper(0.7784315, 0.48681375, 0.1336275);
constexpr color stainless(0.97, 1.0, 1.0);
constexpr color steel(0.62, 0.62, 0.51);
constexpr color tin(0.72, 0.71, 0.61);
}  // namespace colors

/** Assumes a 50% blend (uses gamma correct) */
color blend(const color& x, const color& y);

/** Allows for user blend ratio (uses gamma correct) */
color interpolate(const color& x, const color& y, element_type a);

namespace operators {
/** Blend the colors together (uses gamma correct) */
inline color operator+(const color& x, const color& y) {
    return blend(x, y);
}

/** Scale all the channels together */
inline color operator*(const color& x, element_type a) {
    color y = x;
    y.scale(a);
    return y;
}

/** Scale all the channels together */
inline color operator*(element_type a, const color& x) {
    return operator*(x, a);
}

/** Pairwise Color Mixing (when a light and a surface color self select the output color) */
color operator*(const color& a, const color& b);
}  // namespace operators

inline std::ostream& operator<<(std::ostream& os, const color& c) {
    os << "color(" << c.red() << ", " << c.green() << ", " << c.blue() << ")";
    return os;
}

/** Compares two colors within the epsilon */
bool operator==(const color& a, const color& b);

/**
 * Converts a specific wavelength of light into an LMS color.
 * @see https://en.wikipedia.org/wiki/LMS_color_space
 * @param lambda The wavelength of the photon. Must be between 380nm and 780nm.
 * @note This uses a simplified gaussian response for each LMS peak.
 */
color wavelength_to_color(iso::meters lambda) noexcept(false);

}  // namespace raytrace
