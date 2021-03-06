#pragma once

#include <random>

#include "noise/types.hpp"

/** A group of functions used to create interesting noise models used in various areas
 * @see https://en.wikipedia.org/wiki/Perlin_noise
 * @see https://weber.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf
 * @see https://mrl.cs.nyu.edu/~perlin/paper445.pdf
 * @see https://github.com/sol-prog/Perlin_Noise/blob/master/PerlinNoise.cpp (meh reference)
 * @see https://web.archive.org/web/20160530124230/http://freespace.virgin.net/hugo.elias/models/m_perlin.htm (Good
 * Reference)
 */
namespace noise {

/** Generates a pseudo random sequence */
double sequence_pseudorandom1(uint32_t x);

/** Generates a random seed vector which is normalized */
vector generate_seed();

/** Converts a radian value to a seed vector normalized */
vector convert_to_seed(iso::radians r);

/** Converts a degree value to a seed vector normalized */
vector convert_to_seed(iso::degrees d);

/** A two way interpolation using z = x*(1-a) + y*a */
constexpr double interpolate(double x, double y, double a) {
    return (x * (1.0 - a) + (y * a));
}

/** Maps an input value range to an output value range */
constexpr double map(double value, double in_low, double in_hi, double out_low, double out_hi) {
    value = std::max(in_low, std::min(value, in_hi));
    double in_range = in_hi - in_low;
    double out_range = out_hi - out_low;
    double norm = (value - in_low) / in_range;
    return (norm * out_range) + out_low;
}

/** Returns the integer portion of a pair of numbers. */
point floor(const point& pnt);

/** Returns the fractional portion of a pair of numbers. */
point fract(const point& pnt);

/**
 * Computes a seemingly random, repeatable number from a 2d point and 2d scalars.
 */
double random(const vector& pnt, const vector& scalars, double gain);

/** Improved Perlin Fade, which is defined as 6t^5 - 15t^4 + 10t^3 */
constexpr double fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

namespace {  // in an anonymous namespace for testing
// enforce the correctness at compile time!
static_assert(fade(0.5) == 0.5, "Must be equal");
static_assert(fade(1.0) == 1.0, "Must be equal");
static_assert(fade(2.0) == 32.0, "Must be equal");
}  // namespace

/** @todo The set of 3d perlin noise gradients */

/**
 * Uses the improved perlin noise generation method.
 * @param pnt A point in 2d space. Does not have to have a fractional component.
 * @param scale Scales the point into a different range which can help produce
 * fractional parts of points. The side-effect of a single scale is "square"
 * regular noise patterns.
 * @param seeds The predetermined see values to help shape the noise
 * @param gain The gain to apply to the random value
 * @see https://mrl.nyu.edu/~perlin/noise/
 * @see https://mrl.nyu.edu/~perlin/paper445.pdf
 * @see https://flafla2.github.io/2014/08/09/perlinnoise.html
 * @see https://mzucker.github.io/html/perlin-noise-math-faq.html
 */
double perlin(const point& pnt, double scale, const vector& seeds, double gain);

/** Builds an internal 2D map of random values between 0.0 and 1.0. */
template <typename T, size_t DIM>
class pad_ {
public:
    constexpr static size_t dimensions = DIM;
    pad_() : values{} {
        generate();
    }

    void generate(void) {
        std::default_random_engine generator;
        // create values between [0.0,1.0)
        std::uniform_real_distribution<double> distribution(0.0, 1.0);

        for (size_t y = 0; y < dimensions; y++) {
            for (size_t x = 0; x < dimensions; x++) {
                values[y][x] = distribution(generator);
            }
        }
    }

    T at(size_t y, size_t x) const {
        return values[y % dimensions][x % dimensions];
    }

    T at(double y, double x) const {
        size_t _y = static_cast<size_t>(std::floor(y));
        size_t _x = static_cast<size_t>(std::floor(x));
        return at(_y, _x);
    }

    T at(const point& pnt) const {
        return at(pnt.y, pnt.x);
    }

protected:
    T values[dimensions][dimensions];
};
using pad = pad_<double, 128>;

/**
 * Returns a smoothed value from the Noise map given the point
 * @param pnt The input point.
 * @param map The input noise map
 * @return double
 */
double smooth(const point& pnt, const pad& m);

/**
 * Generates some turbulence from the point.
 * @param pnt
 * @param size
 * @return double
 */
double turbulence(const point& pnt, double size, double scale, const pad& m);

/**
 * Turbulent Sine Noise
 * @param pnt The point in the image.
 * @param xs X pattern scale
 * @param ys Y pattern scale
 * @param power The amount of turbulent
 * @param size The initial size of the turbulence.
 * @param scale The range of the output values
 * @param map The noise pad to pull values from
 * @see https://lodev.org/cgtutor/randomnoise.html
 */
double turbulentsin(const point& pnt, double xs, double ys, double power, double size, double scale, const pad& map);

/**
 * Fractal Brownian Motion
 * @see https://thebookofshaders.com/13/
 * @see https://www.iquilezles.org/www/articles/fbm/fbm.htm
 * @see https://www.iquilezles.org/www/articles/warp/warp.htm
 * @param pnt
 * @param seed
 * @param octaves
 * @param lacunarity
 * @param gain
 * @param amplitude
 * @param frequency
 */
double fractal_brownian(const point& pnt, const vector& seed, size_t octaves, double lacunarity, double gain,
                        double amplitude, double frequency);

/** A collection to hold the various non-parametric assumptions of the noise function */
struct traits {
    constexpr traits() {
    }
};

}  // namespace noise
