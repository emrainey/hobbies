#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <iostream>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(WallTest, Type) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::objects::wall w0{R3::origin, R3::basis::Z, 2.0_p};
    ASSERT_EQ(w0.get_type(), raytrace::objects::Type::Wall);
}

TEST(WallTest, Position) {
    using namespace raytrace;
    using namespace raytrace::objects;
    raytrace::point C{7, 7, 7};
    raytrace::objects::wall w0{C, R3::basis::Z, 2.0_p};
    ASSERT_POINT_EQ(C, w0.position());
}

TEST(WallTest, Normals) {
    using namespace raytrace;
    using namespace raytrace::objects;
    raytrace::point C{7, 7, 7};
    raytrace::objects::wall w0{C, R3::basis::Z, 2.0_p};
    // shouldn't this return a null in the inside?
    auto should_be_null = w0.normal(C);
    auto should_be_Z = w0.normal(raytrace::point{7, 7, 8});
    auto should_be_nZ = w0.normal(raytrace::point{7, 7, 6});
    ASSERT_VECTOR_EQ(R3::null, should_be_null);
    ASSERT_VECTOR_EQ(R3::basis::Z, should_be_Z);
    ASSERT_VECTOR_EQ((-R3::basis::Z), should_be_nZ);
}

TEST(WallTest, InsideOutside) {
    using namespace raytrace;
    using namespace raytrace::objects;
    raytrace::point C{7, 7, 7};
    raytrace::objects::wall w0{C, R3::basis::Z, 2.0_p};
    ASSERT_FALSE(w0.is_outside(C));
    ASSERT_TRUE(w0.is_outside(raytrace::point{9, 9, 9}));
    ASSERT_FALSE(w0.is_outside(raytrace::point{7, 7, 8}));
    ASSERT_FALSE(w0.is_outside(raytrace::point{7, 7, 6}));
    ASSERT_FALSE(w0.is_outside(raytrace::point{12, 12, 7}));
    ASSERT_FALSE(w0.is_outside(raytrace::point{-12, -12, 7}));
}

TEST(WallTest, OnSurface) {
    using namespace raytrace;
    using namespace raytrace::objects;
    raytrace::point C{7, 7, 7};
    raytrace::objects::wall w0{C, R3::basis::Z, 2.0_p};

    ASSERT_TRUE(w0.is_surface_point(raytrace::point{7, 7, 8}));
    ASSERT_TRUE(w0.is_surface_point(raytrace::point{7, 7, 6}));
}

TEST(WallTest, Intersections) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{0, 0, 0};
    raytrace::objects::wall w0{C, R3::basis::X, 2.0_p};

    raytrace::ray r0{raytrace::point{2, 1, 1}, -R3::basis::X};
    raytrace::ray r1{raytrace::point{-2, 1, 1}, R3::basis::X};
    raytrace::ray r2{raytrace::point{2, 1, 1}, R3::basis::Y};
    raytrace::point A{1, 1, 1};
    raytrace::point B{-1, 1, 1};

    geometry::intersection ir1wall = w0.intersect(r0).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir1wall));
    ASSERT_POINT_EQ(A, as_point(ir1wall));

    geometry::intersection ir2wall = w0.intersect(r1).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir2wall));
    ASSERT_POINT_EQ(B, as_point(ir2wall));

    geometry::intersection ir3wall = w0.intersect(r2).intersect;
    ASSERT_EQ(geometry::IntersectionType::None, get_type(ir3wall));
}

TEST(WallTest, Rotations) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{1, 1, 1};
    raytrace::objects::wall w0{C, R3::basis::X, 2.0_p};
    w0.rotation(iso::degrees{0.0_p}, iso::degrees{0.0_p}, iso::degrees{180.0_p});
}

TEST(WallTest, ColumnMisses) {
    using namespace raytrace;
    using namespace geometry::operators;
    objects::wall w0{raytrace::point{0, 0, 0}, R3::basis::X, 2.0_p};
    objects::wall w1{raytrace::point{0, 0, 0}, R3::basis::Y, 2.0_p};
    objects::overlap column{w0, w1, objects::overlap::type::inclusive};
    {
        raytrace::ray r{raytrace::point{2, 2, 2}, -R3::basis::X};
        geometry::intersection is = column.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::None, get_type(is));
    }
    {
        raytrace::ray r{raytrace::point{-2, 2, 2}, -R3::basis::Y};
        geometry::intersection is = column.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::None, get_type(is));
    }
    {
        raytrace::ray r{raytrace::point{-2, -2, 2}, R3::basis::X};
        geometry::intersection is = column.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::None, get_type(is));
    }
    {
        raytrace::ray r{raytrace::point{2, -2, 2}, R3::basis::Y};
        geometry::intersection is = column.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::None, get_type(is));
    }
}

