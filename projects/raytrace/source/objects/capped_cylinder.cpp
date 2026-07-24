#include "raytrace/objects/capped_cylinder.hpp"

#include <iostream>

namespace raytrace {
namespace objects {
using namespace linalg::operators;

capped_cylinder::capped_cylinder(point const& C, precision radius)
    : object{C, 4, Type::Cylinder, true}  // 4 collisions, closed
    , m_half_height{0.0_p}
    , m_radius{radius}
    , m_is_infinite{true} {
    // "infinite" cylinder with caps
}

capped_cylinder::capped_cylinder(point const& C, precision half_height, precision radius)
    : object{C, 4, Type::Cylinder, true}  // 4 collisions, closed
    , m_half_height{half_height}
    , m_radius{radius}
    , m_is_infinite{false} {
    basal::exception::throw_if(basal::nearly_zero(m_half_height), __FILE__, __LINE__, "Half-height can't be zero");
}

capped_cylinder::capped_cylinder(point const& base, point const& apex, precision radius)
    : object{base, 4, Type::Cylinder, true}  // 4 collisions, closed
    , m_half_height{0.0_p}
    , m_radius{radius}
    , m_is_infinite{true} {
    R3::vector axis = apex - base;
    R3::vector semi = axis.normalized();
    m_half_height = axis.magnitude() / 2;
    basal::exception::throw_if(basal::nearly_zero(m_half_height), __FILE__, __LINE__, "Half-height can't be zero");
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

vector capped_cylinder::normal_(point const& object_surface_point) const {
    // For a capped cylinder, we have three types of surfaces:
    // 1. The curved surface (outer cylinder)
    // 2. The top cap
    // 3. The bottom cap
    
    // Check if we're on the top or bottom cap
    precision z = object_surface_point.z();
    if (basal::nearly_equals(z, m_half_height)) {
        // Top cap - normal points upward (positive Z)
        return R3::basis::Z;
    } else if (basal::nearly_equals(z, -m_half_height)) {
        // Bottom cap - normal points downward (negative Z)
        return -R3::basis::Z;
    } else {
        // Curved surface - normal is radial (in the XY plane)
        point C{0, 0, z};  // Center of the circle at this Z height
        return (object_surface_point - C).normalize();
    }
}

hits capped_cylinder::collisions_along(ray const& object_ray) const {
    hits ts;
    
    // Check for intersections with the cylindrical surface
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
    
    // Check the tube intersections
    if (not basal::is_nan(t0)) {
        point R = object_ray.distance_along(t0);
        if (m_is_infinite || linalg::within(-m_half_height, R.z(), m_half_height)) {
            ts.emplace_back(intersection{R}, t0, normal_(R), this);
        }
    }
    
    if (not basal::is_nan(t1)) {
        point Q = object_ray.distance_along(t1);
        if (m_is_infinite || linalg::within(-m_half_height, Q.z(), m_half_height)) {
            ts.emplace_back(intersection{Q}, t1, normal_(Q), this);
        }
    }
    
    // Check for intersections with the top cap
    if (not basal::is_nan(m_half_height) && m_half_height > 0) {
        precision tz = m_half_height - object_ray.location()[2];
        precision dz = object_ray.direction()[2];
        
        if (not basal::is_exactly_zero(dz)) {
            precision t_cap = tz / dz;
            point cap_point = object_ray.distance_along(t_cap);
            
            // Check if this intersection is within the radius of the cap
            precision distance_from_center = sqrt((cap_point.x() * cap_point.x()) + (cap_point.y() * cap_point.y()));
            if (distance_from_center <= m_radius) {
                ts.emplace_back(intersection{cap_point}, t_cap, R3::basis::Z, this);
            }
        }
    }
    
    // Check for intersections with the bottom cap
    if (not basal::is_nan(m_half_height) && m_half_height > 0) {
        precision tz = -m_half_height - object_ray.location()[2];
        precision dz = object_ray.direction()[2];
        
        if (not basal::is_exactly_zero(dz)) {
            precision t_cap = tz / dz;
            point cap_point = object_ray.distance_along(t_cap);
            
            // Check if this intersection is within the radius of the cap
            precision distance_from_center = sqrt((cap_point.x() * cap_point.x()) + (cap_point.y() * cap_point.y()));
            if (distance_from_center <= m_radius) {
                ts.emplace_back(intersection{cap_point}, t_cap, -R3::basis::Z, this);
            }
        }
    }
    
    // Sort intersections by distance
    std::sort(ts.begin(), ts.end());
    
    return ts;
}

bool capped_cylinder::is_surface_point(point const& world_point) const {
    point object_point = reverse_transform(world_point);
    precision x = object_point.x();
    precision y = object_point.y();
    precision z = object_point.z();
    
    if (m_is_infinite) {
        // Infinite cylinder - check if point is on the surface
        return basal::nearly_equals(m_radius * m_radius, (x * x) + (y * y));
    } else {
        // Finite cylinder - check if point is on the surface
        if (basal::nearly_equals(z, m_half_height) || basal::nearly_equals(z, -m_half_height)) {
            // Point is on a cap
            return basal::nearly_equals(m_radius * m_radius, (x * x) + (y * y));
        } else {
            // Point is on the cylinder surface
            return basal::nearly_equals(m_radius * m_radius, (x * x) + (y * y))
                   and linalg::within(-m_half_height, z, m_half_height);
        }
    }
}

image::point capped_cylinder::map(point const& object_surface_point) const {
    precision h = m_is_infinite ? 1.0_p : m_half_height;
    return mapping::cylindrical(h, object_surface_point);
}

void capped_cylinder::print(std::ostream& os, char const str[]) const {
    os << "capped_cylinder @" << this << " " << str << " " << position() << " 1/2H" << m_half_height << " Radius:" << m_radius
       << std::endl;
}

precision capped_cylinder::get_object_extent(void) const {
    if (m_is_infinite) {
        return basal::pos_inf;
    } else {
        return sqrt((m_half_height * m_half_height) + (m_radius * m_radius));
    }
}

}  // namespace objects

}  // namespace raytrace