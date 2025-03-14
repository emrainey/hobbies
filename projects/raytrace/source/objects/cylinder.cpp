#include "raytrace/objects/cylinder.hpp"

#include <iostream>

namespace raytrace {
namespace objects {
using namespace linalg::operators;

cylinder::cylinder(point const& C, precision half_height, precision radius)
    : object{C, 2, false}  // 2 collisions, not closed
    , m_half_height{half_height}
    , m_radius{radius} {
}

cylinder::cylinder(point const& base, point const& apex, precision radius)
    : object{base, 2, false}  // 2 collisions, not closed
    , m_half_height{0.0_p}
    , m_radius{radius} {
    R3::vector axis = apex - base;
    R3::vector semi = axis.normalized();
    m_half_height = axis.magnitude() / 2;
    // reassign position
    position(base + (m_half_height * semi));
    // find the spherical mapping to this semi point
    raytrace::point spherical_point = geometry::cartesian_to_spherical(R3::origin + semi);
    // r, theta, phi
    iso::radians zero(0);
    iso::radians theta(spherical_point[1]);  // rotation around Z
    iso::radians phi(spherical_point[2]);    // rotation around a rotated Y
    rotation(zero, phi, theta);
}

vector cylinder::normal_(point const& object_surface_point) const {
    point C{0, 0, object_surface_point.z};
    return (object_surface_point - C).normalize();
}

hits cylinder::collisions_along(ray const& object_ray) const {
    hits ts;
    precision px = object_ray.location()[0];
    precision py = object_ray.location()[1];
    precision dx = object_ray.direction()[0];
    precision dy = object_ray.direction()[1];
    precision a = (dx * dx + dy * dy);
    precision b = 2.0_p * (dx * px + dy * py);
    precision c = (px * px + py * py) - (m_radius * m_radius);
    auto roots = linalg::quadratic_roots(a, b, c);
    precision t0 = std::get<0>(roots);
    precision t1 = std::get<1>(roots);
    if (not basal::is_nan(t0)) {
        point R = object_ray.distance_along(t0);
        if (linalg::within(-m_half_height, R.z, m_half_height)) {
            ts.emplace_back(intersection{R}, t0, normal_(R), this);
        }
    }
    if (not basal::is_nan(t1)) {
        point Q = object_ray.distance_along(t1);
        if (linalg::within(-m_half_height, Q.z, m_half_height)) {
            ts.emplace_back(intersection{Q}, t1, normal_(Q), this);
        }
    }
    return ts;
}

bool cylinder::is_surface_point(point const& world_point) const {
    point object_point = reverse_transform(world_point);
    precision x = object_point.x;
    precision y = object_point.y;
    precision z = object_point.z;
    return basal::nearly_equals(m_radius * m_radius, (x * x) + (y * y))
           and linalg::within(-m_half_height, z, m_half_height);
}

image::point cylinder::map(point const& object_surface_point) const {
    geometry::point_<2> cartesian(object_surface_point[0], object_surface_point[1]);
    geometry::point_<2> polar = geometry::cartesian_to_polar(cartesian);
    // some range of z based in the half_height we want -h2 to map to zero and +h2 to 1.0
    precision u = (object_surface_point[2] / (-2.0_p * m_half_height)) + 0.5_p;
    // theta goes from +pi to -pi we want to map -pi to 1.0_p and + pi to zero
    precision v = 0.0_p;
    if (basal::is_greater_than_or_equal_to_zero(polar.y)) {
        v = 0.5_p - (polar.y / (+2.0_p * iso::pi));
    } else {
        v = 0.5_p + (polar.y / (-2.0_p * iso::pi));
    }
    return image::point(u, v);
}

void cylinder::print(char const str[]) const {
    std::cout << str << " cylinder @" << this << " " << position() << " 1/2H" << m_half_height << " Radius:" << m_radius
              << std::endl;
}

precision cylinder::get_object_extent(void) const {
    return sqrt((m_half_height * m_half_height) + (m_radius * m_radius));
}

}  // namespace objects
}  // namespace raytrace
