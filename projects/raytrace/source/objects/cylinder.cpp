#include "raytrace/objects/cylinder.hpp"

#include <iostream>

namespace raytrace {
namespace objects {
using namespace linalg::operators;

cylinder::cylinder(const point& C, precision half_height, precision radius)
    : object{C, 2, false}  // 2 collisions, not closed
    , m_half_height{half_height}
    , m_radius{radius} {
}

cylinder::cylinder(const point& base, const point& apex, precision radius)
    : object{base, 2, false}  // 2 collisions, not closed
    , m_half_height{0.0}
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

vector cylinder::normal(const point& world_surface_point) const {
    point object_surface_point = reverse_transform(world_surface_point);
    vector N = object_surface_point - R3::origin;  // position() in object space is the origin
    N[2] = 0.0;  // remove Z component, in object space this is up or down the cylinder
    N.normalize();
    return forward_transform(N);  // copy constructor output
}

hits cylinder::collisions_along(const ray& object_ray) const {
    hits ts;
    precision px = object_ray.location()[0];
    precision py = object_ray.location()[1];
    precision dx = object_ray.direction()[0];
    precision dy = object_ray.direction()[1];
    precision a = (dx * dx + dy * dy);
    precision b = 2.0 * (dx * px + dy * py);
    precision c = (px * px + py * py) - (m_radius * m_radius);
    auto roots = linalg::quadratic_roots(a, b, c);
    precision t0 = std::get<0>(roots);
    precision t1 = std::get<1>(roots);
    if (not basal::is_nan(t0)) {
        point R = object_ray.distance_along(t0);
        if (linalg::within(-m_half_height, R.z, m_half_height)) {
            ts.push_back(t0);
        }
    }
    if (not basal::is_nan(t1)) {
        point Q = object_ray.distance_along(t1);
        if (linalg::within(-m_half_height, Q.z, m_half_height)) {
            ts.push_back(t1);
        }
    }
    return ts;
}

bool cylinder::is_surface_point(const point& world_point) const {
    point object_point = reverse_transform(world_point);
    precision x = object_point.x;
    precision y = object_point.y;
    precision z = object_point.z;
    return basal::nearly_equals(m_radius * m_radius, (x * x) + (y * y)) and linalg::within(-m_half_height, z, m_half_height);
}

image::point cylinder::map(const point& object_surface_point) const {
    geometry::point_<2> cartesian(object_surface_point[0], object_surface_point[1]);
    geometry::point_<2> polar = geometry::cartesian_to_polar(cartesian);
    // some range of z based in the half_height we want -h2 to map to zero and +h2 to 1.0
    precision u = (object_surface_point[2] / (-2.0 * m_half_height)) + 0.5;
    // theta goes from +pi to -pi we want to map -pi to 1.0 and + pi to zero
    precision v = 0.0;
    if (polar.y >= 0) {
        v = 0.5 - (polar.y / (+2.0 * iso::pi));
    } else {
        v = 0.5 + (polar.y / (-2.0 * iso::pi));
    }
    return image::point(u, v);
}

void cylinder::print(const char str[]) const {
    std::cout << str << " cylinder @" << this << " " << position() << " 1/2H" << m_half_height << " Radius:" << m_radius
              << std::endl;
}

precision cylinder::get_object_extent(void) const {
    return sqrt((m_half_height * m_half_height) + (m_radius * m_radius));
}

}  // namespace objects
}  // namespace raytrace
