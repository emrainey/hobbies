
#include "iso/iso.hpp"

namespace iso {
// Following volts = amperes * ohms, watts = volts * amperes
// Equational operators
namespace operators {
volts operator/(watts const& P, amperes const& I) {
    return volts{P.value / I.value};
}
volts operator*(amperes const& I, ohms const& R) {
    return volts{I.value * R.value};
}
watts operator*(volts const& E, amperes const& I) {
    return watts{E.value * I.value};
}
ohms operator/(volts const& E, amperes const& I) {
    return ohms{E.value / I.value};
}
amperes operator/(watts const& P, volts const& E) {
    return amperes{P.value / E.value};
}
amperes operator/(volts const& E, ohms const& R) {
    return amperes{E.value / R.value};
}
}  // namespace operators
}  // namespace iso
