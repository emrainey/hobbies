#include <gtest/gtest.h>

#include <raytrace/raytrace.hpp>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(BoundsTest, Default) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    EXPECT_POINT_EQ(bounds.min, raytrace::point(0, 0, 0));
    EXPECT_POINT_EQ(bounds.max, raytrace::point(1, 1, 1));
    EXPECT_POINT_EQ(bounds.center(), raytrace::point(0.5, 0.5, 0.5));
}

TEST(BoundsTest, Contained) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    EXPECT_TRUE(bounds.contained(raytrace::point(0.5, 0.5, 0.5)));
}

TEST(BoundsTest, Intersects) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    auto r = raytrace::ray{raytrace::point(-1, -1,  -1), raytrace::vector{1, 1, 1}};
    EXPECT_TRUE(bounds.intersects(r));
}

TEST(BoundsTest, NoIntersect) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    auto r1 = raytrace::ray{raytrace::point(1.5, 1.5, 1.5), raytrace::vector{1, 1, 1}};
    EXPECT_FALSE(bounds.intersects(r1));
    auto r2 = raytrace::ray{raytrace::point(-1.5, 0, -1.5), raytrace::vector{-1, 0, -1}};
    EXPECT_FALSE(bounds.intersects(r2));
}

TEST(BoundsTest, Overlap) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    auto b = raytrace::Bounds{raytrace::point(0.5, 0.5, 0.5), raytrace::point(1.5, 1.5, 1.5)};
    EXPECT_TRUE(bounds.intersects(b));
}

TEST(BoundsTest, NoOverlap) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    auto b = raytrace::Bounds{raytrace::point(1.5, 1.5, 1.5), raytrace::point(2.5, 2.5, 2.5)};
    EXPECT_FALSE(bounds.intersects(b));
    auto c = raytrace::Bounds{raytrace::point(-1.5, -1.5, -1.5), raytrace::point(-0.5, -0.5, -0.5)};
    EXPECT_FALSE(bounds.intersects(c));
}

TEST(BoundsTest, Grow) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0),
                                   raytrace::point(1, 1, 1)};
    auto b = raytrace::Bounds{raytrace::point(0.5, 0.5, 0.5),
                             raytrace::point(1.5, 1.5, 1.5)};
    bounds.grow(b);
    EXPECT_POINT_EQ(bounds.min, raytrace::point(0, 0, 0));
    EXPECT_POINT_EQ(bounds.max, raytrace::point(1.5, 1.5, 1.5));
}

TEST(BoundsTest, NotInfinite) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0),
                                    raytrace::point(1, 1, 1)};
    EXPECT_FALSE(bounds.is_infinite());
}

TEST(BoundsTest, Infinite) {
    auto bounds = raytrace::Bounds{raytrace::point(basal::nan, basal::nan, basal::nan), raytrace::point(basal::nan, basal::nan, basal::nan)};
    EXPECT_TRUE(bounds.is_infinite());
}

