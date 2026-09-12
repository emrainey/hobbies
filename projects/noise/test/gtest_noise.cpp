
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
    ASSERT_PRECISION_EQ(1.0_p, f2.x());
    ASSERT_PRECISION_EQ(2.0_p, f2.y());
}

TEST(NoiseTest, FractTest) {
    noise::point f1{1.7_p, 2.4_p};
    noise::point f3 = noise::fract(f1);
    ASSERT_PRECISION_EQ(0.7_p, f3.x());
    ASSERT_PRECISION_EQ(0.4_p, f3.y());
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
    fourcc::image<fourcc::PixelFormat::RGB8> img(128, 128);
    img.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        sum += pad.at(y, x);
        pixel.components.r = 255u * pad.at(y, x);
        pixel.components.g = 255u * pad.at(y, x);
        pixel.components.b = 255u * pad.at(y, x);
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
    fourcc::image<fourcc::PixelFormat::RGB8> image;
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
        pixel.components.r = 255u * std::modf(r, &_r);
        pixel.components.g = 255u * std::modf(g, &_g);
        pixel.components.b = 255u * std::modf(b, &_b);
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
        pixel.components.r = 255u * p;
        pixel.components.g = 255u * p;
        pixel.components.b = 255u * p;
    });
    image_name = "noise_image_perlin.ppm";
}

TEST_F(NoiseImageTest, Turbulence) {
    noise::precision size = 9.0_p;
    noise::precision scale = 1.0_p;
    image.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        noise::point pnt{(noise::precision)x, (noise::precision)y};
        noise::precision value = noise::turbulence(pnt, size, scale, pad);
        pixel.components.r = 255u * value;
        pixel.components.g = 255u * value;
        pixel.components.b = 255u * value;
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
        pixel.components.r = 255u * value;
        pixel.components.g = 255u * value;
        pixel.components.b = 255u * value;
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
        pixel.components.r = 255u * value;
        pixel.components.g = 255u * value;
        pixel.components.b = 255u * value;
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
        pixel.components.r = 255u * value;
        pixel.components.g = 255u * value;
        pixel.components.b = 255u * value;
    });
    image_name = "noise_image_fractal_brownian.ppm";
}

TEST_F(NoiseImageTest, SimplexNoiseImage) {
    noise::vector seeds = noise::convert_to_seed(iso::turns{sqrt(5)});
    noise::precision scale = 32.0_p;
    noise::precision gain = 245.4993546_p;
    image.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        noise::point pnt{(noise::precision)x, (noise::precision)y};
        noise::precision p = noise::simplex(pnt, scale, seeds, gain);
        pixel.components.r = 255u * p;
        pixel.components.g = 255u * p;
        pixel.components.b = 255u * p;
    });
    image_name = "noise_image_simplex.ppm";
}

TEST_F(NoiseImageTest, WorleyNoiseImage) {
    noise::vector seeds = noise::convert_to_seed(iso::turns{sqrt(5)});
    noise::precision scale = 32.0_p;
    noise::precision gain = 245.4993546_p;
    image.for_each([&](size_t y, size_t x, fourcc::rgb8& pixel) {
        noise::point pnt{(noise::precision)x, (noise::precision)y};
        noise::precision p = noise::worley(pnt, scale, seeds, gain);
        pixel.components.r = 255u * p;
        pixel.components.g = 255u * p;
        pixel.components.b = 255u * p;
    });
    image_name = "noise_image_worley.ppm";
}

TEST(NoiseTest, SeedGenerator3D) {
    for (size_t i = 0U; i < 10'000; i++) {
        noise::vector3 seed = noise::generate_seed3();
        auto m = seed.magnitude();
        ASSERT_PRECISION_EQ(1.0_p, m) << "seed " << i;
    }
}

TEST(NoiseTest, ConvertToSeed3D) {
    noise::vector3 x = noise::convert_to_seed3(iso::radians{0.0_p}, iso::radians{0.0_p});
    ASSERT_PRECISION_EQ(1.0_p, x.x());
    ASSERT_PRECISION_EQ(0.0_p, x.y());
    ASSERT_PRECISION_EQ(0.0_p, x.z());
    ASSERT_PRECISION_EQ(1.0_p, x.magnitude());

    // straight up should be purely +Z
    noise::vector3 up = noise::convert_to_seed3(iso::radians{0.0_p}, iso::radians{iso::pi / 2.0_p});
    ASSERT_PRECISION_EQ(1.0_p, up.z());
    ASSERT_PRECISION_EQ(1.0_p, up.magnitude());
}

