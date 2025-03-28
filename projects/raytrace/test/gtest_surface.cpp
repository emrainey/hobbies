#include "basal/gtest_helper.hpp"

#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(SurfaceTest, ShinySurface) {
    using namespace raytrace;
    mediums::plain shiny(colors::white, 0.1_p, colors::white, 0.7_p, 250.0_p);
    ASSERT_TRUE(shiny.diffuse(R3::origin) == colors::white);
    ASSERT_TRUE(shiny.specular(R3::origin, 1.0_p, colors::white) == colors::white);
    precision emitted = 0.0_p;
    precision reflected = 0.0_p;
    precision transmitted = 0.0_p;
    iso::radians incident_angle(0.0_p);
    iso::radians transmitted_angle(0.0_p);
    shiny.radiosity(R3::origin, 1.0_p, incident_angle, transmitted_angle, emitted, reflected, transmitted);
    ASSERT_PRECISION_EQ(1.0_p, reflected);
    ASSERT_PRECISION_EQ(0.0_p, transmitted);
    ASSERT_PRECISION_EQ(0.7_p, shiny.smoothness(R3::origin));
    ASSERT_PRECISION_EQ(250.0_p, shiny.specular_tightness(R3::origin));
}

TEST(SurfaceTest, CheckerboardDiffuse) {
    using namespace raytrace;
    image img(480, 480);
    raytrace::mediums::checkerboard board(6.0_p, colors::red, colors::green);
    img.generate_each([&](image::point const& p1) {
        raytrace::point p3{p1.x / img.width, p1.y / img.height, 0.0_p};
        return board.diffuse(p3);
    });
    img.save("checkerboard_diffuse.ppm");
}

TEST(SurfaceTest, MarbleWeirdSurface) {
    using namespace raytrace;
    image img(512, 512);
    mediums::perlin weird(13 * iso::pi / 19, 0.032_p, 72.9828302_p, colors::black, colors::white);
    img.generate_each([&](image::point const& p1) {
        raytrace::point p3{p1.x / img.width, p1.y / img.height, 0.0_p};
        return weird.diffuse(p3);
    });
    img.save("marble_weird.ppm");
}

TEST(FunctionTest, CheckerboardFunction) {
    using namespace raytrace;
    image img(480, 480);
    palette pal = {colors::blue, colors::yellow, colors::blue, colors::yellow,
                   colors::blue, colors::yellow, colors::blue, colors::yellow};
    precision s = 10.0_p;
    img.generate_each([&](image::point const& p1) {
        // using one divisor will make it even
        image::point p2{s * p1.x / img.width, s * p1.y / img.height};
        return functions::checkerboard(p2, pal);
    });
    img.save("checkerboard_function.ppm");
}

TEST(FunctionTest, Grid) {
    using namespace raytrace;
    image img(480, 480);
    palette pal = {colors::black, colors::green};
    precision s = 10.0_p;
    img.generate_each([&](image::point const& p1) {
        // using one divisor will make it even
        image::point p2{s * p1.x / img.height, s * p1.y / img.height};
        return functions::grid(p2, pal);
    });
    img.save("grid.ppm");
}

TEST(FunctionTest, Polka) {
    using namespace raytrace;
    image img(480, 480);
    precision s = 10.0_p;
    palette pal = {colors::cyan, colors::magenta};
    img.generate_each([&](image::point const& p1) {
        image::point p2{s * p1.x / img.width, s * p1.y / img.height};
        return functions::dots(p2, pal);
    });
    img.save("polka.ppm");
}

TEST(FunctionTest, Diagonal) {
    using namespace raytrace;
    image img(480, 480);
    precision s = 10.0_p;
    palette pal = {colors::red, colors::white};
    img.generate_each([&](image::point const& p1) {
        image::point p2{s * p1.x / img.width, s * p1.y / img.height};
        return functions::diagonal(p2, pal);
    });
    img.save("diagonal.ppm");
}

TEST(FunctionTest, RandomNoise) {
    using namespace raytrace;
    image img(480, 480);
    raytrace::tuning::pseudo_random_noise_params.initialize();
    raytrace::tuning::pseudo_random_noise_params.radius = 137.0_p;
    raytrace::tuning::pseudo_random_noise_params.gain = 3711.125_p;
    raytrace::tuning::pseudo_random_noise_params.update();
    precision s = 2.0_p;
    palette pal;  // not really used
    img.generate_each([&](image::point const& p1) -> raytrace::color {
        image::point uv{s * ((p1.x / img.width) - 0.5_p), s * ((p1.y / img.height) - 0.5_p)};
        return functions::pseudo_random_noise(uv, pal);
        // return colors::red;
    });
    img.save("pseudo_random_noise.ppm");
}

TEST(FunctionTest, HappyFace) {
    using namespace raytrace;
    image img{480, 640};
    palette pal = {colors::black, colors::yellow};
    img.generate_each([&](image::point const& p1) {
        image::point uv{p1.x / img.width, p1.y / img.height};
        return functions::happy_face(uv, pal);
    });
    img.save("happy_face.ppm");
}

TEST(VolumetricTest, DISABLED_Checkerboard) {
    using namespace raytrace;
    size_t pixels = 24;
    palette pal = {colors::cyan, colors::magenta};
    image img(pixels, pixels);
    for (size_t i = 0; i < pixels; i++) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "volumetric_checkers_%03zu.ppm", i);
        img.generate_each([&](image::point const& p1) {
            precision u = 2.0_p * (p1.x / img.width) - 1.0_p;
            precision v = 2.0_p * (p1.y / img.height) - 1.0_p;
            precision w = 2.0_p * ((precision)i / pixels) - 1.0_p;
            raytrace::point p3{u, v, w};
            return functions::checkerboard(p3, pal);
        });
        img.save(buffer);
    }
}

TEST(VolumetricTest, DISABLED_Dots) {
    using namespace raytrace;
    size_t pixels = 24;
    palette pal = {colors::black, colors::white};
    image img(pixels, pixels);
    for (size_t i = 0; i < pixels; i++) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "volumetric_dots_%03zu.ppm", i);
        img.generate_each([&](image::point const& p1) {
            precision u = 2.0_p * (p1.x / img.width) - 1.0_p;
            precision v = 2.0_p * (p1.y / img.height) - 1.0_p;
            precision w = 2.0_p * ((precision)i / pixels) - 1.0_p;
            raytrace::point p3{u, v, w};
            return functions::dots(p3, pal);
        });
        img.save(buffer);
    }
}

TEST(VolumetricTest, DISABLED_Grid) {
    using namespace raytrace;
    size_t pixels = 24;
    palette pal = {colors::black, colors::white};
    image img{pixels, pixels};
    for (size_t i = 0; i < pixels; i++) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "volumetric_grid_%03zu.ppm", i);
        img.generate_each([&](image::point const& p1) {
            precision u = 2.0_p * (p1.x / img.width) - 1.0_p;
            precision v = 2.0_p * (p1.y / img.height) - 1.0_p;
            precision w = 2.0_p * ((precision)i / pixels) - 1.0_p;
            raytrace::point p3{u, v, w};
            return functions::grid(p3, pal);
        });
        img.save(buffer);
    }
}