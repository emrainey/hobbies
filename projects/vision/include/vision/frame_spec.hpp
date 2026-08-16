#pragma once

/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Parses a compact frame-selection spec (e.g. "5" or "3,7-10") into a sorted set
///        of 1-based frame indices for stills extraction and quick preview iteration.
/// @copyright Copyright (c) 2026
///
/// A frame spec is a comma-separated list of single frame numbers and inclusive ranges,
/// for example `30`, `5,30-40`, or `10,90-120,145`. Values are 1-based; anything below 1
/// or unparseable throws @ref basal::exception so callers can surface a useful CLI error.

#include <cstddef>
#include <set>
#include <string>

namespace vision {

/// Parses a frame spec into a sorted, de-duplicated set of 1-based frame indices.
std::set<int> parse_frames(std::string const& spec);

}  // namespace vision