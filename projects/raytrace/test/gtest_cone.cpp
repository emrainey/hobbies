#include "basal/gtest_helper.hpp"

#include <basal/basal.hpp>
#include <raytrace/raytrace.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "raytrace/gtest_helper.hpp"

TEST(ConeTest, Type) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{0, 0, 0};
    cone cone0(C, 1.0_p, 1.0_p);
    ASSERT_EQ(cone0.get_type(), Type::Cone);
}

TEST(ConeTest, RayIntersection) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{0, 0, 0};
    cone cone0(C, 1.0_p, 1.0_p);

    raytrace::point p0{2, 0, 0.5_p};
    raytrace::point p1{0, 2, 0.5_p};
    raytrace::point p2{-2, 0, 0.5_p};
    raytrace::point p3{0, -2, 0.5_p};

    ray r0{p0, -R3::basis::X};
    raytrace::point i0{0.5_p, 0, 0.5_p};
    geometry::intersection ir0cone0 = cone0.intersect(r0).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir0cone0));
    ASSERT_POINT_EQ(i0, as_point(ir0cone0));
    ASSERT_TRUE(cone0.is_surface_point(as_point(ir0cone0)));

    ray r1{p1, -R3::basis::Y};
    raytrace::point i1{0, 0.5_p, 0.5_p};
    geometry::intersection ir1cone0 = cone0.intersect(r1).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir1cone0));
    ASSERT_POINT_EQ(i1, as_point(ir1cone0));
    ASSERT_TRUE(cone0.is_surface_point(as_point(ir1cone0)));

    ray r2{p2, R3::basis::X};
    raytrace::point i2{-0.5_p, 0, 0.5_p};
    geometry::intersection ir2cone0 = cone0.intersect(r2).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir2cone0));
    ASSERT_POINT_EQ(i2, as_point(ir2cone0));
    ASSERT_TRUE(cone0.is_surface_point(as_point(ir2cone0)));

    ray r3{p3, R3::basis::Y};
    raytrace::point i3{0, -0.5_p, 0.5_p};
    geometry::intersection ir3cone0 = cone0.intersect(r3).intersect;
    ASSERT_EQ(geometry::IntersectionType::Point, get_type(ir3cone0));
    ASSERT_POINT_EQ(i3, as_point(ir3cone0));
    ASSERT_TRUE(cone0.is_surface_point(as_point(ir3cone0)));
}

TEST(ConeTest, Normals) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{0, 0, 0};
    cylinder cyl1(C, 1.0_p, 1.0_p);
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

TEST(ConeTest, Mapping) {
    using namespace raytrace;
    using namespace raytrace::objects;

    raytrace::point C{0, 0, 0};
    precision h2 = 1.0_p;
    precision r = 1.0_p;
    cylinder cyl1(C, h2, r);

    raytrace::point p0{1, 0, 0};
    image::point uv0(0.5_p, 0.5_p);
    image::point tmp0 = cyl1.map(p0);
    ASSERT_NEAR(uv0.x(), tmp0.x(), basal::epsilon);
    ASSERT_NEAR(uv0.y(), tmp0.y(), basal::epsilon);

    raytrace::point p1{-1, 0, h2};
    image::point uv1(0.0_p, 0.0_p);
    image::point tmp1 = cyl1.map(p1);
    ASSERT_NEAR(uv1.x(), tmp1.x(), basal::epsilon);
    ASSERT_NEAR(uv1.y(), tmp1.y(), basal::epsilon);

    raytrace::point p2{-1, 0, -h2};
    image::point uv2(1.0_p, 0.0_p);
    image::point tmp2 = cyl1.map(p2);
    ASSERT_NEAR(uv2.x(), tmp2.x(), basal::epsilon);
    ASSERT_NEAR(uv2.y(), tmp2.y(), basal::epsilon);

    raytrace::point p3{0, -1, -h2};
    image::point uv3(1.0_p, 0.75_p);
    image::point tmp3 = cyl1.map(p3);
    ASSERT_NEAR(uv3.x(), tmp3.x(), basal::epsilon);
    ASSERT_NEAR(uv3.y(), tmp3.y(), basal::epsilon);
}
