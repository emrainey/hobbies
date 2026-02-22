#include <gtest/gtest.h>

#include <cmath>

#include <fourcc/fourcc.hpp>

using namespace basal::literals;
using namespace fourcc;
using namespace fourcc::operators;

#define ASSERT_RGBID_COLOR_EQ(simple, compound)                                                 \
    {                                                                                           \
        ASSERT_NEAR((simple).red(), (compound).red(), fourcc::color::equality_limit);     \
        ASSERT_NEAR((simple).green(), (compound).green(), fourcc::color::equality_limit); \
        ASSERT_NEAR((simple).blue(), (compound).blue(), fourcc::color::equality_limit);   \
    }

namespace colors {
constexpr color white(1.0_p, 1.0_p, 1.0_p);
constexpr color grey(0.73536062_p, 0.73536062_p, 0.73536062_p);
constexpr color black(0.0_p, 0.0_p, 0.0_p);
constexpr color red(1.0_p, 0.0_p, 0.0_p);
constexpr color green(0.0_p, 1.0_p, 0.0_p);
constexpr color blue(0.0_p, 0.0_p, 1.0_p);
constexpr color magenta(1.0_p, 0.0_p, 1.0_p);
}  // namespace colors

TEST(ColorTest, DefaultConstruction) {
    color c;
    ASSERT_NEAR(0.0_p, c.red(), basal::epsilon);
    ASSERT_NEAR(0.0_p, c.green(), basal::epsilon);
    ASSERT_NEAR(0.0_p, c.blue(), basal::epsilon);
}

TEST(ColorTest, GammaInterpolation) {
    precision gg = 0.73536062_p;
    precision c = gamma::interpolate(1.0_p, 0.0_p, 0.5_p);
    ASSERT_NEAR(c, gg, basal::epsilon);
}

TEST(ColorTest, GammaBlending) {
    ASSERT_TRUE(colors::grey == fourcc::gamma::blend(colors::black, colors::white));
    ASSERT_RGBID_COLOR_EQ(colors::grey, fourcc::gamma::blend(colors::black, colors::white));
}

TEST(ColorTest, Assignment) {
    color g = colors::grey;
    ASSERT_TRUE(colors::grey == g);
    ASSERT_RGBID_COLOR_EQ(colors::grey, g);
}

TEST(ColorTest, Logarithmic) {
    color tmp(0.7_p, 0.5_p, 0.2_p);
    tmp.ToEncoding(fourcc::Encoding::GammaCorrected);
    ASSERT_NEAR(tmp.red(), 0.44798841244188325_p, basal::epsilon);
    ASSERT_NEAR(tmp.green(), 0.21404114048223255_p, basal::epsilon);
    ASSERT_NEAR(tmp.blue(), 0.033104766570885055_p, basal::epsilon);
}

TEST(ColorTest, Scaling) {
    color g{0.5_p, 0.5_p, 0.5_p};
    g *= 2.0_p;
    ASSERT_TRUE(g == colors::white);
}

TEST(ColorTest, RGBAComponents) {
    color g = colors::white;
    rgba expected;
    expected.components.r = 255;
    expected.components.g = 255;
    expected.components.b = 255;
    expected.components.a = 255;
    g.ToEncoding(fourcc::Encoding::GammaCorrected);
    auto actual = g.to_<PixelFormat::RGBA>();
    ASSERT_TRUE(expected.components.r == actual.components.r);
    ASSERT_TRUE(expected.components.g == actual.components.g);
    ASSERT_TRUE(expected.components.b == actual.components.b);
    ASSERT_TRUE(expected.components.a == actual.components.a);
}

TEST(ColorTest, Accumulate) {
    color qtr(0.25_p, 0.25_p, 0.25_p);
    color value;
    value += qtr;
    value += qtr;
    value += qtr;
    value += qtr;
    ASSERT_RGBID_COLOR_EQ(value, colors::white);
}

