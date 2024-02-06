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
constexpr const uint32_t NUM_SYNAPSES = 20;
constexpr const uint32_t NUM_DENDRITES = 5;
constexpr const uint32_t NUM_NEURONS = 8;
/// The structure that connects neurons in a one-way directed graph
struct synapse {
    float strength;   ///< Contains the strength of the connection
    uint8_t index[3]; ///< Height, Width, Column depth
};

template <size_t _SYN_>
struct dendrite {
    std::bitset<_SYN_> active; ///< Determines if this synapse is operational
    std::array<synapse, _SYN_> synapses;
};

template <size_t _DEN_>
struct neuron {
    /// Proxial Dendrites are summed, and only touch neighbors
    std::array<dendrite<NUM_SYNAPSES>, 1> proxial;

    /// Distal Dendrites are UNIONed (ORR) and can connect long distances
    std::array<dendrite<NUM_SYNAPSES>, _DEN_> distal;

    /// The output of the neuron
    float output;

    /// Returns the total number of synapses in the neuron
    inline size_t num_synapses() const {
        const size_t proxial_synapses = (proxial.size() * proxial[0].synapses.size());
        const size_t distal_synapses = (distal.size() * distal[0].synapses.size());
        return proxial_synapses + distal_synapses;
    }

    /// Indexes a synapse from a neuron's point of view
    inline synapse& operator[](size_t synapse_index) {
        const size_t proxial_synapses = (proxial.size() * proxial[0].synapses.size());
        // /const size_t distal_synapses = (distal.size() * distal[0].synapses.size());

        if (synapse_index < proxial_synapses) {
            return proxial[0].synapses[synapse_index];
        } else {
            size_t distal_idx = (synapse_index - proxial_synapses);
            size_t dendrite_idx = distal_idx / distal[0].synapses.size();
            return distal[dendrite_idx].synapses[distal_idx];
        }
    }
};

template <size_t _NEUR_>
struct column {
    float boost;
    std::array<neuron<NUM_DENDRITES>, _NEUR_> neurons;
    /*
    uint32_t overlap_count;
    uint32_t overlap_threshold;
    bool overlap;
    bool overlapped(const sparse_word& data) {
        overlap = false;
        if (overlap > overlap_threshold) {
            overlap = true;
        }
        return overlap;
    }
    */
};

/// Returns a random number between 0.0 and 1.0 inclusive
float frand();

template <size_t W, size_t H>
class network {
protected:
    /// The grid of columns
    std::array<std::array<column<NUM_NEURONS>, W>, H> grid;
    const float minOverlap;
    const float minActivate;
    const float minStrength;

public:
    network() : minOverlap(2.0_p), minActivate(0.8_p), minStrength(0.7_p) {
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
                        syn.index[0] = rand() % grid.size();
                        syn.index[1] = rand() % grid[j].size();
                        syn.index[2] = rand() % grid[j][i].neurons.size();
                    }
                }
            }
        }
    }
    ~network() {
    }

    void print(void) {
        for (size_t j = 0; j < H; j++) {
            for (size_t i = 0; i < W; i++) {
                uint16_t v = 0;
                for (size_t n = 0; n < grid[j][i].neurons.size(); n++) {
                    if (grid[j][i].neurons[n].output > minActivate) {
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

    void to_image(const char* filename) {
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
                    if (grid[j][i].neurons[n].output > minActivate) {
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
                        if (syn.strength > minStrength) {
                            grid[j][i].overlap += syn.strength;
                        }
                    }
                }
                if (grid[j][i].overlap < minOverlap)
                    grid[j][i].overlap = 0.0_p;
                else
                    grid[j][i].overlap *= grid[j][i].boost;
            }
        }
    }
};

}  // namespace htm
