
#include "iso/iso.hpp"

namespace iso {
// Following volts = amperes * ohms, watts = volts * amperes
// Equational operators
namespace operators {
volts operator/(const watts &P, const amperes &I) {
    return volts(P.value / I.value);
}
volts operator*(const amperes &I, const ohms &R) {
    return volts(I.value * R.value);
}
watts operator*(const volts &E, const amperes &I) {
    return watts(E.value * I.value);
}
ohms operator/(const volts &E, const amperes &I) {
    return ohms(E.value / I.value);
}
amperes operator/(const watts &P, const volts &E) {
    return amperes(P.value / E.value);
}
amperes operator/(const volts &E, const ohms &R) {
    return amperes(E.value / R.value);
}
}  // namespace operators
}  // namespace iso
