
#include "iso/iso.hpp"

namespace iso {

using namespace iso::operators;
using namespace basal::literals;

// feet <=> meters
feet convert(meters& A) {
    feet::type c = 3.28084_p;
    return feet{A.value * c};
}
meters convert(feet& B) {
    feet::type c = 0.3048000097536_p;  // a double value I think?
    return meters{static_cast<precision>(B.value * c)};
}

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

void convert(feet& f, meters const& m) {
    constexpr precision const C = 3.28084_p;
    f = feet(m.value * C);
}

void convert(meters& m, feet const& f) {
    constexpr precision const C = 0.3048000097536_p;  // a double value I think?
    m = meters(static_cast<precision>(f.value * C));
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
