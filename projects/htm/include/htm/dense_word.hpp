
#pragma once

#include <bitset>
#include <cstdlib>

namespace htm {

/// @brief A dense word is a bitset that is exactly the size of the number of bits
/// @tparam BITS
template <size_t BITS>
using dense_word = std::bitset<BITS>;

}  // namespace htm
