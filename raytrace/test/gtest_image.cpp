
#include <gtest/gtest.h>

#include <raytrace/raytrace.hpp>

using namespace raytrace;
using namespace raytrace::colors;
using namespace raytrace::operators;

TEST(ImageTest, OddSizedImage) {
    ASSERT_THROW(image img2(3, 3), basal::exception);
}

TEST(ImageTest, Basics) {
    image img(10, 20);
    ASSERT_EQ(200, img.area());
    ASSERT_EQ(200 * sizeof(fourcc::rgb8), img.bytes());
}

TEST(ImageTest, IteratorsTest) {
    image img3(2, 2);
    image::point p(0, 0);

    img3.generate_each([&](const image::point&) -> color { return colors::red; });
    ASSERT_EQ(255, img3.at(p).r);
    ASSERT_EQ(0, img3.at(p).g);
    ASSERT_EQ(0, img3.at(p).b);

    img3.generate_each([&](const image::point&) -> color { return colors::green; });
    ASSERT_EQ(0, img3.at(p).r);
    ASSERT_EQ(255, img3.at(p).g);
    ASSERT_EQ(0, img3.at(p).b);

    img3.generate_each([&](const image::point&) -> color { return colors::blue; });
    ASSERT_EQ(0, img3.at(p).r);
    ASSERT_EQ(0, img3.at(p).g);
    ASSERT_EQ(255, img3.at(p).b);
}

TEST(ImageTest, SubsamplingCount) {
    raytrace::image img(2, 2);
    size_t counter = 0;
    img.generate_each(
        [&](const image::point&) -> color {
            counter++;
            return colors::yellow;
        },
        2);
    ASSERT_EQ(img.width * img.height * 2, counter);
}

TEST(ImageTest, SingleColors) {
    image img3(480, 640);
    img3.generate_each([&](const image::point&) -> color { return colors::red; });
    img3.save("allred.ppm");

    img3.generate_each([&](const image::point&) -> color { return colors::green; });
    img3.save("allgreen.ppm");

    img3.generate_each([&](const image::point&) -> color { return colors::blue; });
    img3.save("allblue.ppm");
}

TEST(ImageTest, SubsamplerTest) {
    // since the image has 3 subsamples,
    // the averaging should make a dark grey.
    image img4(2, 2);
    element_type g = 0.61250591370193386;  //  -> 155
    color tmp0(g, g, g);
    fourcc::rgb8 dark_grey = tmp0.to_rgb8();

    int i = 0;
    std::array<color, 3> samples = {colors::red, colors::green, colors::blue};
    auto subsampler = [&](const image::point&) -> color {
        i = (i + 1) % samples.size();
        return samples[i];
    };
    auto renderer = [&](size_t row_index, bool is_completed) -> void {
        image::point pnt(row_index, 0);
        fourcc::rgb8 pix = img4.at(pnt);
        EXPECT_EQ(dark_grey.r, pix.r) << " at " << pnt.x << ", " << pnt.y << std::endl;
        EXPECT_EQ(dark_grey.g, pix.g) << " at " << pnt.x << ", " << pnt.y << std::endl;
        EXPECT_EQ(dark_grey.b, pix.b) << " at " << pnt.x << ", " << pnt.y << std::endl;
    };
    img4.generate_each(subsampler, 3, renderer);
    img4.save("averaged_dark_grey.ppm");
}

TEST(ImageTest, SubsamplingChecker) {
    // this is going to show how the random subsample spread out
    // we'll take the cast point and render it into a larger (20x) image
    image image5(480, 640);
    image5.generate_each([&](const image::point&) -> color { return colors::white; });
    image image6(24, 32);
    double ratio = (image5.height / image6.height);
    image6.generate_each(
        [&](const image::point& p) -> color {
            image::point n(p.x * ratio, p.y * ratio);
            double fx = std::floor(p.x);
            bool ix = (static_cast<int>(fx) & 1);
            double fy = std::floor(p.y);
            bool iy = (static_cast<int>(fy) & 1);
            if ((not ix and not iy) or (ix and iy)) {
                image5.at(n) = colors::red.to_rgb8();
            } else {
                image5.at(n) = colors::black.to_rgb8();
            }
            return colors::yellow;
        },
        16);
    image5.save("subsampling.ppm");
}
