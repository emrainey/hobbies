#pragma once
/// @file
/// Definitions of the matrix operations for Neural networks.
/// @copyright Copyright 2019 (C) Erik Rainey.

#include <linalg/linalg.hpp>
#include <basal/ieee754.hpp>
namespace nn {
using namespace linalg;
using precision = basal::precision;

/// Returns a sigmoid from a precision
precision sigmoid(precision x);
/// Returns a sigmoid derivative from a precision
precision sigmoid_deriv(precision x);

/// Returns a sigmoid matrix from a rvalue matrix
matrix sigmoid(matrix&& mat);
/// Returns a sigmoid matrix from a matrix
matrix sigmoid(matrix const& mat);

/// Returns a sigmoid derivative matrix from a r-value matrix
matrix sigmoid_deriv(matrix&& mat);
/// Returns a sigmoid derivative matrix from a matrix
matrix sigmoid_deriv(matrix const& mat);

/// Returns a hyperbolic tangent from a precision
precision tanh_(precision x);
/// Returns a hyperbolic tangent matrix from a r-value matrix
matrix tanh_(matrix&& mat);
/// Returns a hyperbolic tangent matrix from a matrix
matrix tanh_(matrix const& mat);

/// Returns a hyperbolic tangent derivative from a precision
precision tanh_deriv(precision x);
/// Returns a hyperbolic tangent derivative matrix from a r-value matrix
matrix tanh_deriv(matrix&& mat);
/// Returns a hyperbolic tangent derivative matrix from a matrix
matrix tanh_deriv(matrix const& mat);

/// A fudge factor
constexpr static precision const leaky = 0.15_p;

/// Returns a RELU value from a precision
precision relu(precision x);
/// Returns a RELU derivative value from a precision
precision relu_deriv(precision x);

/// Return the RELU value from a matrix
matrix relu(matrix const& mat);

/// Returns the RELU value from an r-value matrix
matrix relu(matrix&& mat);

/// Return the RELU value from an r-value matrix. The derivative of RELU is 1.
matrix relu_deriv(matrix&& mat);
/// Return the RELU value from a matrix. The derivative of RELU is 1
matrix relu_deriv(matrix const& mat);

/// Squares each value in the matrix
matrix squared(matrix const& mat);
/// Squares each value in the r-value matrix
matrix squared(matrix&& mat);
/// Sums all values in the matrix
precision sum(matrix const& mat);
/// Sums all values in the matrix
precision sum(matrix&& mat);

/// Returns the square root of each value in the matrix
matrix sqrt(matrix const& mat);

/// Returns the square root of each value in the r-value matrix
matrix sqrt(matrix&& mat);
}  // namespace nn
