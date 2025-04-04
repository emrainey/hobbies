#ifndef BASAL_GTEST_HELPER_HPP_
#define BASAL_GTEST_HELPER_HPP_

#include <gtest/gtest.h>
#include <basal/ieee754.hpp>

#if defined(USE_PRECISION_AS_FLOAT)
#define ASSERT_PRECISION_EQ(a, b) ASSERT_FLOAT_EQ(a, b)
#define ASSERT_PRECISION_NE(a, b) ASSERT_FLOAT_NE(a, b)
#define EXPECT_PRECISION_EQ(a, b) EXPECT_FLOAT_EQ(a, b)
#define EXPECT_PRECISION_NE(a, b) EXPECT_FLOAT_NE(a, b)
#else
#define ASSERT_PRECISION_EQ(a, b) ASSERT_DOUBLE_EQ(a, b)
#define ASSERT_PRECISION_NE(a, b) ASSERT_PRED_FORMAT2(not::testing::internal::CmpHelperFloatingPointEQ<double>, a, b)
#define EXPECT_PRECISION_EQ(a, b) EXPECT_DOUBLE_EQ(a, b)
#define EXPECT_PRECISION_NE(a, b) EXPECT_PRED_FORMAT2(not::testing::internal::CmpHelperFloatingPointEQ<double>, a, b)
#endif

#define ASSERT_COMPLEX_EQ(a, b)                  \
    {                                            \
        ASSERT_PRECISION_EQ(a.real(), b.real()); \
        ASSERT_PRECISION_EQ(a.imag(), b.imag()); \
    }
#define EXPECT_COMPLEX_EQ(a, b)                  \
    {                                            \
        EXPECT_PRECISION_EQ(a.real(), b.real()); \
        EXPECT_PRECISION_EQ(a.imag(), b.imag()); \
    }

#define ASSERT_COMPLEX_NEAR(a, b)                    \
    {                                                \
        ASSERT_NEAR(a.real(), b.real(), 0.000125_p); \
        ASSERT_NEAR(a.imag(), b.imag(), 0.000125_p); \
    }
#define EXPECT_COMPLEX_NEAR(a, b)                    \
    {                                                \
        EXPECT_NEAR(a.real(), b.real(), 0.000125_p); \
        EXPECT_NEAR(a.imag(), b.imag(), 0.000125_p); \
    }
#endif  // BASAL_GTEST_HELPER_HPP_