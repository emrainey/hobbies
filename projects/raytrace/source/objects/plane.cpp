#include "raytrace/objects/plane.hpp"

#include <iostream>

namespace raytrace {
namespace objects {
using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

plane::plane(point const& C, vector const& N)
    : geometry::plane{C, N}, object{C, 1} {
    // C.print("plane");
    // N.print("plane");
}

vector plane::normal(point const& world_point __attribute__((unused))) const {
    // auto object_point = reverse_transform(world_point);
    vector world_normal = forward_transform(geometry::plane::normal);
    // world_normal.print("plane normal");
    return world_normal;
}

intersection plane::intersect(ray const& world_ray) const {
    auto object_ray = reverse_transform(world_ray);
    hits ts = collisions_along(object_ray);
    if (ts.size() > 0 and ts[0] >= (0 - basal::epsilon)) {
        // check the orientation of the world ray with the normal
        precision d = dot(geometry::plane::normal, object_ray.direction());
        if (d < 0) {  // if the ray points into the plane, yes it collides, else no it's not really colliding
            // only 1 possible element and 1 type
            auto object_point = object_ray.distance_along(ts[0]);
            point world_point = forward_transform(object_point);
            return intersection(world_point);
        }
    }
    return intersection();
}

hits plane::collisions_along(ray const& object_ray) const {
    hits ts;
    // is the ray parallel to the plane?
    // @note in object space, the center point is at the origin
    vector const& N = geometry::plane::normal;
    vector const& V = object_ray.direction();
    precision const proj = dot(V, N);
    // if so the projection is not zero they collide *somewhere*
    // could be positive or negative t.
    if (not basal::nearly_zero(proj)) {
        point const& P = object_ray.location();
        // get the vector of the center to the ray initial
        vector const C = geometry::R3::origin - P;
        // t is the ratio of the projection of the arbitrary center vector divided by the projection ray
        precision const t = dot(C, N) / proj;
        ts.push_back(t);
    }
    return ts;
}

bool plane::is_surface_point(point const& world_point) const {
    point object_point = reverse_transform(world_point);
    vector T = object_point - position();
    return basal::nearly_zero(dot(geometry::plane::normal, T));
}

image::point plane::map(point const& object_surface_point) const {
    // the pattern will map around the point in a polar fashion
    geometry::R2::point cartesian(object_surface_point[0], object_surface_point[1]);
    geometry::R2::point polar_space = geometry::cartesian_to_polar(cartesian);
    // v must be between 0 and 1, no negatives (the zero angle line is -X though!)
    return image::point(polar_space[0] / m_surface_scale.u, ((polar_space[1] + iso::pi) / iso::tau)  / m_surface_scale.v);
}

precision plane::get_object_extent(void) const {
    return basal::nan;
}

bool plane::is_along_infinite_extent(ray const& world_ray) const {
    // is the ray parallel to the plane?
    auto object_ray = reverse_transform(world_ray);
    // @note in object space, the center point is at the origin
    vector const& N = geometry::plane::normal;
    vector const& V = object_ray.direction();
    // projection of normal on ray direction
    precision const proj = dot(V, N);
    // if zero, they can not intersect anywhere
    return basal::nearly_zero(proj);
}

void plane::print(char const str[]) const {
    std::cout << str << " Plane @" << this << " " << position() << " Normal " << normal(position()) << std::endl;
}

}  // namespace objects
}  // namespace raytrace
