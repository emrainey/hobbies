
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
    ASSERT_EQ(200 * sizeof(fourcc::rgbid), img.bytes());
}

TEST(ImageTest, IteratorsTest) {
    image img3(2, 2);
    image::point p{0, 0};

    img3.generate_each([&](image::point const&) -> color { return colors::red; });
    ASSERT_EQ(1.0, img3.at(p).components.r);
    ASSERT_EQ(0.0, img3.at(p).components.g);
    ASSERT_EQ(0.0, img3.at(p).components.b);

    img3.generate_each([&](image::point const&) -> color { return colors::green; });
    ASSERT_EQ(0.0, img3.at(p).components.r);
    ASSERT_EQ(1.0, img3.at(p).components.g);
    ASSERT_EQ(0.0, img3.at(p).components.b);

    img3.generate_each([&](image::point const&) -> color { return colors::blue; });
    ASSERT_EQ(0.0, img3.at(p).components.r);
    ASSERT_EQ(0.0, img3.at(p).components.g);
    ASSERT_EQ(1.0, img3.at(p).components.b);
}

TEST(ImageTest, SubsamplingCount) {
    raytrace::image img(2, 2);
    size_t counter = 0;
    img.generate_each(
        [&](image::point const&) -> color {
            counter++;
            return colors::yellow;
        },
        2);
    ASSERT_EQ(img.width * img.height * 2, counter);
}

TEST(ImageTest, SingleColors) {
    image img3(480, 640);
    img3.generate_each([&](image::point const&) -> color { return colors::red; });
    img3.save("allred.ppm");

    img3.generate_each([&](image::point const&) -> color { return colors::green; });
    img3.save("allgreen.ppm");

    img3.generate_each([&](image::point const&) -> color { return colors::blue; });
    img3.save("allblue.ppm");
}

TEST(ImageTest, DISABLED_SubsamplerTest) {
    // since the image has 3 subsamples,
    // the averaging should make a dark grey.
    image img4(2, 2);
    precision g = 0.61250591370193386_p;  //  -> ~156
    color tmp0(g, g, g);
    fourcc::rgb8 dark_grey = tmp0.to_<fourcc::PixelFormat::RGB8>();
    EXPECT_EQ(156u, dark_grey.components.r);
    EXPECT_EQ(156u, dark_grey.components.g);
    EXPECT_EQ(156u, dark_grey.components.b);
    int i = 0;
    std::array<color, 3> samples = {colors::red, colors::green, colors::blue};
    /// FIXME the lambda so that it will produce the same answers deterministically
    // this will fail if you don't block the multiple render threads from competing to
    // increment i. If it's single threaded it will pass.
    auto lock = std::mutex{};
    auto subsampler = [&](image::point const& pnt) -> color {
        lock.lock();
        auto tmp = samples[i];
        i = (i + 1) % samples.size();
        lock.unlock();
        std::cout << "returning " << tmp << " for " << pnt << std::endl;
        return tmp;
    };
    auto renderer = [&](size_t row_index, bool is_completed) -> void {
        image::point pnt(row_index, 0);
        auto pix = img4.at(pnt);
        EXPECT_TRUE(is_completed);
        EXPECT_EQ(dark_grey.components.r, pix.components.r)
            << pix.components.r << " at " << pnt.x() << ", " << pnt.y() << std::endl;
        EXPECT_EQ(dark_grey.components.g, pix.components.g)
            << pix.components.g << " at " << pnt.x() << ", " << pnt.y() << std::endl;
        EXPECT_EQ(dark_grey.components.b, pix.components.b)
            << pix.components.b << " at " << pnt.x() << ", " << pnt.y() << std::endl;
    };
    img4.generate_each(subsampler, samples.size(), renderer);
    img4.save("averaged_dark_grey.ppm");
}

TEST(ImageTest, SubsamplingChecker) {
    // this is going to show how the random subsample spread out
    // we'll take the cast point and render it into a larger (20x) image
    image image5(480, 640);
    image5.generate_each([&](image::point const&) -> color { return colors::white; });
    image image6(24, 32);
    precision ratio = (image5.height / image6.height);
    image6.generate_each(
        [&](image::point const& p) -> color {
            image::point n{static_cast<precision>(p.x() * ratio), static_cast<precision>(p.y() * ratio)};
            precision fx = std::floor(p.x());
            bool ix = (static_cast<int>(fx) & 1);
            precision fy = std::floor(p.y());
            bool iy = (static_cast<int>(fy) & 1);
            color c;
            if ((not ix and not iy) or (ix and iy)) {
                c = colors::red;
                image5.at(n) = c.to_<fourcc::PixelFormat::RGBId>();
            } else {
                c = colors::blue;
                image5.at(n) = c.to_<fourcc::PixelFormat::RGBId>();
            }
            return colors::yellow;
        },
        32);
    fourcc::image<fourcc::PixelFormat::RGB8> image7(480, 640);
    fourcc::convert(image5, image7);
    image7.save("subsampling.ppm");
}
