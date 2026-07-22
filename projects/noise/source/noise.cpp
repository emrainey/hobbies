#include "noise/noise.hpp"

namespace noise {

using namespace linalg::operators;
using namespace geometry::operators;

namespace {  // in an anonymous namespace for testing
// enforce the correctness at compile time!
// static_assert(fade(0.5_p) == 0.5_p, "Must be equal");
// static_assert(fade(1.0_p) == 1.0_p, "Must be equal");
// static_assert(fade(2.0_p) == 32.0_p, "Must be equal");

/// Deterministically maps a 2D lattice coordinate and a seed to a value in [0, 1).
///
/// All arithmetic is on uint32_t, which wraps modulo 2^32 on overflow per the
/// C++ standard regardless of platform width. The large constants are odd primes
/// chosen to produce good bit-avalanche: each multiplication spreads bits from
/// the low end into the high bits, and the XOR-shift steps mix those back down.
/// The final division uses only the low 31 bits to guarantee a result in [0, 1).
/// @note Written by Deepseek V4 Flash
precision hash_2d(int32_t x, int32_t y, uint32_t seed) {
    uint32_t h = static_cast<uint32_t>(x) * 374761393u;
    h += static_cast<uint32_t>(y) * 668265263u;
    h += seed * 1274126177u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h = h ^ (h >> 16);
    return static_cast<precision>(h & 0x7FFFFFFF) / 2147483648.0_p;
}
/// Derive a uint32_t seed from the user-supplied seed vector and gain.
uint32_t derive_seed(vector const& seed, precision gain) {
    return (static_cast<uint32_t>((seed[0] + 1.0_p) * 1073741824.0_p)
            ^ static_cast<uint32_t>((seed[1] + 1.0_p) * 1073741824.0_p) ^ static_cast<uint32_t>(gain * 16777216.0_p));
}
}  // namespace

noise::point const corners[] = {
    noise::point{0.0_p, 0.0_p},  // top left
    noise::point{1.0_p, 0.0_p},  // top right
    noise::point{0.0_p, 1.0_p},  // btm left
    noise::point{1.0_p, 1.0_p},  // btm right
};

precision sequence_pseudorandom1(uint32_t x) {
    x = (x << 13) ^ x;  // XOR with shifted version of itself
    return (1.0_p - ((x * (x * x * 61051 + 3166613) + 5915587277) & 0x7FFFFFFF) / 1073741824.0_p);
}

vector convert_to_seed(iso::turns t) {
    iso::radians r;
    iso::convert(r, t);
    return convert_to_seed(r);
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
    precision d = (precision)rand() / RAND_MAX;  // 0.0_p to 1.0
    constexpr precision const C = 2.0_p;
    iso::radians r{(C * iso::pi * d) - iso::pi};  // -pi to +pi
    return convert_to_seed(r);
}

point floor(noise::point const& pnt) {
#if defined(USE_XMMT)
    return pnt.floor();
#else
    return point{std::floor(pnt[0]), std::floor(pnt[1])};
#endif
}

point fract(noise::point const& pnt) {
#if defined(USE_XMMT)
    return pnt.fract();
#else
    precision x = pnt[0] - std::floor(pnt[0]);
    precision y = pnt[1] - std::floor(pnt[1]);
    return point{x, y};
#endif
}

precision random(vector const& vec, vector const& seeds, precision gain) {
    int32_t ix = static_cast<int32_t>(std::floor(vec[0]));
    int32_t iy = static_cast<int32_t>(std::floor(vec[1]));
    uint32_t hseed = derive_seed(seeds, gain);
    return hash_2d(ix, iy, hseed);
}

void cell_flows(point const& image_point, precision scale, vector const& seed, precision gain, point& uv,
                vector (&flows)[4]) {
    noise::point flr = noise::floor(image_point * (1.0_p / scale));
    uv = noise::fract(image_point * (1.0_p / scale));
    // Derive a hash seed from the seed vector and gain so the user can still shift the pattern
    uint32_t hseed
        = (static_cast<uint32_t>((seed[0] + 1.0_p) * 1073741824.0_p)
           ^ static_cast<uint32_t>((seed[1] + 1.0_p) * 1073741824.0_p) ^ static_cast<uint32_t>(gain * 16777216.0_p));
    // Hash each corner's integer lattice coordinates independently
    for (int i = 0; i < 4; i++) {
        int32_t cx = static_cast<int32_t>(flr.x() + corners[i][0]);
        int32_t cy = static_cast<int32_t>(flr.y() + corners[i][1]);
        flows[i] = noise::convert_to_seed(iso::turns{hash_2d(cx, cy, hseed)});
    }
}

precision perlin(point const& pnt, precision scale, vector const& seeds, precision gain) {
    noise::point uv;
    noise::vector flows[4];
    noise::cell_flows(pnt, scale, seeds, gain, uv, flows);
    // these are now considered as weights in the interpolation between the four corners
    precision weights[4] = {
        dot(flows[0], uv - corners[0]),
        dot(flows[1], uv - corners[1]),
        dot(flows[2], uv - corners[2]),
        dot(flows[3], uv - corners[3]),
    };
    // now do an interpolation between the 4 weights using the normalized point as the alpha
    precision top = noise::interpolate(weights[0], weights[1], fade(uv.x()));
    precision bot = noise::interpolate(weights[2], weights[3], fade(uv.x()));
    precision mid = noise::interpolate(top, bot, fade(uv.y()));
    return map(mid, -1.0_p, 1.0_p, 0.0_p, 1.0_p);
}

precision simplex(point const& pnt, precision scale, vector const& seed, precision gain) {
    // Skew factors to map a square grid to a triangular (simplicial) grid.
    constexpr precision F2 = 0.5_p * (1.7320508075688772935274463415059_p - 1.0_p);  // (sqrt(3)-1)/2
    constexpr precision G2 = (3.0_p - 1.7320508075688772935274463415059_p) / 6.0_p;  // (3-sqrt(3))/6
    // 8 gradient directions (edge-midpoints of a square).
    static const int grad[8][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, 1}, {1, -1}, {-1, -1},
    };

