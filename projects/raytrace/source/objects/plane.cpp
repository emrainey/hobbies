#include "raytrace/objects/plane.hpp"

#include <iostream>

namespace raytrace {
namespace objects {
using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

plane::plane(point const& C, vector const& N)
    : geometry::plane{C, N}
    , object{C, 1, Type::Plane, false}
    , m_basis{} {  // default to the X axis as the plane's orientation

    // the basis is not set here since we were provided the normal. we need to figure out how to construct a basis from
    // just the normal and potentially either X or Y through

    // only if the N is the X axis, then we'll need to do something else, but if it isn't then we can cross N x X to get
    // the Y axis then cross Y x N to get the real x axis then we're done
    if (N == geometry::R3::basis::X) {
        // fallback case
        // if the normal is the X axis, then know exactly how we need to be oriented.
        m_basis = axes{C, -geometry::R3::basis::Z, geometry::R3::basis::Y, geometry::R3::basis::X};  // set the basis
    } else if (N == -geometry::R3::basis::X) {
        // fallback case
        // if the normal is the -X axis, then know exactly how we need to be oriented.
        m_basis = axes{C, geometry::R3::basis::Z, geometry::R3::basis::Y, -geometry::R3::basis::X};  // set the basis
    } else {
        vector Y = cross(N, geometry::R3::basis::X).normalized();
        vector X = cross(Y, N);
        m_basis = axes{C, X, Y, N};  // set the basis
    }
}

plane::plane(axes const& axes)
    : geometry::plane{axes.origin(), axes.applicate()}, object{axes.origin(), 1, Type::Plane, false}, m_basis{axes} {
}

vector plane::normal_(point const& object_space_point __attribute__((unused))) const {
    return geometry::plane::unormal().normalize();
}

hits plane::collisions_along(ray const& object_ray) const {
    hits ts;
    // is the ray parallel to the plane?
    // @note in object space, the center point is at the origin
    vector const& N = unormal().normalized();
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
    vector T = world_point - position();
    if (T == R3::null) {
        return true;
    }
    return basal::nearly_zero(dot(unormal().normalized(), T));
}

image::point plane::map(point const& object_surface_point) const {
    matrix const& M = m_basis.to_basis();
    // the pattern will map around the point in a polar fashion
    geometry::R3::point basis_point = M * object_surface_point;
    geometry::R2::point uv_point{basis_point.x, basis_point.y};
    geometry::R2::point polar_space = geometry::cartesian_to_polar(uv_point);
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
    vector const& N = unormal().normalized();
    vector const& V = object_ray.direction();
    // projection of normal on ray direction
    precision const proj = dot(V, N);
    // if zero, they can not intersect anywhere
    return basal::nearly_zero(proj);
}

void plane::print(std::ostream& os, char const str[]) const {
    os << str << " Plane @" << this << " " << position() << " Normal " << unormal() << std::endl;
}

}  // namespace objects
}  // namespace raytrace
