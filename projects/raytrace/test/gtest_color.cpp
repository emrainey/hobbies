
#include <gtest/gtest.h>

#include <raytrace/raytrace.hpp>

#include "raytrace/gtest_helper.hpp"

using namespace basal::literals;
using namespace raytrace;
using namespace raytrace::colors;
using namespace raytrace::operators;

TEST(ColorTest, DefaultConstruction) {
    color c;
    ASSERT_PRECISION_EQ(0.0_p, c.red());
    ASSERT_PRECISION_EQ(0.0_p, c.green());
    ASSERT_PRECISION_EQ(0.0_p, c.blue());
}

TEST(ColorTest, Basics) {
    precision gg = 0.73536062_p;
    precision c = gamma_interpolate(1.0_p, 0.0_p, 0.5_p);
    ASSERT_NEAR(c, gg, basal::epsilon);
}

TEST(ColorTest, Blending) {
    ASSERT_TRUE(grey == blend(black, white));
    ASSERT_COLOR_EQ(grey, blend(black, white));
    ASSERT_COLOR_EQ(grey, black + white);
}

TEST(ColorTest, Assignment) {
    color g = grey;
    ASSERT_TRUE(grey == g);
    ASSERT_COLOR_EQ(grey, g);
}

TEST(ColorTest, Logarithmic) {
    color tmp(0.7, 0.5, 0.2);  // starts in linear
    // convert to log
    tmp.to_space(color::space::logarithmic);
    // check values
    ASSERT_NEAR(tmp.red(), 0.44798841244188325, basal::epsilon);
    ASSERT_NEAR(tmp.green(), 0.21404114048223255, basal::epsilon);
    ASSERT_NEAR(tmp.blue(), 0.033104766570885055, basal::epsilon);
}

TEST(ColorTest, Scaling) {
    color g = grey;
    g *= 2.0;  // scale 0.5 to 1.0
    ASSERT_TRUE(g == white);
}

TEST(ColorTest, RGBAComponents) {
    color g = white;
    fourcc::rgba pix1;
    pix1.r = 255;
    pix1.g = 255;
    pix1.b = 255;
    pix1.a = 255;
    fourcc::rgba pix2 = g.to_rgba();
    ASSERT_TRUE(pix1.r == pix2.r);
    ASSERT_TRUE(pix1.g == pix2.g);
    ASSERT_TRUE(pix1.b == pix2.b);
    ASSERT_TRUE(pix1.a == pix2.a);
}

TEST(ColorTest, Accumulate) {
    color qtr(0.25, 0.25, 0.25);
    color value;
    value += qtr;
    value += qtr;
    value += qtr;
    value += qtr;
    ASSERT_COLOR_EQ(value, colors::white);
}

TEST(ColorTest, ChannelComponents) {
    precision gg = 0.73536062;
    color w = colors::white;
    w.per_channel([&](precision c) -> precision { return c * gg; });
    ASSERT_COLOR_EQ(w, colors::grey);
}

TEST(ColorTest, PairWiseMult) {
    color w{0.5, 0.25, 0.75};
    color v{0.5, 1.0, 0.8};
    color u{0.25, 0.25, 0.6};
    color t = w * v;
    ASSERT_COLOR_EQ(u, t);
}

TEST(ColorTest, InterpolateGreyScale) {
    fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> img4(256, 256);
    img4.for_each ([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        y |= 0;
        precision a = precision(x) / img4.width;
        color c = interpolate(colors::white, colors::black, a);
        pixel = c.to_rgb8();
    });
    img4.save("interpolate_greyscale.ppm");
}

TEST(ColorTest, InterpolateCorners) {
    fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> img5(480, 640);
    img5.for_each ([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        precision a = precision(x) / img5.width;
        precision b = precision(y) / img5.height;
        color c = interpolate(colors::red, colors::green, a);
        color d = interpolate(colors::blue, colors::white, b);
        color e = blend(c, d);
        pixel = e.to_rgb8();
    });
    img5.save("interpolate_corners.ppm");
}

TEST(ColorTest, LinearGreyscale) {
    fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> img6(256, 256);
    img6.for_each ([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        y |= 0;
        precision v = precision(x) / img6.width;
        color c{v, v, v};  // starts linear
        c.to_space(color::space::linear);
        pixel = c.to_rgb8();
    });
    img6.save("linear_greyscale.ppm");
}

TEST(ColorTest, LogarithmicGreyscale) {
    fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> img7(256, 256);
    img7.for_each ([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        y |= 0;
        precision v = precision(x) / img7.width;
        color c{v, v, v};
        c.to_space(color::space::logarithmic);
        pixel = c.to_rgb8();
    });
    img7.save("logarithmic_greyscale.ppm");
}

TEST(ColorTest, BlendingSamples) {
    std::vector<color> wb_samples = {colors::white, colors::black};
    color tmp = color::blend_samples(wb_samples);
    ASSERT_COLOR_EQ(colors::grey, tmp);

    precision g = 0.61250591370193386_p;
    std::vector<color> rgb_samples = {colors::red, colors::green, colors::blue};
    tmp = color::blend_samples(rgb_samples);
    color dark_grey(g, g, g);
    ASSERT_COLOR_EQ(dark_grey, tmp);
}

TEST(ColorTest, LMSImage) {
    image img(100, 800);
    img.generate_each([&](const image::point& pnt) -> color {
        // x is 0 - 800
        // w should be in the range of 380 to 780
        precision w = (pnt.x / 2) + 380;
        iso::meters lambda(w * pow(10, -9));
        return wavelength_to_color(lambda);
    });
    img.save("lms.ppm");
}
