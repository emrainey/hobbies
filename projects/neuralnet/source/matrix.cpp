
#include "neuralnet/neuralnet.hpp"

namespace nn {

precision sigmoid(precision x) {
    precision s = 1.0_p / (1.0_p + std::exp(-x));
    if (s == -0.0_p) {
        s = 0.0_p;
    }
    return s;
}

precision sigmoid_deriv(precision x) {
    return sigmoid(x) * (1.0_p - sigmoid(x));
}

precision tanh_(precision x) {
    // return 0.5_p * (1.0_p + std::tanh((x * 2.0_p) - 1.0_p));
    // return tanh(2.0_p * x);
    return tanh(x);
}

precision tanh_deriv(precision x) {
    // return 1.0_p - (tanh(x-0.5_p) + tanh(x-0.5_p));
    // return 2.0_p - 2.0*(tanh(2.0*x) * tanh(2.0*x));
    return 1.0_p - (tanh(x) * tanh(x));
}

precision relu(precision x) {
    return (x > 1.0_p ? 1.0_p : (x < 0.0_p ? (x * leaky) : x));
    // return (x > 1.0_p ? 1.0_p : (x < -1.0_p ? -1.0_p : x));
}

precision relu_deriv(precision x) {
    return (x > 1.0_p ? 0.0_p : (x < 0.0_p ? leaky : 1.0_p));
    // return (x > 1.0_p ? 0.0_p : (x < -1.0_p ? 0.0_p : 1.0_p));
}

matrix sigmoid(matrix&& mat) {
    return mat.for_each([](precision& v) { v = sigmoid(v); });
}

matrix sigmoid(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each([&](size_t y, size_t x, precision& v) { v = sigmoid(mat[y][x]); });
}

matrix sigmoid_deriv(matrix&& mat) {
    return mat.for_each([](precision& v) { v = sigmoid_deriv(v); });
}

matrix sigmoid_deriv(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each([&](size_t y, size_t x, precision& v) { v = sigmoid_deriv(mat[y][x]); });
}

matrix tanh_(matrix&& mat) {
    return mat.for_each([](precision& v) { v = tanh_(v); });
}

matrix tanh_(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each([&](size_t y, size_t x, precision& v) { v = tanh_(mat[y][x]); });
}

matrix tanh_deriv(matrix&& mat) {
    return mat.for_each([](precision& v) { v = tanh_deriv(v); });
}

matrix tanh_deriv(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each([&](size_t y, size_t x, precision& v) { v = tanh_deriv(mat[y][x]); });
}

matrix relu(matrix&& mat) {
    return mat.for_each([](precision& v) { v = relu(v); });
}

matrix relu(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each([&](size_t y, size_t x, precision& v) { v = relu(mat[y][x]); });
}

matrix relu_deriv(matrix&& mat) {
    return mat.for_each([](precision& v) { v = relu_deriv(v); });
}

matrix relu_deriv(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each([&](size_t y, size_t x, precision& v) { v = relu_deriv(mat[y][x]); });
}

matrix squared(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each([&](size_t y, size_t x, precision& v) { v = mat[y][x] * mat[y][x]; });
}

matrix squared(matrix&& mat) {
    return mat.for_each([](precision& v) { v = v * v; });
}

precision sum(matrix const& mat) {
    precision s = 0.0_p;
    mat.for_each([&](precision const& v) { s += v; });
    return s;
}

precision sum(matrix&& mat) {
    precision s = 0.0_p;
    mat.for_each([&](precision& v) { s += v; });
    return s;
}

matrix sqrt(matrix const& mat) {
    matrix m{mat.rows, mat.cols};
    return m.for_each([&](size_t y, size_t x, precision& v) { v = std::sqrt(mat[y][x]); });
}

matrix sqrt(matrix&& mat) {
    return mat.for_each([](precision& v) { v = std::sqrt(v); });
}

}  // namespace nn
