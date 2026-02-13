#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <geometry/geometry.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

TEST(SphereTest, ConstructorsAndAssigns) {
    R3::sphere s1{5};
    R3::sphere s2{s1};
    R3::sphere s3{std::move(s2)};
    R3::sphere s4{1};
    s4 = s1;
    R3::sphere s6{2};
    s6 = std::move(s4);
    ASSERT_THROW(R3::sphere(0), basal::exception);
}

TEST(SphereTest, Contains) {
    R3::sphere s1{5};
    R3::point inside{{1, 1, 1}};
    ASSERT_TRUE(s1.contains(inside));
    R3::point outside{{10, 70, 400}};
    ASSERT_FALSE(s1.contains(outside));
    R3::point surface{{5, 0, 0}};
    ASSERT_TRUE(s1.contains(surface));
    ASSERT_FALSE(s1.on_surface(inside));
    ASSERT_FALSE(s1.on_surface(outside));
    ASSERT_TRUE(s1.on_surface(surface));
}
TEST(SphereTest, Normals) {
    R3::point p1 = R3::origin + R3::basis::X;
    R3::point p2 = R3::origin + R3::basis::Y;
    R3::point p3 = R3::origin + R3::basis::Z;
    R3::point p4{1, 1, 1};
    R3::sphere s1{1};
    R3::vector v1{s1.normal_at(p1)};
    R3::vector v2{s1.normal_at(p2)};
    R3::vector v3{s1.normal_at(p3)};
    ASSERT_VECTOR_EQ(R3::null, s1.normal_at(p4));
    ASSERT_VECTOR_EQ(R3::basis::X, v1);
    ASSERT_VECTOR_EQ(R3::basis::Y, v2);
    ASSERT_VECTOR_EQ(R3::basis::Z, v3);
}

TEST(SphereTest, CartesianToPolar) {
    precision radius = 1.0_p;
    R3::sphere s0{radius};
    R3::point sp_x = R3::origin + R3::basis::X;
    R3::point sp_y = R3::origin + R3::basis::Y;
    R3::point sp_z = R3::origin + R3::basis::Z;
    R3::point sp__x = R3::origin - R3::basis::X;

    R3::point p0 = s0.cart_to_polar(sp_x);
    R3::point p0_{{radius, 0, iso::pi / 2}};
    ASSERT_POINT_EQ(p0_, p0);

    R3::point p1 = s0.cart_to_polar(sp_y);
    R3::point p1_{{radius, iso::tau / 4, iso::pi / 2}};
    ASSERT_POINT_EQ(p1_, p1);

    R3::point p2 = s0.cart_to_polar(sp_z);
    R3::point p2_{{radius, 0, 0}};
    ASSERT_POINT_EQ(p2_, p2);

    R3::point p3 = s0.cart_to_polar(sp__x);
    R3::point p3_{{radius, iso::tau / 2, iso::pi / 2}};
    ASSERT_POINT_EQ(p3_, p3);
}
