#pragma once
/**
 * @file
 * Definitions of the neural network object.
 * @copyright Copyright 2019 (C) Erik Rainey.
 */

#include <chrono>
#include <cstddef>

#include "neuralnet/hidden.hpp"
#include "neuralnet/input.hpp"
#include "neuralnet/output.hpp"

namespace nn {

/// Converts an initializer list to a vector.
std::vector<size_t> convert(std::initializer_list<int> list);

/// The class which contains all the layer and the logic to propagate
/// forwards and backwards
class network {
public:
    /// Default empty network
    network();
    /// Removed copy constructor
    network(const network& other) = delete;
    /// Explicit List Initializer Constructor
    explicit network(std::initializer_list<size_t> list);
    /// Promoted Vector Constructor
    network(std::vector<size_t> list);
    /// Creates a network from the vector of node sizes
    void create(std::vector<size_t> list);
    /// Destructor
    virtual ~network();
    /// Computes the forward direction of the network
    void forward();
    /// Computes the backwards direction of the network for learning
    /// with a given learning rate (alpha) and momentum (gamma).
    void backward(precision alpha, precision gamma);

    /// Updates the weights and biases in the network.
    void update(void);

    /// Resets the layers delta and RMS
    void reset(void);

    /// Draws the current state of the network
    /// @param [in] delay the amount of delay in ms
    void visualize(std::chrono::milliseconds delay);

    /// Allows an append of a layer to the network
    network& operator<<(layer& lay);

    /// Allows an append of an r-value layer to the network
    network& operator<<(layer&& lay);

    /// Gets a reference to a layer out of the network.
    layer& get(size_t i);

    /// Gets a reference to a layer out of the network.
    inline layer& operator[](size_t i) {
        return get(i);
    }

    /// Allows iteration through each layer in a network
    network& for_each(std::function<void(layer& l)> block);

    /// Allows getting a reference to layer as an input
    input& as_input(size_t i) noexcept(false);

    /// Allows getting a reference to layer as a hidden
    hidden& as_hidden(size_t i) noexcept(false);

    /// Allows getting a reference to layer as an output
    output& as_output(size_t i) noexcept(false);

    /// Sets the activation type for all the layers in the network.
    void set(activation_type type);

protected:
    /// Gets a referece to an inner layer
    inner& as_inner(size_t i) noexcept(false);

    /// The vectors of all layers in the network.
    std::vector<layer*> layers;
};

}  // namespace nn
