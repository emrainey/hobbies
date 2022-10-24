#pragma once

#include "iso/measurement.hpp"

namespace imperial {
enum class prefix : short
{
    sixtyfourths = 64,
    thirtyseconds = 32,
    sixteenths = 16,
    eigths = 8,
    quaters = 4,
    half = 2,
    none = 1
};
struct suffix_type_ft {
    constexpr static const char *const suffix = "ft";
};
}  // namespace imperial

namespace iso {
// you can still have your own class with internal stuff
class feet : public measurement<double, imperial::prefix, imperial::suffix_type_ft> {
protected:
    const double unit = 16.0;  // sixteenths
    uint32_t _sixteenths;

public:
    const uint32_t &sixteenths;
    feet(double a);
    feet(const feet &other);
    feet &operator=(const feet &other);
    // TODO add move as well?
    // TODO fill in all the other overload methods...
    ~feet();
};

namespace literals {
iso::feet operator""_ft(long double a);
iso::feet operator""_ft(const char a[]);
}  // namespace literals
}  // namespace iso