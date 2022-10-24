/**
 * @file
 * Googletest Helper Macros for Linear Algebra Objects
 * @copyright Copyright (c) 2020
 */

#pragma once

#include <iostream>
#include <raytrace/types.hpp>

#include "linalg/gtest_helper.hpp"

#define ASSERT_COLOR_EQ(simple, compound)                      \
    {                                                          \
        ASSERT_NEAR(simple.red(), (compound).red(), 1E-5);     \
        ASSERT_NEAR(simple.green(), (compound).green(), 1E-5); \
        ASSERT_NEAR(simple.blue(), (compound).blue(), 1E-5);   \
    }

#define ASSERT_IMAGE_POINT_EQ(simple, compound)   \
    {                                             \
        ASSERT_DOUBLE_EQ(simple.x, (compound).x); \
        ASSERT_DOUBLE_EQ(simple.x, (compound).x); \
    }
