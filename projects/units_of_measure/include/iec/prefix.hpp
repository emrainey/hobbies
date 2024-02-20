#pragma once
#include <cstdint>

namespace IEC {
enum class prefix : short
{
    kibi = 10,
    mebi = 20,
    gibi = 30,
    tebi = 40,
    pebi = 50,
    exbi = 60,
};
}  // namespace IEC
uint64_t operator*(uint64_t const a, IEC::prefix p);

inline uint64_t operator""_kibibytes(long long unsigned int a) {
    return a * IEC::prefix::kibi;
}

inline uint64_t operator""_mebibytes(long long unsigned int a) {
    return a * IEC::prefix::mebi;
}