TEST(ColorTest, ChannelComponents) {
    precision gg = 0.73536062_p;
    color w = colors::white;
    w.per_channel([&](precision c) -> precision { return c * gg; });
    ASSERT_RGBID_COLOR_EQ(w, colors::grey);
}

TEST(ColorTest, PairWiseMult) {
    color w{0.5_p, 0.25_p, 0.75_p};
    color v{0.5_p, 1.0_p, 0.8_p};
    color u{0.25_p, 0.25_p, 0.6_p};
    color t = w * v;
    ASSERT_RGBID_COLOR_EQ(u, t);
}

TEST(ColorTest, InterpolateGreyScale) {
    image<PixelFormat::RGB8> img4(256, 256);
    img4.for_each([&](size_t y, size_t x, rgb8& pixel) {
        y |= 0;
        precision a = precision(x) / img4.width;
        color c = fourcc::linear::interpolate(colors::white, colors::black, a);
        c.ToEncoding(fourcc::Encoding::GammaCorrected );
        pixel = c.to_<PixelFormat::RGB8>();
    });
    img4.save("interpolate_greyscale.ppm");
}

TEST(ColorTest, InterpolateCorners) {
    image<PixelFormat::RGB8> img5(480, 640);
    img5.for_each([&](size_t y, size_t x, rgb8& pixel) {
        precision a = precision(x) / img5.width;
        precision b = precision(y) / img5.height;
        color c = fourcc::linear::interpolate(colors::red, colors::green, a);
        color d = fourcc::linear::interpolate(colors::blue, colors::white, b);
        color e = fourcc::linear::blend(c, d);
        e.clamp();
        e.ToEncoding(fourcc::Encoding::GammaCorrected );
        pixel = e.to_<fourcc::PixelFormat::RGB8>();
    });
    img5.save("interpolate_corners.ppm");
}

TEST(ColorTest, LogarithmicGreyscale) {
    image<PixelFormat::RGB8> img7(256, 256);
    img7.for_each([&](size_t y, size_t x, rgb8& pixel) {
        y |= 0;
        precision v = precision(x) / img7.width;
        color c{v, v, v};
        c.ToEncoding(fourcc::Encoding::GammaCorrected );
        pixel = c.to_<fourcc::PixelFormat::RGB8>();
    });
    img7.save("logarithmic_greyscale.ppm");
}

TEST(ColorTest, BlendingSamplesBW) {
    std::vector<color> wb_samples = {colors::white, colors::black};
    color tmp = color::blend_samples(wb_samples);
    ASSERT_RGBID_COLOR_EQ(color(0.5_p, 0.5_p, 0.5_p), tmp);

    precision g = 0.21404114048223255_p;
    tmp.clamp();
    tmp.ToEncoding(fourcc::Encoding::GammaCorrected );
    color median_grey{g, g, g};
    ASSERT_RGBID_COLOR_EQ(median_grey, tmp);
}

TEST(ColorTest, BlendingSamplesRGB) {
    std::vector<color> rgb_samples = {colors::red, colors::green, colors::blue};
    color tmp = color::blend_samples(rgb_samples);
    ASSERT_RGBID_COLOR_EQ(color(0.333333_p, 0.333333_p, 0.333333_p), tmp);

    precision g = 0.090841711183407683_p;
    tmp.clamp();
    tmp.ToEncoding(fourcc::Encoding::GammaCorrected );
    color dark_grey{g, g, g};
    ASSERT_RGBID_COLOR_EQ(dark_grey, tmp);
}

TEST(ColorTest, LMSImage) {
    image<PixelFormat::RGB8> img(100, 800);
    img.for_each([&](size_t y, size_t x, rgb8& pixel) {
        y |= 0;
        precision w = (x / 2.0_p) + 380.0_p;
        iso::meters lambda(w * std::pow(10.0_p, -9.0_p));
        auto value = wavelength_to_color(lambda);
        value.ToEncoding(fourcc::Encoding::GammaCorrected );
        pixel = value.to_<fourcc::PixelFormat::RGB8>();
    });
    img.save("lms.ppm");
}
