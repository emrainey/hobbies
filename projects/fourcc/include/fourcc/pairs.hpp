#pragma once

/// @file
/// Maps strings of dimensions to their width and height in pixels.
/// @copyright Copyright 2022 (C) Erik Rainey.

#include <fourcc/types.hpp>

/// The Four Character Code Namespace for Images
namespace fourcc {

/// Returns the dimensions given a constant string.
/// @code
/// auto [width, height] = fourcc::dimensions("VGA");
/// @endcode
std::pair<size_t, size_t> dimensions(std::string type);

}  // namespace fourcc
