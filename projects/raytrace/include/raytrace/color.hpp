#pragma once

#include <cmath>
#include <cstdint>
#include <fourcc/fourcc.hpp>
#include "raytrace/configuration.hpp"
#include "raytrace/types.hpp"
#if defined(USE_XMMT)
#include <xmmt/xmmt.hpp>
#endif

namespace raytrace {

using color = fourcc::color;
using fourcc::operator==;
using fourcc::operator<<;

namespace operators {
using fourcc::operators::operator*;
using fourcc::operators::operator+;
}  // namespace operators

inline color wavelength_to_color(iso::meters lambda) noexcept(false) {
    return fourcc::wavelength_to_color(lambda);
}

#if defined(USE_XMMT)
#if defined(USE_PRECISION_AS_FLOAT)
using precision4 = xmmt::float4;
#else
using precision4 = xmmt::double4;
#endif
#endif

/// All color constants are in LINEAR space (the renderer's working space).
/// The original sRGB (gamma-encoded) display values are preserved in the
/// comments. Values which are not /255 color-codes (e.g. the metallic
/// entries) were authored directly in linear space and are left unchanged.
namespace colors {
constexpr color white(1.0_p, 1.0_p, 1.0_p);  // sRGB: white is unchanged
constexpr color grey(0.5_p, 0.5_p, 0.5_p);   // sRGB: 0.73536062 (apply_correction of 0.5)
constexpr color black(0.0_p, 0.0_p, 0.0_p);  // sRGB: black is unchanged
// primaries
constexpr color red(1.0_p, 0.0_p, 0.0_p);    // sRGB: unchanged
constexpr color green(0.0_p, 1.0_p, 0.0_p);  // sRGB: unchanged
constexpr color blue(0.0_p, 0.0_p, 1.0_p);   // sRGB: unchanged
// secondaries
constexpr color yellow(1.0_p, 1.0_p, 0.0_p);   // sRGB: unchanged
constexpr color magenta(1.0_p, 0.0_p, 1.0_p);  // sRGB: unchanged
constexpr color cyan(0.0_p, 1.0_p, 1.0_p);     // sRGB: unchanged
// nice others
constexpr color orange(1.0_p, 0.673859541_p, 0.0_p);                // sRGB: (1.0, 0.84, 0.0)
constexpr color brown(0.304987314_p, 0.070360096_p, 0.0_p);         // sRGB: (0.588235294, 0.294117647, 0)
constexpr color plum(0.539479489_p, 0.291770650_p, 0.266355604_p);  // sRGB: (0.760784314, 0.576470588, 0.552941176)
constexpr color forest_green(0.015996293_p, 0.258182853_p,
                             0.015996293_p);                          // sRGB: (0.133333333, 0.545098039, 0.133333333)
constexpr color silver(0.527115125_p, 0.527115125_p, 0.527115125_p);  // sRGB: (0.752941176, 0.752941176, 0.752941176)

constexpr color aquamarine(0.162029622_p, 0.708376655_p, 0.291771106_p);     // sRGB: (0.439216, 0.858824, 0.576471)
constexpr color blue_violet(0.346703551_p, 0.114435361_p, 0.346703551_p);    // sRGB: (0.623529, 0.372549, 0.623529)
constexpr color brilliant_red(0.420033289_p, 0.136399536_p, 0.136399536_p);  // sRGB: (0.68, 0.405, 0.405)
// constexpr color brown(0.647059_p, 0.164706_p, 0.164706_p);
constexpr color cadet_blue(0.114435361_p, 0.346703551_p, 0.346703551_p);  // sRGB: (0.372549, 0.623529, 0.623529)
constexpr color coral(1.0_p, 0.212230559_p, 0.0_p);                       // sRGB: (1, 0.498039, 0)
constexpr color cornflower_blue(0.054480473_p, 0.054480473_p,
                                0.158960744_p);                           // sRGB: (0.258824, 0.258824, 0.435294)
constexpr color dark_brown(0.031296159_p, 0.003864351_p, 0.003864351_p);  // sRGB: (0.1941, 0.0492, 0.0492)
constexpr color dark_gray(0.088655717_p, 0.088655717_p, 0.088655717_p);   // sRGB: (0.329412, 0.329412, 0.329412)
constexpr color dark_grey(0.088655717_p, 0.088655717_p, 0.088655717_p);   // sRGB: (0.329412, 0.329412, 0.329412)
constexpr color dark_green(0.028426118_p, 0.078187462_p, 0.028426118_p);  // sRGB: (0.184314, 0.309804, 0.184314)
constexpr color dark_olive_green(0.078187462_p, 0.078187462_p,
                                 0.028426118_p);                           // sRGB: (0.309804, 0.309804, 0.184314)
constexpr color dark_orchid(0.318546778_p, 0.031895902_p, 0.603827339_p);  // sRGB: (0.6, 0.196078, 0.8)
constexpr color dark_plum(0.170644936_p, 0.094629627_p, 0.170644936_p);    // sRGB: (0.45, 0.34, 0.45)
constexpr color dark_slate_blue(0.147027383_p, 0.016807396_p,
                                0.270498061_p);  // sRGB: (0.419608, 0.137255, 0.556863)
constexpr color dark_slate_gray(0.028426118_p, 0.078187462_p,
                                0.078187462_p);  // sRGB: (0.184314, 0.309804, 0.309804)
constexpr color dark_slate_grey(0.028426118_p, 0.078187462_p,
                                0.078187462_p);                         // sRGB: (0.184314, 0.309804, 0.309804)
constexpr color dark_tan(0.223413994_p, 0.100481510_p, 0.056661979_p);  // sRGB: (0.51, 0.35, 0.264)
constexpr color dark_turquoise(0.162029622_p, 0.291771106_p,
                               0.708376655_p);                           // sRGB: (0.439216, 0.576471, 0.858824)
constexpr color dim_brown(0.121929543_p, 0.009413232_p, 0.009413232_p);  // sRGB: (0.384, 0.096, 0.096)
constexpr color dim_gray(0.088655717_p, 0.088655717_p, 0.088655717_p);   // sRGB: (0.329412, 0.329412, 0.329412)
constexpr color dim_grey(0.088655717_p, 0.088655717_p, 0.088655717_p);   // sRGB: (0.329412, 0.329412, 0.329412)
constexpr color firebrick(0.270498061_p, 0.016807396_p, 0.016807396_p);  // sRGB: (0.556863, 0.137255, 0.137255)
// constexpr color forest_green(0.137255_p, 0.556863_p, 0.137255_p);
constexpr color gold(0.603827339_p, 0.212230559_p, 0.031895902_p);       // sRGB: (0.8, 0.498039, 0.196078)
constexpr color goldenrod(0.708376655_p, 0.708376655_p, 0.162029622_p);  // sRGB: (0.858824, 0.858824, 0.439216)
constexpr color gray(0.527114849_p, 0.527114849_p, 0.527114849_p);       // sRGB: (0.752941, 0.752941, 0.752941)
constexpr color green_yellow(0.291771106_p, 0.708376655_p,
                             0.162029622_p);  // sRGB: (0.576471, 0.858824, 0.439216)

constexpr color indian_red(0.078187462_p, 0.028426118_p, 0.028426118_p);  // sRGB: (0.309804, 0.184314, 0.184314)
constexpr color khaki(0.346703551_p, 0.346703551_p, 0.114435361_p);       // sRGB: (0.623529, 0.623529, 0.372549)
constexpr color light_blue(0.520996183_p, 0.686685635_p, 0.686685635_p);  // sRGB: (0.74902, 0.847059, 0.847059)
constexpr color light_gray(0.391573097_p, 0.391573097_p, 0.391573097_p);  // sRGB: (0.658824, 0.658824, 0.658824)
constexpr color light_grey(0.391573097_p, 0.391573097_p, 0.391573097_p);  // sRGB: (0.658824, 0.658824, 0.658824)
constexpr color light_sky_blue(0.242281122_p, 0.617206562_p,
                               0.830769877_p);  // sRGB: (135.0/255, 206.0/255, 235.0/255)
constexpr color light_steel_blue(0.274676976_p, 0.274676976_p,
                                 0.502886607_p);                          // sRGB: (0.560784, 0.560784, 0.737255)
constexpr color lime_green(0.031895902_p, 0.603827339_p, 0.031895902_p);  // sRGB: (0.196078, 0.8, 0.196078)

constexpr color maroon(0.270498061_p, 0.016807396_p, 0.147027383_p);  // sRGB: (0.556863, 0.137255, 0.419608)
constexpr color medium_aquamarine(0.031895902_p, 0.603827339_p,
                                  0.318546778_p);                           // sRGB: (0.196078, 0.8, 0.6)
constexpr color medium_blue(0.031895902_p, 0.031895902_p, 0.603827339_p);   // sRGB: (0.196078, 0.196078, 0.8)
constexpr color medium_brown(0.174728005_p, 0.011987086_p, 0.012510409_p);  // sRGB: (0.455, 0.112, 0.115)
constexpr color medium_forest_green(0.147027383_p, 0.270498061_p,
                                    0.016807396_p);  // sRGB: (0.419608, 0.556863, 0.137255)
constexpr color medium_goldenrod(0.822785635_p, 0.822785635_p,
                                 0.417884561_p);                           // sRGB: (0.917647, 0.917647, 0.678431)
constexpr color medium_gray(0.214041140_p, 0.214041140_p, 0.214041140_p);  // sRGB: (0.5, 0.5, 0.5)
constexpr color medium_grey(0.214041140_p, 0.214041140_p, 0.214041140_p);  // sRGB: (0.5, 0.5, 0.5)
constexpr color medium_orchid(0.291771106_p, 0.162029622_p,
                              0.708376655_p);                              // sRGB: (0.576471, 0.439216, 0.858824)
constexpr color medium_plum(0.263273408_p, 0.132868322_p, 0.263273408_p);  // sRGB: (0.55, 0.40, 0.55)
constexpr color medium_sea_green(0.054480473_p, 0.158960744_p,
                                 0.054480473_p);                   // sRGB: (0.258824, 0.435294, 0.258824)
constexpr color medium_slate_blue(0.212230559_p, 0.0_p, 1.0_p);    // sRGB: (0.498039, 0, 1)
constexpr color medium_spring_green(0.212230559_p, 1.0_p, 0.0_p);  // sRGB: (0.498039, 1, 0)
constexpr color medium_turquoise(0.162029622_p, 0.708376655_p,
                                 0.708376655_p);  // sRGB: (0.439216, 0.858824, 0.858824)
constexpr color medium_violet_red(0.708376655_p, 0.162029622_p,
                                  0.291771106_p);  // sRGB: (0.858824, 0.439216, 0.576471)
constexpr color midnight_blue(0.028426118_p, 0.028426118_p,
                              0.078187462_p);                            // sRGB: (0.184314, 0.184314, 0.309804)
constexpr color navy(0.016807396_p, 0.016807396_p, 0.016807396_p);       // sRGB: (0.137255, 0.137255, 0.137255)
constexpr color navy_blue(0.016807396_p, 0.016807396_p, 0.016807396_p);  // sRGB: (0.137255, 0.137255, 0.137255)
// constexpr color orange(0.8_p, 0.196078_p, 0.196078_p);
constexpr color orange_red(1.0_p, 0.0_p, 0.212230559_p);                  // sRGB: (1, 0, 0.498039)
constexpr color orchid(0.708376655_p, 0.162029622_p, 0.708376655_p);      // sRGB: (0.858824, 0.439216, 0.858824)
constexpr color pale_green(0.274676976_p, 0.502886607_p, 0.274676976_p);  // sRGB: (0.560784, 0.737255, 0.560784)
constexpr color pink(0.502886607_p, 0.274676976_p, 0.274676976_p);        // sRGB: (0.737255, 0.560784, 0.560784)
// constexpr color plum (0.917647_p, 0.678431_p, 0.917647_p);

constexpr color salmon(0.158960744_p, 0.054480473_p, 0.054480473_p);     // sRGB: (0.435294, 0.258824, 0.258824)
constexpr color sea_green(0.016807396_p, 0.270498061_p, 0.147027383_p);  // sRGB: (0.137255, 0.556863, 0.419608)
constexpr color sienna(0.270498061_p, 0.147027383_p, 0.016807396_p);     // sRGB: (0.556863, 0.419608, 0.137255)
// constexpr color silver(0.6_p, 0.6_p, 0.7_p);
constexpr color sky_blue(0.031895902_p, 0.318546778_p, 0.603827339_p);    // sRGB: (0.196078, 0.6, 0.8)
constexpr color slate_blue(0.0_p, 0.212230559_p, 1.0_p);                  // sRGB: (0, 0.498039, 1)
constexpr color spring_green(0.0_p, 1.0_p, 0.212230559_p);                // sRGB: (0, 1, 0.498039)
constexpr color steel_blue(0.016807396_p, 0.147027383_p, 0.270498061_p);  // sRGB: (0.137255, 0.419608, 0.556863)
//      this was just "tan" but now tan is a keyword
constexpr color medium_tan(0.708376655_p, 0.291771106_p, 0.162029622_p);  // sRGB: (0.858824, 0.576471, 0.439216)
constexpr color thistle(0.686685635_p, 0.520996183_p, 0.686685635_p);     // sRGB: (0.847059, 0.74902, 0.847059)
constexpr color turquoise(0.417884561_p, 0.822785635_p, 0.822785635_p);   // sRGB: (0.678431, 0.917647, 0.917647)
constexpr color violet(0.078187462_p, 0.028426118_p, 0.078187462_p);      // sRGB: (0.309804, 0.184314, 0.309804)
constexpr color violet_red(0.603827339_p, 0.031895902_p, 0.318546778_p);  // sRGB: (0.8, 0.196078, 0.6)
constexpr color wheat(0.686685635_p, 0.686685635_p, 0.520996183_p);       // sRGB: (0.847059, 0.847059, 0.74902)

constexpr color yellow_green(0.318546778_p, 0.603827339_p, 0.031895902_p);  // sRGB: (0.6, 0.8, 0.196078)
constexpr color lamp_blue(0.003660635_p, 0.005155668_p, 0.446876664_p);     // sRGB: (0.046875, 0.0625, 0.69921875)
constexpr color coffee_brown(0.020148948_p, 0.054059987_p, 0.012208993_p);  // sRGB: (0.15234375, 0.2578125, 0.11328125)
constexpr color beige_peach(0.605150247_p, 0.605150247_p, 0.090111705_p);   // sRGB: (0.80078, 0.80078, 0.33203)

// metallic colors: authored directly in linear space, NOT sRGB /255 codes. Unchanged.
constexpr color aluminum(0.91_p, 0.91_p, 0.91_p);
constexpr color brass(10.0_p, 0.85_p, 0.37_p);
constexpr color bronze(0.47_p, 0.34_p, 0.18_p);
constexpr color chrome(0.8_p, 0.93_p, 1.0_p);
constexpr color copper(0.7784315_p, 0.48681375_p, 0.1336275_p);
constexpr color stainless(0.97_p, 1.0_p, 1.0_p);
constexpr color steel(0.62_p, 0.62_p, 0.51_p);
constexpr color tin(0.72_p, 0.71_p, 0.61_p);
}  // namespace colors

/// Given a color map it to another color using some predefined mapping function.
/// This is used for tone mapping and false color mapping.
class ToneMapper {
public:
    /// The mapping function to use for the tone mapping. This is a function that takes in a color and outputs a color.
    /// @param color The color to map to another color.
    virtual color operator()(color const&) = 0;

protected:
    ~ToneMapper() = default;
};

/// A simple tone mapper that maps colors using the Reinhard Tone Mapping algorithm.
class ReinhardToneMapper : public ToneMapper {
public:
    /// Maps a color to another color using the Reinhard Tone Mapping algorithm.
    /// @param color The color to map to another color.
    inline color operator()(color const& c) override {
        // this is a simple implementation of the Reinhard Tone Mapping algorithm. It maps the color to a new color
        // using the formula:
        color d = c;
        d += colors::white;  // add white to the color to brighten it up (linear)
        // c' = c / (c + 1)
        return color(c.red() / d.red(), c.green() / d.green(), c.blue() / d.blue());
    }
};

}  // namespace raytrace