#include "basal/gtest_helper.hpp"

#include <basal/basal.hpp>
#include <geometry/geometry.hpp>
#include <vector>

#include "geometry/gtest_helper.hpp"

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

TEST(PointTest, NormalConstructions) {
    point<2> p1{};
    point<3> p2{};
    point<4> p3{};
    R2::point r2p1{};
    R3::point r3p1{};
    R4::point r4p1{};
}

// TEST(PointTest, InvalidConstructors) {
//     ASSERT_THROW(point p{-1}, basal::exception);
// }

TEST(PointTest, ParameterInitializer) {
    // Only for R2. R3. R4
    R2::point r2p{1, 2};
    R3::point r3p{1, 2, 3};
    R4::point r4p{1, 2, 3, 4};

    ASSERT_PRECISION_EQ(2, r2p.dimensions);
    ASSERT_PRECISION_EQ(1, r2p.x);
    ASSERT_PRECISION_EQ(2, r2p.y);

    ASSERT_PRECISION_EQ(3, r3p.dimensions);
    ASSERT_PRECISION_EQ(1, r3p.x);
    ASSERT_PRECISION_EQ(2, r3p.y);
    ASSERT_PRECISION_EQ(3, r3p.z);

    ASSERT_PRECISION_EQ(4, r4p.dimensions);
    ASSERT_PRECISION_EQ(1, r4p.x);
    ASSERT_PRECISION_EQ(2, r4p.y);
    ASSERT_PRECISION_EQ(3, r4p.z);
    ASSERT_PRECISION_EQ(4, r4p.w);
}

TEST(PointTest, PointerConstructor) {
    precision _p[3] = {1, 2, 3};
    point<3> p{&_p[0], 3};
    ASSERT_PRECISION_EQ(3, p.dimensions);
}

TEST(PointTest, ArrayConstructor) {
    precision _p[3] = {1, 2, 3};
    point<3> p{_p};
    ASSERT_PRECISION_EQ(3, p.dimensions);
}

TEST(PointTest, InitializerListConstructor) {
    point<3> p{{7, 8, 9}};
    ASSERT_PRECISION_EQ(3, p.dimensions);
}

TEST(PointTest, CopyConstructorToWrapper) {
    {
        point<2> p1{{1, 2}};  // list initializer
        R2::point p2{p1};     // copy constructor from base
        R2::point p3{p2};     // copy constructor from wrapper

        ASSERT_PRECISION_EQ(2, p2.dimensions);
        ASSERT_PRECISION_EQ(1, p3.x);
        ASSERT_PRECISION_EQ(2, p3.y);
    }
    {
        point<3> p1{{1, 2, 3}};  // list initializer
        R3::point p2{p1};        // copy constructor from base
        R3::point p3{p2};        // copy constructor from wrapper

        ASSERT_PRECISION_EQ(3, p3.dimensions);
        ASSERT_PRECISION_EQ(1, p3.x);
        ASSERT_PRECISION_EQ(2, p3.y);
        ASSERT_PRECISION_EQ(3, p3.z);
    }
    {
        point<4> p1{{1, 2, 3, 4}};  // list initializer
        R4::point p2{p1};           // copy constructor from base
        R4::point p3{p2};           // copy constructor from wrapper

        ASSERT_PRECISION_EQ(4, p3.dimensions);
        ASSERT_PRECISION_EQ(1, p3.x);
        ASSERT_PRECISION_EQ(2, p3.y);
        ASSERT_PRECISION_EQ(3, p3.z);
        ASSERT_PRECISION_EQ(4, p3.w);
    }
}

TEST(PointTest, Assignments) {
    point<3> p1{{1, 2, 3}};
    point<3> p2;  // default constructor
    p2 = p1;      // copy
    ASSERT_PRECISION_EQ(p1[0], p2[0]);
    ASSERT_PRECISION_EQ(p1[1], p2[1]);
    ASSERT_PRECISION_EQ(p1[2], p2[2]);
    point<3> p3{std::move(p2)};  // move
    ASSERT_PRECISION_EQ(p1[0], p3[0]);
    ASSERT_PRECISION_EQ(p1[1], p3[1]);
    ASSERT_PRECISION_EQ(p1[2], p3[2]);

    R3::point p4{1, 2, 3};
    R3::point p5;
    p5 = p4;
    ASSERT_PRECISION_EQ(p4.x, p5.x);
    ASSERT_PRECISION_EQ(p4.y, p5.y);
    ASSERT_PRECISION_EQ(p4.z, p5.z);
    R3::point p6{std::move(p5)};
    ASSERT_PRECISION_EQ(p4.x, p6.x);
    ASSERT_PRECISION_EQ(p4.y, p6.y);
    ASSERT_PRECISION_EQ(p4.z, p6.z);
}

TEST(PointTest, ClassOperators) {
    R3::point p1{1, 2, 3};
    R3::vector p2{{4, 5, 6}};
    R3::point p3{5, 7, 9};

    p1 += p2;
    ASSERT_PRECISION_EQ(p3[0], p1.x);
    ASSERT_PRECISION_EQ(p3[1], p1.y);
    ASSERT_PRECISION_EQ(p3[2], p1.z);

    p3 *= 2.0_p;
    ASSERT_PRECISION_EQ(10, p3[0]);
    ASSERT_PRECISION_EQ(14, p3[1]);
    ASSERT_PRECISION_EQ(18, p3[2]);
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
    R3::point p3 = p2 + v1;
    ASSERT_PRECISION_EQ(9, p3[0]);
    ASSERT_PRECISION_EQ(12, p3[1]);
    ASSERT_PRECISION_EQ(15, p3[2]);
}

