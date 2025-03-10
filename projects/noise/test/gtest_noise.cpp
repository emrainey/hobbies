
#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <fourcc/image.hpp>
#include <noise/noise.hpp>
#include <vector>
#include <basal/gtest_helper.hpp>

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
    ASSERT_PRECISION_EQ(2.0_p, r);
    r = map(-9.0_p, -1.0_p, 10.0_p, 2.0_p, 3.0_p);
    ASSERT_PRECISION_EQ(2.0_p, r);
    r = map(9.0_p, 0.0_p, 10.0_p, 2.0_p, 3.0_p);
    ASSERT_PRECISION_EQ(2.9_p, r);
    r = map(11.0_p, -1.0_p, 10.0_p, 2.0_p, 3.0_p);
    ASSERT_PRECISION_EQ(3.0_p, r);
}

TEST(NoiseTest, Interpolate) {
    using namespace noise;
    noise::precision d = noise::interpolate(1.0_p, 0.0_p, 0.5_p);
    ASSERT_PRECISION_EQ(0.5_p, d);
}

TEST(NoiseTest, FloorTest) {
    noise::point f1{1.7_p, 2.4_p};
    noise::point f2 = noise::floor(f1);
    ASSERT_PRECISION_EQ(1.0_p, f2.x);
    ASSERT_PRECISION_EQ(2.0_p, f2.y);
}

TEST(NoiseTest, FractTest) {
    noise::point f1{1.7_p, 2.4_p};
    noise::point f3 = noise::fract(f1);
    ASSERT_PRECISION_EQ(0.7_p, f3.x);
    ASSERT_PRECISION_EQ(0.4_p, f3.y);
}

TEST(NoiseTest, SeedGenerator) {
    for (size_t i = 0U; i < 10'000; i++) {
        noise::vector seed = noise::generate_seed();
        auto m = seed.magnitude();
        ASSERT_PRECISION_EQ(1.0_p, m);
    }
}

TEST(NoiseTest, CellFlows) {
    noise::vector seed = noise::generate_seed();
    noise::vector flows[4];
    noise::point pnt{1.0_p, 1.0_p};
    noise::point uv;
    noise::cell_flows(pnt, 32.0_p, seed, 1.0_p, uv, flows);
    for (size_t i = 0; i < 4; i++) {
        noise::precision m = flows[i].magnitude();
        ASSERT_PRECISION_EQ(1.0_p, m) << "Index " << i;
    }
}

TEST(NoiseTest, ReproducibleFlows) {
    noise::vector seed = noise::generate_seed();
    noise::vector flows1[4];
    for (noise::precision scale = 1.0_p; scale <= 32.0_p; scale += 1.0_p) {
        noise::point pnt1{0.0_p, 0.0_p};
        noise::point uv1;
        noise::cell_flows(pnt1, scale, seed, 1.0_p, uv1, flows1);
        for (noise::precision y = 0.0_p; y < scale; y += (1 / scale)) {
            for (noise::precision x = 0.0_p; x < scale; x += (1 / scale)) {
                noise::point pnt2{x, y};
                noise::vector flows2[4];
                noise::point uv2;
                noise::cell_flows(pnt2, scale, seed, 1.0_p, uv2, flows2);
                // four corners should be the same
                for (size_t i = 0; i < 4; i++) {
                    ASSERT_PRECISION_EQ(flows1[i][0], flows2[i][0]) << "Index " << i;
                    ASSERT_PRECISION_EQ(flows1[i][1], flows2[i][1]) << "Index " << i;
                }
            }
        }
    }
}

TEST(NoiseTest, AdjacentCellFlows) {
    noise::vector seed = noise::generate_seed();
    noise::vector flows1[4];
    noise::precision scale = 32.0_p;
    noise::point pnt1{1.0_p, 1.0_p};
    noise::point uv1;
    noise::cell_flows(pnt1, scale, seed, 1.0_p, uv1, flows1);
    {
        noise::vector flows2[4];
        noise::point pnt2{scale, scale};
        noise::point uv2;
        noise::cell_flows(pnt2, scale, seed, 1.0_p, uv2, flows2);
        ASSERT_PRECISION_EQ(flows1[3][0], flows2[0][0]);
        ASSERT_PRECISION_EQ(flows1[3][1], flows2[0][1]);
    }
    {
        noise::vector flows2[4];
        noise::point pnt2{scale, 1.0_p};
        noise::point uv2;
        noise::cell_flows(pnt2, scale, seed, 1.0_p, uv2, flows2);
        ASSERT_PRECISION_EQ(flows1[1][0], flows2[0][0]);
        ASSERT_PRECISION_EQ(flows1[1][1], flows2[0][1]);
        ASSERT_PRECISION_EQ(flows1[3][0], flows2[2][0]);
        ASSERT_PRECISION_EQ(flows1[3][1], flows2[2][1]);
    }

    {
        noise::vector flows2[4];
        noise::point pnt2{1.0_p, scale};
        noise::point uv2;
        noise::cell_flows(pnt2, scale, seed, 1.0_p, uv2, flows2);
        ASSERT_PRECISION_EQ(flows1[2][0], flows2[0][0]);
        ASSERT_PRECISION_EQ(flows1[2][1], flows2[0][1]);
        ASSERT_PRECISION_EQ(flows1[3][0], flows2[1][0]);
        ASSERT_PRECISION_EQ(flows1[3][1], flows2[1][1]);
    }
}

