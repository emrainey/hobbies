#include "basal/gtest_helper.hpp"

#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;

TEST(TreeTest, IndexOf) {
    Bounds bounds{raytrace::point{-70, -42,  -99}, raytrace::point{55, 147, 22}};
    tree::Node node{bounds};
    // check the middle
    raytrace::point point{-7.5, 52.5, -38.5};
    EXPECT_POINT_EQ(point,bounds.center());
    // check each extrema for being in the correct octant
    point = raytrace::point{-70, -42, -99};
    EXPECT_EQ(0, node.IndexOf(point));
    point = raytrace::point{-70, -42, 22};
    EXPECT_EQ(1, node.IndexOf(point));
    point = raytrace::point{-70, 147, -99};
    EXPECT_EQ(2, node.IndexOf(point));
    point = raytrace::point{-70, 147, 22};
    EXPECT_EQ(3, node.IndexOf(point));
    point = raytrace::point{55, -42, -99};
    EXPECT_EQ(4, node.IndexOf(point));
    point = raytrace::point{55, -42, 22};
    EXPECT_EQ(5, node.IndexOf(point));
    point = raytrace::point{55, 147, -99};
    EXPECT_EQ(6, node.IndexOf(point));
    point = raytrace::point{55, 147, 22};
    EXPECT_EQ(7, node.IndexOf(point));
}

TEST(TreeTest, SplitBounds) {
    Bounds bounds{raytrace::point{-70, -42,  -99}, raytrace::point{55, 147, 22}};
    tree::Node node{bounds};
    objects::sphere s0{raytrace::point{5, 4, 9}, 3};
    EXPECT_TRUE(node.intersects(&s0));
    node.add_object(&s0);
    EXPECT_TRUE(node.has(&s0));
}

TEST(TreeTest, SplitBoundsSubNodes) {
    Bounds bounds{raytrace::point{0, 0, 0}, raytrace::point{2, 2, 2}};
    tree::Node node{bounds};
    auto sub_bounds = node.split_bounds();
    // express the asserts for each min/max pair in the sub_bounds

    EXPECT_POINT_EQ(raytrace::point(0.0, 0.0, 0.0), sub_bounds[0].min); // 0: (-x, -y, -z)
    EXPECT_POINT_EQ(raytrace::point(0.0, 0.0, 1.0), sub_bounds[1].min); // 1: (-x, -y, +z)
    EXPECT_POINT_EQ(raytrace::point(0.0, 1.0, 0.0), sub_bounds[2].min); // 2: (-x, +y, -z)
    EXPECT_POINT_EQ(raytrace::point(0.0, 1.0, 1.0), sub_bounds[3].min); // 3: (-x, +y, +z)
    EXPECT_POINT_EQ(raytrace::point(1.0, 0.0, 0.0), sub_bounds[4].min); // 4: (+x, -y, -z)
    EXPECT_POINT_EQ(raytrace::point(1.0, 0.0, 1.0), sub_bounds[5].min); // 5: (+x, -y, +z)
    EXPECT_POINT_EQ(raytrace::point(1.0, 1.0, 0.0), sub_bounds[6].min); // 6: (+x, +y, -z)
    EXPECT_POINT_EQ(raytrace::point(1.0, 1.0, 1.0), sub_bounds[7].min); // 7: (+x, +y, +z)

    EXPECT_POINT_EQ(raytrace::point(1.0, 1.0, 1.0), sub_bounds[0].max); // 0: (-x, -y, -z)
    EXPECT_POINT_EQ(raytrace::point(1.0, 1.0, 2.0), sub_bounds[1].max); // 1: (-x, -y, +z)
    EXPECT_POINT_EQ(raytrace::point(1.0, 2.0, 1.0), sub_bounds[2].max); // 2: (-x, +y, -z)
    EXPECT_POINT_EQ(raytrace::point(1.0, 2.0, 2.0), sub_bounds[3].max); // 3: (-x, +y, +z)
    EXPECT_POINT_EQ(raytrace::point(2.0, 1.0, 1.0), sub_bounds[4].max); // 4: (+x, -y, -z)
    EXPECT_POINT_EQ(raytrace::point(2.0, 1.0, 2.0), sub_bounds[5].max); // 5: (+x, -y, +z)
    EXPECT_POINT_EQ(raytrace::point(2.0, 2.0, 1.0), sub_bounds[6].max); // 6: (+x, +y, -z)
    EXPECT_POINT_EQ(raytrace::point(2.0, 2.0, 2.0), sub_bounds[7].max); // 7: (+x, +y, +z)
}


