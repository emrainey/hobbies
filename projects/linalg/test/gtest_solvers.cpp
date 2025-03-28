#include "basal/gtest_helper.hpp"

#include <basal/basal.hpp>
#include <linalg/linalg.hpp>
#include <vector>

#include "linalg/gtest_helper.hpp"

using namespace basal::literals;

TEST(LinalgExtraTests, QuadraticRoots) {
    using namespace linalg;
    auto roots = quadratic_roots(4, 4, 1);
    ASSERT_PRECISION_EQ(std::get<0>(roots), std::get<1>(roots));
    // printf("roots=%E\n", std::get<0>(roots));
    ASSERT_PRECISION_EQ(-4.0_p / 8.0_p, std::get<0>(roots));
    roots = quadratic_roots(1, -9, 7);
    ASSERT_PRECISION_EQ((9 + sqrt(53.0_p)) / 2.0_p, std::get<0>(roots));
    ASSERT_PRECISION_EQ((9 - sqrt(53.0_p)) / 2.0_p, std::get<1>(roots));
    roots = quadratic_roots(2, 5, -3);
    ASSERT_PRECISION_EQ(0.5_p, std::get<0>(roots));
    ASSERT_PRECISION_EQ(-3, std::get<1>(roots));
}

TEST(LinalgExtraTests, QuadraticRootsFail) {
    using namespace linalg;
    // y = 3x^2 + 2x + 1, where y = 0
    auto roots = quadratic_roots(3, 2, 1);
    ASSERT_TRUE(basal::is_nan(std::get<0>(roots)));
    ASSERT_TRUE(basal::is_nan(std::get<1>(roots)));
}

TEST(LinalgExtraTests, CubeRootTest) {
    using namespace std::literals::complex_literals;
    std::complex<basal::precision> f = 2.0_p + std::complex<basal::precision>(2.0i);
    std::complex<basal::precision> g = 2.0_p - std::complex<basal::precision>(2.0i);
    // assumptions
    ASSERT_PRECISION_EQ(sqrt(8), abs(f));
    ASSERT_PRECISION_EQ(iso::pi / 4, arg(f));
    // find the 3 roots
    auto f3 = linalg::cbrt(f);
    auto g3 = linalg::cbrt(g);

    // this is what the real part should be
    basal::precision d1 = 2.0_p * sqrt(2) * cos(iso::pi / 12.0_p);
    basal::precision d2 = 2.0_p * sqrt(2) * cos(iso::pi / 12.0_p + 2 * iso::pi / 3);
    basal::precision d3 = 2.0_p * sqrt(2) * cos(iso::pi / 12.0_p + 4 * iso::pi / 3);

    auto h = std::get<0>(f3) + std::get<0>(g3);
    ASSERT_PRECISION_EQ(abs(h), real(h));
    ASSERT_NEAR(d1, real(h), basal::epsilon);
    ASSERT_PRECISION_EQ(0.0_p, imag(h));

    auto k = std::get<2>(f3) + std::get<1>(g3);
    ASSERT_NEAR(d3, real(k), basal::epsilon);

    auto j = std::get<1>(f3) + std::get<2>(g3);
    ASSERT_NEAR(d2, real(j), basal::epsilon);
}

#define EXPECT_TRIPLE_TUPLE_EQ1(Z, tup)                                            \
    {                                                                              \
        basal::precision x1 = std::get<0>(tup);                                    \
        basal::precision x2 = std::get<1>(tup);                                    \
        basal::precision x3 = std::get<2>(tup);                                    \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << std::endl; \
        std::cout << "Should be: " << Z << std::endl;                              \
        if (not basal::is_nan(x1)) {                                               \
            EXPECT_NEAR(Z, x1, basal::epsilon);                                    \
            EXPECT_TRUE(basal::is_nan(x2));                                        \
            EXPECT_TRUE(basal::is_nan(x3));                                        \
        } else if (not basal::is_nan(x2)) {                                        \
            EXPECT_TRUE(basal::is_nan(x1));                                        \
            EXPECT_NEAR(Z, x2, basal::epsilon);                                    \
            EXPECT_TRUE(basal::is_nan(x3));                                        \
        } else if (not basal::is_nan(x3)) {                                        \
            EXPECT_TRUE(basal::is_nan(x1));                                        \
            EXPECT_TRUE(basal::is_nan(x2));                                        \
            EXPECT_NEAR(Z, x3, basal::epsilon);                                    \
        }                                                                          \
    }

