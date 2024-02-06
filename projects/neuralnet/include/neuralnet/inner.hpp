
#pragma once
/// @file
/// Definitions of the inner layer objects
/// @copyright Copyright 2019 (C) Erik Rainey.

#include "neuralnet/layer.hpp"
#include "neuralnet/matrix.hpp"

namespace nn {

/// The enumerations of the activeation types for inner layers
enum class activation_type : int
{
    Sigmoid,  //!< Uses a sigmoid activation
    RELU,     //!< Uses a RELU activation (fastest)
    Tanh,     //!< Uses a (faster than Sigmoid) hyperbolic tangent function */
};

/// Common definition of a non-input layer
class inner : public layer {
public:
    /// Constructor which takes parameters from childern */
    inner(layer::type _type, size_t inputs, size_t num);
    /// Copy
    inner(const inner& other);
    /// Move
    inner(inner&& other);
    /// Copy
    inner& operator=(const inner& other);
    /// Move
    inner& operator=(inner&& other);
    // Dtor
    virtual ~inner();

    /// Allows the user to set the activation type
    virtual void set(activation_type type);

    /// Calls the desired activation function to modify zeta into values.
    virtual matrix activation(matrix& in);

    /// Computes the partial derivative of the activation function over the values.
    virtual matrix activation_derivative(matrix& in);

    /// During forward propagation, the other layer is the lower layer
    virtual void forward(layer& other) override;

    /// During backwards propagation, the other layer is the lower layer.
/// @param alpha The direct learning coefficient
/// @param gamma The "momentum" learning coefficient
    virtual void backward(layer& other, precision alpha, precision gamma) override;

    /// Applies the change in weights and biases to each layer.
    virtual void update(void);

    /// Resets the internal states for changing values (deltas and RMS) to zerp.
    virtual void reset(void);

    activation_type type;    ///< The type of activation function to use.
    linalg::matrix weights;  ///< The weights of all inputs.
    linalg::matrix biases;   ///< The biases to apply to each input.
    linalg::matrix delta;    ///< The matrix which holds the computed error times the slope of the activation
    linalg::matrix zeta;     ///< The weighted inputs plus biases.
    linalg::matrix rms;      ///< The root mean square of the errors (after update)
    size_t count;
    linalg::matrix last_delta;     ///< The previous delta matrix
    linalg::matrix last_bias;      ///< The previous bias matrix
    linalg::matrix delta_weights;  ///< This holds the instantaneous change in weights
    linalg::matrix delta_biases;   ///< This holds the instantaneous change in biases
};
}  // namespace nn
