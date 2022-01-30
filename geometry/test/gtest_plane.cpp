#include <gtest/gtest.h>
#include <geometry/geometry.hpp>
#include <basal/basal.hpp>
#include <vector>
#include "geometry/gtest_helper.hpp"

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

TEST(PlaneTest, ConstructorsAndAssigns) {
    R3::vector n{{1, 1, 1}};
    R3::point p(5, 0, 0);
    plane p1(n, p);
    plane p2(p, n);
    ASSERT_TRUE(p1 || p2);

    plane p3{{4, 5, 6, 7}};
    plane p4(4, 5, 6, 7);
    ASSERT_TRUE(p3 || p4);

    R3::point pt1(5, 0, 0);
    R3::point pt2(0, 5, 0);
    R3::point pt3(0, 0, 5);
    plane p5(pt1, pt2, pt3);
    ASSERT_TRUE(p5 == p2);
    plane p6(p3); // copy construct
    plane p7(std::move(p4)); // move construct
    ASSERT_TRUE(p6 || p7);
    p6 = p3;
    p7 = std::move(p3);
    ASSERT_TRUE(p6 || p7);
}

TEST(PlaneTest, IntersectionWithNullspace) {
    plane P1(2, -1, 1, -5);
    plane P2(1, 1, -1, -1);
    intersection iP1P2 = intersects(P1, P2);
    R3::line p1p2{{0, 3, 3, 2, -1, 0}};
    ASSERT_TRUE(get_type(iP1P2) == IntersectionType::Line);
    //print_this(p1p2);
    //print_this(as_line(iP1P2));
    ASSERT_TRUE(as_line(iP1P2) == p1p2);
    // each plane aught to contain the R3::line of intersection and points on the R3::line
    ASSERT_TRUE(P1.contains(p1p2.solve(0.0)));
    ASSERT_TRUE(P2.contains(p1p2.solve(0.0)));
    ASSERT_TRUE(P1.contains(p1p2));
    ASSERT_TRUE(P2.contains(p1p2));
}

TEST(PlaneTest, Parallel) {
    R3::point A(1, 2, -1);
    R3::point B(2, 3, 1);
    R3::point C(3, -1, 2);
    plane P(A, B, C);
    plane Q{{9, 1, -5, -16}};
    //print_this(P);
    //print_this(Q);
    // see ELA p.156
    ASSERT_TRUE(P || Q);
}

TEST(PlaneTest, Perpendicular) {
    R3::point Pt(1, -4, -3);
    //print_this(Pt);
    plane P0{{2, -3, 6, 1}};
    plane P1(2, -3, 6, 1);
    //print_this(P0);
    //print_this(P1);
    double P0toPt = P0.distance(Pt);
    double P1toPt = P1.distance(Pt);
    //printf("P0toPt = %E, P1toPt = %E, real=%E\n", P0toPt, P1toPt, 3.0 / 7.0);
    ASSERT_NEAR(-3.0 / 7.0, P0toPt, 1.0E-15);
    ASSERT_NEAR(-3.0 / 7.0, P1toPt, 1.0E-15);
    ASSERT_TRUE(P0 || P1);
    plane P2{{3, -1, 1, -4}};
    plane P3{{1, 0, 2, 1}};
    ASSERT_FALSE(P2.perpendicular(P3));
    plane P4{{1, -2, 3, -4}};
    plane P5{{-2, 5, 4, 1}};
    ASSERT_TRUE(P4.perpendicular(P5));
    //print_this(P5);
}
