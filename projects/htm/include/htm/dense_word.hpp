
#pragma once

#include <bitset>
#include <cstdlib>

namespace htm {
template <size_t BITS>
using dense_word = std::bitset<BITS>;
}