TEST(NoiseTest, RandomTest) {
    // noise::point ones{1.0_p, 1.0_p};
    noise::vector seed = noise::generate_seed();
    srandom(time(nullptr));
    // test the range generated
    noise::precision step{(1.0_p / 1024.0_p) * iso::tau};
    for (noise::precision s = -iso::pi; s < iso::pi; s += step) {
        noise::precision a = tan(s);
        noise::vector vec{1.0_p, s};
        noise::precision r = noise::random(vec, seed, 283.23982_p);
        printf("r=%lf\n", r);
        ASSERT_PRECISION_NE(0.0_p, r);
        ASSERT_TRUE(-1.0_p <= r and r < 1.0_p);
    }
}

TEST(NoiseTest, PadImage) {
    noise::precision sum = 0.0_p;
    noise::pad_<noise::precision, 128> pad;
    pad.generate();
    fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> img(128, 128);
    img.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
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
    image.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
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
    noise::vector seeds = noise::convert_to_seed(iso::turns{sqrt(5)});
    noise::precision scale = 32.0_p;
    noise::precision gain = 245.4993546_p;
    image.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
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
    image.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        noise::point pnt{(noise::precision)x, (noise::precision)y};
        noise::precision value = noise::turbulence(pnt, size, scale, pad);
        pixel.r = 255u * value;
        pixel.g = 255u * value;
        pixel.b = 255u * value;
    });
    image_name = "noise_image_turbulence.ppm";
}

TEST_F(NoiseImageTest, TurbulenceSin) {
    noise::precision size = 32.0_p;
    noise::precision scale = 256.0_p;  // how "deep" output pixel values can be
    noise::precision xs = 3.0_p, ys = 3.0_p, power = 2.0_p;
    image.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        noise::point pnt{(noise::precision)x, (noise::precision)y};
        noise::precision value = noise::turbulentsin(pnt, xs, ys, power, size, scale, pad);
        // printf("x=%zu, y=%zu, value=%lf\n", x, y, value);
        pixel.r = value;
        pixel.g = value;
        pixel.b = value;
    });
    image_name = "noise_image_turbulentsin.ppm";
}

TEST_F(NoiseImageTest, TurbulenceSinNormalized) {
    noise::precision size = 32.0_p;
    noise::precision scale = 256.0_p;  // how "deep" output pixel values can be
    noise::precision xs = 3.0_p, ys = 3.0_p, power = 2.0_p;
    image.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        noise::precision u = static_cast<noise::precision>(x) / image.width;
        noise::precision v = static_cast<noise::precision>(y) / image.height;
        noise::point pnt{u * 1024.0_p, v * 1024.0_p};  // scale to the normailzed range to some
        noise::precision value = noise::turbulentsin(pnt, xs, ys, power, size, scale, pad);
        // printf("u,v=%lf,%lf value=%lf\n", u, v, value);
        pixel.r = value;
        pixel.g = value;
        pixel.b = value;
    });
    image_name = "noise_image_turbulentsin_norm.ppm";
}

TEST_F(NoiseImageTest, FractalBrownian) {
    iso::degrees angle{81};
    noise::vector seeds{noise::convert_to_seed(angle)};
    noise::precision octaves = 6.0_p;
    noise::precision lacunarity = 2.0_p;
    noise::precision gain = 0.5_p;
    noise::precision amplitude = 0.5_p;
    noise::precision frequency = 1.0_p;
    image.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        noise::point pnt{(noise::precision)x / image.width, (noise::precision)y / image.height};
        noise::precision value = noise::fractal_brownian(pnt, seeds, octaves, lacunarity, gain, amplitude, frequency);
        pixel.r = 255u * value;
        pixel.g = 255u * value;
        pixel.b = 255u * value;
    });
    image_name = "noise_image_fractal_brownian.ppm";
}
