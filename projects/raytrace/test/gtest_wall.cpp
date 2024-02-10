#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <iostream>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(WallTest, Intersections) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{0, 0, 0};
    raytrace::objects::wall w0{C, R3::basis::X, 2.0, 1.0};

    raytrace::ray r0{raytrace::point{2, 1, 1}, vector{{-1, 0, 0}}};
    raytrace::ray r1{raytrace::point{-2, 1, 1}, vector{{1, 0, 0}}};
    raytrace::ray r2{raytrace::point{2, 1, 1}, vector{0, 1, 0}};
    raytrace::point A{1, 1, 1};
    raytrace::point B{-1, 1, 1};

    geometry::intersection ir1wall = w0.intersect(r0);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir1wall));
    ASSERT_POINT_EQ(A, as_point(ir1wall));

    geometry::intersection ir2wall = w0.intersect(r1);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir2wall));
    ASSERT_POINT_EQ(B, as_point(ir2wall));

    geometry::intersection ir3wall = w0.intersect(r2);
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir3wall));
}
