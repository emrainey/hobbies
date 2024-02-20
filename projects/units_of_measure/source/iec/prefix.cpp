
#include "iec/prefix.hpp"

uint64_t operator*(uint64_t const a, IEC::prefix p) {
    return a * (1 << static_cast<uint8_t>(p));
}
