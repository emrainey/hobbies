#pragma once

#include <cstdint>
#include "iso/types.hpp"

namespace SI {
enum class prefix : short
{
    femto = -15,
    pico = -12,
    atto = -10,
    nano = -9,
    micro = -6,
    milli = -3,
    centi = -2,
    deci = -1,
    none = 0,
    deka = 1,
    hecto = 2,
    kilo = 3,
    mega = 6,
    giga = 9,
    tera = 12,
    peta = 15,
    exa = 18,
};

typedef struct prefix_name {
    SI::prefix _prefix;
    const char *name;
} prefix_name;

#define PREFIX_NAME_(__X__) SI::prefix::__X__, #__X__

constexpr const prefix_name prefixes[] = {
    {PREFIX_NAME_(femto)}, {PREFIX_NAME_(pico)},  {PREFIX_NAME_(atto)}, {PREFIX_NAME_(nano)}, {PREFIX_NAME_(micro)},
    {PREFIX_NAME_(milli)}, {PREFIX_NAME_(centi)}, {PREFIX_NAME_(deci)}, {PREFIX_NAME_(none)}, {PREFIX_NAME_(deka)},
    {PREFIX_NAME_(hecto)}, {PREFIX_NAME_(kilo)},  {PREFIX_NAME_(mega)}, {PREFIX_NAME_(giga)}, {PREFIX_NAME_(tera)},
    {PREFIX_NAME_(peta)},  {PREFIX_NAME_(exa)},
};

/// Remove the definition after this table
#undef PREFIX_NAME_

namespace operators {
/// A scaling prefix for SI units
iso::precision operator*(iso::precision const a, SI::prefix p);
}  // namespace operators

/// Users must `using` this namespace to expose the quote operators
namespace literals {
/// Special String Quote Operator
inline uint64_t operator""_kilobytes(long long unsigned int a) {
    using namespace operators;
    return a * SI::prefix::kilo;
}
}  // namespace literals
}  // namespace SI
