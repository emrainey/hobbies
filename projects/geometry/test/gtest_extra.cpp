#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <chrono>
#include <linalg/linalg.hpp>
#include <geometry/geometry.hpp>
#include <tuple>
#include <vector>

#include "geometry/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;

TEST(GeomExtraTests, ClosestPointsFromSkewLines) {
    R3::line L1{{1, 0, 0, 90, -1, 0}};
    R3::line L2{{0, 0, 1, 1, 1, 70}};

    ASSERT_TRUE(skew(L1, L2));

    R3::point P1{1, -1, 0};
    R3::point P2{1, 1, 0};

    auto points = closest_points_from_skew_lines(L1, L2);

    ASSERT_POINT_EQ(P1, points.first);
    ASSERT_POINT_EQ(P2, points.second);
}

TEST(GeomExtraTests, RotationAroundAxis) {
    iso::radians const theta(iso::pi / 2);
    precision cos_t = std::cos(theta.value);
    precision sin_t = std::sin(theta.value);

    // rotation around Z (in the XY plane)
    matrix _Rxy{{{cos_t, -sin_t, 0}, {sin_t, cos_t, 0}, {0, 0, 1}}};
    // rotation around Y in the xz plane
    matrix _Rxz{{{cos_t, 0, sin_t}, {0, 1, 0}, {-sin_t, 0, cos_t}}};
    // rotation around x in the yz plane
    matrix _Ryz{{{1, 0, 0}, {0, cos_t, -sin_t}, {0, sin_t, cos_t}}};

    matrix Rxy = rotation(R3::basis::Z, theta);
    ASSERT_MATRIX_EQ(_Rxy, Rxy);
    matrix Rxz = rotation(R3::basis::Y, theta);
    ASSERT_MATRIX_EQ(_Rxz, Rxz);
    matrix Ryz = rotation(R3::basis::X, theta);
    ASSERT_MATRIX_EQ(_Ryz, Ryz);
}

TEST(GeomExtraTests, RotationOfAxes) {
    R3::vector _vy = -R3::basis::Y;
    iso::radians const theta(iso::tau / 4);
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
    iso::radians const theta(iso::tau / 4);
    matrix rz = rotation(R3::basis::Z, theta);
    // this rotates around Z
    matrix R2z{{{0.0_p, -1.0_p, 0.0_p}, {1.0_p, 0.0_p, 0.0_p}, {0.0_p, 0.0_p, 1.0_p}}};
    matrix x{{{1}, {0}, {0}}};
    matrix rzx = rz * x;
    matrix y{{{0}, {1}, {0}}};
    ASSERT_TRUE(R2z == rz) << rz;
    ASSERT_TRUE(rzx == y);
}

TEST(GeomExtraTests, RotationOfVectors) {
    R3::vector v0{{0, 0, 1}};

    iso::radians phi(iso::pi / 2);
    linalg::matrix r1 = geometry::rotation(R3::basis::Z, -phi);
    linalg::matrix r2 = geometry::rotation(R3::basis::Y, phi);
    linalg::matrix rot(r2 * r1);
    R3::vector v1{};
    R3::vector v2{};
    R3::vector v3{};

    v1 = r1 * v0;
    ASSERT_VECTOR_EQ(v0, v1);
    v2 = r2 * v1;
    ASSERT_VECTOR_EQ(R3::basis::X, v2);
    v3 = rot * v0;
    ASSERT_VECTOR_EQ(R3::basis::X, v3);

    v0 = R3::vector{{1, 1, 1}};
    R3::vector v4{{1, -1, 1}};
    R3::vector v5{{1, -1, -1}};
    v1 = r1 * v0;
    ASSERT_VECTOR_EQ(v4, v1);
    v2 = r2 * v1;
    ASSERT_VECTOR_EQ(v5, v2);
    v3 = rot * v0;
    ASSERT_VECTOR_EQ(v5, v3);
}

