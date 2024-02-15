/// @file
/// Googletest Helper Macros for Linear Algebra Objects
/// @copyright Copyright (c) 2020

#pragma once

#include <iostream>
#include <raytrace/types.hpp>

#include "basal/gtest_helper.hpp"
#include "linalg/gtest_helper.hpp"
#include "geometry/gtest_helper.hpp"

#define ASSERT_COLOR_EQ(simple, compound)                      \
    {                                                          \
        ASSERT_NEAR(simple.red(), (compound).red(), raytrace::color::equality_limit);     \
        ASSERT_NEAR(simple.green(), (compound).green(), raytrace::color::equality_limit); \
        ASSERT_NEAR(simple.blue(), (compound).blue(), raytrace::color::equality_limit);   \
    }

#define ASSERT_IMAGE_POINT_EQ(simple, compound)   \
    {                                             \
        ASSERT_PRECISION_EQ(simple.x, (compound).x); \
        ASSERT_PRECISION_EQ(simple.y, (compound).y); \
    }
