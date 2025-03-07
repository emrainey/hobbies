
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

color::color(color const& other) : color(other.red(), other.green(), other.blue()) {
}

color::color(color&& other) : color(other.red(), other.green(), other.blue()) {
}

color& color::operator=(color const& other) {
    representation = other.representation;
    for (size_t i = 0; i < NUM_CHANNELS; i++) {
        channels[i] = other.channels[i];
    }
    return (*this);
}

color& color::operator=(color&& other) {
    representation = other.representation;
    for (size_t i = 0; i < NUM_CHANNELS; i++) {
        channels[i] = other.channels[i];
    }
    return (*this);
}

void color::per_channel(std::function<precision(precision c)> iter) {
    for (size_t i = 0; i < NUM_CHANNELS; i++) {
        channels[i] = iter(channels[i]);
    }
}

color& color::operator+=(color const& other) {
    for (size_t i = 0; i < NUM_CHANNELS; i++) {
        channels[i] += other.channels[i];
    }
    clamp();
    return (*this);
}

void color::to_space(space desired) {
    if (desired == space::linear and representation == space::logarithmic) {
        channels[0] = gamma::apply_correction(channels[0]);
        channels[1] = gamma::apply_correction(channels[1]);
        channels[2] = gamma::apply_correction(channels[2]);
        channels[3] = gamma::apply_correction(channels[3]);
        representation = space::linear;
    } else if (desired == space::logarithmic and representation == space::linear) {
        channels[0] = gamma::remove_correction(channels[0]);
        channels[1] = gamma::remove_correction(channels[1]);
        channels[2] = gamma::remove_correction(channels[2]);
        channels[3] = gamma::remove_correction(channels[3]);
        representation = space::logarithmic;
    }
}

constexpr static bool enforce_contract = false;

precision gamma_interpolate(precision const a, precision const b, precision const s) {
    if (enforce_contract) {
        basal::exception::throw_unless(0.0_p <= a and a <= 1.0_p, __FILE__, __LINE__, "Channel is incorrect");
        basal::exception::throw_unless(0.0_p <= b and b <= 1.0_p, __FILE__, __LINE__, "Channel is incorrect");
        basal::exception::throw_unless(0.0_p <= s and s <= 1.0_p, __FILE__, __LINE__,
                                       "Interpolation Scalar is incorrect");
    }
    constexpr static precision const constants[] = {0.0_p, 1.0_p};
    // linear to log
    precision ag = gamma::remove_correction(a);
    precision bg = gamma::remove_correction(b);
    // weighted equation
    precision cg = (ag * s) + (bg * (constants[1] - s));
    // log to linear
    precision c = gamma::apply_correction(cg);
    // limit to 0-1
    precision d = std::clamp(c, constants[0], constants[1]);
    // printf("a %lf b %lf s %lf ag %lf bg %lf cg %lf c %lf d %lf\n", a, b, s, ag, bg, cg, c, d);
    return d;
}

color blend(color const& x, color const& y) {
    return interpolate(x, y, 0.5_p);
}

color interpolate(color const& x, color const& y, precision a) {
    return color(gamma_interpolate(x.red(), y.red(), a), gamma_interpolate(x.green(), y.green(), a),
                 gamma_interpolate(x.blue(), y.blue(), a));
}

bool operator==(color const& a, color const& b) {
    return basal::nearly_equals(a.red(), b.red(), color::equality_limit)
           and basal::nearly_equals(a.green(), b.green(), color::equality_limit)
           and basal::nearly_equals(a.blue(), b.blue(), color::equality_limit);
}

color color::blend_samples(std::vector<color> const& subsamples) {
    color tmp;
    tmp.to_space(color::space::logarithmic);
    precision div = subsamples.size();
    for (auto& sub : subsamples) {
        color c = sub;
        c.to_space(color::space::logarithmic);
        c *= (1.0_p / div);
        tmp += c;
    }
    tmp.to_space(color::space::linear);
    return tmp;
}

color color::accumulate_samples(std::vector<color> const& samples) {
    color tmp;
    // tmp.to_space(color::space::logarithmic);
    for (auto& sample : samples) {
        // sample.to_space(color::space::logarithmic);
        tmp += sample;
    }
    // tmp.to_space(color::space::linear);
    return tmp;
}
namespace jet {

static precision interpolate(precision val, precision y0, precision x0, precision y1, precision x1) {
    return (val - x0) * (y1 - y0) / (x1 - x0) + y0;
}

static precision base(precision val) {
    if (val <= -0.75_p)
        return 0.0_p;
    else if (val <= -0.25_p)
        return interpolate(val, 0.0_p, -0.75_p, 1.0_p, -0.25_p);
    else if (val <= 0.25_p)
        return 1.0_p;
    else if (val <= 0.75_p)
        return interpolate(val, 1.0_p, 0.25_p, 0.0_p, 0.75_p);
    else
        return 0.0_p;
}

static precision red(precision gray) {
    return base(gray - 0.5_p);
}

static precision green(precision gray) {
    return base(gray);
}

static precision blue(precision gray) {
    return base(gray + 0.5_p);
}

}  // namespace jet

color color::jet(precision value) {
    return color(jet::red(value), jet::green(value), jet::blue(value));
}

color color::greyscale(precision d, precision min, precision max) {
    if (min <= d and d <= max) {
        precision range = max - min;
        precision v = (d + std::abs(min)) / range;
        return color(v, v, v);
    } else {
        return colors::magenta;
    }
}

color color::random() {
    return color(basal::rand_range(0.0_p, 1.0_p), basal::rand_range(0.0_p, 1.0_p), basal::rand_range(0.0_p, 1.0_p));
}

namespace operators {
color operator*(color const& a, color const& b) {
    return color(a.red() * b.red(), a.green() * b.green(), a.blue() * b.blue());
}
}  // namespace operators

///
/// Finds a nominal (max 1.0_p, min 0.0_p) response in a gaussian peak within a low to high range.
/// The purpose is to emulate the LMS rod/cone response in our eyes. This is close to
/// CIE 1964 (I think, I'm not an expert) as this is a symmetric peak.
/// @see https://www.desmos.com/calculator To play with a calculator to generate the equation.
/// @code
/// p = (f0 + f1) / 2
/// q = (f1 - f0) / 2
/// a = (x - p) / q
/// c = e^-(a*a)
/// @endcode

///
static precision gaussian_peak(precision x, precision low, precision high) {
    precision p = (low + high) / 2.0_p;
    precision q = (high - low) / 2.0_p;
    precision a = (x - p) / q;
    return std::exp(-(a * a));
}

color wavelength_to_color(iso::meters lambda) noexcept(false) {
    using namespace iso::literals;

    static iso::meters const near_infrared = 780E-9_m;
    static iso::meters const ultra_violet = 380E-9_m;
    basal::exception::throw_unless(near_infrared >= lambda and lambda >= ultra_violet, __FILE__, __LINE__,
                                   "Must be in the visible range");
    // we'll express lambda in nm now, so pull up from there
    precision s = gaussian_peak(lambda.value, 410E-9, 480E-9);  // Peak at 445nm
    precision m = gaussian_peak(lambda.value, 500E-9, 590E-9);  // Peak at 545nm
    precision l = gaussian_peak(lambda.value, 520E-9, 630E-9);  // Peak at 575nm
    return color(l, m, s);
}

}  // namespace raytrace
