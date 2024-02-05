#include "raytrace/objects/cone.hpp"

#include <iostream>

namespace raytrace {
namespace objects {
// FIXME delete or modify!
cone::cone(const point& C, iso::radians angle)
    : object{C, 2, true}  // 2 collisions, closed (infinite)
    , m_bottom_radius{0.0}
    , m_height{0.0}
    , m_angle{angle} {
    basal::exception::throw_if(m_angle >= iso::radians(iso::pi / 2), __FILE__, __LINE__, "Angle %lf is too large", m_angle.value);
}

cone::cone(const point& C, precision bottom_radius, precision height)
    : object{C, 2, false}  // 2 collisions, not closed
    , m_bottom_radius{bottom_radius}
    , m_height{height}
    , m_angle{std::atan2(bottom_radius, height)} {
}

vector cone::normal(const point& world_surface_point) const {
    point object_surface_point = reverse_transform(world_surface_point);
    precision height = m_height;
    precision radius = m_bottom_radius;
    if (m_height > 0) {
        if (object_surface_point.z < 0 or object_surface_point.z > m_height) {
            return R3::null;
        }
    } else {
        height = 1.0;
        radius = std::tan(m_angle.value);
    }
    if (basal::equals_zero(object_surface_point.x) and basal::equals_zero(object_surface_point.y)) {
        return R3::null;
    }
    vector N{{object_surface_point.x, object_surface_point.y, 0}};
    N.normalize();
    N *= height;
    if (m_height > 0) {
        N[2] = radius;
    } else {
        N[2] = object_surface_point.z > 0 ? -radius : radius;
    }
    N.normalize();
    return vector(N);  // copy constructor output
}

hits cone::collisions_along(const ray& object_ray) const {
    hits ts;
    // i used the base formula of
    // z^2 = x^2 + y^2
    // with h and r are the height and radius
    // where the bottom cone is raise up and the radius is set
    // (z-h)^2 = (h^2/r^2)(x^2 + y^2)
    // @see https://www.geogebra.org/3d/dmnjmtuv
    // @see https://www.desmos.com/calculator/q3u1fflt54
    // then substituting the line equation
    // where x,y,z are the base of the ray
    // where i,j,k are the direction of the ray
    // (z+tk-h)^2 = (h^2/r^2)*((x+ti)^2+(y+tj)^2)
    // since we know h^2 / r^2 I reduced to s
    // then solving for t as a quadratic root
    // (with help from https://www.mathpapa.com/algebra-calculator.html)
    // a = s(i^2 + j^2) - k^2
    // b = 2(s(ix + jy) - k(z-h))
    // c = s(x^2+y^2) - (z-h)^2
    precision x = object_ray.location().x;
    precision y = object_ray.location().y;
    precision z = object_ray.location().z;
    precision i = object_ray.direction()[0];
    precision j = object_ray.direction()[1];
    precision k = object_ray.direction()[2];
    precision h = m_height;
    precision br = m_bottom_radius;
    precision f = std::tan(m_angle.value);
    precision s = basal::equals_zero(h) ? 1.0 / (f * f) : (h * h) / (br * br);
    precision z_h = z - h;
    precision a = s * (i * i + j * j) - k * k;
    precision b = 2 * (s * (i * x + j * y) - k * z_h);
    precision c = s * (x * x + y * y) - z_h * z_h;
    auto roots = linalg::quadratic_roots(a, b, c);
    precision t0 = std::get<0>(roots);
    precision t1 = std::get<1>(roots);
    point surface_point;
    if (not basal::is_nan(t0)) {
        surface_point = object_ray.distance_along(t0);
        if (basal::equals_zero(h) or linalg::within(0, surface_point.z, h)) {
            ts.push_back(t0);
        }
    }
    if (not basal::is_nan(t1)) {
        surface_point = object_ray.distance_along(t1);
        if (basal::equals_zero(h) or linalg::within(0, surface_point.z, h)) {
            ts.push_back(t1);
        }
    }
    return ts;
}

bool cone::is_surface_point(const point& world_point) const {
    point object_point = reverse_transform(world_point);
    precision x = object_point.x;
    precision y = object_point.y;
    precision z = object_point.z;
    return basal::equals(z * z, (x * x) + (y * y));
}

image::point cone::map(const point& object_surface_point) const {
    geometry::point_<2> cartesian(object_surface_point[0], object_surface_point[1]);
    geometry::point_<2> polar = geometry::cartesian_to_polar(cartesian);
    // some range of z based in the half_height we want -h2 to map to zero and +h2 to 1.0
    precision u = (object_surface_point[2] / (-2.0 * m_height)) + 0.5;
    // theta goes from +pi to -pi we want to map -pi to 1.0 and + pi to zero
    precision v = 0.0;
    if (polar.y >= 0) {
        v = 0.5 - (polar.y / (+2.0 * iso::pi));
    } else {
        v = 0.5 + (polar.y / (-2.0 * iso::pi));
    }
    return image::point(u, v);
}

void cone::print(const char str[]) const {
    std::cout << str << " cone @" << this << " " << position() << " Height" << m_height << " Radius:" << m_bottom_radius
              << " Angle:" << m_angle.value << std::endl;
}

precision cone::get_object_extant(void) const {
    return sqrt((m_height * m_height) + (m_bottom_radius * m_bottom_radius));
}

}  // namespace objects
}  // namespace raytrace
