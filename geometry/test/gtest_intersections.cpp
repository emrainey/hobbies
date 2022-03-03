#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <geometry/geometry.hpp>

#include "geometry/gtest_helper.hpp"

using namespace geometry;
using namespace geometry::operators;

TEST(IntersectionTest, Constructions) {
    point P{{4, 42, -9}};
    point R{{-1, -0.7, iso::pi}};
    point Q{{-90, 4, 72.0 / 3}};
    R3::vector N{{3, 4, 5}};

    intersection i0;
    ASSERT_EQ(IntersectionType::None, get_type(i0));
    ASSERT_EQ(0, i0.index());

    intersection i1(P);
    ASSERT_EQ(IntersectionType::Point, get_type(i1));
    ASSERT_EQ(1, i1.index());
    ASSERT_TRUE(P == as_point(i1));

    set_of_points sop = {R, Q};
    intersection i2(sop);
    ASSERT_EQ(IntersectionType::Points, get_type(i2));
    ASSERT_EQ(2, i2.index());
    ASSERT_THROW(as_point(i2), std::bad_variant_access);
    ASSERT_TRUE(R == as_points(i2)[0]);
    ASSERT_TRUE(Q == as_points(i2)[1]);

    R3::line L(P, N);
    intersection i3(L);
    ASSERT_EQ(IntersectionType::Line, get_type(i3));
    ASSERT_EQ(3, i3.index());
    ASSERT_TRUE(L == as_line(i3));

    plane A(N, P);
    intersection i4(A);
    ASSERT_EQ(IntersectionType::Plane, get_type(i4));
    ASSERT_EQ(4, i4.index());
    ASSERT_TRUE(A == as_plane(i4));
}

TEST(IntersectionTest, GeneralIntersections) {
    point pt1{{2, 3, 4}};
    point pt2{{2, 3, 5}};
    point pt3{{1, 1, 1}};
    R3::vector nn_z = -R3::basis::Z;
    // print_this(nn_z);
    plane xy(pt1, R3::basis::Z);
    plane xz(pt1, R3::basis::Y);
    plane yz(pt1, R3::basis::X);
    plane xy2(pt2, R3::basis::Z);
    plane xy3(pt2, nn_z);
    plane yz2(pt2, R3::basis::X);
    R3::line x_axis(pt1, R3::basis::X);
    R3::line y_axis(pt1, R3::basis::Y);
    R3::line z_axis(pt1, R3::basis::Z);
    R3::line lx(pt3, R3::basis::X);
    intersection ixyxy2 = intersects(xy, xy2);
    ASSERT_TRUE(get_type(ixyxy2) == IntersectionType::None);

    intersection ixyxy = intersects(xy, xy);
    ASSERT_TRUE(get_type(ixyxy) == IntersectionType::Plane);

    intersection ixy2xy3 = intersects(xy2, xy3);
    ASSERT_TRUE(get_type(ixy2xy3) == IntersectionType::Plane);

    intersection ixyxz = intersects(xy, xz);
    ASSERT_TRUE(get_type(ixyxz) == IntersectionType::Line);
    ASSERT_TRUE(as_line(ixyxz) == x_axis);
    ASSERT_TRUE(as_line(ixyxz) == x_axis);

    intersection ilxyz = intersects(lx, yz2);
    ASSERT_TRUE(get_type(ilxyz) == IntersectionType::Point);
    point lx_yz2 = lx.solve(1.0);  // where it aught to intersect
    // print_this(lx_yz2);
    // print_this(ilxyz.pt);
    ASSERT_TRUE(basal::equals_zero(yz2.distance(lx_yz2)));
    ASSERT_TRUE(as_point(ilxyz) == lx_yz2);
    ASSERT_TRUE(as_point(ilxyz) == lx_yz2);

    intersection copy_of_ilxyz(ilxyz);
    intersection second_copy_of_ilxyz;
    second_copy_of_ilxyz = copy_of_ilxyz;
    ASSERT_TRUE(get_type(second_copy_of_ilxyz) == IntersectionType::Point);
    ASSERT_TRUE(as_point(second_copy_of_ilxyz) == lx_yz2);
}

