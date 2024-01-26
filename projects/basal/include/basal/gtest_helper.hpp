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
#define ASSERT_PRECISION_NE(a, b) ASSERT_DOUBLE_NE(a, b)
#define EXPECT_PRECISION_EQ(a, b) EXPECT_DOUBLE_EQ(a, b)
#define EXPECT_PRECISION_NE(a, b) EXPECT_DOUBLE_NE(a, b)
#endif

#endif  // BASAL_GTEST_HELPER_HPP_