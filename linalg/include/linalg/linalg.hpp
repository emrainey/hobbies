#pragma once
/**
 * @file
 * Common header of the linear algebra library.
 * @copyright Copyright 2019 (C) Erik Rainey.
 */
#include <limits>
#include <basal/exception.hpp>

/** The Linear Algebra Library Namespace */
namespace linalg {
    // Enforce the IEEE-754 compliance
    static_assert(std::numeric_limits<double>::is_iec559, "Platform must be IEEE-754 formatted.");
}

#include <linalg/types.hpp>
#include <linalg/matrix.hpp>
#include <linalg/solvers.hpp>

