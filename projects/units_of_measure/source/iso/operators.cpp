/// @file
/// The specialized operators implementations.
/// @copyright Copyright (c) 2022
#include "iso/iso.hpp"

namespace iso {
namespace literals {

using namespace SI::operators;

// Specialized Quote Operator
iso::amperes operator""_mA(long double a) {
    return iso::amperes(a * SI::prefix::milli);
}
// Specialized Quote Operator
iso::amperes operator""_mA(char const a[]) {
    return iso::amperes(atol(a) * SI::prefix::milli);
}
// Specialized Quote Operator
iso::ohms operator""_mOhm(long double a) {
    return iso::ohms(a * SI::prefix::milli);
}
// Specialized Quote Operator
iso::ohms operator""_mOhm(char const a[]) {
    return iso::ohms(atol(a) * SI::prefix::milli);
}
// Specialized Quote Operator
iso::ohms operator""_KOhm(long double a) {
    return iso::ohms(a * SI::prefix::kilo);
}
// Specialized Quote Operator
iso::ohms operator""_KOhm(char const a[]) {
    return iso::ohms(atol(a) * SI::prefix::kilo);
}
// Specialized Quote Operator
iso::volts operator""_mV(long double a) {
    return iso::volts(a * SI::prefix::milli);
}
// Specialized Quote Operator
iso::volts operator""_mV(char const a[]) {
    return iso::volts(atol(a) * SI::prefix::milli);
}
// Specialized Quote Operator
iso::watts operator""_mW(long double a) {
    return iso::watts(a * SI::prefix::milli);
}
// Specialized Quote Operator
iso::watts operator""_mW(char const a[]) {
    return iso::watts(atol(a) * SI::prefix::milli);
}
}  // namespace literals
}  // namespace iso
