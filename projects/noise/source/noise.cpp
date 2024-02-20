#include "noise/noise.hpp"

namespace noise {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

constexpr bool debug = false;

noise::point const corners[] = {
    noise::point{0.0, 0.0},  // top left
    noise::point{1.0, 0.0},  // top right
    noise::point{0.0, 1.0},  // btm left
    noise::point{1.0, 1.0},  // btm right
};

precision sequence_pseudorandom1(uint32_t x) {
    x = (x << 13) ^ x;  // XOR with shifted version of itself
    return (1.0 - ((x * (x * x * 61051 + 3166613) + 5915587277) & 0x7FFFFFFF) / 1073741824.0);
}

vector convert_to_seed(iso::radians r) {
    return vector{{std::cos(r.value), std::sin(r.value)}};
}

vector convert_to_seed(iso::degrees d) {
    iso::radians r;
    iso::convert(r, d);
    return vector{{std::cos(r.value), std::sin(r.value)}};
}

vector generate_seed() {
    precision d = (precision)rand() / RAND_MAX;           // 0.0 to 1.0
    constexpr precision const C = 2.0;
    iso::radians r{(C * iso::pi * d) - iso::pi};  // -pi to +pi
    return convert_to_seed(r);
}

point floor(point const& pnt) {
#if defined(USE_XMMT)
    return pnt.floor();
#else
    return point{std::floor(pnt[0]), std::floor(pnt[1])};
#endif
}

point fract(point const& pnt) {
#if defined(USE_XMMT)
    return pnt.fract();
#else
    precision x = pnt[0] - std::floor(pnt[0]);
    precision y = pnt[1] - std::floor(pnt[1]);
    return point{x, y};
#endif
}

precision random(vector const& vec, vector const& seeds, precision gain) {
    // use a dot product to create a ratio of how much a particular point
    // is projected unto a set of scalars in a repeatable way.
    // the smaller magnitude vector will control the scale of the value
    // if smaller is normalized, the value should be -1.0 to +1.0 since
    // the vectors could be pointing in opposite directions.
    precision value = dot(vec, seeds);
    // use this value as an input into the sine wave (which is -1.0 to +1.0)
    // the gain will be the amplitude which can extend back over |1.0| or any other range
    precision scaled_value = std::sin(value) * gain;
    // and only return the fractional component
    return (scaled_value - std::floor(scaled_value));
}

precision perlin(point const& pnt, precision scale, vector const& seeds, precision gain) {
    // converts a point into a normalized space ideally (all dims are 0-1)
    noise::point uv_floor = floor(pnt * (1.0 / scale));
    // this point should be the fractional part of uv between 0.0 and 1.0 in each dimension
    noise::point uv = fract(pnt * (1.0 / scale));
    // find the distance from each corner to the point in the normalize coordinate system
    noise::vector distance[] = {
        uv - corners[0],
        uv - corners[1],
        uv - corners[2],
        uv - corners[3],
    };
    // the feed vectors to index the randomness
    noise::vector feed[] = {uv_floor + corners[0], uv_floor + corners[1], uv_floor + corners[2], uv_floor + corners[3]};

    // procedurally generate 4 "random" numbers between 0 and 1.0
    precision rnds[4] = {
        random(feed[0], seeds, gain),
        random(feed[1], seeds, gain),
        random(feed[2], seeds, gain),
        random(feed[3], seeds, gain),
    };
    size_t idx[4] = {0};
    for (size_t i = 0; i < 4; i++) {
        idx[i] = static_cast<size_t>(std::floor(rnds[i] * 2.0) + 2.0);
    };
    // The set of 2d perlin noise gradients
    vector gradients[4] = {vector{{-1, -1}}, vector{{1, -1}}, vector{{-1, 1}}, vector{{1, 1}}};
    // these are now considered as weights in the interpolation between the four corners
    precision weights[4] = {
        dot(gradients[idx[0]], distance[0]),
        dot(gradients[idx[1]], distance[1]),
        dot(gradients[idx[2]], distance[2]),
        dot(gradients[idx[3]], distance[3]),
    };
    // now do an interpolation between the 4 weights using the normalized point as the alpha
    precision top = noise::interpolate(weights[0], weights[1], fade(uv.x));
    precision bot = noise::interpolate(weights[2], weights[3], fade(uv.x));
    precision mid = noise::interpolate(top, bot, fade(uv.y));
    return fade(map(mid, -1.0, 1.0, 0.0, 1.0));
}

precision smooth(point const& pnt, pad const& map) {
    point base = floor(pnt);
    point frat = fract(pnt);

    // base value
    size_t x1 = static_cast<size_t>(base.x) % map.dimensions;
    size_t y1 = static_cast<size_t>(base.y) % map.dimensions;

    // neighbor values
    size_t x2 = (static_cast<size_t>(base.x) + map.dimensions - 1) % map.dimensions;
    size_t y2 = (static_cast<size_t>(base.y) + map.dimensions - 1) % map.dimensions;

    if constexpr (debug) {
        printf("b={%lf,%lf} f={%lf,%lf} x1,y1={%zu,%zu} x2,y2={%zu,%zu}\n", base.x, base.y, frat.x, frat.y, x1, y1, x2,
               y2);
        printf("noise: %lf, %lf, %lf, %lf\n", map.at(y1, x1), map.at(y1, x2), map.at(y2, x1), map.at(y2, x2));
    }

    // smooth the noise with bilinear interpolation
    precision value = 0.0;
    value += frat.x * frat.y * map.at(y1, x1);
    value += (1 - frat.x) * frat.y * map.at(y1, x2);
    value += frat.x * (1 - frat.y) * map.at(y2, x1);
    value += (1 - frat.x) * (1 - frat.y) * map.at(y2, x2);
    return value;
}

precision turbulence(point const& pnt, precision size, precision scale, pad const& map) {
    precision value = 0.0, initialSize = size;
    while (size >= 1.0) {
        point pnt2{pnt};  // copy
        pnt2 *= 1.0 / size;
        if constexpr (debug) {
            printf("pnt={%lf, %lf}, pnt2={%lf, %lf} scale=%lf\n", pnt.x, pnt.y, pnt2.x, pnt2.y, 1.0 / size);
        }
        value += smooth(pnt2, map) * size;
        size /= 2.0;
    }
    return (scale * value) / initialSize;
}

precision turbulentsin(point const& pnt, precision xs, precision ys, precision power, precision size, precision scale, pad const& map) {
    precision x = pnt.x * xs / map.dimensions;
    precision y = pnt.y * ys / map.dimensions;
    precision xyValue = x + y + power * turbulence(pnt, size, scale, map) / scale;
    return scale * fabs(sin(xyValue * M_PI));
}

static precision mix(precision value1, precision value2, precision mixer) {
    return (value2 * mixer) + (value1 * (1.0 - mixer));
}

static noise::point mix(noise::point a, noise::point b, noise::point s) {
    return noise::point{mix(a.x, b.x, s.x), mix(a.y, b.y, s.y)};
}

static precision fractal_noise(point const& pnt, vector const& seed, precision rand_gain = 1.0) {
    noise::point fl = floor(pnt);
    noise::point uv = fract(pnt);
    // the feed vectors
    noise::vector feed[4] = {
        fl + corners[0],
        fl + corners[1],
        fl + corners[2],
        fl + corners[3],
    };
    // the value at each corner
    precision a = random(feed[0], seed, rand_gain);
    precision b = random(feed[1], seed, rand_gain);
    precision c = random(feed[2], seed, rand_gain);
    precision d = random(feed[3], seed, rand_gain);

    // some function?
    // 2x^3 - 3.0x^2
    precision ux = uv.x * uv.x * (3.0 - (2.0 * uv.x));
    precision uy = uv.y * uv.y * (3.0 - (2.0 * uv.y));

    return mix(a, b, ux) + ((c - a) * uy * (1.0 - ux)) + ((d - b) * ux * uy);
}

precision fractal_brownian(point const& pnt, vector const& seed, size_t octaves, precision lacunarity, precision gain,
                        precision initial_amplitude, precision initial_frequency) {
    precision value = 0.0;
    precision amplitude = initial_amplitude;
    precision frequency = initial_frequency;
    for (size_t o = 0; o < octaves; o++) {
        point tmp = pnt * frequency;
        value += amplitude * fractal_noise(tmp, seed, 43758.5453123);
        frequency *= lacunarity;
        amplitude *= gain;
    }
    return value;
}

}  // namespace noise