TEST(IntersectionTest, PointDirectlyOnLine) {
    point P{{0, 4, 7}};
    R3::vector V{{4, -2, -28}};
    R3::line L(V, P);
    ASSERT_TRUE(intersects(P, L));
}

TEST(IntersectionTest, SkewLines) {
    R3::line L1{{-1, 0, 0, 0, 0, 1}};
    R3::line L2{{1, 1, 0, 0, 0, 0}};
    intersection iL1L2 = intersects(L1, L2);
    ASSERT_EQ(IntersectionType::None, get_type(iL1L2));
}

TEST(IntersectionTest, PlaneLine) {
    plane P1(2, 3, 4, 1);
    // 2x + 3y + 4z + 1 = 0
    R3::line L1{{-1, 1, 4, 0, 3, 5}};
    // (-1, 1, 4)*t + (0, 3, 5)
    // x = -t + 0
    // y =  t + 3
    // z = 4t + 5
    // thus t = -30/17
    intersection ip1l1 = intersects(P1, L1);
    ASSERT_EQ(IntersectionType::Point, get_type(ip1l1));
    point pl1 = as_point(ip1l1);
    // a negative value for t indicates the intersection is backwards
    // from the direction of the R3::line from it's constructor
    point pl2 = L1.solve(-30.0 / 17.0);
    ASSERT_POINT_EQ(pl2, pl1);
    // find an intersection that should be in the plane
    R3::vector U{{0, 0, 1}};
    R3::vector J = R3::cross(U, P1.normal);  // parallel to the plane
    R3::line L2(J, L1.solve(0.0));
    // should not intersect at all
    intersection ip1l2 = intersects(P1, L2);
    ASSERT_EQ(IntersectionType::None, get_type(ip1l2));
    // should be in the plane completely
    R3::line L3(J, pl2);
    intersection ip1l3 = intersects(P1, L3);
    ASSERT_EQ(IntersectionType::Line, get_type(ip1l3));
    intersection copy_of_ip1l3(ip1l3);
    intersection second_copy_of_ip1l3;
    second_copy_of_ip1l3 = copy_of_ip1l3;
    ASSERT_EQ(IntersectionType::Line, get_type(second_copy_of_ip1l3));

    R3::line L4{{2, 0, 0, -1, 0, 0}};
    plane XZ(1, 0, 0, 0);
    point O{{0, 0, 0}};
    intersection iL4XZ = intersects(XZ, L4);
    ASSERT_EQ(IntersectionType::Point, get_type(iL4XZ));
    ASSERT_POINT_EQ(O, as_point(iL4XZ));

    R3::line L5{{-2, 0, 0, 1, 0, 0}};
    intersection iL5XZ = intersects(XZ, L5);
    ASSERT_EQ(IntersectionType::Point, get_type(iL5XZ));
    ASSERT_POINT_EQ(O, as_point(iL5XZ));

    R3::line L6{{-1, -1, 1, -3, -3, 2}};
    plane XY(0, 0, 1, 0);
    point xy0{{-1, -1, 0}};
    intersection iL6XY = intersects(XY, L6);
    ASSERT_EQ(IntersectionType::Point, get_type(iL6XY));
    ASSERT_POINT_EQ(xy0, as_point(iL6XY));

    R3::line L7{{1, 1, -1, -3, -3, 2}};
    intersection iL7XY = intersects(XY, L7);
    ASSERT_EQ(IntersectionType::Point, get_type(iL7XY));
    ASSERT_POINT_EQ(xy0, as_point(iL6XY));

    R3::line L8{{1, 0, 1, 0, 0, 0}};
    plane XY2(0, 0, -1, 7);
    intersection iL8XY2 = intersects(XY2, L8);
    ASSERT_EQ(IntersectionType::Point, get_type(iL8XY2));
    point xy1{{7, 0, 7}};
    ASSERT_POINT_EQ(xy1, as_point(iL8XY2));
}