// Moved to Google Benchmark
TEST(GeomExtraTests, DISABLE_DotPerf) {
    R4::vector v{{1.0_p, 0.4_p, 0.8_p, 0.0_p}};
    R4::vector u{{0.5_p, 1.0_p, 0.4_p, 1.0_p}};
    constexpr size_t number_of_ops = 100'000'000;
    auto start = std::chrono::steady_clock::now();
    for (size_t count = 0; count < number_of_ops; /* count++ */) {
        volatile precision d = dot(u, v);
        count += (d > 0 ? 1 : 1);
    }
    auto diff = std::chrono::steady_clock::now() - start;
    precision rate = (precision(number_of_ops) / (diff.count() / 1E9)) / 1E6;
    std::cout << "dots rate:" << rate << " M-dots/sec, Period: " << diff.count() / 1E9 << " sec" << std::endl;
}

// Moved to Google Benchmark
TEST(GeomExtraTests, DISABLE_CrossPerf) {
    R3::vector a{{1.0_p, 3.0_p, 7.2_p}};
    R3::vector b{{-4.0_p, 0.9_p, 12}};
    constexpr size_t number_of_ops = 100'000'000;
    auto start = std::chrono::steady_clock::now();
    for (size_t count = 0; count < number_of_ops; /* count++ */) {
        volatile R3::vector c = R3::cross(a, b);
        count += (c[0] > 0 ? 1 : 1);
    }
    auto diff = std::chrono::steady_clock::now() - start;
    precision rate = (precision(number_of_ops) / (diff.count() / 1E9)) / 1E6;
    std::cout << "cross rate:" << rate << " M-cross/sec, Period: " << diff.count() / 1E9 << " sec" << std::endl;
}

TEST(MappingTests, CartesianToSphericalTest) {
    std::vector<std::tuple<geometry::R3::point, geometry::R3::point>> combos = {
        std::make_tuple(geometry::R3::point(0.0, 0.0, 1.0), geometry::R3::point(1.0, 0.0, 0.0)),
        std::make_tuple(geometry::R3::point(1.0, 0.0, 0.0), geometry::R3::point(1.0, 0.0, iso::pi / 2)),
        std::make_tuple(geometry::R3::point(0.0, 1.0, 0.0), geometry::R3::point(1.0, iso::pi / 2, iso::pi / 2)),
        std::make_tuple(geometry::R3::point(0.0, 0.0, -1.0), geometry::R3::point(1.0, 0.0, iso::pi)),
        std::make_tuple(geometry::R3::point(0.0, -1.0, 0.0), geometry::R3::point(1.0, -iso::pi / 2, iso::pi / 2)),
    };
    for (auto& params : combos) {
        geometry::R3::point cartesian = std::get<0>(params);
        geometry::R3::point spherical = std::get<1>(params);
        geometry::R3::point new_point = cartesian_to_spherical(cartesian);
        ASSERT_POINT_EQ(spherical, new_point);
    }
}

TEST(MappingTests, CartesianToPolarTest) {
    std::vector<std::tuple<geometry::R2::point, geometry::R2::point>> combos = {
        std::make_tuple(geometry::R2::point(0.0, 0.0), geometry::R2::point(0.0, 0.0)),
        std::make_tuple(geometry::R2::point(1.0, 0.0), geometry::R2::point(1.0, 0.0)),
        std::make_tuple(geometry::R2::point(0.0, 1.0), geometry::R2::point(1.0, iso::pi / 2)),
        std::make_tuple(geometry::R2::point(-1.0, 0.0), geometry::R2::point(1.0, iso::pi)),
        std::make_tuple(geometry::R2::point(0.0, -1.0), geometry::R2::point(1.0, -iso::pi / 2)),
    };
    for (auto& params : combos) {
        geometry::R2::point cartesian = std::get<0>(params);
        geometry::R2::point polar = std::get<1>(params);
        geometry::R2::point new_point = cartesian_to_polar(cartesian);
        ASSERT_POINT_EQ(polar, new_point);
    }
}

TEST(Mappers, LinearMapperTest) {
    ASSERT_NEAR(0.0_p, mapping::linear(0.0_p), 1E-6_p);
    ASSERT_NEAR(1.0_p, mapping::linear(1.0_p), 1E-6_p);
}

