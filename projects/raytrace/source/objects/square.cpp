#include "raytrace/objects/square.hpp"

#include <iostream>

namespace raytrace {
namespace objects {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

square::square(point const& C, vector const& N, precision hh, precision hw)
    : raytrace::objects::plane(C, N), m_points{} {
    m_type = Type::Square;
    m_has_definite_volume = false;  // a square is a bounded planar surface
    m_points[0] = raytrace::point{-hw, -hh, 0};
    m_points[1] = raytrace::point{+hw, +hh, 0};
}

square::square(R3::axes const& A, precision hh, precision hw) : raytrace::objects::plane(A), m_points{} {
    m_type = Type::Square;
    m_has_definite_volume = false;  // a square is a bounded planar surface
    m_points[0] = raytrace::point{-hw, -hh, 0};
    m_points[1] = raytrace::point{+hw, +hh, 0};
}

hits square::collisions_along(ray const& object_ray) const {
    hits ts;
    // is the ray parallel to the plane?
    vector const& N = unormal().normalized();
    vector const& V = object_ray.direction();
    precision proj = dot(V, N);
    // if so the projection is not zero they collide *somewhere*
    // could be positive or negative t. Don't check for proj < 0.0_p since we may be
    // concerned with colliding with the back-side of walls
    if (not basal::nearly_zero(proj)) {
        point const& P = object_ray.location();
        // get the vector of the center to the ray initial
        vector const C = R3::origin - P;  // FIXME this seems to work in unit test but create a mess in pictures.
        // vector const C = position() - P; // FIXME this seems to work in practice but is "off"
        // t is the ratio of the projection of the arbitrary center vector divided by the projection ray
        precision const t = dot(C, N) / proj;
        point D = object_ray.distance_along(t);
        // get the transform matrix from object space to plane space
        // transform D to the plane space and check if it's within the 2D points
        R3::point PD = m_basis.to_basis() * D;
        // if point D is contained within the 3D points it's in the square.
        if (linalg::within_inclusive(m_points[0].x, PD.x, m_points[1].x)
            and linalg::within_inclusive(m_points[0].y, PD.y, m_points[1].y)) {
            ts.emplace_back(intersection{D}, t, normal_(D), this);
        }
    }
    return ts;
}

bool square::is_surface_point(point const& world_point) const {
    bool on_plane = plane::is_surface_point(world_point);
    if (not on_plane) {
        return false;
    }
    // now convert the world point to an object point
    // and check if it's within the 2D points
    point object_point = reverse_transform(world_point);
    matrix const M = m_basis.to_basis();
    point plane_point = M * object_point;
    return linalg::within_inclusive(m_points[0].x, plane_point.x, m_points[1].x)
           and linalg::within_inclusive(m_points[0].y, plane_point.y, m_points[1].y);
}

void square::print(std::ostream& os, char const name[]) const {
    os << "square @ " << this << " " << name << " " << position() << " " << m_normal << " from " << m_points[0]
       << " to " << m_points[1] << std::endl;
}

image::point square::map(point const& object_surface_point) const {
    // use the m_basis to map the object_surface_point to the UV space
    matrix const M = m_basis.to_basis();
    R3::point uv_point = M * object_surface_point;
    // scale the UV point to the surface scale
    // ensure the UV point is within the range of 0 to 1
    image::point scaled_point = image::point(uv_point.x * m_surface_scale.u, uv_point.y * m_surface_scale.v);
    return scaled_point;
}

precision square::get_object_extent(void) const {
    return (m_points[1] - R3::origin).magnitude();
}

}  // namespace objects
}  // namespace raytrace
