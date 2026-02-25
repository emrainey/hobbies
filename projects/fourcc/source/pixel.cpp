/// @file
/// Implements the pixel operations

#include <fourcc/pixel.hpp>

namespace fourcc {

color accumulate_samples(std::vector<color> const& samples) {
    color tmp;
    for (auto& sample : samples) {
        basal::exception::throw_unless(sample.GetEncoding() == Encoding::Linear, __FILE__, __LINE__,
                                       "Color should be in linear Encoding");
        tmp += sample;
    }
    return tmp;
}

color jet(precision value) {
    auto interpolate = [](precision val, precision y0, precision x0, precision y1, precision x1) -> precision {
        return (val - x0) * (y1 - y0) / (x1 - x0) + y0;
    };
    auto base = [&](precision val) -> precision {
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
    };
    auto r = base(value - 0.5_p);
    auto g = base(value);
    auto b = base(value + 0.5_p);
    return color(r, g, b);
}

color greyscale(precision value, precision min, precision max) {
    precision v{1.0_p};
    if (min <= value and value <= max) {
        precision range = max - min;
        v = (value + std::abs(min)) / range;
    }
    return color{v, v, v};
}

color random() {
    precision r = basal::rand_range(0.0_p, 1.0_p);
    precision g = basal::rand_range(0.0_p, 1.0_p);
    precision b = basal::rand_range(0.0_p, 1.0_p);
    return color{r, g, b};
}

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
// TODO Candidate to move to linalg extra math functions
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

}  // namespace fourcc