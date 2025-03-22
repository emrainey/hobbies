#include <gtest/gtest.h>

#include <raytrace/raytrace.hpp>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(BoundsTest, Default) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    EXPECT_POINT_EQ(bounds.min, raytrace::point(0, 0, 0));
    EXPECT_POINT_EQ(bounds.max, raytrace::point(1, 1, 1));
    EXPECT_POINT_EQ(bounds.center(), raytrace::point(0.5_p, 0.5_p, 0.5_p));
}

TEST(BoundsTest, Contained) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    EXPECT_TRUE(bounds.contained(raytrace::point(0.5_p, 0.5_p, 0.5_p)));
}

TEST(BoundsTest, Intersects) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    auto r = raytrace::ray{raytrace::point(-1, -1, -1), raytrace::vector{1, 1, 1}};
    EXPECT_TRUE(bounds.intersects(r));
}

TEST(BoundsTest, NoIntersect) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    auto r1 = raytrace::ray{raytrace::point(1.5_p, 1.5_p, 1.5_p), raytrace::vector{1, 1, 1}};
    EXPECT_FALSE(bounds.intersects(r1));
    auto r2 = raytrace::ray{raytrace::point(-1.5_p, 0, -1.5_p), raytrace::vector{-1, 0, -1}};
    EXPECT_FALSE(bounds.intersects(r2));
}

TEST(BoundsTest, Inside) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    auto r = raytrace::ray{raytrace::point(0.5_p, 0.5_p, 0.5_p), raytrace::vector{0, 0, 1}};
    EXPECT_TRUE(bounds.intersects(r));
}

TEST(BoundsTest, Overlap) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    auto b = raytrace::Bounds{raytrace::point(0.5_p, 0.5_p, 0.5_p), raytrace::point(1.5_p, 1.5_p, 1.5_p)};
    EXPECT_TRUE(bounds.intersects(b));
}

TEST(BoundsTest, NoOverlap) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    auto b = raytrace::Bounds{raytrace::point(1.5_p, 1.5_p, 1.5_p), raytrace::point(2.5_p, 2.5_p, 2.5_p)};
    EXPECT_FALSE(bounds.intersects(b));
    auto c = raytrace::Bounds{raytrace::point(-1.5_p, -1.5_p, -1.5_p), raytrace::point(-0.5_p, -0.5_p, -0.5_p)};
    EXPECT_FALSE(bounds.intersects(c));
}

TEST(BoundsTest, TouchingButNotOverlapping) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    auto b = raytrace::Bounds{raytrace::point(1, 0, 0), raytrace::point(2, 1, 1)};
    EXPECT_FALSE(bounds.intersects(b));
    auto c = raytrace::Bounds{raytrace::point(0, 1, 0), raytrace::point(1, 2, 1)};
    EXPECT_FALSE(bounds.intersects(c));
    auto d = raytrace::Bounds{raytrace::point(0, 0, 1), raytrace::point(1, 1, 2)};
    EXPECT_FALSE(bounds.intersects(d));
}

TEST(BoundsTest, Grow) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    auto b = raytrace::Bounds{raytrace::point(0.5_p, 0.5_p, 0.5_p), raytrace::point(1.5_p, 1.5_p, 1.5_p)};
    bounds.grow(b);
    EXPECT_POINT_EQ(bounds.min, raytrace::point(0, 0, 0));
    EXPECT_POINT_EQ(bounds.max, raytrace::point(1.5_p, 1.5_p, 1.5_p));
}

TEST(BoundsTest, Split) {
    raytrace::Bounds bounds{raytrace::point{0, 0, 0}, raytrace::point{2, 2, 2}};
    auto sub_bounds = bounds.split();
    // express the asserts for each min/max pair in the sub_bounds

    EXPECT_POINT_EQ(raytrace::point(0.0_p, 0.0_p, 0.0_p), sub_bounds[0].min);  // 0: (-x, -y, -z)
    EXPECT_POINT_EQ(raytrace::point(0.0_p, 0.0_p, 1.0_p), sub_bounds[1].min);  // 1: (-x, -y, +z)
    EXPECT_POINT_EQ(raytrace::point(0.0_p, 1.0_p, 0.0_p), sub_bounds[2].min);  // 2: (-x, +y, -z)
    EXPECT_POINT_EQ(raytrace::point(0.0_p, 1.0_p, 1.0_p), sub_bounds[3].min);  // 3: (-x, +y, +z)
    EXPECT_POINT_EQ(raytrace::point(1.0_p, 0.0_p, 0.0_p), sub_bounds[4].min);  // 4: (+x, -y, -z)
    EXPECT_POINT_EQ(raytrace::point(1.0_p, 0.0_p, 1.0_p), sub_bounds[5].min);  // 5: (+x, -y, +z)
    EXPECT_POINT_EQ(raytrace::point(1.0_p, 1.0_p, 0.0_p), sub_bounds[6].min);  // 6: (+x, +y, -z)
    EXPECT_POINT_EQ(raytrace::point(1.0_p, 1.0_p, 1.0_p), sub_bounds[7].min);  // 7: (+x, +y, +z)

    EXPECT_POINT_EQ(raytrace::point(1.0_p, 1.0_p, 1.0_p), sub_bounds[0].max);  // 0: (-x, -y, -z)
    EXPECT_POINT_EQ(raytrace::point(1.0_p, 1.0_p, 2.0_p), sub_bounds[1].max);  // 1: (-x, -y, +z)
    EXPECT_POINT_EQ(raytrace::point(1.0_p, 2.0_p, 1.0_p), sub_bounds[2].max);  // 2: (-x, +y, -z)
    EXPECT_POINT_EQ(raytrace::point(1.0_p, 2.0_p, 2.0_p), sub_bounds[3].max);  // 3: (-x, +y, +z)
    EXPECT_POINT_EQ(raytrace::point(2.0_p, 1.0_p, 1.0_p), sub_bounds[4].max);  // 4: (+x, -y, -z)
    EXPECT_POINT_EQ(raytrace::point(2.0_p, 1.0_p, 2.0_p), sub_bounds[5].max);  // 5: (+x, -y, +z)
    EXPECT_POINT_EQ(raytrace::point(2.0_p, 2.0_p, 1.0_p), sub_bounds[6].max);  // 6: (+x, +y, -z)
    EXPECT_POINT_EQ(raytrace::point(2.0_p, 2.0_p, 2.0_p), sub_bounds[7].max);  // 7: (+x, +y, +z)
}

TEST(BoundsTest, NotInfinite) {
    auto bounds = raytrace::Bounds{raytrace::point(0, 0, 0), raytrace::point(1, 1, 1)};
    EXPECT_FALSE(bounds.is_infinite());
}

TEST(BoundsTest, Infinite) {
    auto bounds = raytrace::Bounds{};  // default constructor
    EXPECT_TRUE(bounds.is_infinite());
    auto b = raytrace::Bounds{raytrace::point(0.5_p, 0.5_p, 0.5_p), raytrace::point(1.5_p, 1.5_p, 1.5_p)};
    bounds.grow(b);
    EXPECT_TRUE(bounds.is_infinite());
    auto middle = bounds.center();
    EXPECT_POINT_EQ(raytrace::point(0.0_p, 0.0_p, 0.0_p), middle);
}