TEST(IntersectionTest, LineLine) {
    point origin = R3::origin;
    R3::vector x = R3::basis::X;
    R3::vector nx = -R3::basis::X;
    R3::vector y = R3::basis::Y;
    R3::vector z = R3::basis::Z;
    R3::vector zo = R3::null;
    point di{{1, 1, 0}};
    R3::line lx(origin, x);
    R3::line ly(origin, y);
    R3::line lz(origin, z);
    ASSERT_TRUE(lx != lz);
    R3::line nlx(origin, nx);
    ASSERT_TRUE(lx == nlx);
    ASSERT_TRUE(lx || nlx);
    ASSERT_THROW(R3::line lzz(origin, zo), basal::exception);
    R3::line llz(di, z);
    ASSERT_TRUE(lz || llz);
    ASSERT_FALSE(skew(lz, llz));
    ASSERT_TRUE(skew(lx, llz));
    R3::line ai{{6, 7, 0, 6, 8, 4}};
    R3::line bi{{6, 7, 4, 6, 8, 2}};
    R3::line a(point{{12, 15, 4}}, point{{6, 8, 4}});
    R3::line b(point{{12, 15, 6}}, point{{6, 8, 2}});
    ASSERT_TRUE(ai == a);
    ASSERT_TRUE(bi == b);
    intersection ab = intersects(a, b);
    // print_this(ab);
    ASSERT_TRUE(get_type(ab) == IntersectionType::Point);
    point c{{9, 23.0 / 2, 4}};
    ASSERT_POINT_EQ(c, as_point(ab));
    point px{{2, 0, 0}};
    point py{{0, 2, 0}};
    point pz{{0, 0, 2}};
    ASSERT_TRUE(intersects(px, lx));
    ASSERT_TRUE(intersects(py, ly));
    ASSERT_TRUE(intersects(pz, lz));
    ASSERT_TRUE(intersects(px, lz) == false);

    intersection ixx = intersects(lx, lx);
    ASSERT_EQ(IntersectionType::Line, get_type(ixx));

    intersection ixy = intersects(lx, ly);
    ASSERT_TRUE(get_type(ixy) == IntersectionType::Point);
    ASSERT_TRUE(as_point(ixy) == origin);

    R3::line lxx(px, x);
    ASSERT_TRUE(lxx == lx);
    R3::line lyy(py, y);
    ASSERT_TRUE(lyy == ly);
    intersection ixxyy = intersects(lxx, lyy);
    ASSERT_TRUE(get_type(ixxyy) == IntersectionType::Point);
    ASSERT_TRUE(as_point(ixxyy) == origin);

    intersection ixz = intersects(lx, lz);
    ASSERT_TRUE(get_type(ixz) == IntersectionType::Point);
    ASSERT_TRUE(as_point(ixz) == origin);

    intersection iyz = intersects(ly, lz);
    ASSERT_TRUE(get_type(iyz) == IntersectionType::Point);
    ASSERT_TRUE(as_point(iyz) == origin);

    intersection izz = intersects(lz, llz);
    ASSERT_TRUE(get_type(izz) == IntersectionType::None);
}

TEST(IntersectionTest, PlanePlane) {
    point p1{{3, 5, 7}};
    point p2{{4, 6, 2}};
    R3::vector n1{{1, 1, 1}};
    R3::vector n2{{1, 2, 7}};
    plane P1(n1, p1);
    plane P2(n2, p2);
    intersection iP1P2 = intersects(P1, P2);
    ASSERT_TRUE(get_type(iP1P2) == IntersectionType::Line);
    R3::line p1p2{{5, -6, 1, 0, 15, 0}};
    // print_this(p1p2);
    // print_this(as_line(iP1P2));
    ASSERT_TRUE(as_line(iP1P2) == p1p2);
    ASSERT_TRUE(P1.contains(p1p2.solve(0.0)));
    ASSERT_TRUE(P2.contains(p1p2.solve(0.0)));
    ASSERT_TRUE(P1.contains(p1p2));
    ASSERT_TRUE(P2.contains(p1p2));
}

