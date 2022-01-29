
#include "iso/feet.hpp"
#include <cmath>

namespace iso {
// you can still have your own class with internal stuff
feet::feet(double a) : measurement(a),
                       _sixteenths(std::round(a * unit)),
                       sixteenths(_sixteenths) {}

feet::feet(const feet &other) : measurement(other.value),
                                  _sixteenths(other.value * unit),
                                  sixteenths(_sixteenths) {}

feet& feet::operator=(const feet &other) {
    _value = other.value;
    _sixteenths = other._sixteenths;
    return (*this);
}

feet::~feet() {}
}

iso::feet operator""_ft(long double a) {
    return iso::feet(a);
}

iso::feet operator""_ft(const char a[]) {
    return iso::feet(atol(a));
}
