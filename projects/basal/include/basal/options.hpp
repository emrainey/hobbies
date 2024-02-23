#pragma once
/// @file
/// The command line options argument processor

#include <algorithm>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <variant>

#include "basal/ieee754.hpp"

namespace basal {
namespace options {

/// Define the value in the command processor as a variant
using Value = std::variant<bool, int, size_t, precision, std::string>;

/// Relates the short, long switch and the values
struct config {
    std::string short_switch;
    std::string long_switch;
    Value value;
    std::string description;
};

/// Assigns values from argv into the config structure
void process(size_t num_opts, config options[], int argc, char* argv[]);

template <size_t NUM_OPTS, typename T>
bool find(config const(&options)[NUM_OPTS], std::string long_name, T& value) {
    for (size_t j = 0; j < NUM_OPTS; j++) {
        if (not options[j].long_switch.empty() and options[j].long_switch == long_name) {
            auto value_pointer = std::get_if<T>(&options[j].value);
            if (value_pointer) {
                value = *value_pointer;
                return true;
            } else {
                printf("Could not get type with matching name %s!\n", long_name.c_str());
            }
        }
    }
    return false;
}

/// Prints all the options information
void print(size_t num_options, config options[]);

}  // namespace options
}  // namespace basal
