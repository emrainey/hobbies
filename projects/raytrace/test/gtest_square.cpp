#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <iostream>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(SquareTest, Type) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::objects::square SQ{20};
    ASSERT_EQ(SQ.get_type(), raytrace::objects::Type::Square);
}

TEST(SquareTest, Position) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{14, -18, -1};
    raytrace::objects::square SQ{C, R3::identity, 20};

    ASSERT_POINT_EQ(C, SQ.position());
    raytrace::point D{-3, -3, 19};
    raytrace::vector V = D - C;
    SQ.move_by(V);
    ASSERT_POINT_EQ(D, SQ.position());
}

TEST(SquareTest, OnSurface) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{0, 0, -1};
    raytrace::objects::square SQ{C, R3::identity, 20};
    ASSERT_TRUE(SQ.is_surface_point(raytrace::point{0, 0, -1}));
    ASSERT_TRUE(SQ.is_surface_point(raytrace::point{1, 1, -1}));
    ASSERT_FALSE(SQ.is_surface_point(raytrace::point{0, 0, 1}));
    ASSERT_FALSE(SQ.is_surface_point(raytrace::point{0, 0, -2}));
}

TEST(SquareTest, Intersections) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{0, 0, -1};
    raytrace::objects::square SQ{C, R3::identity, 20};

    raytrace::ray r0{raytrace::point{1, 1, 1}, vector{{1, 1, 1}}.normalized()};
    raytrace::ray r1{raytrace::point{1, 1, 1}, vector{{-1, -1, -1}}.normalized()};
    raytrace::point A{-1, -1, -1};  // in the plane?
    ray r2{C, R3::basis::Z};
    ray r3{C, -R3::basis::Z};

    geometry::intersection ir0PL = SQ.intersect(r0).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir0PL));

    {
        raytrace::objects::hit h0 = SQ.intersect(r1);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h0.intersect));
        ASSERT_POINT_EQ(A, as_point(h0.intersect));
        ASSERT_VECTOR_EQ(R3::basis::Z, h0.normal);
        ASSERT_PRECISION_EQ(2.0_p * basal::sqrt_3, h0.distance);
        ASSERT_EQ(&SQ, h0.object);  // check that the hit object is the square
    }

    geometry::intersection ir2PL = SQ.intersect(r2).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir2PL));

    // Surface point intersection.
    geometry::intersection ir3PL = SQ.intersect(r3).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir3PL));
    ASSERT_POINT_EQ(C, as_point(ir3PL));
}

TEST(SquareTest, Intersection2) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::objects::square SQ{raytrace::point{0, 5, 5}, R3::roll(0.25), 20};

    {
        raytrace::ray r0{raytrace::point{0, 0, 5}, vector{{0, 1, 0}}.normalized()};
        raytrace::objects::hit h0 = SQ.intersect(r0);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h0.intersect));
        ASSERT_POINT_EQ(raytrace::point(0, 5, 5), as_point(h0.intersect));
        ASSERT_VECTOR_EQ(negation(R3::basis::Y), h0.normal);
        ASSERT_PRECISION_EQ(5.0_p, h0.distance);
        ASSERT_EQ(&SQ, h0.object);  // check that the hit object is the square
    }
    {
        raytrace::ray r0{raytrace::point{-1, 0, 6}, vector{{1, 1, 0}}.normalized()};
        raytrace::objects::hit h0 = SQ.intersect(r0);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h0.intersect));
        ASSERT_POINT_EQ(raytrace::point(4, 5, 6), as_point(h0.intersect));
        ASSERT_VECTOR_EQ(negation(R3::basis::Y), h0.normal);
        ASSERT_PRECISION_EQ(5.0_p * basal::sqrt_2, h0.distance);
        ASSERT_EQ(&SQ, h0.object);  // check that the hit object is the square
    }
}

