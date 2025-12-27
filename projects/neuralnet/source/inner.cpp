#include "neuralnet/inner.hpp"

namespace nn {

using namespace linalg;
using namespace linalg::operators;
using namespace basal::literals;

inner::inner(layer::type _type, size_t inputs, size_t num)
    : layer{_type, num}
    , type{activation_type::Sigmoid}
    , weights{num, inputs}
    , biases{num, 1}
    , delta{num, 1}
    , zeta{num, 1}
    , rms{beta.rows, beta.cols}
    , count{0}
    , current_alpha{0.0_p}
    , current_gamma{0.0_p}
    , last_weight_update{num, inputs}
    , last_bias_update{num, 1}
    , delta_weights{num, inputs}
    , delta_biases{num, 1}
    , applied_weight_update{num, inputs}
    , applied_bias_update{num, 1} {
    weights.random(-1.0_p, 1.0_p);
    biases.random(0.0_p, 0.1_p);
    // biases.fill(1.0_p);
    delta.zero();
    zeta.zero();
    rms.zero();
    last_weight_update.zero();
    last_bias_update.zero();
    delta_biases.zero();
    delta_weights.zero();
    applied_weight_update.zero();
    applied_bias_update.zero();
}

inner::inner(inner const& other) : layer(other.layer_type, other.values.rows) {
    type = other.type;
    values = other.values;
    weights = other.weights;
    biases = other.biases;
    delta = other.delta;
    zeta = other.zeta;
    rms = other.rms;
    count = other.count;
    current_alpha = other.current_alpha;
    current_gamma = other.current_gamma;
    last_weight_update = other.last_weight_update;
    last_bias_update = other.last_bias_update;
    delta_biases = other.delta_biases;
    delta_weights = other.delta_weights;
}

inner::inner(inner&& other) : layer(other.layer_type, other.values.rows) {
    type = other.type;
    values = std::move(other.values);
    weights = std::move(other.weights);
    biases = std::move(other.biases);
    delta = std::move(other.delta);
    zeta = std::move(other.zeta);
    rms = std::move(other.rms);
    count = other.count;
    current_alpha = other.current_alpha;
    current_gamma = other.current_gamma;
    last_weight_update = std::move(other.last_weight_update);
    last_bias_update = std::move(other.last_bias_update);
    delta_biases = std::move(other.delta_biases);
    delta_weights = std::move(other.delta_weights);
}

inner::~inner() {
}

inner& inner::operator=(inner const& other) {
    // have to be the same size!
    basal::exception::throw_if(other.values.rows != values.rows or other.weights.rows != weights.rows
                                   or other.weights.cols != weights.cols or other.biases.rows != biases.rows
                                   or other.biases.cols != biases.cols or other.delta_biases.rows != delta_biases.rows
                                   or other.delta_weights.cols != delta_weights.cols
                                   or other.delta_weights.rows != delta_weights.rows,
                               __FILE__, __LINE__);
    type = other.type;
    values = other.values;
    weights = other.weights;
    biases = other.biases;
    delta = other.delta;
    zeta = other.zeta;
    rms = other.rms;
    count = other.count;
    current_alpha = other.current_alpha;
    current_gamma = other.current_gamma;
    last_weight_update = other.last_weight_update;
    last_bias_update = other.last_bias_update;
    delta_biases = other.delta_biases;
    delta_weights = other.delta_weights;
    applied_weight_update = other.applied_weight_update;
    applied_bias_update = other.applied_bias_update;
    return (*this);
}

inner& inner::operator=(inner&& other) {
    // have to be the same size!
    basal::exception::throw_if(other.values.rows != values.rows or other.weights.rows != weights.rows
                                   or other.weights.cols != weights.cols or other.biases.rows != biases.rows
                                   or other.biases.cols != biases.cols or other.delta_biases.rows != delta_biases.rows
                                   or other.delta_weights.cols != delta_weights.cols
                                   or other.delta_weights.rows != delta_weights.rows,
                               __FILE__, __LINE__);
    type = other.type;
    values = std::move(other.values);
    weights = std::move(other.weights);
    biases = std::move(other.biases);
    delta = std::move(other.delta);
    zeta = std::move(other.zeta);
    rms = std::move(other.rms);
    count = other.count;
    current_alpha = other.current_alpha;
    current_gamma = other.current_gamma;
    last_weight_update = std::move(other.last_weight_update);
    last_bias_update = std::move(other.last_bias_update);
    delta_biases = std::move(other.delta_biases);
    delta_weights = std::move(other.delta_weights);
    applied_weight_update = std::move(other.applied_weight_update);
    applied_bias_update = std::move(other.applied_bias_update);
    return (*this);
}

void inner::set(activation_type _type) {
    this->type = _type;
}

matrix inner::activation(matrix& in) {
    matrix m{in.rows, in.cols};
    switch (type) {
        case activation_type::Sigmoid:
            m = sigmoid(in);
            break;
        case activation_type::RELU:
            m = relu(in);
            break;
        case activation_type::Tanh:
            m = tanh_(in);
            break;
        default:
            break;
    }
    return m;
}

matrix inner::activation_derivative(matrix& in) {
    matrix m{in.rows, in.cols};
    switch (type) {
        case activation_type::Sigmoid:
            m = sigmoid_deriv(in);
            break;
        case activation_type::RELU:
            m = relu_deriv(in);
            break;
        case activation_type::Tanh:
            m = tanh_deriv(in);
            break;
    }
    return m;
}

void inner::forward(layer& other) {
    try {
        zeta = (weights * other.values) + biases;
        values = activation(zeta);
    } catch (basal::exception& e) {
        printf("Failure in computation of layer! %s\n", e.why());
    }
}

void inner::backward(layer& other, precision alpha, precision gamma) {
    basal::exception::throw_unless(layer_type == layer::type::output or layer_type == layer::type::hidden, __FILE__,
                                   __LINE__);

    // Store learning parameters for update
    current_alpha = alpha;
    current_gamma = gamma;

    // counts first to prevent divide by zero
    count++;

    // output layers have the delta pre-computed due to learning methods
    // hidden layers need to have their computed
    // input layers do not have a delta (and are not handled here)
    if (other.layer_type == layer::type::hidden) {
        nn::inner& prev = dynamic_cast<nn::inner&>(other);
        // (W^T*δ) * σ'(z)
        prev.delta = hadamard(weights.T() * delta, activation_derivative(prev.zeta));
    }
    // Accumulate gradients
    //  (δ*v^T)
    linalg::matrix dW = delta * other.values.T();
    delta_weights += dW;

    linalg::matrix db = delta;
    delta_biases += db;
}

void inner::reset(void) {
    count = 0;
    delta_weights.zero();
    delta_biases.zero();
    last_weight_update.zero();
    last_bias_update.zero();
    rms.zero();
    // Note: We intentionally do NOT reset applied_weight_update and applied_bias_update
    // so they remain visible for visualization until the next update cycle
}

void inner::update(void) {
    if (count > 0) {
        // Average the accumulated gradients
        linalg::matrix avg_dW = delta_weights / static_cast<precision>(count);
        linalg::matrix avg_db = delta_biases / static_cast<precision>(count);

        // Apply momentum and learning rate
        linalg::matrix weight_update = (current_alpha * avg_dW) + (current_gamma * last_weight_update);
        linalg::matrix bias_update = (current_alpha * avg_db) + (current_gamma * last_bias_update);

        // Update weights and biases
        weights += weight_update;
        biases += bias_update;

        // Store applied updates for visualization
        applied_weight_update = weight_update;
        applied_bias_update = bias_update;

        // Store updates for next momentum calculation
        last_weight_update = weight_update;
        last_bias_update = bias_update;

        rms /= static_cast<precision>(count);
        rms = sqrt(rms);
    }
}

}  // namespace nn
