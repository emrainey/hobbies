
#include <gtest/gtest.h>
#include <noise/noise.hpp>
#include <fourcc/image.hpp>
#include <basal/basal.hpp>
#include <vector>


TEST(NoiseTest, PadTest) {
    noise::pad p0;
    p0.generate();
    size_t histogram[255];
    memset(histogram, 0, sizeof(histogram));
    for (size_t y = 0; y < p0.dimensions; y++) {
        for (size_t x = 0; x < p0.dimensions; x++) {
            // round to a 8 bit number to get a sense of random ness!
            double v = p0.at(y, x);
            uint8_t b = (uint8_t)std::round(v * 255.0);
            histogram[b]++;
        }
    }
    for (size_t i = 1; i < basal::dimof(histogram) - 1; i++) {
        printf("%zu ", histogram[i]);
        ASSERT_LE(10, histogram[i]) << " at element " << i;
        ASSERT_GE(120, histogram[i]) << " at element " << i;
    }
}

TEST(NoiseTest, MapTest) {
    using namespace noise;
    double r = 0.0;
    r = map(-1.0, -1.0, 10.0, 2.0, 3.0);
    ASSERT_FLOAT_EQ(2.0, r);
    r = map(-9.0, -1.0, 10.0, 2.0, 3.0);
    ASSERT_FLOAT_EQ(2.0, r);
    r = map(9.0, 0.0, 10.0, 2.0, 3.0);
    ASSERT_FLOAT_EQ(2.9, r);
    r = map(11.0, -1.0, 10.0, 2.0, 3.0);
    ASSERT_FLOAT_EQ(3.0, r);
}

TEST(NoiseTest, Interpolate) {
    using namespace noise;
    double d = noise::interpolate(1.0, 0.0, 0.5);
    ASSERT_FLOAT_EQ(0.5, d);
}

TEST(NoiseTest, FloorTest) {
    noise::point f1(1.7, 2.4);
    noise::point f2 = noise::floor(f1);
    ASSERT_FLOAT_EQ(1.0, f2.x);
    ASSERT_FLOAT_EQ(2.0, f2.y);
}

TEST(NoiseTest, FractTest) {
    noise::point f1(1.7, 2.4);
    noise::point f3 = noise::fract(f1);
    ASSERT_FLOAT_EQ(0.7, f3.x);
    ASSERT_FLOAT_EQ(0.4, f3.y);
}

TEST(NoiseTest, RandomTest) {
    // noise::point  ones(1.0, 1.0);
    noise::vector seed{{1.0, 1.0}};
    srandom(time(nullptr));
    // test the range generated
    for (double s = -100.0; s < 100.0; s+=(2.0 * double(rand())/RAND_MAX)) {
        noise::vector vec{{s, 0.0}};
        double r = noise::random(vec, seed, 283.23982);
        // printf("r=%lf\n", r);
        ASSERT_TRUE(-1.0 <= r and r < 1.0);
    }
}

TEST(NoiseTest, PadImage) {
    double sum = 0.0;
    noise::pad_<double, 128> pad;
    pad.generate();
    fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> img(128, 128);
    img.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        sum += pad.at(y, x);
        pixel.r = 255u * pad.at(y, x);
        pixel.g = 255u * pad.at(y, x);
        pixel.b = 255u * pad.at(y, x);
    });
    ASSERT_LT(0.0, sum / (128 * 128));
    img.save("pad.ppm");
}

class NoiseImageTest : public ::testing::Test {
public:
    NoiseImageTest()
        : Test()
        , pad()
        , image(1024, 1024)
        {}

    void SetUp() {
        pad.generate();
        image_name = "unknown.ppm";
    }

    void TearDown() {
        image.save(image_name.c_str());
    }
protected:
    noise::pad pad;
    std::string image_name;
    fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> image;
};

TEST_F(NoiseImageTest, RandomNoiseImage) {
    // each point just gets a pixel value
    noise::vector vec_r{{23.198239848, 7.345820}};
    noise::vector vec_g{{29.81020393, 92.9283940}};
    noise::vector vec_b{{128.320293, 29.908293}};
    double gain = 2389.0283;
    image.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        noise::vector vec{{double(x), double(y)}};
        double _r, r = noise::random(vec, vec_r, gain);
        double _g, g = noise::random(vec, vec_g, gain);
        double _b, b = noise::random(vec, vec_b, gain);
        pixel.r = 255u * std::modf(r, &_r);
        pixel.g = 255u * std::modf(g, &_g);
        pixel.b = 255u * std::modf(b, &_b);
    });
    image_name = "noise_image_random.ppm";
}

TEST_F(NoiseImageTest, PerlinNoiseImage) {
    // each point just gets a pixel value
    noise::vector seeds{{8.20203, -9.745978}};
    double scale = 10.0;
    double gain = 245.4993546;
    image.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        noise::point pnt(x, y);
        double p = noise::perlin(pnt, scale, seeds, gain);
        pixel.r = 255u * p;
        pixel.g = 255u * p;
        pixel.b = 255u * p;
    });
    image_name = "noise_image_perlin.ppm";
}

TEST_F(NoiseImageTest, Turbulence) {
    double size = 9.0;
    double scale = 1.0;
    image.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        noise::point pnt(x, y);
        double value = noise::turbulence(pnt, size, scale, pad);
        pixel.r = 255u * value;
        pixel.g = 255u * value;
        pixel.b = 255u * value;
    });
    image_name = "noise_image_turbulence.ppm";
}


TEST_F(NoiseImageTest, TurbulenceSin) {
    double size = 9.0;
    double scale = 10.0;
    double xs = 3.0, ys = 3.0, power = 2.0;
    image.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        noise::point pnt(x, y);
        double value = noise::turbulentsin(pnt, xs, ys, power, size, scale, pad);
        pixel.r = 255u * value;
        pixel.g = 255u * value;
        pixel.b = 255u * value;
    });
    image_name = "noise_image_turbulentsin.ppm";
}