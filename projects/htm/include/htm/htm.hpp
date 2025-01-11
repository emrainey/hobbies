#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <htm/dense_word.hpp>
#include <htm/encoders.hpp>
#include <htm/sparse_word.hpp>
#include <memory>
#include <vector>

#include <basal/ieee754.hpp>

namespace htm {
using precision = basal::precision;
using namespace basal::literals;

// constexpr uint32_t const NUM_SYNAPSES = 20;
// constexpr uint32_t const NUM_DENDRITES = 5;
// constexpr uint32_t const NUM_NEURONS = 8;

struct index_t {
    uint16_t x;
    uint16_t y;
    uint16_t z;
    index_t() : x(UINT16_MAX), y(UINT16_MAX), z(UINT16_MAX) {
    }
    bool valid() const {
        return (x != UINT16_MAX) and (y != UINT16_MAX) and (z != UINT16_MAX);
    }
    void disconnect() {
        x = UINT16_MAX;
        y = UINT16_MAX;
        z = UINT16_MAX;
    }
    bool operator==(index_t const& that) const {
        return (x == that.x) and (y == that.y) and (z == that.z);
    }
};

/// Represents a connection between two neurons through a synapse
struct connection {
    index_t source;  ///< The index of the source neuron
    index_t target;  ///< The index of the target neuron
    bool active;     ///< Based on Source's Activity
};

/// The structure that connects neurons in a one-way directed graph
struct synapse {
    precision strength;     ///< Contains the strength of the connection or permanence
    connection connection;  ///< Contains the information about how the synapse is connected.
    precision threshold;    ///< Contains the threshold of the connection
    /// Contains the weight of the connection (1.0 if strength > threshold, else 0.0)
    precision weight() const {
        return (strength > threshold) ? 1.0_p : 0.0_p;
    }
};

template <size_t SEGMENTS_PER_DENDRITE, size_t SYNAPSES_PER_SEGMENT>
struct dendrite_ {
    struct segment_ {
        std::bitset<SYNAPSES_PER_SEGMENT> active;  ///< Determines if this segment is operational
        std::array<synapse, SYNAPSES_PER_SEGMENT> synapses;
        size_t threshold_count; ///< The number of active synapses required to activate the segment
        bool is_actve() const {
            return (active.count() >= threshold_count);
        }
    };
    std::array<segment_, SEGMENTS_PER_DENDRITE> segments;
    constexpr size_t number_of_synapses() const {
        return segments.size() * segments[0].synapses.size();
    }
    inline synapse& operator[](size_t synapse_index) {
        if (synapse_index < number_of_synapses()) {
            size_t const segment_idx = synapse_index / SYNAPSES_PER_SEGMENT;
            size_t const synapse_idx = synapse_index % SYNAPSES_PER_SEGMENT;
            return segments[segment_idx].synapses[synapse_idx];
        } else {
            return segments[0].synapses[0];
        }
    }
};

using dendrite = dendrite_<30, 100>;

template < size_t NUM_PROXIMAL_DENDRITES, size_t NUM_DISTAL_DENDRITES>
struct neuron_ {
    /// Proxial Dendrites are summed, and only touch neighbors
    std::array<dendrite, NUM_PROXIMAL_DENDRITES> proxial;

    /// Distal Dendrites are UNIONed (ORR) and can connect long distances
    std::array<dendrite, NUM_DISTAL_DENDRITES> distal;

    /// The output of the neuron
    float output;

    /// Returns the total number of synapses in the neuron
    inline size_t num_synapses() const {
        size_t const proxial_synapses = (proxial.size() * proxial.number_of_synapses());
        size_t const distal_synapses = (distal.size() * distal.number_of_synapses());
        return proxial_synapses + distal_synapses;
    }