TEST(WallTest, ColumnHitsDeadCenter) {
    using namespace raytrace;
    using namespace geometry::operators;
    objects::wall w0{raytrace::point{0, 0, 0}, R3::basis::X, 2.0_p};
    objects::wall w1{raytrace::point{0, 0, 0}, R3::basis::Y, 2.0_p};
    objects::overlap column{w0, w1, objects::overlap::type::inclusive};
    {
        raytrace::ray r{raytrace::point{2, 0, 0}, (-R3::basis::X)};
        geometry::intersection is = column.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{1, 0, 0};
        ASSERT_POINT_EQ(A, as_point(is));
    }
    {
        raytrace::ray r{raytrace::point{0, 2, 0}, (-R3::basis::Y)};
        geometry::intersection is = column.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{0, 1, 0};
        ASSERT_POINT_EQ(A, as_point(is));
    }
    {
        raytrace::ray r{raytrace::point{-2, 0, 0}, R3::basis::X};
        geometry::intersection is = column.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{-1, 0, 0};
        ASSERT_POINT_EQ(A, as_point(is));
    }
    {
        raytrace::ray r{raytrace::point{0, -2, 0}, R3::basis::Y};
        geometry::intersection is = column.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{0, -1, 0};
        ASSERT_POINT_EQ(A, as_point(is));
    }
}

TEST(WallTest, ColumnHitsDiagonals) {
    using namespace raytrace;
    using namespace geometry::operators;
    objects::wall w0{raytrace::point{0, 0, 0}, R3::basis::X, 2.0_p};
    objects::wall w1{raytrace::point{0, 0, 0}, R3::basis::Y, 2.0_p};
    objects::overlap column{w0, w1, objects::overlap::type::inclusive};
    {
        raytrace::ray r{raytrace::point{2, 2, 2}, raytrace::vector{-1, -1, -1}.normalized()};
        geometry::intersection is = column.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{1, 1, 1};
        ASSERT_POINT_EQ(A, as_point(is));
    }
    {
        raytrace::ray r{raytrace::point{-2, -2, -2}, raytrace::vector{1, 1, 1}.normalized()};
        geometry::intersection is = column.intersect(r).intersect;
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{-1, -1, -1};
        ASSERT_POINT_EQ(A, as_point(is));
    }
}

TEST(WallTest, BoxFromWalls) {
    using namespace raytrace;
    using namespace geometry::operators;
    objects::wall w0{raytrace::point{0, 0, 0}, R3::basis::X, 2.0_p};
    objects::wall w1{raytrace::point{0, 0, 0}, R3::basis::Y, 2.0_p};
    objects::wall w2{raytrace::point{0, 0, 0}, R3::basis::Z, 2.0_p};
    objects::overlap column{w0, w1, objects::overlap::type::inclusive};
    objects::overlap box{column, w2, objects::overlap::type::inclusive};
    {
        raytrace::ray r{raytrace::point{2, 0, 0}, (-R3::basis::X)};
        geometry::intersection is = box.intersect(r).intersect;
        EXPECT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{1, 0, 0};
        EXPECT_POINT_EQ(A, as_point(is));
    }
    {
        raytrace::ray r{raytrace::point{0, 2, 0}, (-R3::basis::Y)};
        geometry::intersection is = box.intersect(r).intersect;
        EXPECT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{0, 1, 0};
        EXPECT_POINT_EQ(A, as_point(is));
    }
    {
        raytrace::ray r{raytrace::point{-2, 0, 0}, R3::basis::X};
        geometry::intersection is = box.intersect(r).intersect;
        EXPECT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{-1, 0, 0};
        EXPECT_POINT_EQ(A, as_point(is));
    }
    {
        raytrace::ray r{raytrace::point{0, -2, 0}, R3::basis::Y};
        geometry::intersection is = box.intersect(r).intersect;
        EXPECT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{0, -1, 0};
        EXPECT_POINT_EQ(A, as_point(is));
    }
    {
        raytrace::ray r{raytrace::point{0, 0, 2}, -R3::basis::Z};
        geometry::intersection is = box.intersect(r).intersect;
        EXPECT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{0, 0, 1};
        EXPECT_POINT_EQ(A, as_point(is));
    }
    {
        raytrace::ray r{raytrace::point{0, 0, -2}, R3::basis::Z};
        geometry::intersection is = box.intersect(r).intersect;
        EXPECT_EQ(geometry::IntersectionType::Point, get_type(is));
        raytrace::point A{0, 0, -1};
        EXPECT_POINT_EQ(A, as_point(is));
    }
}
