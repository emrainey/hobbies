#include <gtest/gtest.h>
#include <raytrace/raytrace.hpp>
#include <basal/basal.hpp>
#include <vector>

#include "linalg/gtest_helper.hpp"
#include "geometry/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(SceneTest, ObjectIntersections) {
    using namespace raytrace;
    using namespace raytrace::objects;

    // define a single object
    raytrace::objects::sphere s0(raytrace::point(0,0,0), 2);
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
    using namespace raytrace;
    using namespace raytrace::objects;

    // define several objects
    raytrace::objects::sphere s0(raytrace::point(0,0,0), 2);
    raytrace::objects::sphere s1(raytrace::point(0,3,0), 2);
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
    using namespace raytrace;
    using namespace raytrace::objects;

    // define an object
    raytrace::objects::sphere s0(raytrace::point(4, 0, 0), 0.50);
    raytrace::objects::sphere s1(raytrace::point(4,-2, 0), 0.75);
    raytrace::objects::sphere s2(raytrace::point(4, 2, 0), 0.25);
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