TEST(TreeTest, AddObjects) {
    Bounds bounds{raytrace::point{0, 0, 0}, raytrace::point{2, 2, 2}};
    tree::Node node{bounds};
    objects::sphere s0{raytrace::point{0.5, 0.5, 0.5}, 0.4};
    objects::sphere s1{raytrace::point{0.5, 0.5, 1.5}, 0.4};
    objects::sphere s2{raytrace::point{0.5, 1.5, 0.5}, 0.4};
    objects::sphere s3{raytrace::point{0.5, 1.5, 1.5}, 0.4};
    objects::sphere s4{raytrace::point{1.5, 0.5, 0.5}, 0.4};
    objects::sphere s5{raytrace::point{1.5, 0.5, 1.5}, 0.4};
    objects::sphere s6{raytrace::point{1.5, 1.5, 0.5}, 0.4};
    objects::sphere s7{raytrace::point{1.5, 1.5, 1.5}, 0.4};
    EXPECT_TRUE(node.intersects(&s0));
    EXPECT_TRUE(node.intersects(&s1));
    EXPECT_TRUE(node.intersects(&s2));
    EXPECT_TRUE(node.intersects(&s3));
    EXPECT_TRUE(node.intersects(&s4));
    EXPECT_TRUE(node.intersects(&s5));
    EXPECT_TRUE(node.intersects(&s6));
    EXPECT_TRUE(node.intersects(&s7));
    EXPECT_TRUE(node.add_object(&s0));
    EXPECT_TRUE(node.has(&s0));
    EXPECT_TRUE(node.add_object(&s1));
    EXPECT_TRUE(node.has(&s1));
    EXPECT_TRUE(node.add_object(&s2));
    EXPECT_TRUE(node.has(&s2));
    EXPECT_TRUE(node.add_object(&s3));
    EXPECT_TRUE(node.has(&s3));
    EXPECT_TRUE(node.add_object(&s4));
    EXPECT_TRUE(node.has(&s4));
    EXPECT_TRUE(node.add_object(&s5));
    EXPECT_TRUE(node.has(&s5));
    EXPECT_TRUE(node.add_object(&s6));
    EXPECT_TRUE(node.has(&s6));
    EXPECT_TRUE(node.add_object(&s7));
    EXPECT_TRUE(node.has(&s7));
    // no subnodes yet...
    objects::sphere s8{raytrace::point{1.0, 1.0, 1.0}, 0.5}; // should be in all subnodes
    EXPECT_TRUE(node.intersects(&s8));
    EXPECT_TRUE(node.add_object(&s8));
    EXPECT_TRUE(node.has(&s8));
    EXPECT_FALSE(node.has(&s0));
    EXPECT_FALSE(node.has(&s1));
    EXPECT_FALSE(node.has(&s2));
    EXPECT_FALSE(node.has(&s3));
    EXPECT_FALSE(node.has(&s4));
    EXPECT_FALSE(node.has(&s5));
    EXPECT_FALSE(node.has(&s6));
    EXPECT_FALSE(node.has(&s7));
    EXPECT_TRUE(node.under(&s0));
    EXPECT_TRUE(node.under(&s1));
    EXPECT_TRUE(node.under(&s2));
    EXPECT_TRUE(node.under(&s3));
    EXPECT_TRUE(node.under(&s4));
    EXPECT_TRUE(node.under(&s5));
    EXPECT_TRUE(node.under(&s6));
    EXPECT_TRUE(node.under(&s7));
}