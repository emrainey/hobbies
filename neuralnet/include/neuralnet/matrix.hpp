#pragma once
/**
 * @file
 * Definitions of the matrix operations for Neural networks.
 * @copyright Copyright 2019 (C) Erik Rainey.
 */

#include <linalg/linalg.hpp>

namespace nn {
using namespace linalg;

/** Returns a sigmoid from a double */
double sigmoid(double x);
/** Returns a sigmoid derivative from a double */
double sigmoid_deriv(double x);

/** Returns a sigmoid matrix from a rvalue matrix */
matrix sigmoid(matrix&& mat);
/** Returns a sigmoid matrix from a matrix */
matrix sigmoid(const matrix& mat);

/** Returns a sigmoid derivative matrix from a r-value matrix */
matrix sigmoid_deriv(matrix&& mat);
/** Returns a sigmoid derivative matrix from a matrix */
matrix sigmoid_deriv(const matrix& mat);

/** Returns a hyperbolic tangent from a double */
double tanh_(double x);
/** Returns a hyperbolic tangent matrix from a r-value matrix */
matrix tanh_(matrix&& mat);
/** Returns a hyperbolic tangent matrix from a matrix */
matrix tanh_(const matrix& mat);

/** Returns a hyperbolic tangent derivative from a double */
double tanh_deriv(double x);
/** Returns a hyperbolic tangent derivative matrix from a r-value matrix */
matrix tanh_deriv(matrix&& mat);
/** Returns a hyperbolic tangent derivative matrix from a matrix */
matrix tanh_deriv(const matrix& mat);

/** A fudge factor */
constexpr static const double leaky = 0.15;

/** Returns a RELU value from a double */
double relu(double x);
/** Returns a RELU derivative value from a double */
double relu_deriv(double x);

/** Return the RELU value from a matrix */
matrix relu(const matrix& mat);

/** Returns the RELU value from an r-value matrix*/
matrix relu(matrix&& mat);

/** Return the RELU value from an r-value matrix. The derivative of RELU is 1. */
matrix relu_deriv(matrix&& mat);
/** Return the RELU value from a matrix. The derivative of RELU is 1 */
matrix relu_deriv(const matrix& mat);

/// Squares each value in the matrix
matrix squared(const matrix& mat);
/// Squares each value in the r-value matrix
matrix squared(matrix&& mat);
/// Sums all values in the matrix
double sum(const matrix& mat);
/// Sums all values in the matrix
double sum(matrix&& mat);

/** Returns the square root of each value in the matrix */
matrix sqrt(const matrix& mat);

/** Returns the square root of each value in the r-value matrix */
matrix sqrt(matrix&& mat);
}  // namespace nn
