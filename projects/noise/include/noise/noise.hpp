#pragma once

#include <random>

#include "noise/types.hpp"

/// A group of functions used to create interesting noise models used in various areas
/// @see https://en.wikipedia.org/wiki/Perlin_noise
/// @see https://weber.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf
/// @see https://mrl.cs.nyu.edu/~perlin/paper445.pdf
/// @see https://github.com/sol-prog/Perlin_Noise/blob/master/PerlinNoise.cpp (meh reference)
/// @see https://web.archive.org/web/20160530124230/http://freespace.virgin.net/hugo.elias/models/m_perlin.htm (Good
/// Reference)
///
namespace noise {

/// Generates a pseudo random sequence
precision sequence_pseudorandom1(uint32_t x);

/// Generates a random seed vector which is normalized
vector generate_seed();

/// Converts a radian value to a seed vector normalized
vector convert_to_seed(iso::radians r);

/// Converts a degree value to a seed vector normalized
vector convert_to_seed(iso::degrees d);

/// Converts a turn value to a seed vector normalized
vector convert_to_seed(iso::turns t);

/// A two way interpolation using z = x*(1-a) + y*a
constexpr precision interpolate(precision x, precision y, precision a) {
    return (x * (1.0_p - a) + (y * a));
}

/// Maps an input value range to an output value range
constexpr precision map(precision value, precision in_low, precision in_hi, precision out_low, precision out_hi) {
    value = std::max(in_low, std::min(value, in_hi));
    precision in_range = in_hi - in_low;
    precision out_range = out_hi - out_low;
    precision norm = (value - in_low) / in_range;
    return (norm * out_range) + out_low;
}

/// Returns the integer portion of a pair of numbers.
point floor(noise::point const& pnt);

/// Returns the fractional portion of a pair of numbers.
point fract(noise::point const& pnt);

/// Computes a seemingly random, repeatable number from a 2d point and 2d scalars
/// @return A value between 0.0 and 1.0 inclusive
precision random(vector const& pnt, vector const& scalars, precision gain);

/// @brief Computes the perlin cell corner vectors. These are located on the border of a cells and should be completely
/// reproducible.
/// @param image_point The point in the image.
/// @param scale The scale of the cell (how many pixels are along the side of the cell) 32 would mean a 32x32 unit cell.
/// @param scalars The seed values to help shape the noise
/// @param gain The magnification of the seed noise
/// @param[out] uv The output vector which is the fractional vector from the top left corner of the cell.
/// @param[out] turns The output array of vectors which are the corner flow vectors of the cell. The order of corners if
/// top-left, top-right, bottom-left, bottom-right.
void cell_flows(point const& image_point, precision scale, vector const& seed, precision gain, point& uv,
                vector (&flows)[4]);

/// Improved Perlin Fade, which is defined as 6t^5 - 15t^4 + 10t^3
constexpr precision fade(precision t) {
    precision t3 = t * t * t;
    precision t4 = t3 * t;
    precision t5 = t4 * t;
    return 6.0_p * t5 - 15.0_p * t4 + 10.0_p * t3;
}

namespace {  // in an anonymous namespace for testing
// enforce the correctness at compile time!
static_assert(fade(0.5_p) == 0.5_p, "Must be equal");
static_assert(fade(1.0_p) == 1.0_p, "Must be equal");
static_assert(fade(2.0_p) == 32.0_p, "Must be equal");
}  // namespace

/// @todo The set of 3d perlin noise gradients

///
/// Uses the improved perlin noise generation method.
/// @param pnt A point in 2d space. Does not have to have a fractional component.
/// @param scale Scales the point into a different range which can help produce
/// fractional parts of points. The side-effect of a single scale is "square"
/// regular noise patterns.
/// @param seeds The predetermined see values to help shape the noise
/// @param gain The gain to apply to the random value
/// @see https://mrl.nyu.edu/~perlin/noise/
/// @see https://mrl.nyu.edu/~perlin/paper445.pdf
/// @see https://flafla2.github.io/2014/08/09/perlinnoise.html
/// @see https://mzucker.github.io/html/perlin-noise-math-faq.html
///
precision perlin(point const& pnt, precision scale, vector const& seeds, precision gain);

/// Builds an internal 2D map of random values between 0.0_p and 1.0_p.
template <typename T, size_t DIM>
class pad_ {
public:
    constexpr static size_t dimensions = DIM;
    pad_() : values{} {
        generate();
    }

    void generate(void) {
        std::default_random_engine generator;
        // create values between [0.0_p,1.0_p)
        std::uniform_real_distribution<precision> distribution(0.0_p, 1.0_p);

        for (size_t y = 0; y < dimensions; y++) {
            for (size_t x = 0; x < dimensions; x++) {
                values[y][x] = distribution(generator);
            }
        }
    }

    T at(size_t y, size_t x) const {
        return values[y % dimensions][x % dimensions];
    }

    T at(precision y, precision x) const {
        size_t _y = static_cast<size_t>(std::floor(y));
        size_t _x = static_cast<size_t>(std::floor(x));
        return at(_y, _x);
    }

    T at(point const& pnt) const {
        return at(pnt[1], pnt[0]);
    }

protected:
    T values[dimensions][dimensions];
};
using pad = pad_<precision, 128>;

///
/// Returns a smoothed value from the Noise map given the point
/// @param pnt The input point.
/// @param map The input noise map
/// @return precision
///
precision smooth(point const& pnt, pad const& m);

///
/// Generates some turbulence from the point.
/// @param pnt
/// @param size
/// @return precision
///
precision turbulence(point const& pnt, precision size, precision scale, pad const& m);

///
/// Turbulent Sine Noise
/// @param pnt The point in the image.
/// @param xs X pattern scale
/// @param ys Y pattern scale
/// @param power The amount of turbulent
/// @param size The initial size of the turbulence.
/// @param scale The range of the output values
/// @param map The noise pad to pull values from
/// @see https://lodev.org/cgtutor/randomnoise.html
///
precision turbulentsin(point const& pnt, precision xs, precision ys, precision power, precision size, precision scale,
                       pad const& map);

namespace gains {
constexpr precision pink = 1.0_p;
constexpr precision brown = 1.41421356237309504880168872420969807856967187537694807317667973799_p;  // sqrt(2.0_p);
constexpr precision yellow = 0.5_p;
}  // namespace gains

///
/// Fractal Brownian Motion
/// @see https://thebookofshaders.com/13/
/// @see https://www.iquilezles.org/www/articles/fbm/fbm.htm
/// @see https://www.iquilezles.org/www/articles/warp/warp.htm
/// @param pnt The point in the space (does not need to be pre-fract)
/// @param seed The noise seed value
/// @param octaves The number of iterative octaves
/// @param lacunarity How the frequency is modified after every octave. Usually 2.0_p
/// @param gain How the amplitude if modified after every octave. Usually 0.5_p or some value x where 2^-y
/// @param initial_amplitude The initial amplitude
/// @param initial_frequency The initial starting frequency
///
precision fractal_brownian(point const& pnt, vector const& seed, size_t octaves, precision lacunarity = 2.0_p,
                           precision gain = 0.5_p, precision initial_amplitude = 1.0_p,
                           precision initial_frequency = 1.0_p);

}  // namespace noise
