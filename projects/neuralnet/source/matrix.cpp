
#include "neuralnet/neuralnet.hpp"

namespace nn {

precision sigmoid(precision x) {
    precision s = 1.0 / (1.0 + std::exp(-x));
    if (s == -0.0) {
        s = 0.0;
    }
    return s;
}

precision sigmoid_deriv(precision x) {
    return sigmoid(x) * (1.0 - sigmoid(x));
}

precision tanh_(precision x) {
    // return 0.5 * (1.0 + std::tanh((x * 2.0) - 1.0));
    // return tanh(2.0 * x);
    return tanh(x);
}

precision tanh_deriv(precision x) {
    // return 1.0 - (tanh(x-0.5) + tanh(x-0.5));
    // return 2.0 - 2.0*(tanh(2.0*x) * tanh(2.0*x));
    return 1.0 - (tanh(x) * tanh(x));
}

precision relu(precision x) {
    return (x > 1.0 ? 1.0 : (x < 0.0 ? (x * leaky) : x));
    // return (x > 1.0 ? 1.0 : (x < -1.0 ? -1.0 : x));
}

precision relu_deriv(precision x) {
    return (x > 1.0 ? 0.0 : (x < 0.0 ? leaky : 1.0));
    // return (x > 1.0 ? 0.0 : (x < -1.0 ? 0.0 : 1.0));
}

matrix sigmoid(matrix&& mat) {
    return mat.for_each ([](precision& v) { v = sigmoid(v); });
}

matrix sigmoid(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each ([&](size_t y, size_t x, precision& v) { v = sigmoid(mat[y][x]); });
}

matrix sigmoid_deriv(matrix&& mat) {
    return mat.for_each ([](precision& v) { v = sigmoid_deriv(v); });
}

matrix sigmoid_deriv(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each ([&](size_t y, size_t x, precision& v) { v = sigmoid_deriv(mat[y][x]); });
}

matrix tanh_(matrix&& mat) {
    return mat.for_each ([](precision& v) { v = tanh_(v); });
}

matrix tanh_(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each ([&](size_t y, size_t x, precision& v) { v = tanh_(mat[y][x]); });
}

matrix tanh_deriv(matrix&& mat) {
    return mat.for_each ([](precision& v) { v = tanh_deriv(v); });
}

matrix tanh_deriv(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each ([&](size_t y, size_t x, precision& v) { v = tanh_deriv(mat[y][x]); });
}

matrix relu(matrix&& mat) {
    return mat.for_each ([](precision& v) { v = relu(v); });
}

matrix relu(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each ([&](size_t y, size_t x, precision& v) { v = relu(mat[y][x]); });
}

matrix relu_deriv(matrix&& mat) {
    return mat.for_each ([](precision& v) { v = relu_deriv(v); });
}

matrix relu_deriv(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each ([&](size_t y, size_t x, precision& v) { v = relu_deriv(mat[y][x]); });
}

matrix squared(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each ([&](size_t y, size_t x, precision& v) { v = mat[y][x] * mat[y][x]; });
}

matrix squared(matrix&& mat) {
    return mat.for_each ([](precision& v) { v = v * v; });
}

precision sum(matrix const& mat) {
    precision s = 0.0;
    mat.for_each ([&](precision const& v) { s += v; });
    return s;
}

precision sum(matrix&& mat) {
    precision s = 0.0;
    mat.for_each ([&](precision& v) { s += v; });
    return s;
}

matrix sqrt(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each ([&](size_t y, size_t x, precision& v) { v = std::sqrt(mat[y][x]); });
}

matrix sqrt(matrix&& mat) {
    return mat.for_each ([](precision& v) { v = std::sqrt(v); });
}

}  // namespace nn
