#include <gtest/gtest.h>
#include <geometry/geometry.hpp>
#include <basal/basal.hpp>
#include <vector>
#include <chrono>
#include "geometry/gtest_helper.hpp"

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;

TEST(GeomExtraTests, ClosestPointsFromSkewLines) {
    R3::line L1{{1, 0, 0, 90,-1, 0}};
    R3::line L2{{0, 0, 1,  1, 1, 70}};

    ASSERT_TRUE(skew(L1, L2));

    point P1{{1,-1, 0}};
    point P2{{1, 1, 0}};

    auto points = closest_points_from_skew_lines(L1, L2);

    ASSERT_POINT_EQ(P1, points.first);
    ASSERT_POINT_EQ(P2, points.second);
}

TEST(GeomExtraTests, RotationAroundAxis) {
    const iso::radians theta(iso::pi / 2);
    double cos_t = std::cos(theta.value);
    double sin_t = std::sin(theta.value);

    // rotation around Z (in the XY plane)
    matrix _Rxy{ {{cos_t, -sin_t, 0}, {sin_t, cos_t, 0}, {0, 0, 1}} };
    // rotation around Y in the xz plane
    matrix _Rxz{ {{cos_t, 0, sin_t}, {0, 1, 0}, {-sin_t, 0, cos_t}} };
    // rotation around x in the yz plane
    matrix _Ryz{ {{1, 0, 0}, {0, cos_t, -sin_t}, {0, sin_t, cos_t}} };

    matrix Rxy = rotation(R3::basis::Z, theta);
    ASSERT_MATRIX_EQ(_Rxy, Rxy);
    matrix Rxz = rotation(R3::basis::Y, theta);
    ASSERT_MATRIX_EQ(_Rxz, Rxz);
    matrix Ryz = rotation(R3::basis::X, theta);
    ASSERT_MATRIX_EQ(_Ryz, Ryz);
}

TEST(GeomExtraTests, RotationOfAxes) {
    R3::vector _vy = -R3::basis::Y;
    const iso::radians theta(iso::tau / 4);
    matrix Rxz = rotation(_vy, theta);
    R3::vector rx = R3::vector(Rxz * R3::basis::X);
    ASSERT_VECTOR_EQ(R3::basis::Z, rx);
    matrix Ryz = rotation(R3::basis::X, theta);
    R3::vector ry = R3::vector(Ryz * R3::basis::X);
    ASSERT_VECTOR_EQ(R3::basis::X, ry);
    matrix Rzx = rotation(R3::basis::Y, theta);
    R3::vector rz = R3::vector(Rzx * R3::basis::Z);
    ASSERT_VECTOR_EQ(R3::basis::X, rz);
}

TEST(GeomExtraTests, RotationMatch) {
    const iso::radians theta(iso::tau / 4);
    matrix rz = rotation(R3::basis::Z, theta);
    // this rotates around Z
    matrix R2z{{{0.0, -1.0, 0.0},
                {1.0,  0.0, 0.0},
                {0.0,  0.0, 1.0}}};
    matrix x{{{1}, {0}, {0}}};
    matrix rzx = rz * x;
    matrix y{{{0}, {1}, {0}}};
    ASSERT_TRUE(R2z == rz) << rz;
    ASSERT_TRUE(rzx == y);
}

TEST(GeomExtraTests, RotationOfVectors) {
    R3::vector v0{{0, 0, 1}};

    iso::radians phi(iso::pi/2);
    linalg::matrix r1 = geometry::rotation(R3::basis::Z, -phi);
    linalg::matrix r2 = geometry::rotation(R3::basis::Y, phi);
    linalg::matrix rot(r2 * r1);
    R3::vector v1;
    R3::vector v2;
    R3::vector v3;

    v1 = r1 * v0;
    ASSERT_VECTOR_EQ(v0, v1);
    v2 = r2 * v1;
    ASSERT_VECTOR_EQ(R3::basis::X, v2);
    v3 = rot * v0;
    ASSERT_VECTOR_EQ(R3::basis::X, v3);

    v0 = R3::vector{{1, 1, 1}};
    R3::vector v4{{1, -1,  1}};
    R3::vector v5{{1, -1, -1}};
    v1 = r1 * v0;
    ASSERT_VECTOR_EQ(v4, v1);
    v2 = r2 * v1;
    ASSERT_VECTOR_EQ(v5, v2);
    v3 = rot * v0;
    ASSERT_VECTOR_EQ(v5, v3);
}

TEST(GeomExtraTests, DotPerf) {
    R4::vector v{{1.0, 0.4, 0.8, 0.0}};
    R4::vector u{{0.5, 1.0, 0.4, 1.0}};
    constexpr size_t number_of_ops = 100'000'000;
    auto start = std::chrono::steady_clock::now();
    for (size_t count = 0; count < number_of_ops; /* count++ */) {
        volatile double d = dot(u, v);
        count += (d > 0 ? 1 : 1);
    }
    auto diff = std::chrono::steady_clock::now() - start;
    double rate = (double(number_of_ops) / (diff.count() / 1E9)) / 1E6;
    std::cout << "dots rate:" << rate << " M-dots/sec, Period: " << diff.count()/1E9 << " sec" << std::endl;
}

TEST(GeomExtraTests, CrossPerf) {
    R3::vector a{{1.0, 3.0, 7.2}};
    R3::vector b{{-4.0, 0.9, 12}};
    constexpr size_t number_of_ops = 100'000'000;
    auto start = std::chrono::steady_clock::now();
    for (size_t count = 0; count < number_of_ops; /* count++ */) {
        volatile R3::vector c = R3::cross(a, b);
        count += (c[0] > 0 ? 1 : 1);
    }
    auto diff = std::chrono::steady_clock::now() - start;
    double rate = (double(number_of_ops) / (diff.count() / 1E9)) / 1E6;
    std::cout << "cross rate:" << rate << " M-cross/sec, Period: " << diff.count()/1E9 << " sec" << std::endl;
}
