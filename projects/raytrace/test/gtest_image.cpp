
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
    image::point p{0, 0};

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
    precision g = 0.61250591370193386_p;  //  -> ~156
    color tmp0(g, g, g);
    fourcc::rgb8 dark_grey = tmp0.to_rgb8();
    EXPECT_EQ(156u, dark_grey.r);
    EXPECT_EQ(156u, dark_grey.g);
    EXPECT_EQ(156u, dark_grey.b);
    int i = 0;
    std::array<color, 3> samples = {colors::red, colors::green, colors::blue};
    // this will fail if you don't block the multiple render threads from competing to
    // increment i. If it's single threaded it will pass.
    auto lock = std::mutex{};
    auto subsampler = [&](const image::point& pnt) -> color {
        lock.lock();
        auto tmp = samples[i];
        i = (i + 1) % samples.size();
        lock.unlock();
        std::cout << "returning " << tmp << " for " << pnt << std::endl;
        return tmp;
    };
    auto renderer = [&](size_t row_index, bool is_completed) -> void {
        image::point pnt(row_index, 0);
        fourcc::rgb8 pix = img4.at(pnt);
        EXPECT_TRUE(is_completed);
        EXPECT_EQ(dark_grey.r, pix.r) << pix.r << " at " << pnt.x << ", " << pnt.y << std::endl;
        EXPECT_EQ(dark_grey.g, pix.g) << pix.g << " at " << pnt.x << ", " << pnt.y << std::endl;
        EXPECT_EQ(dark_grey.b, pix.b) << pix.b << " at " << pnt.x << ", " << pnt.y << std::endl;
    };
    img4.generate_each(subsampler, samples.size(), renderer);
    img4.save("averaged_dark_grey.ppm");
}

TEST(ImageTest, SubsamplingChecker) {
    // this is going to show how the random subsample spread out
    // we'll take the cast point and render it into a larger (20x) image
    image image5(480, 640);
    image5.generate_each([&](const image::point&) -> color { return colors::white; });
    image image6(24, 32);
    precision ratio = (image5.height / image6.height);
    image6.generate_each(
        [&](const image::point& p) -> color {
            image::point n{static_cast<precision>(p.x * ratio), static_cast<precision>(p.y * ratio)};
            precision fx = std::floor(p.x);
            bool ix = (static_cast<int>(fx) & 1);
            precision fy = std::floor(p.y);
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
