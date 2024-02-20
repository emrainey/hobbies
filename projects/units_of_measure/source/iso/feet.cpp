
#include "iso/feet.hpp"

#include <cmath>

namespace iso {
// you can still have your own class with internal stuff
feet::feet(precision a) : measurement{a}, _sixteenths(std::round(a * subunit)), sixteenths{_sixteenths} {
}

feet::feet(const feet &other) : measurement(other.value), _sixteenths(other.value * subunit), sixteenths{_sixteenths} {
}

feet &feet::operator=(const feet &other) {
    _value = other.value;
    _sixteenths = other._sixteenths;
    return (*this);
}

feet::~feet() {
}
namespace literals {
iso::feet operator""_ft(long double a) {
    return iso::feet(static_cast<precision>(a));
}

iso::feet operator""_ft(char const a[]) {
    return iso::feet(atol(a));
}
}  // namespace literals

}  // namespace iso