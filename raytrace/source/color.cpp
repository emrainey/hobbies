
#include "raytrace/color.hpp"

#include <algorithm>
#include <cmath>

namespace raytrace {

fourcc::rgba color::to_rgba() const {
    fourcc::rgba pixel;
    pixel.r = static_cast<uint8_t>(std::round(red() * 255));
    pixel.g = static_cast<uint8_t>(std::round(green() * 255));
    pixel.b = static_cast<uint8_t>(std::round(blue() * 255));
    pixel.a = 255;
    return pixel;
}

fourcc::abgr color::to_abgr() const {
    fourcc::abgr pixel;
    pixel.r = static_cast<uint8_t>(std::round(red() * 255));
    pixel.g = static_cast<uint8_t>(std::round(green() * 255));
    pixel.b = static_cast<uint8_t>(std::round(blue() * 255));
    pixel.a = 255;
    return pixel;
}

fourcc::rgb8 color::to_rgb8() const {
    fourcc::rgb8 pixel;
    pixel.r = static_cast<uint8_t>(std::round(red() * 255));
    pixel.g = static_cast<uint8_t>(std::round(green() * 255));
    pixel.b = static_cast<uint8_t>(std::round(blue() * 255));
    return pixel;
}

fourcc::bgr8 color::to_bgr8() const {
    fourcc::bgr8 pixel;
    pixel.r = static_cast<uint8_t>(std::round(red() * 255));
    pixel.g = static_cast<uint8_t>(std::round(green() * 255));
    pixel.b = static_cast<uint8_t>(std::round(blue() * 255));
    return pixel;
}

color::color(const color& other) : color(other.red(), other.green(), other.blue()) {
}

color::color(color&& other) : color(other.red(), other.green(), other.blue()) {
}

color& color::operator=(const color& other) {
    representation = other.representation;
#if defined(USE_SIMD_INSTRUCTIONS)
    simd_channels = other.simd_channels;
#else
    for (size_t i = 0; i < NUM_CHANNELS; i++) {
        channels[i] = other.channels[i];
    }
#endif
    return (*this);
}

color& color::operator=(color&& other) {
    representation = other.representation;
#if defined(USE_SIMD_INSTRUCTIONS)
    simd_channels = other.simd_channels;
#else
    for (size_t i = 0; i < NUM_CHANNELS; i++) {
        channels[i] = other.channels[i];
    }
#endif
    return (*this);
}

void color::per_channel(std::function<element_type(element_type c)> iter) {
    for (size_t i = 0; i < NUM_CHANNELS; i++) {
        channels[i] = iter(channels[i]);
    }
}

color& color::operator+=(const color& other) {
#if defined(USE_SIMD_INSTRUCTIONS)
    simd_channels = _mm256_add_pd(simd_channels, other.simd_channels);
#else
    for (size_t i = 0; i < NUM_CHANNELS; i++) {
        channels[i] += other.channels[i];
    }
#endif
    clamp();
    return (*this);
}

void color::to_space(space desired) {
    if (desired == space::linear and representation == space::logarithmic) {
        for (auto& chan : channels) {
            chan = gamma::apply_correction(chan);
        }
        representation = space::linear;
    } else if (desired == space::logarithmic and representation == space::linear) {
        for (auto& chan : channels) {
            chan = gamma::remove_correction(chan);
        }
        representation = space::logarithmic;
    }
}

constexpr static bool enforce_contract = false;

element_type gamma_interpolate(const element_type a, const element_type b, const element_type s) {
    if (enforce_contract) {
        basal::exception::throw_unless(0.0 <= a and a <= 1.0, __FILE__, __LINE__, "Channel is incorrect");
        basal::exception::throw_unless(0.0 <= b and b <= 1.0, __FILE__, __LINE__, "Channel is incorrect");
        basal::exception::throw_unless(0.0 <= s and s <= 1.0, __FILE__, __LINE__, "Interpolation Scalar is incorrect");
    }
    // linear to log
    element_type ag = gamma::remove_correction(a);
    element_type bg = gamma::remove_correction(b);
    // weighted equation
    element_type cg = (ag * s) + (bg * (1.0 - s));
    // log to linear
    element_type c = gamma::apply_correction(cg);
    // limit to 0-1
    return std::clamp(c, 0.0, 1.0);
}

color blend(const color& x, const color& y) {
    return interpolate(x, y, 0.5);
}

color interpolate(const color& x, const color& y, element_type a) {
    return color(gamma_interpolate(x.red(), y.red(), a), gamma_interpolate(x.green(), y.green(), a),
                 gamma_interpolate(x.blue(), y.blue(), a));
}

bool operator==(const color& a, const color& b) {
    return basal::equals(a.red(), b.red(), color::precision) and
           basal::equals(a.green(), b.green(), color::precision) and
           basal::equals(a.blue(), b.blue(), color::precision);
}

color color::blend_samples(const std::vector<color>& subsamples) {
    color tmp;
    tmp.to_space(color::space::logarithmic);
    element_type div = subsamples.size();
    for (auto& sub : subsamples) {
        color c = sub;
        c.to_space(color::space::logarithmic);
        c *= (1.0 / div);
        tmp += c;
    }
    tmp.to_space(color::space::linear);
    return tmp;
}

color color::accumulate_samples(const std::vector<color>& samples) {
    color tmp;
    // tmp.to_space(color::space::logarithmic);
    for (auto& sample : samples) {
        color c = sample;
        // c.to_space(color::space::logarithmic);
        tmp += c;
    }
    // tmp.to_space(color::space::linear);
    return tmp;
}
namespace jet {

static double interpolate(double val, double y0, double x0, double y1, double x1) {
    return (val - x0) * (y1 - y0) / (x1 - x0) + y0;
}

static double base(double val) {
    if (val <= -0.75)
        return 0.0;
    else if (val <= -0.25)
        return interpolate(val, 0.0, -0.75, 1.0, -0.25);
    else if (val <= 0.25)
        return 1.0;
    else if (val <= 0.75)
        return interpolate(val, 1.0, 0.25, 0.0, 0.75);
    else
        return 0.0;
}

static double red(double gray) {
    return base(gray - 0.5);
}

static double green(double gray) {
    return base(gray);
}

static double blue(double gray) {
    return base(gray + 0.5);
}

}  // namespace jet

color color::jet(element_type value) {
    return color(jet::red(value), jet::green(value), jet::blue(value));
}

color color::greyscale(element_type d, element_type min, element_type max) {
    if (min <= d and d <= max) {
        element_type range = max - min;
        element_type v = (d + std::abs(min)) / range;
        return color(v, v, v);
    } else {
        return colors::magenta;
    }
}

color color::random() {
    return color(basal::rand_range(0.0, 1.0), basal::rand_range(0.0, 1.0), basal::rand_range(0.0, 1.0));
}

namespace operators {
color operator*(const color& a, const color& b) {
#if defined(USE_SIMD_INSTRUCTIONS)
    color c;
    c.simd_channels = _mm256_mul_pd(a.simd_channels, b.simd_channels);
    return c;
#else
    return color(a.red() * b.red(), a.green() * b.green(), a.blue() * b.blue());
#endif
}

}  // namespace operators

/**
 * Finds a nominal (max 1.0, min 0.0) response in a gaussian peak within a low to high range.
 * The purpose is to emulate the LMS rod/cone response in our eyes. This is close to
 * CIE 1964 (I think, I'm not an expert) as this is a symmetric peak.
 * @see https://www.desmos.com/calculator To play with a calculator to generate the equation.
 * @code
 * p = (f0 + f1) / 2
 * q = (f1 - f0) / 2
 * a = (x - p) / q
 * c = e^-(a*a)
 * @endcode
 *
 */
static element_type gaussian_peak(element_type x, element_type low, element_type high) {
    element_type p = (low + high) / 2.0;
    element_type q = (high - low) / 2.0;
    element_type a = (x - p) / q;
    return std::exp(-(a * a));
}

color wavelength_to_color(iso::meters lambda) noexcept(false) {
    using namespace iso::literals;

    static const iso::meters near_infrared = 780E-9_m;
    static const iso::meters ultra_violet = 380E-9_m;
    basal::exception::throw_unless(near_infrared >= lambda and lambda >= ultra_violet, __FILE__, __LINE__,
                                   "Must be in the visible range");
    // we'll express lambda in nm now, so pull up from there
    element_type s = gaussian_peak(lambda.value, 410E-9, 480E-9);  // Peak at 445nm
    element_type m = gaussian_peak(lambda.value, 500E-9, 590E-9);  // Peak at 545nm
    element_type l = gaussian_peak(lambda.value, 520E-9, 630E-9);  // Peak at 575nm
    return color(l, m, s);
}

}  // namespace raytrace
