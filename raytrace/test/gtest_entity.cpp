#include <gtest/gtest.h>
#include <raytrace/raytrace.hpp>
#include "geometry/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;
using namespace linalg;

TEST(EntityTest, Basics) {
    entity e;
    raytrace::point c(0,0,0);
    matrix I = matrix::identity(3, 3);
    ASSERT_POINT_EQ(c, e.position());
    ASSERT_MATRIX_EQ(I, e.rotation());

    // since we haven't moved it, object space is world space
    raytrace::point P(1,1,1);
    raytrace::point O = e.forward_transform(P);
    ASSERT_POINT_EQ(P, O);
    raytrace::point Q = e.reverse_transform(O);
    ASSERT_POINT_EQ(P, Q);
}

TEST(EntityTest, Transforms) {
    entity e;
    // Rotate the thing and move it then
    iso::degrees rX(0);
    iso::degrees rY(0);
    iso::degrees rZ(90);
    e.rotation(rX, rY, rZ);
    raytrace::point center(1,1,1);
    e.position(center);

    raytrace::point c(0,0,0);
    raytrace::point p0 = e.forward_transform(c);
    ASSERT_POINT_EQ(center, p0);

    raytrace::point p1(2,2,2);
    raytrace::point p2 = e.forward_transform(p1);
    raytrace::point p3(-1, 3, 3);
    ASSERT_POINT_EQ(p3, p2);
    raytrace::point p4 = e.reverse_transform(p3);
    ASSERT_POINT_EQ(p1, p4);
}


