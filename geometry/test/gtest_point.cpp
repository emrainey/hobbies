#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <geometry/geometry.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

TEST(PointTest, NormalConstructions) {
    point p1{1};
    point p2{2};
    point p3{3};
}

TEST(PointTest, InvalidConstructors) {
    ASSERT_THROW(point p{-1}, basal::exception);
}

TEST(PointTest, ListInitializer) {
    point p{{1, 2, 3, 4}};
    ASSERT_DOUBLE_EQ(4, p.dimensions);
}

TEST(PointTest, ArrayConstructor) {
    double _p[] = {1, 2, 3};
    point *p = new point(_p, 3);
    ASSERT_DOUBLE_EQ(3, p->dimensions);
    delete p;
}

TEST(PointTest, VectorConstructor) {
    std::vector<double> v;
    v.push_back(7);
    v.push_back(8);
    v.push_back(9);
    point p{v};
    ASSERT_DOUBLE_EQ(3, p.dimensions);
}

TEST(PointTest, Assignments) {
    point p1{{1, 2, 3}};
    point p2 = p1;  // copy
    ASSERT_DOUBLE_EQ(p1[0], p2[0]);
    ASSERT_DOUBLE_EQ(p1[1], p2[1]);
    ASSERT_DOUBLE_EQ(p1[2], p2[2]);
    point p3{std::move(p2)};  // move
    ASSERT_DOUBLE_EQ(p1[0], p3[0]);
    ASSERT_DOUBLE_EQ(p1[1], p3[1]);
    ASSERT_DOUBLE_EQ(p1[2], p3[2]);
}

TEST(PointTest, ClassOperators) {
    R3::point p1{1, 2, 3};
    R3::vector p2{{4, 5, 6}};
    R3::point p3{5, 7, 9};

    p1 += p2;
    ASSERT_DOUBLE_EQ(p3[0], p1.x);
    ASSERT_DOUBLE_EQ(p3[1], p1.y);
    ASSERT_DOUBLE_EQ(p3[2], p1.z);

    p3 *= 2.0;
    ASSERT_DOUBLE_EQ(10, p3[0]);
    ASSERT_DOUBLE_EQ(14, p3[1]);
    ASSERT_DOUBLE_EQ(18, p3[2]);
}

TEST(PointTest, NamespaceOperators) {
    R3::point p1{1, 2, 3};
    R3::point p2{4, 5, 6};
    R3::vector v1{{5, 7, 9}};

    // vector between two points
    // operator+ (vector = point + point)
    R3::vector v2 = p1 + p2;
    ASSERT_VECTOR_EQ(v1, v2);

    // point translation
    // operator+ (point = point + vector)
    point p3 = p2 + v1;
    ASSERT_DOUBLE_EQ(9, p3[0]);
    ASSERT_DOUBLE_EQ(12, p3[1]);
    ASSERT_DOUBLE_EQ(15, p3[2]);
}

TEST(PointTest, Equality) {
    point p1{{1, 2, 3}};
    point p2{{1, 2, 3}};
    point p3{{5, 6, 7}};
    ASSERT_TRUE(p1 == p2);
    ASSERT_TRUE(p1 != p3);
    ASSERT_FALSE(p1 == p3);
}

TEST(PointTest, Templates) {
    point p1{{90, -39}};
    point_<2> p2(p1);
    ASSERT_DOUBLE_EQ(p1[0], p2.x);
    ASSERT_DOUBLE_EQ(p1[1], p2.y);
}

TEST(PointTest, PointHomogenizing) {
    point_<2> u(2.3, 4.5);
    R3::point v{u};  // homogenizing 2d->3d
    ASSERT_DOUBLE_EQ(2.3, v.x);
    ASSERT_DOUBLE_EQ(4.5, v.y);
    ASSERT_DOUBLE_EQ(1.0, v.z);
}

TEST(PointTest, VectorToPoint) {
    R3::vector v1{{1, 2, 3}};
    point p1{as_point(v1)};  // CopyConstruct
    point p2{{1, 2, 3}};
    p1.print("p1");
    ASSERT_POINT_EQ(p2, p1);
    p1 = as_point(v1);  // CopyAssign
    ASSERT_POINT_EQ(p2, p1);
}

TEST(PointTest, Scaling) {
    point p1{{1, 2, 3}};
    point p2{{0, 0, 0}};
    p2 = p1 * 2.0;
    point p3{{2, 4, 6}};
    ASSERT_POINT_EQ(p3, p2);
    p2 = 2.0 * p1;
    ASSERT_POINT_EQ(p3, p2);
}

TEST(PointTest, MatrixMult) {
    matrix m1{matrix::ones(3, 3)};
    point p1{{1, 2, 3}};
    p1.print("p1");
    point p2{{0, 0, 0}};
    p2 = m1 * p1;
    p2.print("p2");
    point p3{{6, 6, 6}};
    ASSERT_POINT_EQ(p3, p2);
}

TEST(PointTest, PairWiseOps) {
    point p1{{1, 2, 3}};
    point p2{{4, 5, 6}};
    point p3{{4, 10, 18}};
    ASSERT_POINT_EQ(p3, geometry::pairwise::multiply(p1, p2));
    point p4{{2, 2, 2}};
    point p5{{2, 5, 9}};
    ASSERT_POINT_EQ(p5, geometry::pairwise::divide(p3, p4));
}

TEST(PointTest, Centroid) {
    point A{{2, 3, 4}};
    point B{{-1, -10, 30}};
    point C{{0, 4, -6}};
    point E{{1.0 / 3.0, -1.0, 28.0 / 3.0}};
    point D = centroid(A, B, C);
    ASSERT_POINT_EQ(E, D);
}