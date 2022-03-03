
#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(RingTest, RayIntersection) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C(0, 0, 0);
    vector N{{0, 0, 1}};
    ring R(C, N, 0.1, 10.0);

    ray r0(raytrace::point(0.5, 0.5, 1), vector{{0, 0, -1}});

    geometry::intersection ir0T = R.intersect(r0);
    raytrace::point P(0.5, 0.5, 0);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir0T));
    ASSERT_POINT_EQ(P, as_point(ir0T));

    ray r1(raytrace::point(0.0, 0.0, 1), vector{{0, 0, -1}});
    geometry::intersection ir1T = R.intersect(r1);
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir1T));

    ray r2(raytrace::point(5.6, 4.2, 1), vector{{0, 0, -1}});
    geometry::intersection ir2T = R.intersect(r2);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir2T));
    raytrace::point P2(5.6, 4.2, 0);
    ASSERT_POINT_EQ(P2, as_point(ir2T));

    // Since we're colliding with the back-side there shouldn't be an intersection.
    ray r3(raytrace::point(3, -4, -1), vector{{0, 0, 1}});
    geometry::intersection ir3T = R.intersect(r3);
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir3T));
}
