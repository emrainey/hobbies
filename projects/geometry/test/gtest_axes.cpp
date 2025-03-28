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

TEST(GeomAxesTest, R2AxesFailure) {
    // the axes constructor should throw if the basis are not linearly independent det(M) > 0
    ASSERT_THROW(R2::axes f(R2::point(1.0_p, 2.0_p), R2::vector({1.0_p, 1.0_p}), R2::vector({2.0_p, 2.0_p})),
                 basal::exception);
}

TEST(GeomAxesTest, R2Axes) {
    R2::axes a{R2::point{1.0_p, 2.0_p}, R2::vector{1.0_p, 1.0_p}, R2::vector{-1.0_p, 1.0_p}};
    ASSERT_POINT_EQ(R2::point(1.0_p, 2.0_p), a.origin());
    ASSERT_VECTOR_EQ(R2::vector({1.0_p, 1.0_p}), a.abscissa());
    ASSERT_VECTOR_EQ(R2::vector({-1.0_p, 1.0_p}), a.ordinate());
    ASSERT_TRUE(a.is_basis());
    matrix m{{{1.0_p, -1.0_p}, {1.0_p, 1.0_p}}};
    ASSERT_MATRIX_EQ(m, a.from_basis());
    ASSERT_MATRIX_EQ(m.inverse(), a.to_basis());
    // each basis vector should map to a unit point
    auto i = a.to_basis() * a.abscissa();
    auto j = a.to_basis() * a.ordinate();
    ASSERT_VECTOR_EQ(R2::basis::X, i);
    ASSERT_VECTOR_EQ(R2::basis::Y, j);
    // convert back from units to basis
    auto u = a.from_basis() * i;
    auto v = a.from_basis() * j;
    ASSERT_VECTOR_EQ(a.abscissa(), u);
    ASSERT_VECTOR_EQ(a.ordinate(), v);
}

TEST(GeomAxesTest, R3Axes) {
    R3::axes a{R3::point{1.0_p, 2.0_p, 3.0_p}, R3::vector{1.0_p, 1.0_p, 0.0_p}, R3::vector{-1.0_p, 1.0_p, 0.0_p},
               R3::vector{0.0_p, 0.0_p, 1.0_p}};
    ASSERT_POINT_EQ(R3::point(1.0_p, 2.0_p, 3.0_p), a.origin());
    ASSERT_VECTOR_EQ(R3::vector({1.0_p, 1.0_p, 0.0_p}), a.abscissa());
    ASSERT_VECTOR_EQ(R3::vector({-1.0_p, 1.0_p, 0.0_p}), a.ordinate());
    ASSERT_VECTOR_EQ(R3::vector({0.0_p, 0.0_p, 1.0_p}), a.applicate());
    ASSERT_TRUE(a.is_basis());
    matrix m{{{1.0_p, -1.0_p, 0.0_p}, {1.0_p, 1.0_p, 0.0_p}, {0.0_p, 0.0_p, 1.0_p}}};
    ASSERT_MATRIX_EQ(m, a.from_basis());
    ASSERT_MATRIX_EQ(m.inverse(), a.to_basis());
    auto i = a.to_basis() * a.abscissa();
    auto j = a.to_basis() * a.ordinate();
    auto k = a.to_basis() * a.applicate();
    // each basis vector should map to a unit point
    ASSERT_VECTOR_EQ(R3::basis::X, i);
    ASSERT_VECTOR_EQ(R3::basis::Y, j);
    ASSERT_VECTOR_EQ(R3::basis::Z, k);
    // convert back from units to basis
    auto u = a.from_basis() * i;
    auto v = a.from_basis() * j;
    auto w = a.from_basis() * k;
    ASSERT_VECTOR_EQ(a.abscissa(), u);
    ASSERT_VECTOR_EQ(a.ordinate(), v);
    ASSERT_VECTOR_EQ(a.applicate(), w);
}
