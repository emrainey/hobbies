#pragma once
/// @file
/// Definitions of the hidden layer objects
/// @copyright Copyright 2019 (C) Erik Rainey.

#include "neuralnet/inner.hpp"

namespace nn {
/// Common definition of an internal hidden layer which is neither input nor output
class hidden : public inner {
public:
    /// Constructor
    hidden(size_t prev, size_t num);
    /// Destructor
    virtual ~hidden();
};
}  // namespace nn
