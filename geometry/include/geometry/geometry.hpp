#pragma once
/**
 * @file
 * Common header of the Geometry library.
 * @copyright Copyright 2020 (C) Erik Rainey.
 */

#include <limits>

#include <basal/exception.hpp>

/** The Geometry Namespace */
namespace geometry {
    // Enforce the IEEE-754 compliance
    static_assert(std::numeric_limits<double>::is_iec559, "Platform must be IEEE-754 formatted.");
}

#include "geometry/types.hpp" // <-- linalg comes in here
#include "geometry/point.hpp"
#include "geometry/vector.hpp"
#include "geometry/line.hpp"
#include "geometry/ray.hpp"
#include "geometry/plane.hpp"
#include "geometry/sphere.hpp"
#include "geometry/intersection.hpp"
#include "geometry/extra_math.hpp"

