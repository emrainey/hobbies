#include "basal/gtest_helper.hpp"

#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(TorusTest, RayIntersection) {
    using namespace raytrace;
    using namespace raytrace::objects;

    precision r = 2.0;
    precision t = 1.0;
    raytrace::point C{0, 0, 0};
    torus shape(C, r, 1.0);

    raytrace::point p0{r, 0, t};
    raytrace::point p1{0, r, -t};
    raytrace::point p2{-r, 0, -t};
    raytrace::point p3{0, -r, t};

    raytrace::point p4{r + t, 0, 0};
    raytrace::point p5{0, r + t, 0};
    raytrace::point p6{-r - t, 0, 0};
    raytrace::point p7{0, -r - t, 0};

    ray r0{p0 + R3::basis::Z, -R3::basis::Z};
    geometry::intersection isect = shape.intersect(r0);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p0, as_point(isect));

    ray r1{p1 + -1.0 * R3::basis::Z, R3::basis::Z};
    isect = shape.intersect(r1);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p1, as_point(isect));

    ray r2{p2 + -1.0 * R3::basis::Z, R3::basis::Z};
    isect = shape.intersect(r2);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p2, as_point(isect));

    ray r3{p3 + R3::basis::Z, -R3::basis::Z};
    isect = shape.intersect(r3);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p3, as_point(isect));

    ray r4{p4 + R3::basis::X, -R3::basis::X};
    isect = shape.intersect(r4);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p4, as_point(isect));

    ray r5{p5 + R3::basis::Y, -R3::basis::Y};
    isect = shape.intersect(r5);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p5, as_point(isect));

    ray r6{p6 + -1.0 * R3::basis::X, R3::basis::X};
    isect = shape.intersect(r6);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p6, as_point(isect));

    ray r7{p7 + -1.0 * R3::basis::Y, R3::basis::Y};
    isect = shape.intersect(r7);
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(isect));
    ASSERT_POINT_EQ(p7, as_point(isect));
}

TEST(TorusTest, Normals) {
    using namespace raytrace;
    using namespace raytrace::objects;

    precision r = 2;
    precision t = 1;
    raytrace::point C{0, 0, 0};
    torus shape(C, r, t);

    raytrace::point p0{r, 0, t};
    raytrace::point p1{0, r, -t};
    raytrace::point p2{-r, 0, -t};
    raytrace::point p3{0, -r, t};

    raytrace::point p4{r + t, 0, 0};
    raytrace::point p5{0, r + t, 0};
    raytrace::point p6{-r - t, 0, 0};
    raytrace::point p7{0, -r - t, 0};

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

    ASSERT_VECTOR_EQ(R3::basis::Z, n0);
    ASSERT_VECTOR_EQ(bnz, n1);
    ASSERT_VECTOR_EQ(bnz, n2);
    ASSERT_VECTOR_EQ(R3::basis::Z, n3);

    ASSERT_VECTOR_EQ(R3::basis::X, n4);
    ASSERT_VECTOR_EQ(R3::basis::Y, n5);
    ASSERT_VECTOR_EQ(bnx, n6);
    ASSERT_VECTOR_EQ(bny, n7);
}

TEST(TorusTest, Mapping) {
    using namespace raytrace;
    using namespace raytrace::objects;

    precision r = 2.0_p;
    precision t = 1.0_p;
    raytrace::point C{0, 0, 0};
    torus shape(C, r, t);

    {
        raytrace::point p{ -1, 0, 0};
        image::point uv(1.0, 1.0);
        image::point tmp = shape.map(p);
        EXPECT_PRECISION_EQ(uv.x, tmp.x);
        EXPECT_PRECISION_EQ(uv.y, tmp.y);
    }
    {
        raytrace::point p{-3, 0, 0};
        image::point uv(1.0, 0.5);
        image::point tmp = shape.map(p);
        EXPECT_PRECISION_EQ(uv.x, tmp.x);
        EXPECT_PRECISION_EQ(uv.y, tmp.y);
    }
    {
        raytrace::point p{ 1, 0, 0};
        image::point uv(0.5, 1.0);
        image::point tmp = shape.map(p);
        EXPECT_PRECISION_EQ(uv.x, tmp.x);
        EXPECT_PRECISION_EQ(uv.y, tmp.y);
    }
    {
        raytrace::point p{ 2, 0, 1};
        image::point uv(0.5, 0.75);
        image::point tmp = shape.map(p);
        EXPECT_PRECISION_EQ(uv.x, tmp.x);
        EXPECT_PRECISION_EQ(uv.y, tmp.y);
    }
    {
        raytrace::point p{ 3, 0, 0};
        image::point uv(0.5, 0.5);
        image::point tmp = shape.map(p);
        EXPECT_PRECISION_EQ(uv.x, tmp.x);
        EXPECT_PRECISION_EQ(uv.y, tmp.y);
    }
    {
        raytrace::point p{ 2, 0, -1};
        image::point uv(0.5, 0.25);
        image::point tmp = shape.map(p);
        EXPECT_PRECISION_EQ(uv.x, tmp.x);
        EXPECT_PRECISION_EQ(uv.y, tmp.y);
    }
    //--------------------------------------
    {
        raytrace::point p{ 0, 3, 0};
        image::point uv(0.75, 0.5);
        image::point tmp = shape.map(p);
        EXPECT_PRECISION_EQ(uv.x, tmp.x);
        EXPECT_PRECISION_EQ(uv.y, tmp.y);
    }
    {
        raytrace::point p{ 0, 1, 0};
        image::point uv(0.75, 1.0);
        image::point tmp = shape.map(p);
        EXPECT_PRECISION_EQ(uv.x, tmp.x);
        EXPECT_PRECISION_EQ(uv.y, tmp.y);
    }
    {
        raytrace::point p{ 0, -1, 0};
        image::point uv(0.25, 1.0);
        image::point tmp = shape.map(p);
        EXPECT_PRECISION_EQ(uv.x, tmp.x);
        EXPECT_PRECISION_EQ(uv.y, tmp.y);
    }
    {
        raytrace::point p{ 0, -3, 0};
        image::point uv(0.25, 0.5);
        image::point tmp = shape.map(p);
        EXPECT_PRECISION_EQ(uv.x, tmp.x);
        EXPECT_PRECISION_EQ(uv.y, tmp.y);
    }
}
