#pragma once
/**
 * @file
 * Common header of the linear algebra library.
 * @copyright Copyright 2019 (C) Erik Rainey.
 */
#include <basal/exception.hpp>
#include <limits>

/** The Linear Algebra Library Namespace */
namespace linalg {
// Enforce the IEEE-754 compliance
static_assert(std::numeric_limits<double>::is_iec559, "Platform must be IEEE-754 formatted.");
}  // namespace linalg

#include <linalg/matrix.hpp>
#include <linalg/solvers.hpp>
#include <linalg/types.hpp>