TEST(NoiseTest, Perlin3DRange) {
    noise::vector3 seed = noise::generate_seed3();
    constexpr noise::precision scale = 32.0_p;
    constexpr noise::precision gain = 245.4993546_p;
    for (size_t i = 0; i < 200; i++) {
        noise::point3 pnt{(noise::precision)(i % 3), (noise::precision)(i % 5), (noise::precision)(i % 7)};
        noise::point3 scaled{pnt.x() * 100.0_p, pnt.y() * 100.0_p, pnt.z() * 100.0_p};
        noise::precision value = noise::perlin(scaled, scale, seed, gain);
        ASSERT_GE(value, 0.0_p);
        ASSERT_LE(value, 1.0_p) << "iteration " << i;
    }
}

TEST(NoiseTest, Simplex3DRange) {
    noise::vector3 seed = noise::generate_seed3();
    constexpr noise::precision scale = 32.0_p;
    constexpr noise::precision gain = 245.4993546_p;
    for (size_t i = 0; i < 200; i++) {
        noise::point3 pnt{(noise::precision)(i % 5) / 5.0_p, (noise::precision)(i % 7) / 7.0_p,
                          (noise::precision)(i % 11) / 11.0_p};
        noise::point3 scaled{pnt.x() * 100.0_p, pnt.y() * 100.0_p, pnt.z() * 100.0_p};
        noise::precision value = noise::simplex(scaled, scale, seed, gain);
        ASSERT_GE(value, 0.0_p);
        ASSERT_LE(value, 1.0_p) << "iteration " << i;
    }
}

TEST(NoiseTest, Perlin3DContinuity) {
    // Adjacent samples a tiny step apart must stay close (continuous field).
    noise::vector3 seed = noise::generate_seed3();
    constexpr noise::precision scale = 32.0_p;
    constexpr noise::precision gain = 245.4993546_p;
    noise::point3 a{0.1_p, 0.2_p, 0.3_p};
    noise::point3 b{0.1_p + 0.001_p, 0.2_p + 0.001_p, 0.3_p + 0.001_p};
    noise::point3 sa{a.x() * 100.0_p, a.y() * 100.0_p, a.z() * 100.0_p};
    noise::point3 sb{b.x() * 100.0_p, b.y() * 100.0_p, b.z() * 100.0_p};
    noise::precision va = noise::perlin(sa, scale, seed, gain);
    noise::precision vb = noise::perlin(sb, scale, seed, gain);
    ASSERT_LT(std::abs(va - vb), 0.05_p) << "va=" << va << " vb=" << vb;
}

TEST(NoiseTest, Fbm3DMeanUnitRange) {
    noise::vector3 seed = noise::generate_seed3();
    noise::precision sum = 0.0_p;
    size_t count = 0;
    constexpr size_t samples_per_axis = 24;
    for (size_t z = 0; z < samples_per_axis; z++) {
        for (size_t y = 0; y < samples_per_axis; y++) {
            for (size_t x = 0; x < samples_per_axis; x++) {
                noise::point3 pnt{(noise::precision)x / samples_per_axis, (noise::precision)y / samples_per_axis,
                                  (noise::precision)z / samples_per_axis};
                noise::precision value = noise::fractal_brownian(pnt, seed, 6, 2.0_p, 0.5_p, 0.5_p, 1.0_p);
                sum += value;
                count++;
            }
        }
    }
    noise::precision mean = sum / count;
    ASSERT_GT(mean, 0.25_p) << "mean=" << mean;
    ASSERT_LT(mean, 0.75_p) << "mean=" << mean;
    (void)count;
}

TEST_F(NoiseImageTest, Fbm3DSlices) {
    // Renders three constant-z slices (near bottom, middle, near top) of the 3D
    // FBM density so the evolving structures along the z-axis are visible.
    noise::vector3 seed = noise::generate_seed3();
    constexpr size_t tile = 256;
    constexpr size_t dim = tile * 3;
    fourcc::image<fourcc::PixelFormat::RGB8> slice{tile, dim};
    noise::precision zvals[3] = {0.1_p, 0.5_p, 0.9_p};
    for (size_t t = 0; t < 3; t++) {
        noise::precision z = zvals[t];
        for (size_t y = 0; y < tile; y++) {
            for (size_t x = 0; x < tile; x++) {
                noise::point3 pnt{(noise::precision)x / tile, (noise::precision)y / tile, z};
                noise::precision value = noise::fractal_brownian(pnt, seed, 6, 2.0_p, 0.5_p, 0.5_p, 1.0_p);
                uint8_t grey = static_cast<uint8_t>(std::round(value * 255.0_p));
                fourcc::rgb8& pixel = slice.at(y, t * tile + x);
                pixel.components.r = grey;
                pixel.components.g = grey;
                pixel.components.b = grey;
            }
        }
    }
    slice.save("noise_image_fbm3_slices.ppm");
}
