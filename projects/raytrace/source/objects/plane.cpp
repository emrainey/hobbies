#include "raytrace/objects/plane.hpp"

#include <iostream>

namespace raytrace {
namespace objects {
using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

plane::plane() : object{R3::origin, 1, Type::Plane, false} {
    m_rotation = R3::identity;
    m_inv_rotation = R3::identity;
    compute_transforms();
}

plane::plane(point const& C, matrix const& rotation) : object{C, 1, Type::Plane, false} {
    m_rotation = rotation;
    m_inv_rotation = rotation.inverse();
    compute_transforms();
}

vector plane::normal_(point const&) const {
    return R3::basis::Z;
}

hits plane::collisions_along(ray const& object_ray) const {
    hits ts;
    // is the ray parallel to the plane?
    // @note in object space, the center point is at the origin
    vector const& N = normal_(R3::origin);
    vector const& V = object_ray.direction();
    precision const proj = dot(V, N);
    // if so the projection is not zero they collide *somewhere*
    // could be positive or negative t. Don't check for proj < 0.0_p since we may be
    // concerned with colliding with the back-side of walls
    if (not basal::nearly_zero(proj)) {  // they collide *somewhere*
        point const& P = object_ray.location();
        // get the vector of the center to the ray initial
        vector const C = R3::origin - P;
        // t is the ratio of the projection of the arbitrary center vector divided by the projection ray
        precision const t = dot(C, N) / proj;
        // D is the point on the line t distance along
        point D = object_ray.distance_along(t);
        ts.emplace_back(intersection{D}, t, N, this);
    }
    return ts;
}

bool plane::is_surface_point(point const& world_point) const {
    vector world_delta = world_point - position();
    if (world_delta == R3::null) {
        return true;
    }
    // @note in object space, the center point is at the origin
    return geometry::orthogonal(world_delta, normal_(R3::origin));
}

image::point plane::map(point const& object_surface_point) const {
    // in object space the origin is the center and the normal is +Z.
    image::point uv_point{object_surface_point.x, object_surface_point.y};
    image::point polar_space = geometry::cartesian_to_polar(uv_point);
    return image::point(polar_space[0] / m_surface_scale.u,
                        ((polar_space[1] + iso::pi) / iso::tau) / m_surface_scale.v);
}

precision plane::get_object_extent(void) const {
    return basal::pos_inf;
}

bool plane::is_along_infinite_extent(ray const& world_ray) const {
    // is the ray parallel to the plane?
    auto object_ray = reverse_transform(world_ray);
    // @note in object space, the center point is at the origin
    vector const& N = normal_(R3::origin);
    vector const& V = object_ray.direction();
    // projection of normal on ray direction
    precision const proj = dot(V, N);
    // if zero, they can not intersect anywhere
    return basal::nearly_zero(proj);
}

void plane::print(std::ostream& os, char const str[]) const {
    os << str << " Plane @" << this << " " << position() << " rotation: " << m_rotation << std::endl;
}

}  // namespace objects
}  // namespace raytrace
