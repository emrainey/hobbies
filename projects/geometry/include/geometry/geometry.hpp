#pragma once
/// @file
/// Common header of the Geometry library.
/// @copyright Copyright 2020 (C) Erik Rainey.

#include <basal/exception.hpp>
#include <basal/ieee754.hpp>
#include <limits>

namespace geometry {
/// Determines if points are on surfaces, thus having a normal
static constexpr bool check_on_surface{true};

/// Determines if the check_on_surface should print a warning when the point is not on the surface.
static constexpr bool surface_check_debug{false};
}  // namespace geometry

// clang-format off
#include "geometry/types.hpp"  // <-- linalg comes in here
#include "geometry/point.hpp"
#include "geometry/vector.hpp"
#include "geometry/line.hpp"
#include "geometry/ray.hpp"
#include "geometry/plane.hpp"
#include "geometry/sphere.hpp"
#include "geometry/intersection.hpp"
#include "geometry/extra_math.hpp"
#include "geometry/axes.hpp"
// clang-format on