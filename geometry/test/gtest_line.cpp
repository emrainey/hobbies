#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <geometry/geometry.hpp>

#include "geometry/gtest_helper.hpp"

using namespace geometry;
using namespace geometry::operators;
using namespace linalg;
using namespace linalg::operators;

TEST(LineTest, ConstructorsAndAssigns) {
    R3::vector v1{{1, 2, 3}};
    R3::point p1(4, 5, 6);
    R3::point p2 = p1 + v1;
    R3::line l1(v1, p1);
    R3::line l2(p1, p2);
    ASSERT_TRUE(l1 == l2);
    ASSERT_TRUE(parallel(l1, l2));
    R3::line l3{{1, 2, 3, 4, 5, 6}};  // v then p
    ASSERT_TRUE(l1 == l3);
}

TEST(LineTest, Distance) {
    R3::vector v{{1, 1, 0}};
    R3::line l(geometry::R3::origin, v);
    R3::point p(3, 3, 5);
    R3::point q(-2, -3, 0);
    ASSERT_DOUBLE_EQ(5.0, l.distance(p));
    ASSERT_DOUBLE_EQ(sqrt(2.0) / 2, l.distance(q));
}

TEST(LineTest, Solves) {
    R3::point A(1, 2, 3);
    R3::point B(4, 5, 6);
    R3::point C(9, 12, 0);
    R3::vector vA{{1, 1, 1}};

    R3::line L0(vA, A);
    element_type t;
    ASSERT_TRUE(L0.solve(A, t));
    ASSERT_FLOAT_EQ(0, t);

    ASSERT_TRUE(L0.solve(B, t));
    ASSERT_FLOAT_EQ(3.0, t);

    ASSERT_FALSE(L0.solve(C, t));
}

TEST(LineTest, SolvesFlattened) {
    R3::point A(1, 2, 3);
    R3::point B(4, 2, 0);
    R3::point C(9, 3, 10);
    R3::vector vA{{1, 0, -1}};

    R3::line L0(vA, A);
    element_type t;
    ASSERT_TRUE(L0.solve(A, t));
    ASSERT_FLOAT_EQ(0, t);

    ASSERT_TRUE(L0.solve(B, t));
    ASSERT_FLOAT_EQ(3.0, t);

    ASSERT_FALSE(L0.solve(C, t));
}
