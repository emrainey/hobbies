/// @file
/// Googletest Helper Macros for Linear Algebra Objects
/// @copyright Copyright (c) 2020

#pragma once

#include <iostream>
#include <linalg/matrix.hpp>

#define ASSERT_MATRIX_EQ(a, b)                                                                \
    {                                                                                         \
        ASSERT_EQ(a.cols, b.cols);                                                            \
        ASSERT_EQ(a.rows, b.rows);                                                            \
        for (size_t j = 0; j < a.rows; j++) {                                                 \
            for (size_t i = 0; i < a.cols; i++) {                                             \
                ASSERT_NEAR(a[j][i], b[j][i], basal::epsilon)                                          \
                    << "at [" << j << "][" << i << "]"                                        \
                    << " from " << #a << " " << a << " from " << #b << " " << b << std::endl; \
            }                                                                                 \
        }                                                                                     \
    }

#define EXPECT_MATRIX_EQ(a, b)                                                                \
    {                                                                                         \
        ASSERT_EQ(a.cols, b.cols);                                                            \
        ASSERT_EQ(a.rows, b.rows);                                                            \
        for (size_t j = 0; j < a.rows; j++) {                                                 \
            for (size_t i = 0; i < a.cols; i++) {                                             \
                EXPECT_NEAR(a[j][i], b[j][i], basal::epsilon)                                          \
                    << "at [" << j << "][" << i << "]"                                        \
                    << " from " << #a << " " << a << " from " << #b << " " << b << std::endl; \
            }                                                                                 \
        }                                                                                     \
    }
