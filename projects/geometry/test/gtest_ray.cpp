
#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <geometry/geometry.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"

using namespace geometry;
using namespace geometry::operators;

TEST(RayTests, RayCross) {
    R3::ray a{R3::origin, R3::basis::X};
    R3::ray b{R3::origin, R3::basis::Y};
    R3::ray c{R3::origin, R3::basis::Z};

    R3::ray d = cross(a, b);
    R3::ray e = cross(c, a);
    R3::ray f = cross(b, c);

    ASSERT_TRUE(c == d);
    ASSERT_TRUE(b == e);
    ASSERT_TRUE(a == f);
}

TEST(RayTests, InvalidCross) {
    R3::point o;  // defaults to origin
    R3::vector x{{1.0_p, 0.0_p, 0.0_p}};
    R3::ray a{o, x};
    R3::point w{1.0_p, 1.0_p, 1.0_p};
    R3::vector v{{-1.0_p, -1.0_p, -1.0_p}};
    R3::ray j{w, v};
    // Can't cross rays if they aren't at the same point
    ASSERT_THROW(R3::ray f = cross(a, j), basal::exception);
}

TEST(RayTests, Overloads) {
    R3::point p{1, 1, 1};
    R3::ray a{p, R3::basis::Z};

    ASSERT_POINT_EQ(p, a.location());
    ASSERT_VECTOR_EQ(R3::basis::Z, a.direction());

    R3::vector v2{{-1, -1, -1}};
    // ray = ray + vector (move the point of the ray)
    a.print("a");
    R3::ray b = a + v2;
    b.print("b");
    ASSERT_POINT_EQ(R3::origin, b.location());
    ASSERT_VECTOR_EQ(R3::basis::Z, b.direction());

    iso::radians rad(iso::pi);
    // rotate 180 around X (should make a +Z into a -Z)
    matrix R = rotation(R3::basis::X, rad);
    // ray = matrix * ray.vector (rotates the ray)
    R3::ray c = R * a;
    c.print("Rotated Ray");
    ASSERT_POINT_EQ(p, c.location());
    R3::vector nz = -R3::basis::Z;
    ASSERT_VECTOR_EQ(nz, c.direction());
}
