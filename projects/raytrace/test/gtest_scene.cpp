#include "basal/gtest_helper.hpp"

#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(SceneTest, ObjectIntersections) {
    using namespace raytrace;
    using namespace raytrace::objects;

    // define a single object
    raytrace::objects::sphere s0{raytrace::point{0, 0, 0}, 2};
    // define a ray
    ray r0{raytrace::point{0, 0, 5}, vector{{0, 0, -1}}};
    // define the object list
    scene scene;
    scene.add_object(&s0);
    // find the intersections (this should pass through the center)
    objects::hits list = scene.find_intersections(r0);
    // that is a point
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(list[0].intersect));
    // that point is at 0,0,2
    raytrace::point truth(0, 0, 2);
    ASSERT_POINT_EQ(truth, as_point(list[0].intersect));
}

TEST(SceneTest, NearestObjectIntersections) {
    using namespace raytrace;
    using namespace raytrace::objects;

    // define several objects
    raytrace::objects::sphere s0{raytrace::point{0, 0, 0}, 2};
    raytrace::objects::sphere s1{raytrace::point{0, 3, 0}, 2};
    // define a ray
    ray r0{raytrace::point{0, -3, 0}, vector{{0, 1, 0}}};
    // define the object list
    scene scene;
    scene.add_object(&s0);
    scene.add_object(&s1);
    // find the intersections
    objects::hits list = scene.find_intersections(r0);
    // there should both be points (non-facing normals removed already)
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(list[0].intersect));
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(list[1].intersect));
    // find the nearest object to the ray
    objects::hit nearest = scene.nearest_object(r0, list);
    ASSERT_PRECISION_EQ(1, nearest.distance);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(nearest.intersect));
    ASSERT_EQ(&s0, nearest.object);
}

TEST(SceneTest, LowResSpheres) {
    using namespace raytrace;
    using namespace raytrace::objects;

    // define an object
    raytrace::objects::sphere s0{raytrace::point{4, 0, 0}, 0.50_p};
    raytrace::objects::sphere s1{raytrace::point{4, -2, 0}, 0.75_p};
    raytrace::objects::sphere s2{raytrace::point{4, 2, 0}, 0.25_p};
    raytrace::mediums::checkerboard c0{6.0_p, colors::red, colors::green};
    raytrace::lights::beam sunlight{raytrace::vector{0, 0, -1}, raytrace::colors::white, lights::intensities::blinding};
    s0.material(&c0);
    s1.material(&raytrace::mediums::metals::bronze);
    s2.material(&raytrace::mediums::dull);

    iso::degrees fov(55);
    // tiny image, simple camera placement
    scene scene;
    raytrace::camera view(120, 180, fov);
    raytrace::point look_from(-1, 0, 0);
    raytrace::point look_at(0, 0, 0);
    view.move_to(look_from, look_at);

    ASSERT_POINT_EQ(look_from, view.position());
    ASSERT_VECTOR_EQ(R3::basis::X, view.forward().direction().normalized());
    ASSERT_VECTOR_EQ(R3::basis::Z, view.up().direction().normalized());

    scene.add_object(&s0);
    scene.add_object(&s1);
    scene.add_object(&s2);
    scene.add_light(&sunlight);
    scene.render(view, "low_res_sphere.ppm");
}
