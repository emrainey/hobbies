#include "raytrace/objects/wall.hpp"
#include "basal/ieee754.hpp"

#include <iostream>

namespace raytrace {
namespace objects {
using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

wall::wall(point const& C, vector const& N, precision thickness)
    : object{C, 2, false} // a wall is not a closed surface
    // the "wall space" has the center at the origin and the walls are just offset from origin
    , m_front_{R3::origin + (N.normalized() * thickness/2.0_p), N.normalized()}
    , m_back_{R3::origin + ((-N.normalized()) * thickness/2.0_p), -N.normalized()} {
}

vector wall::normal(point const& world_point) const {
    using namespace geometry::operators;
    // world_point.print("request normal from");
    auto const wall_point = reverse_transform(world_point);
    vector const& Nf = m_front_.normal(wall_point);
    vector const& Nb = m_back_.normal(wall_point);
    if (geometry::operators::operator==(wall_point,m_front_.position())) {
        return Nf;
    }
    if (geometry::operators::operator==(wall_point, m_back_.position())) {
        return Nb;
    }
    // these are within wall space, not world space
    vector const Vf = wall_point - m_front_.position();
    vector const Vb = wall_point - m_back_.position();
    precision projection_to_front_plane = dot(Nf, Vf);
    precision projection_to_back_plane = dot(Nb, Vb);
    if (basal::nearly_zero(projection_to_front_plane)) {
        return forward_transform(Nf); // already forward transformed
    } else if (basal::nearly_zero(projection_to_back_plane)) {
        return forward_transform(Nb); // already forward transformed
    }
    // std::abort();
    // either between or nearly so
    return geometry::R3::null;
}

// intersection wall::intersect(ray const& world_ray) const {
//     auto object_ray = reverse_transform(world_ray);
//     hits ts = collisions_along(object_ray); // either 0 or two
//     if (ts.size() == 2 and ts[0] >= (0 - basal::epsilon) and ts[1] >= (0 - basal::epsilon)) {
//         size_t index = (ts[0] < ts[1]) ? 0 : 1u;
//         auto object_point = object_ray.distance_along(ts[index]);
//         auto world_point = forward_transform(object_point);
//         return intersection(world_point);
//     }
//     return intersection();
// }

hits wall::collisions_along(ray const& wall_ray) const {
    auto plane_rayA = m_front_.reverse_transform(wall_ray);
    hits ts0 = m_front_.collisions_along(plane_rayA);
    auto plane_rayB = m_back_.reverse_transform(wall_ray);
    hits ts1 = m_back_.collisions_along(plane_rayB);
    ts0.insert(ts0.end(), ts1.begin(), ts1.end());
    return ts0;
}

bool wall::is_surface_point(point const& world_point) const {
    return m_front_.is_surface_point(world_point) or m_back_.is_surface_point(world_point);
}

image::point wall::map(point const& object_surface_point) const {
    if (m_front_.is_surface_point(object_surface_point)) {
        return m_front_.map(object_surface_point);
    }
    else if (m_back_.is_surface_point(object_surface_point)) {
        return m_back_.map(object_surface_point);
    }
    return geometry::R2::origin;
}

precision wall::get_object_extent(void) const {
    return basal::nan;
}

void wall::print(char const str[]) const {
    m_front_.print(str);
    m_back_.print(str);
}

bool wall::is_outside(point const& world_point) const {
    auto wall_point = reverse_transform(world_point);
    return m_front_.is_outside(wall_point) or m_back_.is_outside(wall_point);
}

bool wall::is_along_infinite_extent(ray const& world_ray) const {
    auto wall_ray = reverse_transform(world_ray);
    // @TODO Not sure about the logic here
    return m_front_.is_along_infinite_extent(wall_ray) or m_back_.is_along_infinite_extent(wall_ray);
}

}  // namespace objects
}  // namespace raytrace
