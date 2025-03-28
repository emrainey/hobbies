
#include "geometry/sphere.hpp"

#include "geometry/intersection.hpp"
#include "geometry/plane.hpp"

namespace geometry {

using namespace operators;

namespace R3 {

sphere::sphere(precision r) : m_radius{r} {
    basal::exception::throw_if(basal::nearly_zero(m_radius), __FILE__, __LINE__, "Radius can't be zero");
}
sphere::sphere(sphere const& s) : sphere(s.radius) {
}

sphere::sphere(sphere&& s) : m_radius(s.radius) {
}

sphere& sphere::operator=(sphere const& s) {
    m_radius = s.m_radius;
    return (*this);
}

sphere& sphere::operator=(sphere&& s) {
    m_radius = s.m_radius;
    return (*this);
}

bool sphere::contains(R3::point const& object_point) const {
    R3::vector v = object_point - R3::origin;
    return (v.norm() <= radius);
}

bool sphere::on_surface(R3::point const& object_point) const {
    R3::vector v = object_point - R3::origin;
    return (basal::nearly_equals(v.norm(), radius));
}

R3::vector sphere::normal_at(R3::point const& object_point) const {
    R3::vector n = object_point - R3::origin;
    if constexpr (geometry::check_on_surface) {
        precision r = n.norm();
        if (not basal::nearly_equals(r, radius)) {
            std::cerr << "Warning: sphere::normal_at() called with point not on surface: " << object_point
                      << " radius=" << std::setprecision(20) << radius << " r=" << r << std::endl;
            // not on the sphere.
            return R3::null;
        }
    }
    // is on the sphere
    return R3::vector(n.normalize());
}

R3::point sphere::cart_to_polar(R3::point const& object_point) const {
    R3::point pol;
    // Any number > 0
    pol[0] = m_radius;
    // 0 to Tau
    pol[1] = std::atan2(object_point[1], object_point[0]);
    // 0 to Pi
    pol[2] = std::acos(object_point[2] / m_radius);
    return R3::point{pol};
}

}  // namespace R3

}  // namespace geometry
