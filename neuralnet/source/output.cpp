
#include "neuralnet/output.hpp"

namespace nn {

output::output(size_t prev, size_t num)
      : inner(layer::type::output, prev, num)
      , error(num, 1)
      , error_value(0.0)
      , rms_value(0.0)
      { }

output::~output() {}

size_t output::infer_label(double& out) {
    double tmp = 0.0;
    size_t index = 0;
    values.for_each([&](size_t y, size_t x, const double& v){
        x |= 0;
        if (tmp < v) {
            tmp = v;
            out = v;
            index = size_t(y);
        }
    });
    return index;
}

void output::reset(void) {
    inner::reset();
    error.zero();
    error_value = rms_value = 0.0;
}

void output::update(void) {
    inner::update();
    rms_value = nn::sum(rms) / rms.rows;
    error_value = nn::sum(error) / error.rows;
}

void output::learn_label(size_t index, double min, double max) {
    basal::exception::throw_unless(index < values.rows, __FILE__, __LINE__);
    linalg::matrix desired = linalg::matrix::zeros(values.rows, 1);
    desired = min;
    desired[index][0] = max;
    beta = desired - values;
    rms += linalg::hadamard(beta, beta);
    error += linalg::hadamard(beta, beta);
    delta = hadamard(values - desired, activation_derivative(zeta));
}

void output::learn_label(const linalg::matrix& desired) {
    basal::exception::throw_unless(desired.rows == values.rows and desired.cols == 1, __FILE__, __LINE__);
    beta = desired - values;
    rms += linalg::hadamard(beta, beta);
    error += linalg::hadamard(beta, beta);
    delta = hadamard(values - desired, activation_derivative(zeta));
}

void output::learn_label(linalg::matrix&& desired) {
    basal::exception::throw_unless(desired.rows == values.rows and desired.cols == 1, __FILE__, __LINE__);
    beta = desired - values;
    rms += linalg::hadamard(beta, beta);
    error += linalg::hadamard(beta, beta);
    delta = hadamard(values - desired, activation_derivative(zeta));
}

}
