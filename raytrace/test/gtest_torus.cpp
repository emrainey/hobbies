#include <vector>

#include <gtest/gtest.h>
#include <basal/basal.hpp>

#include <raytrace/raytrace.hpp>
#include "linalg/gtest_helper.hpp"
#include "geometry/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

using namespace raytrace;

TEST(TorusTest, RayIntersection) {
    element_type r = 2.0;
    element_type t = 1.0;
    raytrace::point C(0, 0, 0);
    torus shape(C, r, 1.0);

    raytrace::point p0( r,  0,  t);
    raytrace::point p1( 0,  r, -t);
    raytrace::point p2(-r,  0, -t);
    raytrace::point p3( 0, -r,  t);

    raytrace::point p4( r+t,  0,  0);
    raytrace::point p5( 0,  r+t,  0);
    raytrace::point p6(-r-t,  0,  0);
    raytrace::point p7( 0, -r-t,  0);

    ray r0(p0 + R3::basis::Z, -R3::basis::Z);
    geometry::intersection isect = shape.intersect(r0);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p0, as_point(isect));

    ray r1(p1 + -1.0*R3::basis::Z, R3::basis::Z);
    isect = shape.intersect(r1);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p1, as_point(isect));

    ray r2(p2 + -1.0*R3::basis::Z, R3::basis::Z);
    isect = shape.intersect(r2);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p2, as_point(isect));

    ray r3(p3 + R3::basis::Z, -R3::basis::Z);
    isect = shape.intersect(r3);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p3, as_point(isect));

    ray r4(p4 + R3::basis::X, -R3::basis::X);
    isect = shape.intersect(r4);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p4, as_point(isect));

    ray r5(p5 + R3::basis::Y, -R3::basis::Y);
    isect = shape.intersect(r5);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p5, as_point(isect));

    ray r6(p6 + -1.0*R3::basis::X, R3::basis::X);
    isect = shape.intersect(r6);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p6, as_point(isect));

    ray r7(p7 + -1.0*R3::basis::Y, R3::basis::Y);
    isect = shape.intersect(r7);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p7, as_point(isect));
}

TEST(TorusTest, Normals) {
    element_type r = 2;
    element_type t = 1;
    raytrace::point C(0, 0, 0);
    torus shape(C, r, t);

    raytrace::point p0( r,  0,  t);
    raytrace::point p1( 0,  r, -t);
    raytrace::point p2(-r,  0, -t);
    raytrace::point p3( 0, -r,  t);

    raytrace::point p4( r+t,  0,  0);
    raytrace::point p5( 0,  r+t,  0);
    raytrace::point p6(-r-t,  0,  0);
    raytrace::point p7( 0, -r-t,  0);

    raytrace::vector n0 = shape.normal(p0);
    raytrace::vector n1 = shape.normal(p1);
    raytrace::vector n2 = shape.normal(p2);
    raytrace::vector n3 = shape.normal(p3);

    raytrace::vector n4 = shape.normal(p4);
    raytrace::vector n5 = shape.normal(p5);
    raytrace::vector n6 = shape.normal(p6);
    raytrace::vector n7 = shape.normal(p7);

    raytrace::vector bnx = -R3::basis::X;
    raytrace::vector bny = -R3::basis::Y;
    raytrace::vector bnz = -R3::basis::Z;

    ASSERT_VECTOR_EQ( R3::basis::Z, n0);
    ASSERT_VECTOR_EQ(bnz, n1);
    ASSERT_VECTOR_EQ(bnz, n2);
    ASSERT_VECTOR_EQ( R3::basis::Z, n3);

    ASSERT_VECTOR_EQ( R3::basis::X, n4);
    ASSERT_VECTOR_EQ( R3::basis::Y, n5);
    ASSERT_VECTOR_EQ(bnx, n6);
    ASSERT_VECTOR_EQ(bny, n7);
}

TEST(TorusTest, Mapping) {
    element_type r = 2;
    element_type t = 1;
    raytrace::point C(0, 0, 0);
    torus shape(C, r, t);

    /*
    raytrace::point C(0, 0, 0);
    element_type h2 = 1.0;
    element_type r = 1.0;
    cylinder cyl1(C, h2, r);

    raytrace::point p0( 1, 0, 0);
    image::point uv0(0.5, 0.5);
    image::point tmp0 = cyl1.map(p0);
    ASSERT_DOUBLE_EQ(uv0.x, tmp0.x);
    ASSERT_DOUBLE_EQ(uv0.y, tmp0.y);

    raytrace::point p1(-1, 0, h2);
    image::point uv1(0.0, 0.0);
    image::point tmp1 = cyl1.map(p1);
    ASSERT_DOUBLE_EQ(uv1.x, tmp1.x);
    ASSERT_DOUBLE_EQ(uv1.y, tmp1.y);

    raytrace::point p2(-1, 0,-h2);
    image::point uv2(1.0, 0.0);
    image::point tmp2 = cyl1.map(p2);
    ASSERT_DOUBLE_EQ(uv2.x, tmp2.x);
    ASSERT_DOUBLE_EQ(uv2.y, tmp2.y);

    raytrace::point p3( 0,-1, -h2);
    image::point uv3(1.0, 0.75);
    image::point tmp3 = cyl1.map(p3);
    ASSERT_DOUBLE_EQ(uv3.x, tmp3.x);
    ASSERT_DOUBLE_EQ(uv3.y, tmp3.y);
    */
}
