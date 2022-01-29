
#include "iso/amperes.hpp"

DECLARE_MEASUREMENT_CLASS(amperes, double, SI::prefix, A)

// Specialized Quote Operators
iso::amperes operator""_mA(long double a) {
    return iso::amperes(a * SI::prefix::milli);
}

iso::amperes operator""_mA(const char a[]) {
    return iso::amperes(atol(a) * SI::prefix::milli);
}
