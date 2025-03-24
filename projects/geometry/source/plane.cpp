
#include <cassert>
#include <cmath>
#include <geometry/extra_math.hpp>
#include <geometry/plane.hpp>

namespace geometry {

using namespace geometry::operators;

precision coefficients_t::x() {
    return -d / a;
}
precision coefficients_t::y() {
    return -d / b;
}
precision coefficients_t::z() {
    return -d / c;
}

plane::plane(R3::vector const& on, R3::point const& op) : plane(op, on) {
}

plane::plane(R3::point const& op, R3::vector const& on) : m_normal{on}, m_center_point{op} {
    basal::exception::throw_if(basal::nearly_zero(m_normal.norm()), __FILE__, __LINE__,
                               "The magnitude of the normal can't be zero");
    eq.a = m_normal[0];
    eq.b = m_normal[1];
    eq.c = m_normal[2];
    eq.d = -dot(m_normal, m_center_point);
    m_magnitude = m_normal.norm();
    m_normal.normalize();
}

plane::plane(std::vector<precision> const& list) : plane(list[0], list[1], list[2], list[3]) {
    assert(list.size() == 4);
}

plane::plane(precision a, precision b, precision c, precision d) : m_normal{{a, b, c}}, m_center_point{} {
    // before we normalize, we solve for the point
    // normal line through the origin
    R3::line l{m_normal, geometry::R3::origin};
    eq.a = a;
    eq.b = b;
    eq.c = c;
    eq.d = d;
    // find the length of the segment of the normal line through the origin to the plane.
    precision tn = -d / dot(m_normal, m_normal);
    // the plane point is that length on normal line.
    m_center_point = l.solve(tn);
    // the plane's t
    m_magnitude = m_normal.norm();
    m_normal.normalize();
    // precision check that the plane point is indeed on the plane
    basal::exception::throw_unless(contains(m_center_point), __FILE__, __LINE__);
}

plane::plane(R3::point const& a, R3::point const& b, R3::point const& c) : m_normal{}, m_center_point{b} {
    // we have to declare the vector as a 3 element object (above) so that the move assignment works.
    R3::vector ba = b - a;
    R3::vector ca = c - a;
    m_normal = R3::cross(ba, ca);
    eq.a = m_normal[0];
    eq.b = m_normal[1];
    eq.c = m_normal[2];
    basal::exception::throw_unless(!basal::nearly_zero(m_normal.norm()), __FILE__, __LINE__,
                                   "The magnitude of the normal can't be zero");
    eq.d = -dot(m_normal, b);
    m_magnitude = m_normal.norm();
    m_normal.normalize();  // hessian form requires n to be normalized
}

plane::plane(plane const& other)
    : m_normal{other.m_normal}, m_center_point{other.m_center_point}, eq{other.eq}, m_magnitude{other.m_magnitude} {
}

plane::plane(plane&& other)
    : m_normal{std::move(other.m_normal)}
    , m_center_point{std::move(other.m_center_point)}
    , eq{std::move(other.eq)}
    , m_magnitude{other.m_magnitude} {
}

plane& plane::operator=(plane const& other) {
    m_normal = other.m_normal;
    m_center_point = other.m_center_point;
    eq = other.eq;
    m_magnitude = other.m_magnitude;
    return (*this);
}

plane& plane::operator=(plane&& other) {
    m_normal = std::move(other.m_normal);
    m_center_point = std::move(other.m_center_point);
    eq = std::move(other.eq);
    m_magnitude = other.m_magnitude;
    return (*this);
}

R3::vector plane::unormal() const {
    return R3::vector{{eq.a, eq.b, eq.c}};
}

precision plane::distance(R3::point const& a) const {
    R3::vector n_{{eq.a, eq.b, eq.c}};
    precision top = dot(n_, a) + eq.d;
    return top / n_.norm();
}

bool plane::parallel(plane const& P) const {
    using namespace operators;
    // determine if the normals are parallel
    return R3::parallel(m_normal, P.unormal());
}

bool plane::perpendicular(plane const& P) const {
    using namespace operators;
    return (m_normal | P.unormal());  // check for orthogonal
}

void plane::print(std::ostream& os, char const name[]) const {
    os << name << " " << (*this) << " hessian " << m_normal << ", " << m_center_point << std::endl;
}

bool plane::contains(R3::point const& pt) const {
    constexpr bool use_vector_method = false;
    constexpr bool use_equation_method = true;  // seems like the fewest steps
    constexpr bool use_distance_method = false;
    precision value = 1.0_p;  // checking for nearly_equals to zero so initialize to nonzero
    // static_assert(use_distance_method or use_equation_method or use_vector_method, "Only one should be active");

    // determine if pt is in the plane by the vector method, the equation method and the distance method
    if constexpr (use_vector_method) {
        // the vector from the plane point to the point which is potentially in the plane
        R3::vector v = pt - m_center_point;
        // value is the projection of the normal on a vector which aught to be in the plane
        // which will be zero when normal and v are perpendicular
        value = dot(v, m_normal);
    } else {
        value = 1.0_p;
    }
    bool const vector_method = basal::nearly_zero(value);

    if constexpr (use_equation_method) {
        // solve the equation with this point and it must be zero
        value = pt.x * eq.a + pt.y * eq.b + pt.z * eq.c + eq.d;
    } else {
        value = 1.0_p;
    }
    bool const equation_method = basal::nearly_zero(value);

    if constexpr (use_distance_method) {
        // find the distance from the plane
        value = distance(pt);
    } else {
        value = 1.0_p;
    }
    bool const distance_method = basal::nearly_zero(value);

    // these can disagree in very minor values (around a millionth of a unit)
    // basal::exception::throw_unless(vector_method == eq_method && eq_method == distance_method, __FILE__, __LINE__);
    return (vector_method or equation_method or distance_method);
}

bool plane::contains(R3::line const& l) const {
    R3::point p1 = l.solve(0.0_p);
    R3::point p2 = l.solve(1.0_p);
    return (contains(p1) && contains(p2));
}

const struct coefficients_t& plane::coefficient() const {
    return eq;
}

bool plane::operator==(plane const& other) const {
    using namespace operators;
    // normals must be parallel
    bool normals = R3::parallel(m_normal, other.m_normal);
    // our point must be contained on their plane
    bool points = other.contains(m_center_point);
    return normals && points;
    // they don't have to have the same points.
    // they don't have to have d == d they can be |d| == |d|
    // return (nearly_equals(eq.d,other.eq.d) && (m_center_point == other.m_center_point) && (m_normal ||
    // other.m_normal)); // the normal could be negative?
}

bool plane::operator!=(plane const& other) const {
    return !operator==(other);
}

precision plane::angle(plane const& P) const {
    return acos(dot(m_normal, P.unormal()));
}

std::ostream& operator<<(std::ostream& os, plane const& p) {
    os << "plane(" << p.coefficient().a << "x+" << p.coefficient().b << "y+" << p.coefficient().c << "z+"
       << p.coefficient().d << "=0.0_p)";
    return os;
}

}  // namespace geometry
