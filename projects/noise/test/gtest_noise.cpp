
#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <fourcc/image.hpp>
#include <noise/noise.hpp>
#include <vector>

size_t histogram[255];

TEST(NoiseTest, PadTest) {
    noise::pad p0;
    p0.generate();
    memset(histogram, 0, sizeof(histogram));
    for (size_t y = 0; y < p0.dimensions; y++) {
        for (size_t x = 0; x < p0.dimensions; x++) {
            // round to a 8 bit number to get a sense of random ness!
            noise::precision v = p0.at(y, x);
            uint8_t b = (uint8_t)std::round(v * 255.0_p);
            histogram[b]++;
        }
    }
    for (size_t i = 1; i < (basal::dimof(histogram) - 1); i++) {
        printf("%zu ", histogram[i]);
        ASSERT_LE(10, histogram[i]) << " at element " << i;
        ASSERT_GE(120, histogram[i]) << " at element " << i;
    }
}

TEST(NoiseTest, MapTest) {
    using namespace noise;
    noise::precision r = 0.0_p;
    r = map(-1.0_p, -1.0_p, 10.0_p, 2.0_p, 3.0_p);
    ASSERT_FLOAT_EQ(2.0_p, r);
    r = map(-9.0_p, -1.0_p, 10.0_p, 2.0_p, 3.0_p);
    ASSERT_FLOAT_EQ(2.0_p, r);
    r = map(9.0_p, 0.0_p, 10.0_p, 2.0_p, 3.0_p);
    ASSERT_FLOAT_EQ(2.9_p, r);
    r = map(11.0_p, -1.0_p, 10.0_p, 2.0_p, 3.0_p);
    ASSERT_FLOAT_EQ(3.0_p, r);
}

TEST(NoiseTest, Interpolate) {
    using namespace noise;
    noise::precision d = noise::interpolate(1.0_p, 0.0_p, 0.5_p);
    ASSERT_FLOAT_EQ(0.5_p, d);
}

TEST(NoiseTest, FloorTest) {
    noise::point f1{1.7_p, 2.4_p};
    noise::point f2 = noise::floor(f1);
    ASSERT_FLOAT_EQ(1.0_p, f2.x);
    ASSERT_FLOAT_EQ(2.0_p, f2.y);
}

TEST(NoiseTest, FractTest) {
    noise::point f1{1.7_p, 2.4_p};
    noise::point f3 = noise::fract(f1);
    ASSERT_FLOAT_EQ(0.7_p, f3.x);
    ASSERT_FLOAT_EQ(0.4_p, f3.y);
}

TEST(NoiseTest, RandomTest) {
    // noise::point ones{1.0_p, 1.0_p};
    noise::vector seed{{1.0_p, 1.0_p}};
    srandom(time(nullptr));
    // test the range generated
    for (noise::precision s = -100.0_p; s < 100.0_p; s += (2.0_p * noise::precision(rand()) / RAND_MAX)) {
        noise::vector vec{{s, 0.0_p}};
        noise::precision r = noise::random(vec, seed, 283.23982_p);
        // printf("r=%lf\n", r);
        ASSERT_TRUE(-1.0_p <= r and r < 1.0_p);
    }
}

TEST(NoiseTest, PadImage) {
    noise::precision sum = 0.0_p;
    noise::pad_<noise::precision, 128> pad;
    pad.generate();
    fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> img(128, 128);
    img.for_each ([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        sum += pad.at(y, x);
        pixel.r = 255u * pad.at(y, x);
        pixel.g = 255u * pad.at(y, x);
        pixel.b = 255u * pad.at(y, x);
    });
    ASSERT_LT(0.0_p, sum / (128 * 128));
    img.save("pad.ppm");
}

class NoiseImageTest : public ::testing::Test {
public:
    NoiseImageTest() : Test{}, pad{}, image{1024, 1024} {
    }

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
    noise::vector vec_r{{23.198239848_p, 7.345820_p}};
    noise::vector vec_g{{29.81020393_p, 92.9283940_p}};
    noise::vector vec_b{{128.320293_p, 29.908293_p}};
    noise::precision gain = 2389.0283_p;
    image.for_each ([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        noise::vector vec{{noise::precision(x), noise::precision(y)}};
        noise::precision _r, r = noise::random(vec, vec_r, gain);
        noise::precision _g, g = noise::random(vec, vec_g, gain);
        noise::precision _b, b = noise::random(vec, vec_b, gain);
        pixel.r = 255u * std::modf(r, &_r);
        pixel.g = 255u * std::modf(g, &_g);
        pixel.b = 255u * std::modf(b, &_b);
    });
    image_name = "noise_image_random.ppm";
}

TEST_F(NoiseImageTest, PerlinNoiseImage) {
    // each point just gets a pixel value
    noise::vector seeds{{8.20203_p, -9.745978_p}};
    noise::precision scale = 10.0_p;
    noise::precision gain = 245.4993546_p;
    image.for_each ([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        noise::point pnt{(noise::precision)x, (noise::precision)y};
        noise::precision p = noise::perlin(pnt, scale, seeds, gain);
        pixel.r = 255u * p;
        pixel.g = 255u * p;
        pixel.b = 255u * p;
    });
    image_name = "noise_image_perlin.ppm";
}

TEST_F(NoiseImageTest, Turbulence) {
    noise::precision size = 9.0_p;
    noise::precision scale = 1.0_p;
    image.for_each ([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        noise::point pnt{(noise::precision)x, (noise::precision)y};
        noise::precision value = noise::turbulence(pnt, size, scale, pad);
        pixel.r = 255u * value;
        pixel.g = 255u * value;
        pixel.b = 255u * value;
    });
    image_name = "noise_image_turbulence.ppm";
}

TEST_F(NoiseImageTest, TurbulenceSin) {
    noise::precision size = 9.0_p;
    noise::precision scale = 10.0_p;
    noise::precision xs = 3.0_p, ys = 3.0_p, power = 2.0_p;
    image.for_each ([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        noise::point pnt{(noise::precision)x, (noise::precision)y};
        noise::precision value = noise::turbulentsin(pnt, xs, ys, power, size, scale, pad);
        pixel.r = 255u * value;
        pixel.g = 255u * value;
        pixel.b = 255u * value;
    });
    image_name = "noise_image_turbulentsin.ppm";
}