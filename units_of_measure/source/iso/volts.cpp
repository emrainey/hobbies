#include "iso/volts.hpp"

DECLARE_MEASUREMENT_CLASS(volts, double, SI::prefix, V)

iso::volts operator""_mV(long double a) {
    return iso::volts(a * SI::prefix::milli);
}

iso::volts operator""_mV(const char a[]) {
    return iso::volts(atol(a) * SI::prefix::milli);
}
