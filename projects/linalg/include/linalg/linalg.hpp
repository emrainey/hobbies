#pragma once
/// @file
/// Common header of the linear algebra library.
/// @copyright Copyright 2019 (C) Erik Rainey.
#include <basal/exception.hpp>
#include <basal/ieee754.hpp>

#include <limits>

namespace linalg {
namespace debug {
/// Determines if points are on surfaces, thus having a normal
static constexpr bool root{false};
static constexpr bool plotting{false};  ///< Enables plotting of points
}  // namespace debug
}  // namespace linalg

#include <linalg/matrix.hpp>
#include <linalg/solvers.hpp>
#include <linalg/types.hpp>
