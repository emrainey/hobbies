
#include "iso/ohms.hpp"

DECLARE_MEASUREMENT_CLASS(ohms, double, SI::prefix, Ohm)

iso::ohms operator""_mOhm(long double a) {
    return iso::ohms(a * SI::prefix::milli);
}
iso::ohms operator""_mOhm(const char a[]) {
    return iso::ohms(atol(a) * SI::prefix::milli);
}

iso::ohms operator""_KOhm(long double a) {
    return iso::ohms(a * SI::prefix::kilo);
}
iso::ohms operator""_KOhm(const char a[]) {
    return iso::ohms(atol(a) * SI::prefix::kilo);
}
