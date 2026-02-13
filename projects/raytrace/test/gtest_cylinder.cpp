#include "basal/gtest_helper.hpp"
#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(CylinderTest, Type) {
    using namespace raytrace;
    using namespace raytrace::objects;
    cylinder cyl1{R3::origin, 1.0_p, 1.0_p};
    ASSERT_EQ(cyl1.get_type(), Type::Cylinder);
}

TEST(CylinderTest, Normals) {
    using namespace raytrace;
    using namespace raytrace::objects;

    cylinder cyl1(R3::origin, 1.0_p, 1.0_p);

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

TEST(CylinderTest, RayIntersection) {
    using namespace raytrace;
    using namespace raytrace::objects;

    cylinder cyl1(R3::origin, 1.0_p, 1.0_p);

    {
        raytrace::point p{2, 0, 0.5_p};
        ray r{p, -R3::basis::X};
        raytrace::point i{1, 0, 0.5_p};
        raytrace::objects::object::hit h = cyl1.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        ASSERT_POINT_EQ(i, as_point(h.intersect));
    }
    {
        raytrace::point p{0, 2, 0.5_p};
        ray r{p, -R3::basis::Y};
        raytrace::point i{0, 1, 0.5_p};
        raytrace::objects::object::hit h = cyl1.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        ASSERT_POINT_EQ(i, as_point(h.intersect));
    }
    {
        raytrace::point p{-2, 0, -0.5_p};
        ray r{p, R3::basis::X};
        raytrace::point i{-1, 0, -0.5_p};
        raytrace::objects::object::hit h = cyl1.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        ASSERT_POINT_EQ(i, as_point(h.intersect));
    }
    {
        raytrace::point p{0, -2, -0.5_p};
        ray r{p, R3::basis::Y};
        raytrace::point i{0, -1, -0.5_p};
        raytrace::objects::object::hit h = cyl1.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        ASSERT_POINT_EQ(i, as_point(h.intersect));
    }
}

TEST(CylinderTest, RayIntersectionInfinite) {
    using namespace raytrace;
    using namespace raytrace::objects;

    cylinder cyl1(R3::origin, 1.0_p);
    precision w = 1E9;

    {
        raytrace::point p{2, 0, w};
        ray r{p, -R3::basis::X};
        raytrace::point i{1, 0, w};
        raytrace::objects::object::hit h = cyl1.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        ASSERT_POINT_EQ(i, as_point(h.intersect));
        ASSERT_PRECISION_EQ(1.0_p, h.distance);
        ASSERT_EQ(&cyl1, h.object);
    }
    {
        raytrace::point p{0, 2, w};
        ray r{p, -R3::basis::Y};
        raytrace::point i{0, 1, w};
        raytrace::objects::object::hit h = cyl1.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        ASSERT_POINT_EQ(i, as_point(h.intersect));
        ASSERT_PRECISION_EQ(1.0_p, h.distance);
        ASSERT_EQ(&cyl1, h.object);
    }
    {
        raytrace::point p{-2, 0, -w};
        ray r{p, R3::basis::X};
        raytrace::point i{-1, 0, -w};
        raytrace::objects::object::hit h = cyl1.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        ASSERT_POINT_EQ(i, as_point(h.intersect));
        ASSERT_PRECISION_EQ(1.0_p, h.distance);
        ASSERT_EQ(&cyl1, h.object);
    }
    {
        raytrace::point p{0, -2, -w};
        ray r{p, R3::basis::Y};
        raytrace::point i{0, -1, -w};
        raytrace::objects::object::hit h = cyl1.intersect(r);
        ASSERT_EQ(geometry::IntersectionType::Point, get_type(h.intersect));
        ASSERT_POINT_EQ(i, as_point(h.intersect));
        ASSERT_PRECISION_EQ(1.0_p, h.distance);
        ASSERT_EQ(&cyl1, h.object);
    }
}

TEST(CylinderTest, Mapping) {
    using namespace raytrace;
    using namespace raytrace::objects;

    for (precision h2 = 1.0_p; h2 < 10.0_p; h2 += 1.0_p) {
        for (precision r = 1.0_p; r < 10.0_p; r += 1.0_p) {
            cylinder cyl1(R3::origin, h2, r);
            {
                raytrace::point p{r, 0, 0};
                image::point uv(0.5_p, 0.5_p);
                image::point tmp = cyl1.map(p);
                ASSERT_NEAR(uv.x(), tmp.x(), basal::epsilon);
                ASSERT_NEAR(uv.y(), tmp.y(), basal::epsilon);
            }
            {
                raytrace::point p{-r, 0, h2};
                image::point uv(0.0_p, 0.0_p);
                image::point tmp = cyl1.map(p);
                ASSERT_NEAR(uv.x(), tmp.x(), basal::epsilon);
                ASSERT_NEAR(uv.y(), tmp.y(), basal::epsilon);
            }
            {
                raytrace::point p{-r, 0, -h2};
                image::point uv(1.0_p, 0.0_p);
                image::point tmp = cyl1.map(p);
                ASSERT_NEAR(uv.x(), tmp.x(), basal::epsilon);
                ASSERT_NEAR(uv.y(), tmp.y(), basal::epsilon);
            }
            {
                raytrace::point p{0, r, -h2};
                image::point uv(1.0_p, 0.25_p);
                image::point tmp = cyl1.map(p);
                ASSERT_NEAR(uv.x(), tmp.x(), basal::epsilon);
                ASSERT_NEAR(uv.y(), tmp.y(), basal::epsilon);
            }
            {
                raytrace::point p{0, -r, -h2};
                image::point uv(1.0_p, 0.75_p);
                image::point tmp = cyl1.map(p);
                ASSERT_NEAR(uv.x(), tmp.x(), basal::epsilon);
                ASSERT_NEAR(uv.y(), tmp.y(), basal::epsilon);
            }
        }
    }
}
