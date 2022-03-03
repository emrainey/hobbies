
#include "geometry/sphere.hpp"

#include "geometry/intersection.hpp"
#include "geometry/plane.hpp"

namespace geometry {

using namespace operators;

namespace R3 {

sphere::sphere(element_type r) : m_radius(r) {
    basal::exception::throw_if(m_radius == 0.0, __FILE__, __LINE__, "Radius can't be zero");
}
sphere::sphere(const sphere &s) : sphere(s.radius) {
}

sphere::sphere(sphere &&s) : m_radius(s.radius) {
}

sphere &sphere::operator=(const sphere &s) {
    m_radius = s.m_radius;
    return (*this);
}

sphere &sphere::operator=(sphere &&s) {
    m_radius = s.m_radius;
    return (*this);
}

bool sphere::contains(const R3::point &object_point) const {
    R3::vector v = object_point - R3::origin;
    return (v.norm() <= radius);
}

bool sphere::surface(const R3::point &object_point) const {
    R3::vector v = object_point - R3::origin;
    return (basal::equals(v.norm(), radius));
}

R3::vector sphere::normal(const R3::point &object_point) const {
    constexpr static bool check_on_surface = true;
    R3::vector n = object_point - R3::origin;
    if constexpr (check_on_surface) {
        if (not basal::equals(n.norm(), radius)) {
            // not on the sphere.
            return R3::null;
        }
    }
    // is on the sphere
    return R3::vector(n.normalize());
}

R3::point sphere::cart_to_polar(const R3::point &object_point) const {
    point pol(object_point.dimensions);
    // Any number > 0
    pol[0] = m_radius;
    // 0 to Tau
    pol[1] = std::atan2(object_point[1], object_point[0]);
    // 0 to Pi
    pol[2] = std::acos(object_point[2] / m_radius);
    return point(pol);
}

}  // namespace R3

}  // namespace geometry
