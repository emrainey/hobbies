#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <iostream>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(PlaneTest, Position) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{14, -18, -1};
    raytrace::objects::plane PL{C, R3::basis::Z};

    ASSERT_POINT_EQ(C, PL.position());
    raytrace::point D{-3, -3, 19};
    raytrace::vector V = D - C;
    PL.move_by(V);
    ASSERT_POINT_EQ(D, PL.position());
}

TEST(PlaneTest, Intersections) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{0, 0, -1};
    raytrace::objects::plane PL{C, R3::basis::Z};

    raytrace::ray r0{raytrace::point{1, 1, 1}, vector{{1, 1, 1}}};
    raytrace::ray r1{raytrace::point{1, 1, 1}, vector{{-1, -1, -1}}};
    raytrace::point A{-1, -1, -1};
    ray r2{C, R3::basis::Z};
    ray r3{C, -R3::basis::Z};

    geometry::intersection ir0PL = PL.intersect(r0);
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir0PL));

    geometry::intersection ir1PL = PL.intersect(r1);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir1PL));
    ASSERT_POINT_EQ(A, as_point(ir1PL));

    geometry::intersection ir2PL = PL.intersect(r2);
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir2PL));

    // Surface point intersection.
    geometry::intersection ir3PL = PL.intersect(r3);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir3PL));
    ASSERT_POINT_EQ(C, as_point(ir3PL));
}

TEST(PlaneTest, SandwichRays) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::objects::plane P0{raytrace::point{0, 0, 10}, vector{{0, 0, -1}}};
    raytrace::objects::plane P1{raytrace::point{0, 0, -10}, vector{{0, 0, 1}}};
    ray r0{raytrace::point{0, 0, 0}, vector{{1, 0, 1}}};
    ray r1{raytrace::point{0, 0, 0}, vector{{1, 0, -1}}};

    raytrace::point p0{10, 0, 10};
    raytrace::point p1{10, 0, -10};

    geometry::intersection ir0P0 = P0.intersect(r0);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir0P0));
    ASSERT_POINT_EQ(p0, as_point(ir0P0));

    geometry::intersection ir0P1 = P1.intersect(r0);
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir0P1));

    geometry::intersection ir1P0 = P0.intersect(r1);
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir1P0));

    geometry::intersection ir1P1 = P1.intersect(r1);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir1P1));
    ASSERT_POINT_EQ(p1, as_point(ir1P1));
}

TEST(PlaneTest, ScaleRotateTranslate) {
    using namespace raytrace;
    using namespace raytrace::objects;
    raytrace::point C{7, 7, 7};
    raytrace::objects::plane P0{C, geometry::R3::basis::X};
    P0.rotation(iso::degrees{0.0}, iso::degrees{0.0}, iso::degrees{180.0});
    vector const& should_be_negative_X = P0.normal(C);
    vector const negative_X = -geometry::R3::basis::X;
    ASSERT_EQ(negative_X.dimensions, 3ul);
    ASSERT_VECTOR_EQ(negative_X, should_be_negative_X);
    {
        raytrace::ray const world_ray{geometry::R3::origin, geometry::R3::basis::X};
        auto inter = P0.intersect(world_ray);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(inter));
        raytrace::point const P{7, 0, 0};
        ASSERT_POINT_EQ(P, as_point(inter));
    }

    // translate it back a bit
    raytrace::point const new_C{12, 12, 12};
    P0.position(new_C);

}