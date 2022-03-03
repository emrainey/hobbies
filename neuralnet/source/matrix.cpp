
#include "neuralnet/neuralnet.hpp"

namespace nn {

double sigmoid(double x) {
    double s = 1.0 / (1.0 + std::exp(-x));
    if (s == -0.0) {
        s = 0.0;
    }
    return s;
}

double sigmoid_deriv(double x) {
    return sigmoid(x) * (1.0 - sigmoid(x));
}

double tanh_(double x) {
    // return 0.5 * (1.0 + std::tanh((x * 2.0) - 1.0));
    // return tanh(2.0 * x);
    return tanh(x);
}

double tanh_deriv(double x) {
    // return 1.0 - (tanh(x-0.5) + tanh(x-0.5));
    // return 2.0 - 2.0*(tanh(2.0*x) * tanh(2.0*x));
    return 1.0 - (tanh(x) * tanh(x));
}

double relu(double x) {
    return (x > 1.0 ? 1.0 : (x < 0.0 ? (x * leaky) : x));
    // return (x > 1.0 ? 1.0 : (x < -1.0 ? -1.0 : x));
}

double relu_deriv(double x) {
    return (x > 1.0 ? 0.0 : (x < 0.0 ? leaky : 1.0));
    // return (x > 1.0 ? 0.0 : (x < -1.0 ? 0.0 : 1.0));
}

matrix sigmoid(matrix&& mat) {
    return mat.for_each([](double& v) { v = sigmoid(v); });
}

matrix sigmoid(const matrix& mat) {
    matrix m(mat.rows, mat.cols);
    return m.for_each([&](size_t y, size_t x, double& v) { v = sigmoid(mat[y][x]); });
}

matrix sigmoid_deriv(matrix&& mat) {
    return mat.for_each([](double& v) { v = sigmoid_deriv(v); });
}

matrix sigmoid_deriv(const matrix& mat) {
    matrix m(mat.rows, mat.cols);
    return m.for_each([&](size_t y, size_t x, double& v) { v = sigmoid_deriv(mat[y][x]); });
}

matrix tanh_(matrix&& mat) {
    return mat.for_each([](double& v) { v = tanh_(v); });
}

matrix tanh_(const matrix& mat) {
    matrix m(mat.rows, mat.cols);
    return m.for_each([&](size_t y, size_t x, double& v) { v = tanh_(mat[y][x]); });
}

matrix tanh_deriv(matrix&& mat) {
    return mat.for_each([](double& v) { v = tanh_deriv(v); });
}

matrix tanh_deriv(const matrix& mat) {
    matrix m(mat.rows, mat.cols);
    return m.for_each([&](size_t y, size_t x, double& v) { v = tanh_deriv(mat[y][x]); });
}

matrix relu(matrix&& mat) {
    return mat.for_each([](double& v) { v = relu(v); });
}

matrix relu(const matrix& mat) {
    matrix m(mat.rows, mat.cols);
    return m.for_each([&](size_t y, size_t x, double& v) { v = relu(mat[y][x]); });
}

matrix relu_deriv(matrix&& mat) {
    return mat.for_each([](double& v) { v = relu_deriv(v); });
}

matrix relu_deriv(const matrix& mat) {
    matrix m(mat.rows, mat.cols);
    return m.for_each([&](size_t y, size_t x, double& v) { v = relu_deriv(mat[y][x]); });
}

matrix squared(const matrix& mat) {
    matrix m(mat.rows, mat.cols);
    return m.for_each([&](size_t y, size_t x, double& v) { v = mat[y][x] * mat[y][x]; });
}

matrix squared(matrix&& mat) {
    return mat.for_each([](double& v) { v = v * v; });
}

double sum(const matrix& mat) {
    double s = 0.0;
    mat.for_each([&](const double& v) { s += v; });
    return s;
}

double sum(matrix&& mat) {
    double s = 0.0;
    mat.for_each([&](double& v) { s += v; });
    return s;
}

matrix sqrt(const matrix& mat) {
    matrix m(mat.rows, mat.cols);
    return m.for_each([&](size_t y, size_t x, double& v) { v = std::sqrt(mat[y][x]); });
}

matrix sqrt(matrix&& mat) {
    return mat.for_each([](double& v) { v = std::sqrt(v); });
}

}  // namespace nn
