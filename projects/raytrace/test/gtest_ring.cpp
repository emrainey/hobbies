
#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(RingTest, Type) {
    using namespace raytrace;
    using namespace raytrace::objects;

    ring R{1.0_p, 2.0_p};
    ASSERT_EQ(R.get_type(), Type::Ring);
}

TEST(RingTest, RayIntersectionAtOrigin) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{0, 0, 0};
    vector N{{0, 0, 1}};
    ring R{0.1_p, 10.0_p};

    ray r0{raytrace::point{0.5_p, 0.5_p, 1}, vector{{0, 0, -1}}};

    geometry::intersection ir0T = R.intersect(r0).intersect;
    raytrace::point P{0.5_p, 0.5_p, 0};
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir0T));
    ASSERT_POINT_EQ(P, as_point(ir0T));

    ray r1{raytrace::point{0.0_p, 0.0_p, 1}, vector{{0, 0, -1}}};
    geometry::intersection ir1T = R.intersect(r1).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir1T));

    ray r2{raytrace::point{5.6_p, 4.2_p, 1}, vector{{0, 0, -1}}};
    geometry::intersection ir2T = R.intersect(r2).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir2T));
    raytrace::point P2{5.6_p, 4.2_p, 0};
    ASSERT_POINT_EQ(P2, as_point(ir2T));

    // In plane, but not colliding I think
    // Since we're colliding with the back-side there shouldn't be an intersection.
    ray r3{raytrace::point{3, -4, -1}, vector{{0, 0, 1}}};
    geometry::intersection ir3T = R.intersect(r3).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir3T));
}

TEST(RingTest, RayIntersectionAwayFromOrigin) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{60, -50, 42};
    vector N{R3::basis::X};
    ring R{C, R3::pitch(0.25), 4.0_p, 10.0_p};
    ASSERT_POINT_EQ(C, R.position());
    ASSERT_VECTOR_EQ(N, R.normal(C));

    ray r0{raytrace::point{70, -55, 42}, -R3::basis::X};
    geometry::intersection ir0T = R.intersect(r0).intersect;
    raytrace::point P{60, -55, 42};
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir0T));
    ASSERT_POINT_EQ(P, as_point(ir0T));

    ray r1{C, -R3::basis::X};
    geometry::intersection ir1T = R.intersect(r1).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir1T));

    ray r2{raytrace::point{70, -50, 47}, -R3::basis::X};
    geometry::intersection ir2T = R.intersect(r2).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir2T));
    raytrace::point P2{60, -50, 47};
    ASSERT_POINT_EQ(P2, as_point(ir2T));

    // Since we're colliding with the back-side there shouldn't be an intersection.
    ray r3{raytrace::point{40, -42, 42}, R3::basis::X};
    geometry::intersection ir3T = R.intersect(r3).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir3T));
}