TEST(IntersectionTest, SphereOffCenter) {
    R3::sphere S0(5);
    point Z0{{5, 5, 0}};
    point Z1{{1, 0, 0}};
    R3::line L0(Z1, Z0);
    // this should pass off-center
    intersection I0 = intersects(S0, L0);
    ASSERT_EQ(IntersectionType::Points, get_type(I0));
    std::cout << "I0" << I0 << std::endl;
    set_of_points p0p1 = as_points(I0);
    ASSERT_TRUE(S0.surface(p0p1[0]));
    ASSERT_TRUE(S0.surface(p0p1[1]));
}

TEST(IntersectionTest, SphereOffCenterNonOrigin) {
    R3::sphere S0(2);
    point Z0{{1, -4, 3}};
    point Z1{{0, 0, 1}};
    R3::line L0(Z1, Z0);
    // this should pass off-center
    intersection I0 = intersects(S0, L0);
    ASSERT_EQ(IntersectionType::Points, get_type(I0));
    std::cout << "I0" << I0 << std::endl;
    set_of_points p0p1 = as_points(I0);
    ASSERT_TRUE(S0.surface(p0p1[0]));
    ASSERT_TRUE(S0.surface(p0p1[1]));
}

TEST(IntersectionTest, SphereCenter) {
    point p1{{15, 15, 15}};
    R3::vector n{{-1, -1, -1}};
    R3::sphere s1(5.0);
    R3::line l1(p1, n);
    // this should pass through the center
    intersection is1l1 = intersects(s1, l1);
    ASSERT_EQ(IntersectionType::Points, get_type(is1l1));
    R3::vector v = as_points(is1l1)[0] - R3::origin;
    R3::vector u = as_points(is1l1)[1] - R3::origin;
    ASSERT_DOUBLE_EQ(5.0, v.norm());
    ASSERT_DOUBLE_EQ(5.0, u.norm());

    R3::sphere s2(1.0);
    R3::point p2(2.0, 0.0, 0.0);
    R3::vector v2 = -R3::basis::X;
    R3::line l2(v2, p2);
    // this should pass through the center
    intersection is2l2 = intersects(s2, l2);
    ASSERT_EQ(IntersectionType::Points, get_type(is2l2));
    R3::point R = as_points(is2l2)[0];
    R3::point Q = as_points(is2l2)[1];
    std::cout << "is2l2" << is2l2 << std::endl;
    // either R or Q is +X or -X
    R3::vector d = R - Q;
    ASSERT_DOUBLE_EQ(2.0, d.norm());

    ASSERT_TRUE(s2.surface(R));
    ASSERT_TRUE(s2.surface(Q));
}

TEST(IntersectionTest, SphereNone) {
    R3::sphere s1(5.0);
    R3::line l3(point{{72, 42, 99}}, point{{73, 42, 99}});
    // this should not intersect at all.
    intersection iss1l3 = intersects(s1, l3);
    ASSERT_EQ(IntersectionType::None, get_type(iss1l3));
}

TEST(IntersectionTest, SphereInside) {
    R3::sphere s1(5.0);
    R3::line l3(point{{1, 1, 1}}, point{{2, 2, 1}});
    // this will intersect with two points which both point away from the zero
    intersection iss1l3 = intersects(s1, l3);
    ASSERT_EQ(IntersectionType::Points, get_type(iss1l3));
    point R = as_points(iss1l3)[0];
    point Q = as_points(iss1l3)[1];
    R.print("R");
    Q.print("Q");
    ASSERT_TRUE(s1.surface(R));
    ASSERT_TRUE(s1.surface(Q));
}

TEST(IntersectionTest, SphereTangent) {
    R3::sphere s1(5.0);
    R3::line l4{{0, 0, -1, 5, 0, 5}};
    // a tangent R3::line should only have a point as an intersection.
    intersection isl4s1 = intersects(s1, l4);
    ASSERT_EQ(IntersectionType::Point, get_type(isl4s1));
    point R = as_point(isl4s1);
    ASSERT_POINT_EQ(R3::point(5, 0, 0), R);
}
