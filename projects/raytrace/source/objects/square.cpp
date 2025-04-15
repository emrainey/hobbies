#include "raytrace/objects/square.hpp"

#include <iostream>

namespace raytrace {
namespace objects {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

square::square(precision side) : square(R3::origin, R3::identity, side) {
}

square::square(point const& C, matrix const& R, precision S)
    : raytrace::objects::plane(C, R), min_{-S / 2, -S / 2}, max_{S / 2, S / 2} {
    m_type = Type::Square;
    m_has_definite_volume = false;  // a square is a bounded planar surface
}

hits square::collisions_along(ray const& object_ray) const {
    hits ts;
    // is the ray parallel to the plane?
    vector const& N = R3::basis::Z;  // same as plane()
    vector const& V = object_ray.direction();
    precision proj = dot(V, N);
    // if so the projection is not zero they collide *somewhere*
    // could be positive or negative t. Don't check for proj < 0.0_p since we may be
    // concerned with colliding with the back-side of walls
    if (not basal::nearly_zero(proj)) {
        point const& P = object_ray.location();
        // get the vector of the center to the ray initial
        vector const C = R3::origin - P;
        // t is the ratio of the projection of the arbitrary center vector divided by the projection ray
        precision const t = dot(C, N) / proj;
        point D = object_ray.distance_along(t);
        // get the transform matrix from object space to plane space
        // if point D is contained within the 3D points it's in the square.
        if (linalg::within_inclusive(min_.x, D.x, max_.x) and linalg::within_inclusive(min_.y, D.y, max_.y)) {
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
    return linalg::within_inclusive(min_.x, object_point.x, max_.x)
           and linalg::within_inclusive(min_.y, object_point.y, max_.y);
}

void square::print(std::ostream& os, char const name[]) const {
    os << "square @ " << this << " " << name << " " << position() << " from " << min_ << " to " << max_ << std::endl;
}

image::point square::map(point const& object_surface_point) const {
    // scale the UV point to the surface scale
    // ensure the UV point is within the range of 0 to 1
    image::point uv
        = image::point(object_surface_point.x * m_surface_scale.u, object_surface_point.y * m_surface_scale.v);
    return uv;
}

precision square::get_object_extent(void) const {
    return (max_ - R3::origin).magnitude();
}

}  // namespace objects
}  // namespace raytrace