TEST(Mappers, QuadraticMapperTest) {
    ASSERT_NEAR(0.0_p, mapping::quadratic(0.0_p), 1E-6_p);
    ASSERT_NEAR(1.0_p, mapping::quadratic(1.0_p), 1E-6_p);
}

TEST(Mappers, Cubic) {
    ASSERT_NEAR(0.0_p, mapping::cubic(0.0_p), 1E-6_p);
    ASSERT_NEAR(1.0_p, mapping::cubic(1.0_p), 1E-6_p);
}

TEST(Mappers, Sqrt) {
    ASSERT_NEAR(0.0_p, mapping::sqrt(0.0_p), 1E-6_p);
    ASSERT_NEAR(1.0_p, mapping::sqrt(1.0_p), 1E-6_p);
}

TEST(Mappers, Sin) {
    ASSERT_NEAR(0.0_p, mapping::sin(0.0_p), 1E-6_p);
    ASSERT_NEAR(1.0_p, mapping::sin(1.0_p), 1E-6_p);
}

TEST(Mappers, Ease) {
    ASSERT_NEAR(0.0_p, mapping::ease(0.0_p), 1E-6_p);
    ASSERT_NEAR(1.0_p, mapping::ease(1.0_p), 1E-6_p);
}

TEST(Mappers, Curb) {
    ASSERT_NEAR(0.0_p, mapping::curb(0.0_p), 1E-6_p);
    ASSERT_NEAR(1.0_p, mapping::curb(1.0_p), 1E-6_p);
}

TEST(Mappers, RootSpindle) {
    ASSERT_NEAR(0.0_p, mapping::root_spindle(0.0_p), 1E-6_p);
    ASSERT_NEAR(1.0_p, mapping::root_spindle(1.0_p), 1E-6_p);
}

TEST(Mappers, Spindle) {
    ASSERT_NEAR(0.0_p, mapping::spindle(0.0_p), 1E-6_p);
    ASSERT_NEAR(1.0_p, mapping::spindle(1.0_p), 1E-6_p);
}

TEST(Mappers, Exponential) {
    ASSERT_NEAR(0.0_p, mapping::exponential(0.0_p), 1E-6_p);
    ASSERT_NEAR(1.0_p, mapping::exponential(1.0_p), 1E-6_p);
}

TEST(Mappers, Lerp) {
    ASSERT_NEAR(0.0_p, lerp(0.0_p, 1.0_p, 0.0_p), 1E-6_p);
    ASSERT_NEAR(0.5_p, lerp(0.0_p, 1.0_p, 0.5_p), 1E-6_p);
    ASSERT_NEAR(1.0_p, lerp(0.0_p, 1.0_p, 1.0_p), 1E-6_p);
}

TEST(Mappers, LerpFunctors) {
    std::vector<mapper> mappers
        = {mapping::linear, mapping::quadratic, mapping::cubic,        mapping::sqrt,    mapping::sin,
           mapping::ease,   mapping::curb,      mapping::root_spindle, mapping::spindle, mapping::exponential};

    for (auto& m : mappers) {
        for (precision i = 0.0_p; i <= 1.0_p; i += 1.0_p / 256.0_p) {
            auto v = lerp(0.0_p, 1.0_p, m(i));
            ASSERT_GE(v, 0.0_p);
            ASSERT_LE(v, 1.0_p);
        }
    }
}

TEST(Rotations, Identity) {
    ASSERT_MATRIX_EQ(R3::identity, R3::roll(0.0));
    ASSERT_MATRIX_EQ(R3::identity, R3::pitch(0.0));
    ASSERT_MATRIX_EQ(R3::identity, R3::yaw(0.0));
    ASSERT_MATRIX_EQ(R3::identity, R3::roll(1.0));
    ASSERT_MATRIX_EQ(R3::identity, R3::pitch(1.0));
    ASSERT_MATRIX_EQ(R3::identity, R3::yaw(1.0));
}

