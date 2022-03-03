#include <gtest/gtest.h>

#include <basal/basal.hpp>
#include <linalg/linalg.hpp>
#include <vector>

#include "linalg/gtest_helper.hpp"

TEST(LinalgExtraTests, QuadraticRoots) {
    using namespace linalg;
    auto roots = quadratic_roots(4, 4, 1);
    ASSERT_DOUBLE_EQ(std::get<0>(roots), std::get<1>(roots));
    // printf("roots=%E\n", std::get<0>(roots));
    ASSERT_DOUBLE_EQ(-4.0 / 8.0, std::get<0>(roots));
    roots = quadratic_roots(1, -9, 7);
    ASSERT_DOUBLE_EQ((9 + sqrt(53.0)) / 2.0, std::get<0>(roots));
    ASSERT_DOUBLE_EQ((9 - sqrt(53.0)) / 2.0, std::get<1>(roots));
    roots = quadratic_roots(2, 5, -3);
    ASSERT_DOUBLE_EQ(0.5, std::get<0>(roots));
    ASSERT_DOUBLE_EQ(-3, std::get<1>(roots));
}

TEST(LinalgExtraTests, QuadraticRootsFail) {
    using namespace linalg;
    // y = 3x^2 + 2x + 1, where y = 0
    auto roots = quadratic_roots(3, 2, 1);
    ASSERT_TRUE(std::isnan(std::get<0>(roots)));
    ASSERT_TRUE(std::isnan(std::get<1>(roots)));
}

TEST(LinalgExtraTests, CubeRootTest) {
    using namespace std::literals::complex_literals;
    std::complex<double> f = 2.0 + 2.0i;
    std::complex<double> g = 2.0 - 2.0i;
    // assumptions
    ASSERT_DOUBLE_EQ(sqrt(8), abs(f));
    ASSERT_DOUBLE_EQ(iso::pi / 4, arg(f));
    // find the 3 roots
    auto f3 = linalg::cbrt(f);
    auto g3 = linalg::cbrt(g);

    // this is what the real part should be
    double d1 = 2 * sqrt(2) * cos(iso::pi / 12.0);
    double d2 = 2 * sqrt(2) * cos(iso::pi / 12.0 + 2 * iso::pi / 3);
    double d3 = 2 * sqrt(2) * cos(iso::pi / 12.0 + 4 * iso::pi / 3);

    auto h = std::get<0>(f3) + std::get<0>(g3);
    ASSERT_DOUBLE_EQ(abs(h), real(h));
    ASSERT_NEAR(d1, real(h), 1E-9);
    ASSERT_DOUBLE_EQ(0.0, imag(h));

    auto k = std::get<2>(f3) + std::get<1>(g3);
    ASSERT_NEAR(d3, real(k), 1E-9);

    auto j = std::get<1>(f3) + std::get<2>(g3);
    ASSERT_NEAR(d2, real(j), 1E-9);
}

#define EXPECT_TRIPLE_TUPLE_EQ1(Z, tup)                                            \
    {                                                                              \
        element_type x1 = std::get<0>(tup);                                        \
        element_type x2 = std::get<1>(tup);                                        \
        element_type x3 = std::get<2>(tup);                                        \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << std::endl; \
        std::cout << "Should be: " << Z << std::endl;                              \
        if (not std::isnan(x1)) {                                                  \
            EXPECT_NEAR(Z, x1, 1E-6);                                              \
            EXPECT_TRUE(std::isnan(x2));                                           \
            EXPECT_TRUE(std::isnan(x3));                                           \
        } else if (not std::isnan(x2)) {                                           \
            EXPECT_TRUE(std::isnan(x1));                                           \
            EXPECT_NEAR(Z, x2, 1E-6);                                              \
            EXPECT_TRUE(std::isnan(x3));                                           \
        } else if (not std::isnan(x3)) {                                           \
            EXPECT_TRUE(std::isnan(x1));                                           \
            EXPECT_TRUE(std::isnan(x2));                                           \
            EXPECT_NEAR(Z, x3, 1E-6);                                              \
        }                                                                          \
    }

TEST(LinalgExtraTests, CubicRoots01) {
    using namespace linalg;
    // @see https://www.desmos.com/calculator/nnybnwsnkn
    // for playing with the a,b,c,d dials and checking if D > 0 for these cases
    auto roots = cubic_roots(1, 1, -4, 3);
    EXPECT_TRIPLE_TUPLE_EQ1(-2.80630071674, roots);
    roots = cubic_roots(1, 0, -15, -126);  // from a mathologer video
    EXPECT_TRIPLE_TUPLE_EQ1(6, roots);
    roots = cubic_roots(1, 1, -3, -6);
    EXPECT_TRIPLE_TUPLE_EQ1(2, roots);
}

