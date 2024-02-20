#pragma once

#include <functional>

#include "raytrace/color.hpp"
#include "raytrace/image.hpp"
#include "raytrace/types.hpp"

namespace raytrace {

/// A palette is a set of colors
using palette = std::vector<color>;

namespace functions {

/// A color map takes an image::point (u,v) and fmods it into the range of [0.0_p, 1.0_p) to make a pattern
/// and a set of colors as a palette and produces a color. Each map may take it's own number
/// of colors in the palette.
/// @param p A point in unit R2 u,v space (or repeated)
/// @param pal The palette of colors to chose from.
using texture_color_map = std::function<raytrace::color(image::point const& p, palette const& pal)>;

/// A scalar map takes an image::point (u,v) which in the range of [0.0_p, 1.0_p)
/// and returns a scalar value to describe that point
using texture_scalar_map = std::function<raytrace::color(image::point const&)>;

///
/// A volumetric color map takes an raytrace::point (R3) (x,y,z) and fmods it into the range of [0.0_p, 1.0_p) to make a
/// pattern and a set of colors as a palette and produces a color. Each map may take it's own number of colors in the
/// palette.
/// @param p A point in unit R3 space (or repeated)
/// @param pal The palette of colors to chose from.
///
using volumetric_color_map = std::function<raytrace::color(raytrace::point const& p, palette const& pal)>;

/// A scalar map takes an image::point (u,v) which in the range of [0.0_p, 1.0_p)
/// and returns a scalar value to describe that point
using volumetric_scalar_map = std::function<raytrace::color(raytrace::point const&)>;

/// Just returns the first color in the palette
color simple(image::point const& p, palette const& pal);

/// Produces a diagonal pattern from 2D points and 2 colors
color diagonal(image::point const& p2, palette const& pal);

/// Produces a dot pattern
color dots(image::point const& p, palette const& pal);

/// Produces a volumetric dot pattern
color dots(raytrace::point const& p, palette const& pal);

/// Produces a grid on a surface
color grid(image::point const& p, palette const& pal);

/// Produces a grid in a volume
color grid(raytrace::point const& p, palette const& pal);

/// Normal checkerboards
color checkerboard(image::point const& p, palette const& pal);

/// Volumetric Checkerboard (looks like a wood-turned object etc)
color checkerboard(raytrace::point const& p, palette const& pal);

/// Randomly generates a color for each pixel (palette is unused)
color pseudo_random_noise(image::point const& p, palette const& pal);

}  // namespace functions

namespace tuning {
class prn_parameters {
public:
    precision gain;
    precision radius;
    iso::radians theta_r;
    iso::radians theta_g;
    iso::radians theta_b;
    noise::vector vec_r;
    noise::vector vec_g;
    noise::vector vec_b;
    bool _initialized;

    // Constructs and initializes the values
    prn_parameters();

    // initializes the values
    void initialize(bool again = false);

    // updates the vectors from the angles
    void update();
};

// the structure used with pseudo random noise
extern prn_parameters pseudo_random_noise_params;

}  // namespace tuning

}  // namespace raytrace