TEST(Rotations, Rolls) {
    {
        R3::vector v = R3::basis::Z;
        matrix M = R3::roll(0.25);
        R3::vector a = M * v;
        ASSERT_VECTOR_EQ(R3::vector({0, -1, 0}), a);
    }
    {
        R3::vector v = R3::basis::Y;
        matrix M = R3::roll(0.25);
        R3::vector a = M * v;
        ASSERT_VECTOR_EQ(R3::vector({0, 0, 1}), a);
    }
    {
        R3::vector v = R3::basis::X;
        matrix M = R3::roll(0.25);
        R3::vector a = M * v;
        ASSERT_VECTOR_EQ(R3::vector({1, 0, 0}), a);
    }
    {
        R3::vector v = R3::basis::Z;
        matrix M = R3::roll(0.125);
        R3::vector a = M * v;
        ASSERT_VECTOR_EQ(R3::vector({0, -basal::inv_sqrt_2, basal::inv_sqrt_2}), a);
    }
}

TEST(Rotations, Pitch) {
    {
        R3::vector v = R3::basis::Z;
        matrix M = R3::pitch(0.25);
        R3::vector a = M * v;
        ASSERT_VECTOR_EQ(R3::vector({1, 0, 0}), a);
    }
    {
        R3::vector v = R3::basis::Y;
        matrix M = R3::pitch(0.25);
        R3::vector a = M * v;
        ASSERT_VECTOR_EQ(R3::vector({0, 1, 0}), a);
    }
    {
        R3::vector v = R3::basis::X;
        matrix M = R3::pitch(0.25);
        R3::vector a = M * v;
        ASSERT_VECTOR_EQ(R3::vector({0, 0, -1}), a);
    }
    {
        R3::vector v = R3::basis::Z;
        matrix M = R3::pitch(0.125);
        R3::vector a = M * v;
        ASSERT_VECTOR_EQ(R3::vector({basal::inv_sqrt_2, 0, basal::inv_sqrt_2}), a);
    }
}

TEST(Rotations, Yaw) {
    {
        R3::vector v = R3::basis::Z;
        matrix M = R3::yaw(0.25);
        R3::vector a = M * v;
        ASSERT_VECTOR_EQ(R3::vector({0, 0, 1}), a);
    }
    {
        R3::vector v = R3::basis::Y;
        matrix M = R3::yaw(0.25);
        R3::vector a = M * v;
        ASSERT_VECTOR_EQ(R3::vector({-1, 0, 0}), a);
    }
    {
        R3::vector v = R3::basis::X;
        matrix M = R3::yaw(0.25);
        R3::vector a = M * v;
        ASSERT_VECTOR_EQ(R3::vector({0, 1, 0}), a);
    }
    {
        R3::vector v = R3::basis::X;
        matrix M = R3::yaw(0.125);
        R3::vector a = M * v;
        ASSERT_VECTOR_EQ(R3::vector({basal::inv_sqrt_2, basal::inv_sqrt_2, 0}), a);
    }
}

TEST(Rotations, Compounds) {
    using namespace iso::operators;
    // we want to achieve a |1,1,1| vector from a X basis vector
    // the vector makes an angle of 45 degrees with the x axis
    // and an angle of 45 degrees with the y axis
    // but the angle from the XY plane
    R3::vector f({basal::inv_sqrt_3, basal::inv_sqrt_3, basal::inv_sqrt_3});
    iso::radians alpha = angle(R3::basis::X, f);
    iso::radians beta = angle(R3::basis::Y, f);
    iso::radians gamma = angle(R3::basis::Z, f);
    iso::radians rise = iso::radians(iso::pi / 2) - gamma;
    std::cout << "Turns -> palpha: " << alpha.value / iso::tau << " beta: " << beta.value / iso::tau
              << " gamma: " << gamma.value / iso::tau << " rise: " << rise.value / iso::tau << std::endl;
    R3::vector v = R3::basis::X;
    matrix M = rotation(iso::radians{iso::pi / 4}, iso::radians{-rise}, iso::radians{0});
    //--------------------
    R3::vector a = M * v;
    //--------------------
    ASSERT_VECTOR_EQ(f, a);
}