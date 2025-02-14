#include "basal/gtest_helper.hpp"
#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(CylinderTest, RayIntersection) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{0, 0, 0};
    cylinder cyl1(C, 1.0, 1.0);

    raytrace::point p0{2, 0, 0.5};
    raytrace::point p1{0, 2, 0.5};
    raytrace::point p2{-2, 0, -0.5};
    raytrace::point p3{0, -2, -0.5};

    ray r0{p0, -R3::basis::X};
    raytrace::point i0{1, 0, 0.5};
    geometry::intersection ir0cyl1 = cyl1.intersect(r0).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir0cyl1));
    ASSERT_POINT_EQ(i0, as_point(ir0cyl1));

    ray r1{p1, -R3::basis::Y};
    raytrace::point i1{0, 1, 0.5};
    geometry::intersection ir1cyl1 = cyl1.intersect(r1).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir1cyl1));
    ASSERT_POINT_EQ(i1, as_point(ir1cyl1));

    ray r2{p2, R3::basis::X};
    raytrace::point i2{-1, 0, -0.5};
    geometry::intersection ir2cyl1 = cyl1.intersect(r2).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir2cyl1));
    ASSERT_POINT_EQ(i2, as_point(ir2cyl1));

    ray r3{p3, R3::basis::Y};
    raytrace::point i3{0, -1, -0.5};
    geometry::intersection ir3cyl1 = cyl1.intersect(r3).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir3cyl1));
    ASSERT_POINT_EQ(i3, as_point(ir3cyl1));
}

TEST(CylinderTest, Normals) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{0, 0, 0};
    cylinder cyl1(C, 1.0, 1.0);

    raytrace::point p0{1, 0, 1};
    raytrace::point p1{0, 1, 1};
    raytrace::point p2{-1, 0, -1};
    raytrace::point p3{0, -1, -1};

    vector nx = -R3::basis::X;
    vector ny = -R3::basis::Y;

    ASSERT_VECTOR_EQ(R3::basis::X, cyl1.normal(p0));
    ASSERT_VECTOR_EQ(R3::basis::Y, cyl1.normal(p1));
    ASSERT_VECTOR_EQ(nx, cyl1.normal(p2));
    ASSERT_VECTOR_EQ(ny, cyl1.normal(p3));
}

TEST(CylinderTest, Mapping) {
    using namespace raytrace;
    using namespace raytrace::objects;

    for (precision h2 = 1.0; h2 < 10.0; h2 += 1.0) {
        for (precision r = 1.0; r < 10.0; r += 1.0) {
            cylinder cyl1(R3::origin, h2, r);
            {
                raytrace::point p{r, 0, 0};
                image::point uv(0.5, 0.5);
                image::point tmp = cyl1.map(p);
                ASSERT_NEAR(uv.x, tmp.x, basal::epsilon);
                ASSERT_NEAR(uv.y, tmp.y, basal::epsilon);
            }
            {
                raytrace::point p{-r, 0, h2};
                image::point uv(0.0, 0.0);
                image::point tmp = cyl1.map(p);
                ASSERT_NEAR(uv.x, tmp.x, basal::epsilon);
                ASSERT_NEAR(uv.y, tmp.y, basal::epsilon);
            }
            {
                raytrace::point p{-r, 0, -h2};
                image::point uv(1.0, 0.0);
                image::point tmp = cyl1.map(p);
                ASSERT_NEAR(uv.x, tmp.x, basal::epsilon);
                ASSERT_NEAR(uv.y, tmp.y, basal::epsilon);
            }
            {
                raytrace::point p{0, r, -h2};
                image::point uv(1.0, 0.25);
                image::point tmp = cyl1.map(p);
                ASSERT_NEAR(uv.x, tmp.x, basal::epsilon);
                ASSERT_NEAR(uv.y, tmp.y, basal::epsilon);
            }
            {
                raytrace::point p{0, -r, -h2};
                image::point uv(1.0, 0.75);
                image::point tmp = cyl1.map(p);
                ASSERT_NEAR(uv.x, tmp.x, basal::epsilon);
                ASSERT_NEAR(uv.y, tmp.y, basal::epsilon);
            }
        }
    }
}
