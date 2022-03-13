#include "noise/noise.hpp"

namespace noise {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

constexpr bool debug = false;

double sequence_pseudorandom1(uint32_t x) {
    x = (x << 13) ^ x;  // XOR with shifted version of itself
    return (1.0 - ((x * (x * x * 61051 + 3166613) + 5915587277) & 0x7FFFFFFF) / 1073741824.0);
}

vector convert_to_seed(iso::radians r) {
    return vector{{std::cos(r.value), std::sin(r.value)}};
}

vector generate_seed() {
    double d = (double)rand() / RAND_MAX;           // 0.0 to 1.0
    iso::radians r((2.0 * iso::pi * d) - iso::pi);  // -pi to +pi
    return convert_to_seed(r);
}

point floor(const point& pnt) {
    return point(std::floor(pnt.x), std::floor(pnt.y));
}

point fract(const point& pnt) {
    double _x, x = std::modf(pnt.x, &_x);
    double _y, y = std::modf(pnt.y, &_y);
    // _x and _y are thrown away intentionally
    return point(x, y);
}

double random(const vector& vec, const vector& seeds, double gain) {
    // use a dot product to create a ratio of how much a particular point
    // is projected unto a set of scalars in a repeatable way.
    // the smaller magnitude vector will control the scale of the value
    // if smaller is normalized, the value should be 0.0 - 1.0
    double value = dot(vec, seeds);
    // use this value as an input into the sine wave (which is 0.0-1.0)
    // the gain will be the amplitude which can extend back over 1.0
    double scaled_value = std::sin(value) * gain;
    // now forget anything over 1.0 (surprise!)
    double _integer = 0.0;
    double fractional = std::modf(scaled_value, &_integer);
    // and only return the fractional component
    return fractional;
}

double perlin(const point& pnt, double scale, const vector& seeds, double gain) {
    // converts a point into a normalized space ideally (all dims are 0-1)
    noise::point uv_s = floor(pnt * (1.0 / scale));
    // this point should be the fractional part of uv between 0.0 and 1.0 in each dimension
    noise::point uv = fract(pnt * (1.0 / scale));
    // the corners of the norminalized area
    noise::point corners[4] = {
        point(0.0, 0.0),
        point(1.0, 0.0),
        point(0.0, 1.0),
        point(1.0, 1.0),
    };
    // find the distance from each corner to the point in the normalize coorindate system
    noise::vector distance[4] = {
        uv - corners[0],
        uv - corners[1],
        uv - corners[2],
        uv - corners[3],
    };
    // the feed vectors to index the randomness
    noise::vector feed[4] = {uv_s + corners[0], uv_s + corners[1], uv_s + corners[2], uv_s + corners[3]};
    // procedurally generate 4 "random" numbers between 0 and 1.0
    double rnds[4] = {
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
    double weights[4] = {
        dot(gradients[idx[0]], distance[0]),
        dot(gradients[idx[1]], distance[1]),
        dot(gradients[idx[2]], distance[2]),
        dot(gradients[idx[3]], distance[3]),
    };
    // now do an interpolation between the 4 weights using the normalized point as the alpha
    double top = noise::interpolate(weights[0], weights[1], fade(uv.x));
    double bot = noise::interpolate(weights[2], weights[3], fade(uv.x));
    double mid = noise::interpolate(top, bot, fade(uv.y));
    return fade(map(mid, -1.0, 1.0, 0.0, 1.0));
}

double smooth(const point& pnt, const pad& map) {
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
    double value = 0.0;
    value += frat.x * frat.y * map.at(y1, x1);
    value += (1 - frat.x) * frat.y * map.at(y1, x2);
    value += frat.x * (1 - frat.y) * map.at(y2, x1);
    value += (1 - frat.x) * (1 - frat.y) * map.at(y2, x2);
    return value;
}

double turbulence(const point& pnt, double size, double scale, const pad& map) {
    double value = 0.0, initialSize = size;
    while (size >= 1.0) {
        point pnt2(pnt);  // copy
        pnt2 *= 1.0 / size;
        if constexpr (debug) {
            printf("pnt={%lf, %lf}, pnt2={%lf, %lf} scale=%lf\n", pnt.x, pnt.y, pnt2.x, pnt2.y, 1.0 / size);
        }
        value += smooth(pnt2, map) * size;
        size /= 2.0;
    }
    return (scale * value) / initialSize;
}

double turbulentsin(const point& pnt, double xs, double ys, double power, double size, double scale, const pad& map) {
    double x = pnt.x * xs / map.dimensions;
    double y = pnt.y * ys / map.dimensions;
    double xyValue = x + y + power * turbulence(pnt, size, scale, map) / scale;
    return scale * fabs(sin(xyValue * M_PI));
}

}  // namespace noise