#pragma once

#include "iso/measurement.hpp"

namespace imperial {
enum class prefix : short {
    sixtyfourths = 64,
    thirtyseconds = 32,
    sixteenths = 16,
    eigths = 8,
    quaters = 4,
    half = 2,
    none = 1
};
struct suffix_type_ft {
    constexpr static char const* const suffix = "ft";
};
}  // namespace imperial

namespace iso {
// you can still have your own class with internal stuff
class feet : public measurement<precision, imperial::prefix, imperial::suffix_type_ft> {
protected:
    constexpr static uint32_t const subunit = 16;  // sixteenths
    uint32_t _sixteenths;

public:
    uint32_t const& sixteenths;
    feet(precision a);
    feet(feet const& other);
    feet& operator=(feet const& other);
    // TODO add move as well?
    // TODO fill in all the other overload methods...
    ~feet();
};

namespace literals {
iso::feet operator""_ft(long double a);
iso::feet operator""_ft(char const a[]);
}  // namespace literals
}  // namespace iso