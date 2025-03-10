
#include "iso/feet.hpp"

#include <cmath>

namespace iso {
// you can still have your own class with internal stuff
feet::feet(precision a)
    : measurement{a}, _sixteenths(static_cast<uint32_t>(std::round(a * subunit))), sixteenths{_sixteenths} {
}

feet::feet(feet const& other)
    : measurement(other.value), _sixteenths(static_cast<uint32_t>(other.value * subunit)), sixteenths{_sixteenths} {
}

feet& feet::operator=(feet const& other) {
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
    iso::precision p = static_cast<iso::precision>(atol(a));
    return iso::feet(p);
}
}  // namespace literals

}  // namespace iso