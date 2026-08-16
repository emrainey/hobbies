/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Parses compact frame-selection specs into sorted sets of 1-based frame indices.
/// @copyright Copyright (c) 2026
///

#include <vision/frame_spec.hpp>

#include <basal/basal.hpp>

#include <algorithm>
#include <cctype>
#include <string>

namespace vision {

namespace {

/// Parses a single positive 1-based frame index from a token.
int parse_index(std::string const& token) {
    long value = 0;
    for (char const c : token) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            basal::exception::throw_unless(false, __FILE__, __LINE__,
                                           "frame spec token must be a positive integer or a lo-hi range: '%s'",
                                           token.c_str());
        }
        value = value * 10 + static_cast<long>(c - '0');
    }
    basal::exception::throw_unless(value >= 1, __FILE__, __LINE__, "frame indices are 1-based; got %ld", value);
    return static_cast<int>(value);
}

/// Trims ASCII whitespace from both ends of a string.
std::string trim(std::string s) {
    auto const not_space = [](char c) { return !std::isspace(static_cast<unsigned char>(c)); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    return s;
}

}  // namespace

std::set<int> parse_frames(std::string const& spec) {
    std::set<int> frames;
    std::string::size_type start = 0;
    while (start <= spec.size()) {
        std::string::size_type const comma = spec.find(',', start);
        std::string const token
            = trim(spec.substr(start, comma == std::string::npos ? std::string::npos : comma - start));
        if (!token.empty()) {
            std::string::size_type const dash = token.find('-');
            if (dash == std::string::npos) {
                frames.insert(parse_index(token));
            } else {
                int const lo = parse_index(trim(token.substr(0, dash)));
                int const hi = parse_index(trim(token.substr(dash + 1)));
                basal::exception::throw_unless(lo <= hi, __FILE__, __LINE__, "frame range is empty: %d-%d", lo, hi);
                for (int i = lo; i <= hi; ++i) {
                    frames.insert(i);
                }
            }
        }
        if (comma == std::string::npos) {
            break;
        }
        start = comma + 1;
    }
    return frames;
}

}  // namespace vision