TEST(SquareTest, Intersection3) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::objects::square SQ{raytrace::point{0, 0, 10}, R3::pitch(-0.5), 20};

    {
        raytrace::ray r0{raytrace::point{0, 0, 5}, R3::basis::Z};
        raytrace::objects::hit h0 = SQ.intersect(r0);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h0.intersect));
        ASSERT_POINT_EQ(raytrace::point(0, 0, 10), as_point(h0.intersect));
        ASSERT_VECTOR_EQ(negation(R3::basis::Z), h0.normal);
        ASSERT_PRECISION_EQ(5.0_p, h0.distance);
        ASSERT_EQ(&SQ, h0.object);  // check that the hit object is the square
    }
    {
        raytrace::ray r0{raytrace::point{-1, 0, 6}, vector{{1, 0, 1}}.normalized()};
        raytrace::objects::hit h0 = SQ.intersect(r0);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h0.intersect));
        ASSERT_POINT_EQ(raytrace::point(3, 0, 10), as_point(h0.intersect));
        ASSERT_VECTOR_EQ(negation(R3::basis::Z), h0.normal);
        ASSERT_PRECISION_EQ(4.0_p * basal::sqrt_2, h0.distance);
        ASSERT_EQ(&SQ, h0.object);  // check that the hit object is the square
    }
}

TEST(SquareTest, SandwichRays) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::objects::square SQ0{raytrace::point{0, 0, 10}, R3::pitch(-0.5), 20};
    raytrace::objects::square SQ1{raytrace::point{0, 0, -10}, R3::identity, 20};
    ray r0{raytrace::point{0, 0, 0}, vector{{1, 0, 1}}.normalized()};
    ray r1{raytrace::point{0, 0, 0}, vector{{1, 0, -1}}.normalized()};

    raytrace::point p0{10, 0, 10};
    raytrace::point p1{10, 0, -10};

    geometry::intersection ir0P0 = SQ0.intersect(r0).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir0P0));
    ASSERT_POINT_EQ(p0, as_point(ir0P0));

    geometry::intersection ir0P1 = SQ1.intersect(r0).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir0P1));

    geometry::intersection ir1P0 = SQ0.intersect(r1).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir1P0));

    geometry::intersection ir1P1 = SQ1.intersect(r1).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir1P1));
    ASSERT_POINT_EQ(p1, as_point(ir1P1));
}

TEST(SquareTest, ScaleRotateTranslate) {
    using namespace raytrace;
    using namespace raytrace::objects;
    raytrace::point C{7, 7, 7};

    raytrace::objects::square SQ{C, R3::pitch(-0.25), 20.0_p};
    vector const negative_X = -geometry::R3::basis::X;
    ASSERT_EQ(negative_X.dimensions, 3ul);
    raytrace::vector N = SQ.normal(C);
    ASSERT_VECTOR_EQ(negative_X, N);
    {
        raytrace::ray const world_ray{geometry::R3::origin, geometry::R3::basis::X};
        auto inter = SQ.intersect(world_ray).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(inter));
        raytrace::point const P{7, 0, 0};
        ASSERT_POINT_EQ(P, as_point(inter));
    }

    // translate it back a bit
    raytrace::point const new_C{12, 12, 12};
    SQ.position(new_C);
}

// Squares use cartesian coordinates for mapping
TEST(SquareTest, Mapping) {
    using namespace raytrace;
    using namespace raytrace::objects;
    raytrace::objects::square SQ{20.0_p};
    SQ.set_surface_scale(1.0_p, 1.0_p);
    precision r = 1.0_p;
    {
        raytrace::point const p{0, 0, 0};
        image::point const uv = SQ.map(p);
        image::point const tmp{0.0_p, 0.0_p};
        EXPECT_POINT_EQ(uv, tmp);
    }
    {
        raytrace::point const p{r, 0, 0};
        image::point const uv = SQ.map(p);
        image::point const tmp{r, 0.0_p};
        EXPECT_POINT_EQ(uv, tmp);
    }
    {
        raytrace::point const p{0, r, 0};
        image::point const uv = SQ.map(p);
        image::point const tmp{0.0_p, r};
        EXPECT_POINT_EQ(uv, tmp);
    }
    {
        raytrace::point const p{-r, 0, 0};
        image::point const uv = SQ.map(p);
        image::point const tmp{-r, 0.0_p};
        EXPECT_POINT_EQ(uv, tmp);
    }
    {
        raytrace::point const p{0, -r, 0};
        image::point const uv = SQ.map(p);
        image::point const tmp{0.0_p, -r};
        EXPECT_POINT_EQ(uv, tmp);
    }
}
