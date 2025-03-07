#pragma once
/// @file
/// Definitions of the layer object.
/// @copyright Copyright 2019 (C) Erik Rainey.

#include <linalg/linalg.hpp>
#include <basal/ieee754.hpp>

namespace nn {

using precision = basal::precision;
/// The common layer object which other layer objects are childern of
class layer {
public:
    /// The enumeration of layer types
    enum class type : int {
        input,   ///< inputs just have inputs which are transformed from some other data
                 ///< and they have encoders which translate some data into 0.0-1.0 values.
        hidden,  ///< hidden layers have reference to their input layer
                 ///< and they have their own weights and biases and values
        output,  ///< output layers have reference to a hidden layers
                 ///< and they have their own weights and biases and values
    };
    bool isa(type t) const;

    /// A Constant View of the layer type
    type const& layer_type;

    /// No Default or Just inputs?
    layer();
    /// Parameter Ctor
    layer(type _t, size_t num);
    /// Copy Ctor
    layer(layer const& other);
    /// Move Ctor
    layer(layer&& other);
    /// Copy Assign
    layer& operator=(layer const& other);
    /// Move Assign
    layer& operator=(layer&& other);
    /// Virtual Dtor
    virtual ~layer();

    size_t get_num(void) const;

    /// Does any necessary layer computation in the forward direction
    virtual void forward();

    /// Does any necessary layer computation in the forward direction
    virtual void forward(layer& other);

    /// Does any necessary layer computation in the backward direction
    virtual void backward();

    /// Does any necessary layer computation in the backward direction
    virtual void backward(layer& other, precision alpha, precision gamma);

    /// The computed value of this layer
    linalg::matrix values;

    /// The computed error from the desired of this layer
    linalg::matrix beta;

protected:
    /// The mutable type value
    layer::type m_type;
};
}  // namespace nn
