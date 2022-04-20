
#include "geometry/line.hpp"

#include <cassert>
#include <cmath>

namespace geometry {

namespace R3 {

line::line(const R3::vector &ov, const R3::point &op) : m_udir(ov), m_zero(op) {
    basal::exception::throw_if(m_udir.is_zero(), __FILE__, __LINE__, "Vector can not be zero length");
}

line::line(const R3::point &op, const R3::vector &ov) : m_udir(ov), m_zero(op) {
    basal::exception::throw_if(m_udir.is_zero(), __FILE__, __LINE__, "Vector can not be zero length");
}

line::line(const R3::point &a, const R3::point &b) : m_udir(a - b), m_zero(b) {
    basal::exception::throw_if(m_udir.is_zero(), __FILE__, __LINE__, "Points can not be the same");
}

line::line(const std::vector<element_type> &list)
    : m_udir{{list[0], list[1], list[2]}}, m_zero(list[3], list[4], list[5]) {
    basal::exception::throw_if(m_udir.is_zero(), __FILE__, __LINE__, "Vector cannot be zero");
    assert(list.size() == 6);
}

line::line(const line &other) : m_udir(other.m_udir), m_zero(other.m_zero) {
}

line::line(line &&other) : m_udir(std::move(other.m_udir)), m_zero(std::move(other.m_zero)) {
}

const R3::point &line::position() const {
    return m_zero;
}

const R3::vector &line::direction() const {
    return m_udir;
}

line &line::operator=(const line &other) {
    m_zero = other.m_zero;
    m_udir = other.m_udir;
    return (*this);
}

line &line::operator=(line &&other) {
    m_zero = std::move(other.m_zero);
    m_udir = std::move(other.m_udir);
    return (*this);
}

R3::point line::distance_along(element_type t) const {
    R3::vector nd = direction().normalized();
    nd *= t;
    return position() + nd;
}

R3::point line::solve(element_type t) const {
    using namespace geometry::operators;
    return position() + (t * direction());
}

bool line::solve(const point &P, element_type &t) const {
    using namespace geometry::operators;
    // This no longer requires R3 lines.
    // Example:
    // x = x0 + t*a, t = (x - x0)/a
    // y = y0 + t*b, t = (y - y0)/b
    // z = z0 + t*c, t = (z - z0)/c

    // if the point is the start of the vector
    if (geometry::operators::operator==(P, position())) {  // HACK (operators) why does this work, but not just ==?
                                                           // if (P == position()) {
        //  this just helps eliminate math problems later
        t = 0.0;
        return true;
    }
    const size_t dims = position().dimensions;
    std::vector<element_type> ts;
    for (size_t i = 0; i < dims; i++) {
        element_type d = m_udir[i];
        element_type p = m_zero[i];
        bool zero_slope = basal::equals_zero(d);
        bool same_component = basal::equals(P[i], p);
        if (zero_slope and not same_component) {
            // a shortcut to eliminate problems later
            return false;
        }
        // don't consider the divide by zero numbers
        if (not zero_slope) {
            ts.push_back((P[i] - p) / d);
        }
    }
    if (std::all_of(ts.begin(), ts.end(), [&](element_type v) { return basal::equals(v, ts[0]); })) {
        t = ts[0];
        return true;
    }
    return false;
}

void line::print(const char name[]) const {
    position().print(name);
    direction().print(name);
}

element_type line::distance(const R3::point &p) const {
    R3::vector v = p - closest(p);
    return v.magnitude();
}

R3::point line::closest(const R3::point &p) const {
    R3::vector side = p - position();
    element_type t = dot(side, direction()) / direction().quadrance();
    return solve(t);
}

bool operator==(const line &a, const line &b) {
    using namespace operators;
    // shortcut (are they literally the same?)
    if ((a.direction() == b.direction()) and (geometry::operators::operator==(a.position(), b.position()))) {
        return true;
    }
    // equal lines means
    // 1.) vectors are parallel
    // 2.) their point is on our line!
    bool pv = R3::parallel(a.direction(), b.direction());
    element_type t = 0.0;
    bool pp = a.solve(b.position(), t);
    return (pv && pp);
}

bool operator!=(const line &a, const line &b) {
    return not(operator==(a, b));
}

bool parallel(const line &a, const line &b) {
    R3::vector an{a.direction().normalized()};
    R3::vector bn{b.direction().normalized()};
    return ((an == bn) || ((!an) == bn));
}

bool skew(const R3::line &i, const R3::line &j) {
    element_type x1 = i.position()[0], x2 = i.position()[0] + i.direction()[0];
    element_type x3 = j.position()[0], x4 = j.position()[0] + j.direction()[0];
    element_type y1 = i.position()[1], y2 = i.position()[1] + i.direction()[1];
    element_type y3 = j.position()[1], y4 = j.position()[1] + j.direction()[1];
    element_type z1 = i.position()[2], z2 = i.position()[2] + i.direction()[2];
    element_type z3 = j.position()[2], z4 = j.position()[2] + j.direction()[2];
    linalg::matrix A{{
        {x1, y1, z1, 1},
        {x2, y2, z2, 1},
        {x3, y3, z3, 1},
        {x4, y4, z4, 1},
    }};
    return not basal::equals_zero(det(A));
}

}  // namespace R3

}  // namespace geometry
