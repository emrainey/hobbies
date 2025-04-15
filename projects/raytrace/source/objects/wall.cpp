#include "raytrace/objects/wall.hpp"
#include "basal/ieee754.hpp"

#include <iostream>

namespace raytrace {
namespace objects {
using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

wall::wall(precision thickness) : wall(R3::origin, R3::identity, thickness) {
}

wall::wall(point const& C, matrix const& R, precision thickness)
    : object{C, 2, Type::Wall, false}
    // a wall is not a "closed" surface
    // the "wall space" has the center at the origin and the walls are just offset from origin
    // each wall is just the thickness of the wall divided by 2 away from the center in Z
    // an extra rotation is applied  to the back wall to get the correct normal
    , m_front_{R3::origin + (R3::basis::Z * (thickness / 2.0_p)), R3::identity}
    , m_back_{R3::origin - (R3::basis::Z * (thickness / 2.0_p)), R3::roll(0.5)} {
    m_rotation = R;
    m_inv_rotation = R.inverse();
    compute_transforms();
}

vector wall::normal_(point const& wall_point) const {
    using namespace geometry::operators;
    // wall space is NOT world space, it's been transformed!
    vector const& Nf = m_front_.normal(wall_point);
    vector const& Nb = m_back_.normal(wall_point);
    if (wall_point == m_front_.position()) {
        return Nf;
    }
    if (wall_point == m_back_.position()) {
        return Nb;
    }
    // these are within wall space, not world space
    vector const Vf = wall_point - m_front_.position();
    vector const Vb = wall_point - m_back_.position();
    precision projection_to_front_plane = dot(Nf, Vf);
    precision projection_to_back_plane = dot(Nb, Vb);
    if (basal::nearly_zero(projection_to_front_plane)) {
        return Nf;  // already forward transformed
    } else if (basal::nearly_zero(projection_to_back_plane)) {
        return Nb;  // already forward transformed
    }
    // either between or nearly so
    return geometry::R3::null;
}

hits wall::collisions_along(ray const& wall_ray) const {
    auto plane_rayA = m_front_.reverse_transform(wall_ray);
    hits ts0 = m_front_.collisions_along(plane_rayA);
    // each hit is in object space, not wall space and needs to be updated
    for (auto& t : ts0) {
        t.intersect = m_front_.forward_transform(as_point(t.intersect));
        t.normal = m_front_.forward_transform(t.normal);
    }
    auto plane_rayB = m_back_.reverse_transform(wall_ray);
    hits ts1 = m_back_.collisions_along(plane_rayB);
    // each hit is in object space, not wall space and needs to be updated
    for (auto& t : ts1) {
        t.intersect = m_back_.forward_transform(as_point(t.intersect));
        t.normal = m_back_.forward_transform(t.normal);
    }
    ts0.insert(ts0.end(), ts1.begin(), ts1.end());
    return ts0;
}

bool wall::is_surface_point(point const& world_point) const {
    point wall_point = reverse_transform(world_point);
    return m_front_.is_surface_point(wall_point) or m_back_.is_surface_point(wall_point);
}

image::point wall::map(point const& world_point) const {
    point wall_point = reverse_transform(world_point);
    if (m_front_.is_surface_point(wall_point)) {
        return m_front_.map(wall_point);
    } else if (m_back_.is_surface_point(wall_point)) {
        return m_back_.map(wall_point);
    }
    return geometry::R2::origin;
}

precision wall::get_object_extent(void) const {
    return basal::pos_inf;
}

void wall::print(std::ostream& os, char const str[]) const {
    os << "Wall of two planes: " << std::endl;
    m_front_.print(os, str);
    m_back_.print(os, str);
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
