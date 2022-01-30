
#pragma once

#include <cstdlib>
#include <bitset>

namespace htm {
    template <size_t BITS> using dense_word = std::bitset<BITS>;
}
