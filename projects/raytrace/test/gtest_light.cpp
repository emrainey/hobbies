#include <gtest/gtest.h>

#include <raytrace/raytrace.hpp>

#include "raytrace/gtest_helper.hpp"

using namespace raytrace;
using namespace raytrace::colors;
using namespace raytrace::operators;

TEST(LightTest, SpeckLightColor) {
    raytrace::point source(0.0, 0.0, 4.0);
    raytrace::point surface_point(0.0, 0.0, 0.0);
    vector diff = source - surface_point;

    lights::speck light0(source, colors::white, 1.0);

    // Specks obey the inverse square law
    precision scaling = 1.0 / diff.quadrance();
    ASSERT_FLOAT_EQ(scaling, light0.intensity_at(surface_point));

    color C2{scaling, scaling, scaling};
    ASSERT_TRUE(C2 == light0.color_at(surface_point));
}

TEST(LightTest, BeamLightColor) {
    raytrace::vector down = -geometry::R3::basis::Z;

    raytrace::point p0{0, 0, 0};
    raytrace::point p1{0, 0, -10};

    lights::beam light0(down, colors::white, 1E9);

    ASSERT_FLOAT_EQ(1.0, light0.intensity_at(p0));
    ASSERT_FLOAT_EQ(1.0, light0.intensity_at(p1));

    ASSERT_VECTOR_EQ(geometry::R3::basis::Z, light0.incident(p0, 0).direction().normalized());
}

TEST(LightTest, DISABLED_TriColorSpots) {
    raytrace::objects::sphere shape(raytrace::point{0, 0, 3}, 3);
    raytrace::objects::plane floor(raytrace::point{0, 0, 0}, R3::basis::Z, 1.0);
    raytrace::point p0{10, 0, 50};
    raytrace::matrix rot120 = rotation(R3::basis::Z, iso::radians(2 * iso::pi / 3));
    raytrace::point p1 = rot120 * p0;
    raytrace::point p2 = rot120 * p1;
    raytrace::lights::spot red_light(raytrace::ray(p0, -R3::basis::Z), colors::red, 1E5, iso::degrees(25));
    raytrace::lights::spot grn_light(raytrace::ray(p1, -R3::basis::Z), colors::green, 1E5, iso::degrees(25));
    raytrace::lights::spot blu_light(raytrace::ray(p2, -R3::basis::Z), colors::blue, 1E5, iso::degrees(25));
    raytrace::scene scene;
    raytrace::camera view(480, 640, iso::degrees(55));
    view.move_to(raytrace::point{60, 60, 60}, raytrace::point{59, 59, 59});
    scene.add_light(&red_light);
    scene.add_light(&grn_light);
    scene.add_light(&blu_light);
    scene.add_object(&floor);
    scene.add_object(&shape);
    scene.print("TriColor");
    scene.render(view, "tricolor_spots.ppm", 1, 1, std::nullopt, true);
}

TEST(LightTest, DISABLED_BulbTest) {
    raytrace::objects::sphere shape(raytrace::point{0, 0, 3}, 3);
    raytrace::objects::plane floor(raytrace::point{0, 0, 0}, R3::basis::Z, 1.0);
    raytrace::lights::bulb light(raytrace::point{0, 0, 9}, 0.2, colors::white, 1E11, 100);
    raytrace::scene scene;
    raytrace::camera view(480, 640, iso::degrees(55));
    view.move_to(raytrace::point{30, 30, 30}, raytrace::point{29, 29, 29});
    scene.add_light(&light);
    scene.add_object(&floor);
    scene.add_object(&shape);
    scene.print("BulbTest");
    scene.render(view, "bulb_and_sphere.ppm", 1, 1, std::nullopt, true);
}