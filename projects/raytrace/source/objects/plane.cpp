#include "raytrace/objects/plane.hpp"

#include <iostream>

namespace raytrace {
namespace objects {
using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

plane::plane(point const& C, vector const& N) : geometry::plane{C, N}, object{C, 1} {
    // C.print("plane");
    // N.print("plane");
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
        vector const C = geometry::R3::origin - P;
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
    // the pattern will map around the point in a polar fashion
    geometry::R2::point cartesian(object_surface_point[0], object_surface_point[1]);
    geometry::R2::point polar_space = geometry::cartesian_to_polar(cartesian);
    // v must be between 0 and 1, no negatives (the zero angle line is -X though!)
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

void plane::print(char const str[]) const {
    std::cout << str << " Plane @" << this << " " << position() << " Normal " << unormal() << std::endl;
}

}  // namespace objects
}  // namespace raytrace
