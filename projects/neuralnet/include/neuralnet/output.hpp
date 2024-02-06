
#pragma once
/// @file
/// Definitions of the output layers of the Neural Network.
/// @copyright Copyright 2019 (C) Erik Rainey.

#include "neuralnet/inner.hpp"

namespace nn {
/// The Output layer of a network drives the learning process by learning and infering labels
class output : public inner {
public:
    output(size_t prev, size_t num);

    virtual ~output();

    /// @return The index with the highest score.
    /// @param [out] out The value of the highest score.
    ///
    size_t infer_label(precision& out);

    ///
    /// This will create a desired output matrix and compute the error and
    /// store that as a side effect into \ref beta.
    /// @param [in] index Tells the output node which node should have the
    /// desired max value.
    /// @param [in] max The value to assign to the maximum node.
    ///
    void learn_label(size_t index, precision min = -1.0, precision max = 1.0);

    ///
    /// This will use the desired to start the training process for
    /// the network.
    /// @param [in] desired A rowsx1 column matrix.
    /// @{
    ///
    void learn_label(const linalg::matrix& desired);
    void learn_label(linalg::matrix&& desired);
    /// @}

    // Overrides
    void reset(void) override;
    void update(void) override;

    /// Holds the error cost of the layer
    linalg::matrix error;

    /// Holds to total error
    precision error_value;

    /// Holds the true RMS of the layer
    precision rms_value;
};
}  // namespace nn
