#include <gtest/gtest.h>

#include <raytrace/raytrace.hpp>
#include "raytrace/gtest_helper.hpp"

using namespace geometry;
using namespace geometry::operators;
using namespace raytrace;
using namespace raytrace::colors;
using namespace raytrace::operators;

TEST(LightTest, SpeckLightColor) {
    raytrace::point source(0.0_p, 0.0_p, 0.0_p);
    raytrace::point surface_point(0.0_p, 0.0_p, 0.0_p);
    vector diff = source - surface_point;
    lights::speck light0(source, colors::white, 1.0_p);
    ASSERT_FLOAT_EQ(1.0_p, light0.intensity_at(surface_point));
    ASSERT_TRUE(colors::white == light0.color_at(surface_point));
}

TEST(LightTest, InverseSquareLawRange) {
    for (precision d = 0.0_p; d < 10.0_p; d += 0.0625_p) {
        raytrace::point source(0.0_p, 0.0_p, d);
        raytrace::point surface_point(0.0_p, 0.0_p, 0.0_p);
        vector diff = source - surface_point;

        lights::speck light0(source, colors::white, 1.0_p);

        // Specks obey some form of fade which drops off as distance increases
        precision intensity = light0.intensity_at(surface_point);
        ASSERT_GE(1.0_p, intensity);
        ASSERT_GE(1.0_p / d, intensity) << "The simple distance law is not obeyed";
        ASSERT_LE(0.0_p, intensity);
    }
}

TEST(LightTest, BeamLightColor) {
    raytrace::vector down = -geometry::R3::basis::Z;

    raytrace::point p0{0, 0, 0};
    raytrace::point p1{0, 0, -10};
    precision intensity = 1E9;
    lights::beam light0(down, colors::white, intensity);

    ASSERT_FLOAT_EQ(intensity, light0.intensity_at(p0));
    ASSERT_FLOAT_EQ(intensity, light0.intensity_at(p1));

    ASSERT_VECTOR_EQ(geometry::R3::basis::Z, light0.incident(p0, 0).direction().normalized());
}

TEST(LightTest, DISABLED_TriColorSpots) {
    raytrace::objects::sphere shape(raytrace::point{0, 0, 3}, 3);
    raytrace::objects::plane floor;
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
    scene.print(std::cout, "TriColor");
    scene.render(view, "tricolor_spots.ppm", 1, 1, std::nullopt, true);
}

TEST(LightTest, DISABLED_BulbTest) {
    raytrace::objects::sphere shape(raytrace::point{0, 0, 3}, 3);
    raytrace::objects::plane floor;
    raytrace::lights::bulb light(raytrace::point{0, 0, 9}, 0.2_p, colors::white, lights::intensities::intense, 16);
    raytrace::scene scene;
    raytrace::camera view(480, 640, iso::degrees(55));
    view.move_to(raytrace::point{30, 30, 30}, raytrace::point{29, 29, 29});
    scene.add_light(&light);
    scene.add_object(&floor);
    scene.add_object(&shape);
    scene.print(std::cout, "BulbTest");
    scene.render(view, "bulb_and_sphere.ppm", 1, 1, std::nullopt, true);
}

TEST(LightTest, EmittedSpeckLight) {
    raytrace::point source(0.0_p, 0.0_p, 0.0_p);
    lights::speck light0(source, colors::white, 1.0_p);

    const size_t num_samples = 1000u;
    size_t hits_origin = 0u;
    for (size_t i = 0u; i < num_samples; ++i) {
        ray emitted = light0.emit();
        // Check if the emitted ray is going away from the origin
        vector dir = emitted.direction().normalized();
        ASSERT_PRECISION_EQ(1.0_p, emitted.direction().magnitude());  // Should be normalized
        ASSERT_VECTOR_NE(R3::null, dir);             // Should not be null
        ASSERT_POINT_EQ(source, emitted.location());
    }
}

TEST(LightTest, EmittedBulbLight) {
    raytrace::point source(0.0_p, 0.0_p, 0.0_p);
    lights::bulb light0(source, 0.2_p, colors::white, 1.0_p, 16);

    const size_t num_samples = 1000u;
    size_t hits_origin = 0u;
    for (size_t i = 0u; i < num_samples; ++i) {
        ray emitted = light0.emit();
        // Check if the emitted ray is going away from the origin
        vector dir = emitted.direction().normalized();
        ASSERT_PRECISION_EQ(1.0_p, emitted.direction().magnitude());  // Should be normalized
        ASSERT_VECTOR_NE(R3::null, dir);             // Should not be null
        ASSERT_POINT_EQ(source, emitted.location());
    }
}

TEST(LightTest, EmittedSpotLight) {
    raytrace::point source(0.0_p, 0.0_p, 0.0_p);
    lights::spot light0(raytrace::ray(source, R3::basis::Z), colors::white, 1.0_p, iso::degrees(25));

    const size_t num_samples = 1000u;
    size_t hits_origin = 0u;
    for (size_t i = 0u; i < num_samples; ++i) {
        ray emitted = light0.emit();
        // ensure the source is correct
        ASSERT_POINT_EQ(source, emitted.location());

        // ensure the angle is within the cone
        iso::radians angle_rad = angle(emitted.direction(), R3::basis::Z);
        iso::degrees angle_deg;
        // printf("Emitted ray direction: %lf, %lf, %lf\n",
        //         emitted.direction()[0],
        //         emitted.direction()[1],
        //         emitted.direction()[2]);
        iso::convert(angle_deg, angle_rad);
        ASSERT_LE(angle_deg, iso::degrees(25));
    }
}