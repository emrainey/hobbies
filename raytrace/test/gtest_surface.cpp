#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(SurfaceTest, ShinySurface) {
    using namespace raytrace;
    mediums::plain shiny(colors::white, 0.1, colors::white, 0.7, 250.0);
    ASSERT_TRUE(shiny.diffuse(R3::origin) == colors::white);
    ASSERT_TRUE(shiny.specular(R3::origin, 1.0, colors::white) == colors::white);
    element_type emitted = 0.0;
    element_type reflected = 0.0;
    element_type transmitted = 0.0;
    iso::radians incident_angle(0.0);
    iso::radians transmitted_angle(0.0);
    shiny.radiosity(R3::origin, 1.0, incident_angle, transmitted_angle, emitted, reflected, transmitted);
    ASSERT_DOUBLE_EQ(1.0, reflected);
    ASSERT_DOUBLE_EQ(0.0, transmitted);
    ASSERT_DOUBLE_EQ(0.7, shiny.smoothness(R3::origin));
    ASSERT_DOUBLE_EQ(250.0f, shiny.specular_tightness(R3::origin));
}

TEST(SurfaceTest, CheckboardDifuse) {
    using namespace raytrace;
    image img(480, 480);
    raytrace::mediums::checkerboard board(6.0, colors::red, colors::green);
    img.generate_each([&](const image::point& p1) {
        raytrace::point p2(p1.x / img.width, p1.y / img.height, 0.0);
        return board.diffuse(p2);
    });
    img.save("checkerboard_diffuse.ppm");
}

TEST(SurfaceTest, MarbleWeirdSurface) {
    using namespace raytrace;
    image img(512, 512);
    mediums::perlin weird(13 * iso::pi / 19, 0.032, 72.9828302, colors::black, colors::white);
    img.generate_each([&](const image::point& p1) {
        raytrace::point p2(p1.x / img.width, p1.y / img.height, 0.0);
        return weird.diffuse(p2);
    });
    img.save("marble_weird.ppm");
}

TEST(FunctionTest, CheckboardFunction) {
    using namespace raytrace;
    image img(480, 480);
    palette pal = {colors::blue, colors::yellow};
    element_type s = 10.0;
    img.generate_each([&](const image::point& p1) {
        // using one divisor will make it even
        image::point p2(s * p1.x / img.width, s * p1.y / img.height);
        return functions::checkerboard(p2, pal);
    });
    img.save("checkerboard_function.ppm");
}

TEST(FunctionTest, Grid) {
    using namespace raytrace;
    image img(480, 480);
    palette pal = {colors::black, colors::green};
    element_type s = 10.0;
    img.generate_each([&](const image::point& p1) {
        // using one divisor will make it even
        image::point p2(s * p1.x / img.height, s * p1.y / img.height);
        return functions::grid(p2, pal);
    });
    img.save("grid.ppm");
}

TEST(FunctionTest, Polka) {
    using namespace raytrace;
    image img(480, 480);
    element_type s = 10.0;
    palette pal = {colors::cyan, colors::magenta};
    img.generate_each([&](const image::point& p1) {
        image::point p2(s * p1.x / img.width, s * p1.y / img.height);
        return functions::dots(p2, pal);
    });
    img.save("polka.ppm");
}

TEST(FunctionTest, Diagonal) {
    using namespace raytrace;
    image img(480, 480);
    element_type s = 10.0;
    palette pal = {colors::red, colors::white};
    img.generate_each([&](const image::point& p1) {
        image::point p2(s * p1.x / img.width, s * p1.y / img.height);
        return functions::diagonal(p2, pal);
    });
    img.save("diagonal.ppm");
}

TEST(FunctionTest, RandomNoise) {
    using namespace raytrace;
    image img(480, 480);
    element_type s = 1.0;
    palette pal;  // not really used
    img.generate_each([&](const image::point& p1) {
        image::point p2(s * p1.x / img.width, s * p1.y / img.height);
        return functions::pseudo_random_noise(p2, pal);
    });
    img.save("pseudo_random_noise.ppm");
}

TEST(VolumetricTest, DISABLED_Checkerboard) {
    using namespace raytrace;
    size_t pixels = 24;
    palette pal = {colors::cyan, colors::magenta};
    image img(pixels, pixels);
    for (size_t i = 0; i < pixels; i++) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "volumetric_checkers_%03zu.ppm", i);
        img.generate_each([&](const image::point& p1) {
            element_type u = 2.0 * (p1.x / img.width) - 1.0;
            element_type v = 2.0 * (p1.y / img.height) - 1.0;
            element_type w = 2.0 * ((double)i / pixels) - 1.0;
            raytrace::point p2(u, v, w);
            return functions::checkerboard(p2, pal);
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
        img.generate_each([&](const image::point& p1) {
            element_type u = 2.0 * (p1.x / img.width) - 1.0;
            element_type v = 2.0 * (p1.y / img.height) - 1.0;
            element_type w = 2.0 * ((double)i / pixels) - 1.0;
            raytrace::point p2(u, v, w);
            return functions::dots(p2, pal);
        });
        img.save(buffer);
    }
}

TEST(VolumetricTest, DISABLED_Grid) {
    using namespace raytrace;
    size_t pixels = 24;
    palette pal = {colors::black, colors::white};
    image img(pixels, pixels);
    for (size_t i = 0; i < pixels; i++) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "volumetric_grid_%03zu.ppm", i);
        img.generate_each([&](const image::point& p1) {
            element_type u = 2.0 * (p1.x / img.width) - 1.0;
            element_type v = 2.0 * (p1.y / img.height) - 1.0;
            element_type w = 2.0 * ((double)i / pixels) - 1.0;
            raytrace::point p2(u, v, w);
            return functions::grid(p2, pal);
        });
        img.save(buffer);
    }
}