TEST(LinalgExtraTests, CubicRoots01) {
    using namespace linalg;
    // @see https://www.desmos.com/calculator/nnybnwsnkn
    // for playing with the a,b,c,d dials and checking if D > 0 for these cases
    auto roots = cubic_roots(1, 1, -4, 3);
    EXPECT_TRIPLE_TUPLE_EQ1(-2.80630071674_p, roots);
    roots = cubic_roots(1, 0, -15, -126);  // from a mathologer video
    EXPECT_TRIPLE_TUPLE_EQ1(6, roots);
    roots = cubic_roots(1, 1, -3, -6);
    EXPECT_TRIPLE_TUPLE_EQ1(2, roots);
}

#define EXPECT_TRIPLE_TUPLE_EQ2(A, B, tup)                                         \
    {                                                                              \
        basal::precision x1 = std::get<0>(tup);                                    \
        basal::precision x2 = std::get<1>(tup);                                    \
        basal::precision x3 = std::get<2>(tup);                                    \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << std::endl; \
        std::cout << "Should be: " << A << ", " << B << std::endl;                 \
        if (basal::is_nan(x3)) {                                                   \
            EXPECT_NEAR(A, x1, basal::epsilon);                                    \
            EXPECT_NEAR(B, x2, basal::epsilon);                                    \
            EXPECT_TRUE(basal::is_nan(x3));                                        \
        } else if (basal::is_nan(x2)) {                                            \
            EXPECT_NEAR(A, x1, basal::epsilon);                                    \
            EXPECT_TRUE(basal::is_nan(x2));                                        \
            EXPECT_NEAR(B, x3, basal::epsilon);                                    \
        } else if (basal::is_nan(x1)) {                                            \
            EXPECT_TRUE(basal::is_nan(x1));                                        \
            EXPECT_NEAR(A, x2, basal::epsilon);                                    \
            EXPECT_NEAR(B, x3, basal::epsilon);                                    \
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
        basal::precision x1 = std::get<0>(tup);                                    \
        basal::precision x2 = std::get<1>(tup);                                    \
        basal::precision x3 = std::get<2>(tup);                                    \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << std::endl; \
        std::cout << "Should be: " << A << ", " << B << ", " << C << std::endl;    \
        EXPECT_NEAR(A, x1, basal::epsilon);                                        \
        EXPECT_NEAR(B, x2, basal::epsilon);                                        \
        EXPECT_NEAR(C, x3, basal::epsilon);                                        \
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
        basal::precision x1 = std::get<0>(tup);                                                  \
        basal::precision x2 = std::get<1>(tup);                                                  \
        basal::precision x3 = std::get<2>(tup);                                                  \
        basal::precision x4 = std::get<3>(tup);                                                  \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << ", " << x4 << std::endl; \
        std::cout << "Should be: " << A << ", " << B << ", " << C << ", " << D << std::endl;     \
        EXPECT_NEAR(A, x1, basal::epsilon);                                                      \
        EXPECT_NEAR(B, x2, basal::epsilon);                                                      \
        EXPECT_NEAR(C, x3, basal::epsilon);                                                      \
        EXPECT_NEAR(D, x4, basal::epsilon);                                                      \
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
    EXPECT_QUAD_TUPLE_EQ4(3.7320508075688767_p, 0.26794919243112258_p, 1.7320508075688774_p, -1.7320508075688767_p,
                          roots);

    roots = quartic_roots(1, 6, -5, -10, -3);
    EXPECT_QUAD_TUPLE_EQ4((1 + sqrt(5)) / 2, (1 - sqrt(5)) / 2, (-7 + sqrt(37)) / 2, (-7 - sqrt(37)) / 2, roots);
}

#define EXPECT_QUAD_TUPLE_EQ3(A, B, C, tup)                                                      \
    {                                                                                            \
        basal::precision x1 = std::get<0>(tup);                                                  \
        basal::precision x2 = std::get<1>(tup);                                                  \
        basal::precision x3 = std::get<2>(tup);                                                  \
        basal::precision x4 = std::get<3>(tup);                                                  \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << ", " << x4 << std::endl; \
        std::cout << "Should be: " << A << ", " << B << ", " << C << ", " << std::endl;          \
        if (basal::is_nan(x1)) {                                                                 \
            EXPECT_NEAR(A, x2, basal::epsilon);                                                  \
            EXPECT_NEAR(B, x3, basal::epsilon);                                                  \
            EXPECT_NEAR(C, x4, basal::epsilon);                                                  \
        } else if (basal::is_nan(x2)) {                                                          \
            EXPECT_NEAR(A, x1, basal::epsilon);                                                  \
            EXPECT_NEAR(B, x3, basal::epsilon);                                                  \
            EXPECT_NEAR(C, x4, basal::epsilon);                                                  \
        } else if (basal::is_nan(x3)) {                                                          \
            EXPECT_NEAR(A, x1, basal::epsilon);                                                  \
            EXPECT_NEAR(B, x2, basal::epsilon);                                                  \
            EXPECT_NEAR(C, x4, basal::epsilon);                                                  \
        } else if (basal::is_nan(x4)) {                                                          \
            EXPECT_NEAR(A, x1, basal::epsilon);                                                  \
            EXPECT_NEAR(B, x2, basal::epsilon);                                                  \
            EXPECT_NEAR(C, x3, basal::epsilon);                                                  \
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
        basal::precision x1 = std::get<0>(tup);                                                  \
        basal::precision x2 = std::get<1>(tup);                                                  \
        basal::precision x3 = std::get<2>(tup);                                                  \
        basal::precision x4 = std::get<3>(tup);                                                  \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << ", " << x4 << std::endl; \
        std::cout << "Should be: " << A << ", " << B << std::endl;                               \
        EXPECT_NEAR(A, x1, basal::epsilon);                                                      \
        EXPECT_TRUE(basal::is_nan(x2));                                                          \
        EXPECT_NEAR(B, x3, basal::epsilon);                                                      \
        EXPECT_TRUE(basal::is_nan(x4));                                                          \
    }

TEST(LinalgExtraTests, QuarticRoots02) {
    using namespace linalg;
    // @see https://www.desmos.com/calculator/nnybnwsnkn
    // for playing with the a,b,c,d dials and checking if D < 0 for these cases
    auto roots = quartic_roots(1, -4, 6, -4, -3);
    EXPECT_QUAD_TUPLE_EQ2(2.4142135623730949_p, -0.414214_p, roots);

    roots = quartic_roots(1, 6, 7, -7, -12);
    EXPECT_QUAD_TUPLE_EQ2(1.1478990357_p, -4, roots);
}

#define EXPECT_QUAD_TUPLE_EQ1(A, tup)                                                            \
    {                                                                                            \
        basal::precision x1 = std::get<0>(tup);                                                  \
        basal::precision x2 = std::get<1>(tup);                                                  \
        basal::precision x3 = std::get<2>(tup);                                                  \
        basal::precision x4 = std::get<3>(tup);                                                  \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << ", " << x4 << std::endl; \
        std::cout << "Should be all nan" << std::endl;                                           \
        if (not basal::is_nan(x1)) {                                                             \
            EXPECT_NEAR(A, x1, basal::epsilon);                                                  \
        }                                                                                        \
        if (not basal::is_nan(x2)) {                                                             \
            EXPECT_NEAR(A, x2, basal::epsilon);                                                  \
        }                                                                                        \
        if (not basal::is_nan(x3)) {                                                             \
            EXPECT_NEAR(A, x3, basal::epsilon);                                                  \
        }                                                                                        \
        if (not basal::is_nan(x4)) {                                                             \
            EXPECT_NEAR(A, x4, basal::epsilon);                                                  \
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
        basal::precision x1 = std::get<0>(tup);                                                  \
        basal::precision x2 = std::get<1>(tup);                                                  \
        basal::precision x3 = std::get<2>(tup);                                                  \
        basal::precision x4 = std::get<3>(tup);                                                  \
        std::cout << "Solutions: " << x1 << ", " << x2 << ", " << x3 << ", " << x4 << std::endl; \
        std::cout << "Should be all nan" << std::endl;                                           \
        EXPECT_TRUE(basal::is_nan(x1));                                                          \
        EXPECT_TRUE(basal::is_nan(x2));                                                          \
        EXPECT_TRUE(basal::is_nan(x3));                                                          \
        EXPECT_TRUE(basal::is_nan(x4));                                                          \
    }
TEST(LinalgExtraTests, QuarticRoots00) {
    using namespace linalg;
    // @see https://www.desmos.com/calculator/nnybnwsnkn
    // for playing with the a,b,c,d dials and checking if D < 0 for these cases
    auto roots = quartic_roots(0.6_p, -3.4_p, 5.8_p, -2.8_p, 1.4_p);
    EXPECT_QUAD_TUPLE_EQ0(roots);
}