    /// Indexes a synapse from a neuron's point of view
    inline synapse& operator[](size_t synapse_index) {
        size_t const proxial_synapses = (proxial.size() * proxial.number_of_synapses());
        size_t const distal_synapses = (distal.size() * distal.number_of_synapses());

        if (synapse_index < proxial_synapses) {
            size_t dendrite_idx = synapse_index / proxial.number_of_synapses();
            return proxial[dendrite_idx][synapse_index];
        } else if (synapse_index < (proxial_synapses + distal_synapses)) {
            size_t distal_idx = (synapse_index - proxial_synapses);
            size_t dendrite_idx = distal_idx / distal.number_of_synapses();
            return distal[dendrite_idx].synapses[distal_idx];
        }
    }
};

using neuron = neuron_<1, 5>;

/// @brief The foundational structure of HTM.
/// Each Column has
template <size_t NEURONS_PER_COLUMN>
struct column_ {
    float boost;
    std::array<neuron, NEURONS_PER_COLUMN> neurons;
    /*
    uint32_t overlap_count;
    uint32_t overlap_threshold;
    bool overlap;
    bool overlapped(sparse_word const& data) {
        overlap = false;
        if (overlap > overlap_threshold) {
            overlap = true;
        }
        return overlap;
    }
    */
};

/// Some default values for the column
using column = column_<100>;

/// Returns a random number between 0.0 and 1.0 inclusive
float frand();

template <size_t W, size_t H>
class network_ {
protected:
    /// The grid of columns
    std::array<std::array<column, W>, H> grid;
    float const minimum_overlap;
    float const minimum_activate;
    float const minimum_strength;

public:
    network_() : minimum_overlap(2.0_p), minimum_activate(0.8_p), minimum_strength(0.7_p) {
        srand(time(nullptr));
        for (size_t j = 0; j < H; j++) {
            for (size_t i = 0; i < W; i++) {
                // grid[j][i].overlap = frand();
                grid[j][i].boost = frand();
                for (size_t n = 0; n < grid[j][i].neurons.size(); n++) {
                    grid[j][i].neurons[n].proxial[0].active = 0;
                    grid[j][i].neurons[n].output = frand();
                    for (size_t s = 0; s < grid[j][i].neurons[n].num_synapses(); s++) {
                        synapse& syn = grid[j][i].neurons[n][s];
                        syn.strength = frand();

                        syn.connection.source.x = rand() % grid.size();
                        syn.connection.source.y = rand() % grid[j].size();
                        syn.connection.source.z = rand() % grid[j][i].neurons.size();

                        syn.connection.target.x = rand() % grid.size();
                        syn.connection.target.y = rand() % grid[j].size();
                        syn.connection.target.z = rand() % grid[j][i].neurons.size();
                    }
                }
            }
        }
    }
    ~network_() {
    }

    void print(void) {
        for (size_t j = 0; j < H; j++) {
            for (size_t i = 0; i < W; i++) {
                uint16_t v = 0;
                for (size_t n = 0; n < grid[j][i].neurons.size(); n++) {
                    if (grid[j][i].neurons[n].output > minimum_activate) {
                        v |= (1 << n);
                    }
                }
                if (grid[j][i].neurons.size() <= 8)
                    printf("%02x ", v & 0xFF);
                else if (grid[j][i].neurons.size() <= 16)
                    printf("%04x ", v);
            }
            printf("\n");
        }
    }

    void to_image(char const* filename) {
        FILE* fp = fopen(filename, "wb");
        fprintf(fp, "P5\n");
        fprintf(fp, "%u %u\n", W, H);
        fprintf(fp, "%lu\n", (1 << grid[0][0].neurons.size()) - 1);
        // for each row
        for (size_t j = 0; j < H; j++) {
            // buffer the values into an array
            uint8_t values[W];
            // for each column
            for (size_t i = 0; i < W; i++) {
                values[i] = 0;
                // for each neuron in the column
                for (size_t n = 0; n < grid[j][i].neurons.size(); n++) {
                    if (grid[j][i].neurons[n].output > minimum_activate) {
                        values[i] |= (1 << n);
                    }
                }
            }
            fwrite(values, 1, sizeof(values), fp);
        }
        fclose(fp);
    }

    void overlap() {
        for (size_t j = 0; j < H; j++) {
            for (size_t i = 0; i < W; i++) {
                grid[j][i].overlap = 0.0_p;
                for (size_t n = 0; n < grid[j][i].neurons.size(); n++) {
                    for (size_t s = 0; s < grid[j][i].neurons[n].num_synapses(); s++) {
                        synapse& syn = grid[j][i].neurons[n][s];
                        if (syn.strength > minimum_strength) {
                            grid[j][i].overlap += syn.strength;
                        }
                    }
                }
                if (grid[j][i].overlap < minimum_overlap)
                    grid[j][i].overlap = 0.0_p;
                else
                    grid[j][i].overlap *= grid[j][i].boost;
            }
        }
    }
};

using network = network_<64, 64>;

}  // namespace htm
