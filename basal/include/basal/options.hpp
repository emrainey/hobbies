#pragma once
/**
 * @file
 * The command line options argument processor
 */

#include <algorithm>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <variant>

#ifndef PRIz
#define PRIz "%zu"
#endif

namespace basal {
namespace options {

using Value = std::variant<bool, int, size_t, double, std::string>;

struct config {
    std::string short_switch;
    std::string long_switch;
    Value value;
    std::string description;
};

template <size_t NUM_OPTS>
void process(config (&options)[NUM_OPTS], int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (argv[i] == nullptr) break;  // shouldn't hit this.
        for (size_t j = 0; j < NUM_OPTS; j++) {
            if ((not options[j].short_switch.empty() and strcmp(options[j].short_switch.c_str(), argv[i]) == 0) or
                (not options[j].long_switch.empty() and strcmp(options[j].long_switch.c_str(), argv[i]) == 0)) {
                if (std::holds_alternative<bool>(options[j].value)) {
                    options[j].value = true;
                } else if (std::holds_alternative<int>(options[j].value) and i + 1 < argc) {
                    int v = 0;
                    sscanf(argv[i + 1], "%" PRId32, &v);
                    options[j].value = v;
                } else if (std::holds_alternative<size_t>(options[j].value) and i + 1 < argc) {
                    size_t v = 0;
                    sscanf(argv[i + 1], "%" PRIz, &v);
                    options[j].value = v;
                } else if (std::holds_alternative<double>(options[j].value)) {
                    double v;
                    sscanf(argv[i + 1], "%lf", &v);
                    options[j].value = v;
                } else if (std::holds_alternative<std::string>(options[j].value)) {
                    char input[41];
                    sscanf(argv[i + 1], "%40s", input);
                    options[j].value = std::string(input);
                }
            }
        }
    }
}

template <size_t NUM_OPTS, typename T>
bool find(const config (&options)[NUM_OPTS], std::string long_name, T& value) {
    for (size_t j = 0; j < NUM_OPTS; j++) {
        if (not options[j].long_switch.empty() and options[j].long_switch == long_name) {
            auto pval = std::get_if<T>(&options[j].value);
            if (pval) {
                value = *pval;
                return true;
            } else {
                printf("Could not get type with matching name %s!\n", long_name.c_str());
            }
        }
    }
    return false;
}

template <size_t NUM_OPTS>
void print(config (&options)[NUM_OPTS]) {
    printf("basal::options::config:\n");
    for (size_t j = 0; j < NUM_OPTS; j++) {
        if (std::holds_alternative<bool>(options[j].value)) {
            printf("\t%3s, %20s (default: %s) desc: %s\n", options[j].short_switch.c_str(),
                   options[j].long_switch.c_str(), std::get<bool>(options[j].value) ? "true" : "false",
                   options[j].description.c_str());
        } else if (std::holds_alternative<int>(options[j].value)) {
            printf("\t%3s, %20s (default: %" PRId32 ") desc: %s\n", options[j].short_switch.c_str(),
                   options[j].long_switch.c_str(), std::get<int>(options[j].value), options[j].description.c_str());
        } else if (std::holds_alternative<size_t>(options[j].value)) {
            printf("\t%3s, %20s (default: %" PRIz ") desc: %s\n", options[j].short_switch.c_str(),
                   options[j].long_switch.c_str(), std::get<size_t>(options[j].value), options[j].description.c_str());
        } else if (std::holds_alternative<double>(options[j].value)) {
            printf("\t%3s, %20s (default: %lf) desc: %s\n", options[j].short_switch.c_str(),
                   options[j].long_switch.c_str(), std::get<double>(options[j].value), options[j].description.c_str());
        } else if (std::holds_alternative<std::string>(options[j].value)) {
            printf("\t%3s, %20s (default: %s) desc: %s\n", options[j].short_switch.c_str(),
                   options[j].long_switch.c_str(), std::get<std::string>(options[j].value).c_str(),
                   options[j].description.c_str());
        }
    }
}

}  // namespace options
}  // namespace basal
