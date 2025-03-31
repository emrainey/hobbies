#pragma once
/// @file
/// Definitions for smaller types within the linalg space
/// @copyright Copyright 2019 (C) Erik Rainey.

#include <basal/basal.hpp>
#include <iso/iso.hpp>
#include <limits>
#include <linalg/linalg.hpp>

namespace geometry {

/// All the linalg element types are the same
using precision = linalg::precision;

/// We just bring this into our namespace
using matrix = linalg::matrix;

namespace R2 {
constexpr static size_t dimensions = 2;
}

namespace R3 {
constexpr static size_t dimensions = 3;
}

/// Collects the statistics from the geometry library
struct statistics {
    /// The number of dot products
    size_t dot_operations{0U};
    /// The number of cross products
    size_t cross_products{0U};
    /// The number of magnitudes of a vector
    size_t magnitudes{0U};

    static statistics& get() {
        static statistics s;
        return s;
    }
};
}  // namespace geometry
