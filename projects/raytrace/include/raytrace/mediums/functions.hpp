#pragma once

#include <functional>

#include "raytrace/color.hpp"
#include "raytrace/image.hpp"
#include "raytrace/types.hpp"

namespace raytrace {

/** A palette is a set of colors */
using palette = std::vector<color>;

namespace functions {

/**
 * A color map takes an image::point (u,v) and fmods it into the range of [0.0, 1.0) to make a pattern
 * and a set of colors as a palette and produces a color. Each map may take it's own number
 * of colors in the palette.
 * @param p A point in unit R2 u,v space (or repeated)
 * @param pal The palette of colors to chose from.
 */
using texture_color_map = std::function<raytrace::color(const image::point& p, const palette& pal)>;

/**
 * A scalar map takes an image::point (u,v) which in the range of [0.0, 1.0)
 * and returns a scalar value to describe that point
 */
using texture_scalar_map = std::function<raytrace::color(const image::point&)>;

/**
 * A volumetric color map takes an raytrace::point (R3) (x,y,z) and fmods it into the range of [0.0, 1.0) to make a
 * pattern and a set of colors as a palette and produces a color. Each map may take it's own number of colors in the
 * palette.
 * @param p A point in unit R3 space (or repeated)
 * @param pal The palette of colors to chose from.
 */
using volumetric_color_map = std::function<raytrace::color(const raytrace::point& p, const palette& pal)>;

/**
 * A scalar map takes an image::point (u,v) which in the range of [0.0, 1.0)
 * and returns a scalar value to describe that point
 */
using volumetric_scalar_map = std::function<raytrace::color(const raytrace::point&)>;

/** Just returns the first color in the palette */
color simple(const image::point& p, const palette& pal);

/** Produces a diagonal pattern from 2D points and 2 colors */
color diagonal(const image::point& p2, const palette& pal);

/** Produces a dot pattern */
color dots(const image::point& p, const palette& pal);

/** Produces a volumetric dot pattern */
color dots(const raytrace::point& p, const palette& pal);

/** Produces a grid on a surface */
color grid(const image::point& p, const palette& pal);

/** Produces a grid in a volume */
color grid(const raytrace::point& p, const palette& pal);

/** Normal checkerboards */
color checkerboard(const image::point& p, const palette& pal);

/** Volumetric Checkerboard (looks like a woodturned object etc) */
color checkerboard(const raytrace::point& p, const palette& pal);

/** Randomly generates a color for each pixel (palette is unused) */
color pseudo_random_noise(const image::point& p, const palette& pal);

}  // namespace functions

namespace tuning {
class prn_parameters {
public:
    double gain;
    double radius;
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