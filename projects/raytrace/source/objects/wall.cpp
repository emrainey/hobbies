#include "raytrace/objects/wall.hpp"
#include "basal/ieee754.hpp"

#include <iostream>

namespace raytrace {
namespace objects {
using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

wall::wall(const point& C, const vector& N, precision thickness, precision s)
    : m_front_{C + ((N.normalized()/2.0) * thickness), N, s}
    , m_back_{C + ((-N.normalized()/2.0) * thickness), -N, s} {
}

vector wall::normal(const point& P) const {
    if (m_front_.is_surface_point(P)) {
        return m_front_.normal(P);
    } else if (m_back_.is_surface_point(P)) {
        return m_back_.normal(P);
    }
    return geometry::R3::null;
}

hits wall::collisions_along(const ray& world_ray) const {
    hits ts0 = m_front_.collisions_along(world_ray);
    hits ts1 = m_back_.collisions_along(world_ray);
    ts0.insert(ts0.end(), ts1.begin(), ts1.end());
    return ts0;
}

bool wall::is_surface_point(const point& world_point) const {
    return m_front_.is_surface_point(world_point) or m_back_.is_surface_point(world_point);
}

image::point wall::map(const point& object_surface_point) const {
    if (m_front_.is_surface_point(object_surface_point)) {
        return m_front_.map(object_surface_point);
    }
    else if (m_back_.is_surface_point(object_surface_point)) {
        return m_back_.map(object_surface_point);
    }
    return geometry::R2::origin;
}

precision wall::get_object_extant(void) const {
    return basal::nan;
}

void wall::print(const char str[]) const {
    m_front_.print(str);
    m_back_.print(str);
}

}  // namespace objects
}  // namespace raytrace