#define EXPECT_TRIPLE_TUPLE_EQ2(A, B, tup)                                         \
    {                                                                              \
        element_type x1 = std::get<0>(tup);                                        \
        element_type x2 = std::get<1>(tup);                                        \
        element_type x3 = std::get<2>(tup);                                        \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << std::endl; \
        std::cout << "Should be: " << A << ", " << B << std::endl;                 \
        if (std::isnan(x3)) {                                                      \
            EXPECT_NEAR(A, x1, 1E-6);                                              \
            EXPECT_NEAR(B, x2, 1E-6);                                              \
            EXPECT_TRUE(std::isnan(x3));                                           \
        } else if (std::isnan(x2)) {                                               \
            EXPECT_NEAR(A, x1, 1E-6);                                              \
            EXPECT_TRUE(std::isnan(x2));                                           \
            EXPECT_NEAR(B, x3, 1E-6);                                              \
        } else if (std::isnan(x1)) {                                               \
            EXPECT_TRUE(std::isnan(x1));                                           \
            EXPECT_NEAR(A, x2, 1E-6);                                              \
            EXPECT_NEAR(B, x3, 1E-6);                                              \
        }                                                                          \
    }

TEST(LinalgExtraTests, CubicRoots02) {
    using namespace linalg;
    // @see https://www.desmos.com/calculator/nnybnwsnkn
    // for playing with the a,b,c,d dials and checking if D ==0  0 for these cases
    auto roots = cubic_roots(1, -5, 8, -4);
    EXPECT_TRIPLE_TUPLE_EQ2(1, 2, roots);
}

#define EXPECT_TRIPLE_TUPLE_EQ3(A, B, C, tup)                                      \
    {                                                                              \
        element_type x1 = std::get<0>(tup);                                        \
        element_type x2 = std::get<1>(tup);                                        \
        element_type x3 = std::get<2>(tup);                                        \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << std::endl; \
        std::cout << "Should be: " << A << ", " << B << ", " << C << std::endl;    \
        EXPECT_NEAR(A, x1, 1E-6);                                                  \
        EXPECT_NEAR(B, x2, 1E-6);                                                  \
        EXPECT_NEAR(C, x3, 1E-6);                                                  \
    }

TEST(LinalgExtraTests, CubicRoots03) {
    using namespace linalg;
    // @see https://www.desmos.com/calculator/nnybnwsnkn
    // for playing with the a,b,c,d dials and checking if D < 0 for these cases
    auto roots = cubic_roots(1, -6, 11, -6);
    EXPECT_TRIPLE_TUPLE_EQ3(3, 2, 1, roots);
}

#define EXPECT_QUAD_TUPLE_EQ4(A, B, C, D, tup)                                                   \
    {                                                                                            \
        element_type x1 = std::get<0>(tup);                                                      \
        element_type x2 = std::get<1>(tup);                                                      \
        element_type x3 = std::get<2>(tup);                                                      \
        element_type x4 = std::get<3>(tup);                                                      \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << ", " << x4 << std::endl; \
        std::cout << "Should be: " << A << ", " << B << ", " << C << ", " << D << std::endl;     \
        EXPECT_NEAR(A, x1, 1E-6);                                                                \
        EXPECT_NEAR(B, x2, 1E-6);                                                                \
        EXPECT_NEAR(C, x3, 1E-6);                                                                \
        EXPECT_NEAR(D, x4, 1E-6);                                                                \
    }

TEST(LinalgExtraTests, QuarticRoots04) {
    using namespace linalg;
    // @see https://www.desmos.com/calculator/nnybnwsnkn
    // for playing with the a,b,c,d dials and checking if D < 0 for these cases
    auto roots = quartic_roots(1, -7, 5, 31, -30);
    EXPECT_QUAD_TUPLE_EQ4(5, 1, 3, -2, roots);

    roots = quartic_roots(1, -2, -7, 8, 12);
    EXPECT_QUAD_TUPLE_EQ4(3, -1, 2, -2, roots);

    roots = quartic_roots(1, -4, -2, 12, -3);
    EXPECT_QUAD_TUPLE_EQ4(3.7320508075688767, 0.26794919243112258, 1.7320508075688774, -1.7320508075688767, roots);

    roots = quartic_roots(1, 6, -5, -10, -3);
    EXPECT_QUAD_TUPLE_EQ4((1 + sqrt(5)) / 2, (1 - sqrt(5)) / 2, (-7 + sqrt(37)) / 2, (-7 - sqrt(37)) / 2, roots);
}

#define EXPECT_QUAD_TUPLE_EQ3(A, B, C, tup)                                                      \
    {                                                                                            \
        element_type x1 = std::get<0>(tup);                                                      \
        element_type x2 = std::get<1>(tup);                                                      \
        element_type x3 = std::get<2>(tup);                                                      \
        element_type x4 = std::get<3>(tup);                                                      \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << ", " << x4 << std::endl; \
        std::cout << "Should be: " << A << ", " << B << ", " << C << ", " << std::endl;          \
        if (std::isnan(x1)) {                                                                    \
            EXPECT_NEAR(A, x2, 1E-6);                                                            \
            EXPECT_NEAR(B, x3, 1E-6);                                                            \
            EXPECT_NEAR(C, x4, 1E-6);                                                            \
        } else if (std::isnan(x2)) {                                                             \
            EXPECT_NEAR(A, x1, 1E-6);                                                            \
            EXPECT_NEAR(B, x3, 1E-6);                                                            \
            EXPECT_NEAR(C, x4, 1E-6);                                                            \
        } else if (std::isnan(x3)) {                                                             \
            EXPECT_NEAR(A, x1, 1E-6);                                                            \
            EXPECT_NEAR(B, x2, 1E-6);                                                            \
            EXPECT_NEAR(C, x4, 1E-6);                                                            \
        } else if (std::isnan(x4)) {                                                             \
            EXPECT_NEAR(A, x1, 1E-6);                                                            \
            EXPECT_NEAR(B, x2, 1E-6);                                                            \
            EXPECT_NEAR(C, x3, 1E-6);                                                            \
        }                                                                                        \
    }

