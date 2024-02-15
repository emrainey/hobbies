#include "raytrace/objects/wall.hpp"
#include "basal/ieee754.hpp"

#include <iostream>

namespace raytrace {
namespace objects {
using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

wall::wall(const point& C, const vector& N, precision thickness)
    : object{C, 2, true} // a wall is a closed surface, even through it's infinite?
    , m_front_{C + (N.normalized() * thickness/2.0_p), N.normalized()}
    , m_back_{C + ((-N.normalized()) * thickness/2.0_p), -N.normalized()} {
}

vector wall::normal(const point& world_point) const {
    auto object_point = reverse_transform(world_point);
    vector const& Nf = m_front_.normal(object_point);
    vector const& Nb = m_back_.normal(object_point);
    vector const P = object_point - position();
    precision df = dot(Nf, P);
    precision db = dot(Nb, P);
    if (basal::nearly_zero(df) and basal::nearly_zero(db)) {
        return geometry::R3::null;
    } else if (df > 0.0 and db < 0.0) {
        return forward_transform(Nb); // closer to back
    } else if (df < 0.0 and db > 0.0) {
        return forward_transform(Nf); // closer to front
    } // else if not possible but return anyway
    return geometry::R3::null;
}

intersection wall::intersect(const ray& world_ray) const {
    hits ts = collisions_along(world_ray); // either 0 or two
    if (ts.size() == 2 and ts[0] >= (0 - basal::epsilon) and ts[1] >= (0 - basal::epsilon)) {
        statistics::get().intersections_with_objects++;
        if (ts[0] < ts[1]) {
            point world_point = world_ray.distance_along(ts[0]);
            return intersection(world_point);
        } else {
            point world_point = world_ray.distance_along(ts[1]);
            return intersection(world_point);
        }
    }
    return intersection();
}

hits wall::collisions_along(const ray& world_ray) const {
    hits ts0 = m_front_.collisions_along(world_ray);
    std::cout << "ts0.size() = " << ts0.size() << "\n";
    hits ts1 = m_back_.collisions_along(world_ray);
    std::cout << "ts1.size() = " << ts1.size() << "\n";
    ts0.insert(ts0.end(), ts1.begin(), ts1.end());
    std::cout << "ts0.size() = " << ts0.size() << "\n";
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

precision wall::get_object_extent(void) const {
    return basal::nan;
}

void wall::print(const char str[]) const {
    m_front_.print(str);
    m_back_.print(str);
}

}  // namespace objects
}  // namespace raytrace
