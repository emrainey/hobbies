
#pragma once
/// @file
/// Definitions of the input layer object
/// @copyright Copyright 2019 (C) Erik Rainey.

#include "neuralnet/layer.hpp"
#include "neuralnet/mnist.hpp"

namespace nn {
/// An input layer take raw data and encodes it into it's value
class input : public layer {
public:
    /// Constructor with single parameter
    input(size_t num);
    /// Destructor
    virtual ~input();
    // TODO (NN) add encoders from fourcc::image and others
    virtual void encode(mnist& db, size_t index);
};
}  // namespace nn