TEST(LinalgExtraTests, QuarticRoots03) {
    using namespace linalg;
    // @see https://www.desmos.com/calculator/nnybnwsnkn
    // for playing with the a,b,c,d dials and checking if D < 0 for these cases
    auto roots = quartic_roots(1, -7, 17, -17, 6);
    EXPECT_QUAD_TUPLE_EQ3(1, 2, 3, roots);
}

#define EXPECT_QUAD_TUPLE_EQ2(A, B, tup)                                                         \
    {                                                                                            \
        element_type x1 = std::get<0>(tup);                                                      \
        element_type x2 = std::get<1>(tup);                                                      \
        element_type x3 = std::get<2>(tup);                                                      \
        element_type x4 = std::get<3>(tup);                                                      \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << ", " << x4 << std::endl; \
        std::cout << "Should be: " << A << ", " << B << std::endl;                               \
        EXPECT_NEAR(A, x1, 1E-6);                                                                \
        EXPECT_TRUE(std::isnan(x2));                                                             \
        EXPECT_NEAR(B, x3, 1E-6);                                                                \
        EXPECT_TRUE(std::isnan(x4));                                                             \
    }

TEST(LinalgExtraTests, QuarticRoots02) {
    using namespace linalg;
    // @see https://www.desmos.com/calculator/nnybnwsnkn
    // for playing with the a,b,c,d dials and checking if D < 0 for these cases
    auto roots = quartic_roots(1, -4, 6, -4, -3);
    EXPECT_QUAD_TUPLE_EQ2(2.4142135623730949, -0.414214, roots);

    roots = quartic_roots(1, 6, 7, -7, -12);
    EXPECT_QUAD_TUPLE_EQ2(1.1478990357, -4, roots);
}

#define EXPECT_QUAD_TUPLE_EQ1(A, tup)                                                            \
    {                                                                                            \
        element_type x1 = std::get<0>(tup);                                                      \
        element_type x2 = std::get<1>(tup);                                                      \
        element_type x3 = std::get<2>(tup);                                                      \
        element_type x4 = std::get<3>(tup);                                                      \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << ", " << x4 << std::endl; \
        std::cout << "Should be all nan" << std::endl;                                           \
        if (not std::isnan(x1)) {                                                                \
            EXPECT_NEAR(A, x1, 1E-6);                                                            \
        }                                                                                        \
        if (not std::isnan(x2)) {                                                                \
            EXPECT_NEAR(A, x2, 1E-6);                                                            \
        }                                                                                        \
        if (not std::isnan(x3)) {                                                                \
            EXPECT_NEAR(A, x3, 1E-6);                                                            \
        }                                                                                        \
        if (not std::isnan(x4)) {                                                                \
            EXPECT_NEAR(A, x4, 1E-6);                                                            \
        }                                                                                        \
    }

TEST(LinalgExtraTests, QuarticRoots01) {
    using namespace linalg;
    // @see https://www.desmos.com/calculator/nnybnwsnkn
    // for playing with the a,b,c,d dials and checking if D < 0 for these cases
    auto roots = quartic_roots(1, -4, 6, -4, 1);
    EXPECT_QUAD_TUPLE_EQ1(1, roots);
}

#define EXPECT_QUAD_TUPLE_EQ0(tup)                                                               \
    {                                                                                            \
        element_type x1 = std::get<0>(tup);                                                      \
        element_type x2 = std::get<1>(tup);                                                      \
        element_type x3 = std::get<2>(tup);                                                      \
        element_type x4 = std::get<3>(tup);                                                      \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << ", " << x4 << std::endl; \
        std::cout << "Should be all nan" << std::endl;                                           \
        EXPECT_TRUE(std::isnan(x1));                                                             \
        EXPECT_TRUE(std::isnan(x2));                                                             \
        EXPECT_TRUE(std::isnan(x3));                                                             \
        EXPECT_TRUE(std::isnan(x4));                                                             \
    }
TEST(LinalgExtraTests, QuarticRoots00) {
    using namespace linalg;
    // @see https://www.desmos.com/calculator/nnybnwsnkn
    // for playing with the a,b,c,d dials and checking if D < 0 for these cases
    auto roots = quartic_roots(0.6, -3.4, 5.8, -2.8, 1.4);
    EXPECT_QUAD_TUPLE_EQ0(roots);
}