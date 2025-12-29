#pragma once

/// @file
/// Definitions for the linalg::softmax function.
/// @copyright Copyright 2019 (C) Erik Rainey.

#include <linalg/linalg.hpp>

namespace linalg {

/// Computes the softmax of a column matrix
/// @param [in] m The column matrix to compute the softmax of
/// @return Returns a column matrix of the softmax values
matrix softmax(const matrix &m);

}  // namespace linalg