    precision xs = pnt.x() / scale;
    precision ys = pnt.y() / scale;
    precision s = (xs + ys) * F2;
    int32_t i = static_cast<int32_t>(std::floor(xs + s));
    int32_t j = static_cast<int32_t>(std::floor(ys + s));
    precision t = static_cast<precision>(i + j) * G2;

    // Relative coordinates within the simplex cell.
    precision x0 = xs - (static_cast<precision>(i) - t);
    precision y0 = ys - (static_cast<precision>(j) - t);

    // Determine which triangle (upper or lower) the point lies in.
    int32_t i1 = (x0 > y0) ? 1 : 0;
    int32_t j1 = (x0 > y0) ? 0 : 1;

    // Offsets for the second and third vertices.
    precision x1 = x0 - i1 + G2;
    precision y1 = y0 - j1 + G2;
    precision x2 = x0 - 1.0_p + 2.0_p * G2;
    precision y2 = y0 - 1.0_p + 2.0_p * G2;

    uint32_t hseed = derive_seed(seed, gain);

    auto contrib = [&](int32_t ix, int32_t iy, precision dx, precision dy) -> precision {
        int gi = static_cast<int>(hash_2d(ix, iy, hseed) * 8.0_p) & 7;
        precision d2 = dx * dx + dy * dy;
        if (d2 >= 0.5_p)
            return 0.0_p;
        precision t2 = 0.5_p - d2;
        t2 *= t2;
        return t2 * t2 * (grad[gi][0] * dx + grad[gi][1] * dy);
    };

    precision n0 = contrib(i, j, x0, y0);
    precision n1 = contrib(i + i1, j + j1, x1, y1);
    precision n2 = contrib(i + 1, j + 1, x2, y2);

    // Scale the sum to approximately [-1, 1] (factor ~70 is conventional).
    return map((n0 + n1 + n2) * 70.0_p, -1.0_p, 1.0_p, 0.0_p, 1.0_p);
}

precision worley(point const& pnt, precision scale, vector const& seed, precision gain) {
    precision xs = pnt.x() / scale;
    precision ys = pnt.y() / scale;

    int32_t ix = static_cast<int32_t>(std::floor(xs));
    int32_t iy = static_cast<int32_t>(std::floor(ys));

    uint32_t hseed = derive_seed(seed, gain);

    precision min_dist = std::numeric_limits<precision>::max();

    // Search the 3x3 neighbourhood of cells for the nearest feature point.
    for (int32_t dx = -1; dx <= 1; dx++) {
        for (int32_t dy = -1; dy <= 1; dy++) {
            int32_t cx = ix + dx;
            int32_t cy = iy + dy;

            // Generate two independent offsets in [0, 1) for the feature
            // point inside this cell.
            precision fx = static_cast<precision>(cx) + hash_2d(cx, cy, hseed);
            precision fy = static_cast<precision>(cy) + hash_2d(cx, cy, hseed ^ 0x9E3779B9u);

            precision ddx = fx - xs;
            precision ddy = fy - ys;
            precision dist = ddx * ddx + ddy * ddy;

            if (dist < min_dist) {
                min_dist = dist;
            }
        }
    }

    // Normalise by sqrt(2) so the result fits in [0, 1].
    return std::sqrt(min_dist) / 1.4142135623730951_p;
}

