#include "raytrace/objects/plane.hpp"

#include <iostream>

namespace raytrace {
namespace objects {
using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

plane::plane(const point& C, vector const& N)
    : geometry::plane{C, N}, object{C, 1} {
}

vector plane::normal(const point&) const {
    return vector(geometry::plane::normal);
}

intersection plane::intersect(const ray& world_ray) const {
    hits ts = collisions_along(world_ray);
    if (ts.size() > 0 and ts[0] >= (0 - basal::epsilon)) {
        // check the orientation of the world ray with the normal
        precision d = dot(geometry::plane::normal, world_ray.direction());
        if (d < 0) {  // if the ray points into the plane, yes it collides, else no it's not really colliding
            // only 1 possible element and 1 type
            point world_point = world_ray.distance_along(ts[0]);
            statistics::get().intersections_with_objects++;
            return intersection(world_point);
        }
    }
    return intersection();
}

hits plane::collisions_along(const ray& world_ray) const {
    hits ts;
    // is the ray parallel to the plane?
    // @note in object space, the center point is at the origin
    vector const& N = geometry::plane::normal;
    vector const& V = world_ray.direction();
    precision const proj = dot(V, N);
    // if so the projection is not zero they collide *somewhere*
    // could be positive or negative t.
    if (not basal::nearly_zero(proj)) {
        const point& P = world_ray.location();
        // get the vector of the center to the ray initial
        vector const C = position() - P;
        // t is the ratio of the projection of the arbitrary center vector divided by the projection ray
        precision const t = dot(C, N) / proj;
        ts.push_back(t);
    }
    return ts;
}

bool plane::is_surface_point(const point& world_point) const {
    point object_point = reverse_transform(world_point);
    vector T = object_point - position();
    return basal::nearly_zero(dot(geometry::plane::normal, T));
}

image::point plane::map(const point& object_surface_point) const {
    // the pattern will map around the point in a polar fashion
    geometry::R2::point cartesian(object_surface_point[0] / m_surface_scale.u, object_surface_point[1] / m_surface_scale.v);
    geometry::R2::point polar_space = geometry::cartesian_to_polar(cartesian);
    return image::point(polar_space[0] / 1.0, polar_space[1] / iso::tau);
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
