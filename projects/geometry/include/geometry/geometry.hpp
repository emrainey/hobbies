#pragma once
/// @file
/// Common header of the Geometry library.
/// @copyright Copyright 2020 (C) Erik Rainey.

#include <basal/exception.hpp>
#include <basal/ieee754.hpp>
#include <limits>

namespace geometry {
/// Determines if points are on surfaces, thus having a normal
constexpr static bool check_on_surface = true;
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
// clang-format on