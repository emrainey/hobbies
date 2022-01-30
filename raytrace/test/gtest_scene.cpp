#include <gtest/gtest.h>
#include <raytrace/raytrace.hpp>
#include <basal/basal.hpp>
#include <vector>

#include "linalg/gtest_helper.hpp"
#include "geometry/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;

TEST(SceneTest, ObjectIntersections) {
    // define a single object
    raytrace::sphere s0(raytrace::point(0,0,0), 2);
    // define a ray
    ray r0(raytrace::point(0,0,5),vector{{0,0,-1}});
    // define the object list
    scene::object_list objects;
    objects.push_back(&s0);
    // find the intersections (this should pass through the center)
    scene::intersect_list list = scene::find_intersections(r0, objects);
    // there must be same as object list
    ASSERT_EQ(objects.size(), list.size());
    // that is a point
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(list[0]));
    // that point is at 0,0,2
    raytrace::point truth(0,0,2);
    ASSERT_POINT_EQ(truth, as_point(list[0]));
}

TEST(SceneTest, NearestObjectIntersections) {
    // define several objects
    raytrace::sphere s0(raytrace::point(0,0,0), 2);
    raytrace::sphere s1(raytrace::point(0,3,0), 2);
    // define a ray
    ray r0(raytrace::point(0, -3, 0), vector{{0, 1, 0}});
    // define the object list
    scene::object_list objects;
    objects.push_back(&s0);
    objects.push_back(&s1);
    // find the intersections
    scene::intersect_list list = scene::find_intersections(r0, objects);
    // there must be 2
    ASSERT_EQ(objects.size(), list.size());
    // there should both be points (non-facing normals removed already)
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(list[0]));
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(list[1]));
    // find the nearest object to the ray
    scene::intersect_set nearest = scene::nearest_object(r0, list, objects);
    ASSERT_DOUBLE_EQ(1, nearest.distance);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(nearest.intersector));
    ASSERT_EQ(&s0, nearest.objptr);
}

TEST(SceneTest, LowResSpheres) {
    // define an object
    raytrace::sphere s0(raytrace::point(4, 0, 0), 0.50);
    raytrace::sphere s1(raytrace::point(4,-2, 0), 0.75);
    raytrace::sphere s2(raytrace::point(4, 2, 0), 0.25);
    iso::degrees fov(55);
    // tiny image, simple camera placement
    scene scene(120, 180, fov);
    raytrace::point look_from(-1, 0, 0);
    raytrace::point look_at(0, 0, 0);
    scene.view.move_to(look_from, look_at);

    ASSERT_POINT_EQ(look_from, scene.view.position());
    ASSERT_VECTOR_EQ(R3::basis::X, scene.view.forward().direction().normalized());
    ASSERT_VECTOR_EQ(R3::basis::Z, scene.view.up().direction().normalized());

    scene.add_object(&s0);
    scene.add_object(&s1);
    scene.add_object(&s2);

    scene.render("low_res_sphere.ppm");
}

TEST(SceneTest, SurfaceColor) {
    raytrace::point center(2, 2, 2);
    raytrace::sphere s0(center, 2.0);
    plain surf0(colors::black, 0.0, colors::white, 0.7, 100);
    s0.material(&surf0);

    raytrace::point look_at(0, 2, 2);
    raytrace::point look_from(-1, 0, 2);
    vector look(look_at - look_from);

    scene::object_list objects;
    objects.push_back(&s0);

    raytrace::point surface_point = s0.position() + vector{{-s0.radius, 0, 0}};
    ASSERT_POINT_EQ(look_at, surface_point);

    raytrace::point P0(-1, 2, 2);
    raytrace::point P1(5, 2, 2);

    // the light is 1 away from the point
    speck l0(P0, colors::red, 100);
    // the light is behind the sphere
    speck l1(P1, colors::blue, 100);
    l0.print("Light0");
    l1.print("Light1");
    scene::light_list lights;
    lights.push_back(&l0);
    lights.push_back(&l1);

    raytrace::vector surface_normal = s0.normal(surface_point);
    raytrace::vector reflection = s0.reflection(look, surface_point);
    raytrace::point object_surface_point = s0.reverse_transform(surface_point);
    image::point texture_point = s0.map(object_surface_point);
    color surface_color = scene::find_surface_color(s0, surface_point, surface_normal, object_surface_point, texture_point, reflection, objects, lights);
    std::cout << "Surface Color: " << surface_color << std::endl;
    // only red should exist, no blue
    EXPECT_LT(0.5, surface_color.red());
    EXPECT_NEAR(s0.material().ambient(texture_point).green(), surface_color.green(), 0.01);
    EXPECT_NEAR(s0.material().ambient(texture_point).blue(), surface_color.blue(), 0.01);

    // try the point on the other side
    surface_point = s0.position() + vector{{s0.radius, 0, 0}};
    surface_normal = s0.normal(surface_point);
    look = vector{{-1, 1, 0}};
    reflection = s0.reflection(look, surface_point);
    surface_color = scene::find_surface_color(s0, surface_point, surface_normal, object_surface_point, texture_point, reflection, objects, lights);
    std::cout << "Surface Color: " << surface_color << std::endl;
    EXPECT_NEAR(s0.material().ambient(texture_point).red(), surface_color.red(), 0.01);
    EXPECT_NEAR(s0.material().ambient(texture_point).green(), surface_color.green(), 0.01);
    EXPECT_LT(0.5, surface_color.blue());

    // try the point in between
    surface_point = s0.position() + vector{{0, -s0.radius, 0}};
    surface_normal = s0.normal(surface_point);
    look = vector{{0, 1, 0}}; // look head on to the sphere
    reflection = s0.reflection(look, surface_point);
    surface_color = scene::find_surface_color(s0, surface_point, surface_normal, object_surface_point, texture_point, reflection, objects, lights);
    std::cout << "Surface Color: " << surface_color << std::endl;
    EXPECT_NEAR(s0.material().ambient(texture_point).red(), surface_color.red(), 0.01);
    EXPECT_NEAR(s0.material().ambient(texture_point).green(), surface_color.green(), 0.01);
    EXPECT_NEAR(s0.material().ambient(texture_point).blue(), surface_color.blue(), 0.01);
}
