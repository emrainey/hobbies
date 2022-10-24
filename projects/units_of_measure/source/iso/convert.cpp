
#include "iso/iso.hpp"

namespace iso {

using namespace iso::operators;

// feet <=> meters
feet convert(meters& A) {
    return feet{A.value * 3.28084};
}
meters convert(feet& B) {
    return meters{B.value * 0.3048000097536};
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
    return hertz{1.0 / A.value};
}
seconds convert(hertz& B) {
    return seconds{1.0 / B.value};
}

void convert(feet& f, const meters& m) {
    f = feet(m.value * 3.28084);
}

void convert(meters& m, const feet& f) {
    m = meters(f.value * 0.3048000097536);
}

void convert(radians& r, const turns& t) {
    r = radians(t.value * tau);
}

void convert(turns& t, const radians& r) {
    t = turns(r.value / tau);
}

void convert(degrees& d, const radians& r) {
    d = degrees(r.value * (180.0 / pi));
}

void convert(radians& r, const degrees& d) {
    r = radians(d.value * (pi / 180.0));
}

void convert(hertz& hz, const seconds& sec) {
    hz = 1.0 / sec;
}

void convert(seconds& sec, const hertz& hz) {
    sec = 1.0 / hz;
}

namespace operators {
hertz operator/(const double num, const seconds& denom) {
    return hertz{num / denom.value};
}

seconds operator/(const double num, const hertz& denom) {
    return seconds{num / denom.value};
}
}  // namespace operators

}  // namespace iso
