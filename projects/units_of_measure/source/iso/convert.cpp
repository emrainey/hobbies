
#include "iso/iso.hpp"

namespace iso {

using namespace iso::operators;
using namespace basal::literals;

// radians <=> turns
radians convert(turns& A) {
    return radians{A.value * tau};
}
turns convert(radians& A) {
    return turns{A.value / tau};
}

// seconds <=> cycles
hertz convert(seconds& A) {
    constexpr precision const C = 1.0_p;
    return hertz{C / A.value};
}
seconds convert(hertz& B) {
    constexpr precision const C = 1.0_p;
    return seconds{C / B.value};
}

void convert(radians& r, turns const& t) {
    r = radians(t.value * tau);
}

void convert(turns& t, radians const& r) {
    t = turns(r.value / tau);
}

void convert(degrees& d, radians const& r) {
    d = degrees(r.value * (180.0_p / pi));
}

void convert(radians& r, degrees const& d) {
    r = radians(d.value * (pi / 180.0_p));
}

void convert(hertz& hz, seconds const& sec) {
    constexpr precision const C = 1.0_p;
    hz = C / sec;
}

void convert(seconds& sec, hertz const& hz) {
    constexpr precision const C = 1.0_p;
    sec = C / hz;
}

namespace operators {
hertz operator/(precision const num, seconds const& denom) {
    return hertz{num / denom.value};
}

seconds operator/(precision const num, hertz const& denom) {
    return seconds{num / denom.value};
}
}  // namespace operators

}  // namespace iso