precision smooth(point const& pnt, pad const& map) {
    point base = noise::floor(pnt);
    point frat = noise::fract(pnt);

    // base value
    size_t x1 = static_cast<size_t>(base.x()) % map.dimensions;
    size_t y1 = static_cast<size_t>(base.y()) % map.dimensions;

    // neighbor values
    size_t x2 = (static_cast<size_t>(base.x()) + map.dimensions - 1) % map.dimensions;
    size_t y2 = (static_cast<size_t>(base.y()) + map.dimensions - 1) % map.dimensions;

    if constexpr (debug::smooth) {
        printf("b={%lf,%lf} f={%lf,%lf} x1,y1={%zu,%zu} x2,y2={%zu,%zu}\n", base.x(), base.y(), frat.x(), frat.y(), x1,
               y1, x2, y2);
        printf("noise: %lf, %lf, %lf, %lf\n", map.at(y1, x1), map.at(y1, x2), map.at(y2, x1), map.at(y2, x2));
    }

    // smooth the noise with bilinear interpolation
    precision value = 0.0_p;
    value += frat.x() * frat.y() * map.at(y1, x1);
    value += (1 - frat.x()) * frat.y() * map.at(y1, x2);
    value += frat.x() * (1 - frat.y()) * map.at(y2, x1);
    value += (1 - frat.x()) * (1 - frat.y()) * map.at(y2, x2);
    return value;
}

precision turbulence(point const& pnt, precision size, precision scale, pad const& map) {
    precision value = 0.0_p, initialSize = size;
    while (size >= 1.0_p) {
        point pnt2{pnt};  // copy
        pnt2 = pnt2 * (1.0_p / size);
        if constexpr (debug::turbulence) {
            printf("pnt={%lf, %lf}, pnt2={%lf, %lf} scale=%lf\n", pnt.x(), pnt.y(), pnt2.x(), pnt2.y(), 1.0_p / size);
        }
        value += smooth(pnt2, map) * size;
        size /= 2.0_p;
    }
    return (scale * value) / initialSize;
}

precision turbulentsin(point const& pnt, precision xs, precision ys, precision power, precision size, precision scale,
                       pad const& map) {
    if constexpr (debug::turbulentsin) {
        printf("pnt={%lf, %lf} xs,ys={%lf, %lf}, power=%lf size=%lf, scale=%lf\n", pnt.x(), pnt.y(), xs, ys, power,
               size, scale);
    }
    precision x = pnt.x() * xs / map.dimensions;
    precision y = pnt.y() * ys / map.dimensions;
    precision xyValue = x + y + power * turbulence(pnt, size, scale, map) / scale;
    if constexpr (debug::turbulentsin) {
        printf("x=%lf, y=%lf, xyValue = %lf\n", x, y, xyValue);
    }
    // converts any input x into an output between 0.0_p and 1.0_p then scaled to whatever you need
    return scale * fabs(sin(xyValue * iso::pi));
}

static precision mix(precision value1, precision value2, precision mixer) {
    return (value2 * mixer) + (value1 * (1.0_p - mixer));
}

// static noise::point mix(noise::point a, noise::point b, noise::point s) {
//     return noise::point{mix(a.x, b.x, s.x), mix(a.y, b.y, s.y)};
// }

static precision fractal_noise(point const& pnt, vector const& seed, precision rand_gain = 1.0_p) {
    noise::point fl = noise::floor(pnt);
    noise::point uv = noise::fract(pnt);
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
    precision ux = uv.x() * uv.x() * (3.0_p - (2.0_p * uv.x()));
    precision uy = uv.y() * uv.y() * (3.0_p - (2.0_p * uv.y()));

    return mix(a, b, ux) + ((c - a) * uy * (1.0_p - ux)) + ((d - b) * ux * uy);
}

precision fractal_brownian(point const& pnt, vector const& seed, size_t octaves, precision lacunarity, precision gain,
                           precision initial_amplitude, precision initial_frequency) {
    precision value = 0.0_p;
    precision amplitude = initial_amplitude;
    precision frequency = initial_frequency;
    for (size_t o = 0; o < octaves; o++) {
        point tmp = pnt * frequency;
        value += amplitude * fractal_noise(tmp, seed, 43758.5453123_p);
        frequency *= lacunarity;
        amplitude *= gain;
    }
    return value;
}

}  // namespace noise
