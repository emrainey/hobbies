
#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;
using namespace raytrace::objects;

TEST(TriangleTest, Basic) {
    raytrace::point A{0.0_p, 0.0_p, 0.0_p};
    raytrace::point B{1.0_p, 1.0_p, 0.0_p};
    raytrace::point C{-1.0_p, 1.0_p, 0.0_p};
    raytrace::objects::triangle shape{A, B, C};
    ASSERT_POINT_EQ(raytrace::point(0.0_p / 3.0_p, 2.0_p / 3.0_p, 0.0_p / 3.0_p), shape.position());
    ASSERT_TRUE(shape.is_contained(raytrace::point{0.0_p, 0.25_p, 0.0_p}));
    auto displacement = raytrace::point{1, 1, 0} - raytrace::point{0, 2.0_p / 3.0_p, 0};
    ASSERT_PRECISION_EQ(displacement.magnitude(), shape.get_object_extent());
    ASSERT_POINT_EQ(raytrace::point(0.0_p / 3.0_p, 2.0_p / 3.0_p, 0.0_p / 3.0_p), shape.position());
}

TEST(TriangleTest, PlaneParallelTriangle) {
    raytrace::point A{0.0_p, 0.0_p, 0.0_p};
    raytrace::point B{1.0_p, 1.0_p, 0.0_p};
    raytrace::point C{-1.0_p, 1.0_p, 0.0_p};
    raytrace::objects::triangle D{A, B, C};
    geometry::plane E = as_plane(D);
    vector F{{0.0_p, 0.0_p, 1.0_p}};  // +Z
    vector G = E.unormal();

    using namespace linalg::operators;
    ASSERT_TRUE(G || F);
}

TEST(TriangleTest, RayIntersection) {
    raytrace::point A{1, 0, 0};
    raytrace::point B{-1, -1, 0};
    raytrace::point C{0, 1, 0};
    raytrace::objects::triangle T{A, B, C};
    ray r0{raytrace::point{0.5_p, 0.5_p, 1}, vector{{0, 0, -1}}};

    geometry::intersection ir0T = T.intersect(r0).intersect;
    raytrace::point P{0.5_p, 0.5_p, 0};
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir0T));
    ASSERT_POINT_EQ(P, as_point(ir0T));

    ray r1{raytrace::point{0.7_p, 0.7_p, 1}, vector{{0, 0, -1}}};
    geometry::intersection ir1T = T.intersect(r1).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir1T));

    ray r2{raytrace::point{0.5_p, 0.5_p, -1}, vector{{0, 0, 1}}};
    geometry::intersection ir2T = T.intersect(r2).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir2T));

    ray r3{raytrace::point{0.7_p, 0.7_p, -1}, vector{{0, 0, 1}}};
    geometry::intersection ir3T = T.intersect(r3).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir3T));
}