TEST(PointTest, Equality) {
    point<3> p1{{1, 2, 3}};
    point<3> p2{{1, 2, 3}};
    point<3> p3{{5, 6, 7}};
    ASSERT_TRUE(p1 == p2);
    ASSERT_TRUE(p1 != p3);
    ASSERT_FALSE(p1 == p3);
}

TEST(PointTest, Templates) {
    point<2> p1{{90, -39}};
    point_<2> p2{p1};
    point_<2> p3{p2};
    ASSERT_PRECISION_EQ(p1[0], p2.x);
    ASSERT_PRECISION_EQ(p1[1], p2.y);
    ASSERT_PRECISION_EQ(p1[0], p3.x);
    ASSERT_PRECISION_EQ(p1[1], p3.y);
}

TEST(PointTest, PointHomogenizing) {
    point_<2> u(2.3_p, 4.5_p);
    point_<3> v{u};  // homogenizing 2d->3d
    ASSERT_PRECISION_EQ(2.3_p, v.x);
    ASSERT_PRECISION_EQ(4.5_p, v.y);
    ASSERT_PRECISION_EQ(1.0_p, v.z);

    point_<3> p3{1, 2, 3};
    point_<4> p4{p3};  // homogenizing 3d->4d
    ASSERT_PRECISION_EQ(1, p4.x);
    ASSERT_PRECISION_EQ(2, p4.y);
    ASSERT_PRECISION_EQ(3, p4.z);
    ASSERT_PRECISION_EQ(1, p4.w);
}

TEST(PointTest, PointToVector) {
    R3::point p1{1, 2, 3};
    R3::point p2{4, 5, 6};
    R3::vector v1{{1 + 4, 2 + 5, 3 + 6}};
    R3::vector v2 = p1 + p2;
    R3::vector v3{{1 - 4, 2 - 5, 3 - 6}};
    R3::vector v4 = p1 - p2;
    ASSERT_VECTOR_EQ(v1, v2);
    ASSERT_VECTOR_EQ(v3, v4);
}

TEST(PointTest, VectorToPoint) {
    R3::vector v1{{1, 2, 3}};
    R3::point p1{as_point(v1)};  // CopyConstruct
    point<3> p2{{1, 2, 3}};
    p1.print(std::cout, "p1");
    ASSERT_POINT_EQ(p2, p1);
    p1 = as_point(v1);  // CopyAssign
    ASSERT_POINT_EQ(p2, p1);
}

TEST(PointTest, Scaling) {
    point<3> p1{{1, 2, 3}};
    point<3> p2{{0, 0, 0}};
    p2 = p1 * 2.0_p;
    point p3{{2, 4, 6}};
    ASSERT_POINT_EQ(p3, p2);
    p2 = 2.0_p * p1;
    ASSERT_POINT_EQ(p3, p2);
}

TEST(PointTest, MatrixMult) {
    matrix m1{matrix::ones(3, 3)};
    point<3> p1{{1, 2, 3}};
    p1.print(std::cout, "p1");
    point<3> p2{{0, 0, 0}};
    p2 = m1 * p1;
    p2.print(std::cout, "p2");
    point<3> p3{{6, 6, 6}};
    ASSERT_POINT_EQ(p3, p2);
}

TEST(PointTest, PairWiseOps) {
    point<3> p1{{1, 2, 3}};
    point<3> p2{{4, 5, 6}};
    point<3> p3{{4, 10, 18}};
    ASSERT_POINT_EQ(p3, geometry::pairwise::multiply(p1, p2));
    point<3> p4{{2, 2, 2}};
    point<3> p5{{2, 5, 9}};
    ASSERT_POINT_EQ(p5, geometry::pairwise::divide(p3, p4));
}

TEST(PointTest, Centroid) {
    point<3> A{{2, 3, 4}};
    point<3> B{{-1, -10, 30}};
    point<3> C{{0, 4, -6}};
    point<3> E{{1.0_p / 3.0_p, -1.0_p, 28.0_p / 3.0_p}};
    R3::point D = centroid(A, B, C);
    ASSERT_POINT_EQ(E, D);

    R3::point F = centroid(A, B);
    point<3> G{{0.5_p, -3.5_p, 17}};
    ASSERT_POINT_EQ(G, F);
}

TEST(PointTest, CentroidFromVector) {
    std::vector<R3::point> points;
    points.push_back(R3::point{1, 2, 3});
    points.push_back(R3::point{4, 5, 6});
    points.push_back(R3::point{7, 8, 9});
    point<3> E{{4, 5, 6}};
    R3::point D = centroid(points);
    ASSERT_POINT_EQ(E, D);
}

TEST(PointTest, Accumulate) {
    point<3> p1{{1, 2, 3}};
    vector_<3> v1{{4, 5, 6}};
    p1 += v1;
    R3::point p2{1 + 4, 2 + 5, 3 + 6};
    ASSERT_POINT_EQ(p2, p1);
}

TEST(PointTest, Deaccumulate) {
    point<3> p1{{1, 2, 3}};
    vector_<3> v1{{4, 5, 6}};
    p1 -= v1;
    R3::point p2{1 - 4, 2 - 5, 3 - 6};
    ASSERT_POINT_EQ(p2, p1);
}