
#include "iec/prefix.hpp"

uint64_t operator*(const uint64_t a, IEC::prefix p) {
  return a * (1 << static_cast<uint8_t>(p));
}
