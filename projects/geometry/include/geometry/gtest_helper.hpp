/// @file
/// Googletest Helper Macros for Linear Algebra Objects
/// @copyright Copyright (c) 2020

#pragma once

#include <iostream>
#include <geometry/geometry.hpp>

#include "basal/gtest_helper.hpp"

#define ASSERT_XYZ_EQ(ax, ay, az, obj) \
    {                                  \
        ASSERT_PRECISION_EQ(ax, obj[0]);  \
        ASSERT_PRECISION_EQ(ay, obj[1]);  \
        ASSERT_PRECISION_EQ(az, obj[2]);  \
    }

#define ASSERT_POINT_EQ(pa, pb)                                                               \
    {                                                                                         \
        ASSERT_EQ(pa.dimensions, pb.dimensions);                                              \
        for (size_t i = 0; i < pa.dimensions; i++) {                                          \
            ASSERT_NEAR(pa[i], pb[i], basal::epsilon)                                         \
                << "at [" << i << "]"                                                         \
                << " from " << #pa << " " << pa << " from " << #pb << " " << pb << std::endl; \
        }                                                                                     \
    }

#define ASSERT_VECTOR_EQ(va, vb)                                                              \
    {                                                                                         \
        ASSERT_EQ(va.dimensions, vb.dimensions);                                              \
        for (size_t i = 0; i < vb.dimensions; i++) {                                          \
            ASSERT_NEAR(va[i], vb[i], basal::epsilon)                                         \
                << "at [" << i << "]"                                                         \
                << " from " << #va << " " << va << " from " << #vb << " " << vb << std::endl; \
        }                                                                                     \
    }

#define ASSERT_RAY3_EQ(px, py, pz, vx, vy, vz, r) _assert_ray3_eq(px, py, pz, vx, vy, vz, #r, r)

inline void _assert_ray3_eq(geometry::precision px, geometry::precision py, geometry::precision pz, geometry::precision vx, geometry::precision vy, geometry::precision vz,
                            char const* name __attribute__((unused)), geometry::ray_<3> const& r) {
    ASSERT_XYZ_EQ(px, py, pz, r.location());
    ASSERT_XYZ_EQ(vx, vy, vz, r.direction());
}

#define ASSERT_RAY_EQ(pnt, vec, ray)            \
    {                                           \
        ASSERT_POINT_EQ(pnt, ray.location());   \
        ASSERT_VECTOR_EQ(vec, ray.direction()); \
    }
