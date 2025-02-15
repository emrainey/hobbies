#include "raytrace/objects/square.hpp"

#include <iostream>

namespace raytrace {
namespace objects {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

square::square(point const& C, vector const& N, precision hh, precision hw)
    : raytrace::objects::plane(C, N), m_points{} {
    m_points[0] = raytrace::point{-hw, -hh, 0};
    m_points[1] = raytrace::point{+hw, +hh, 0};
}

hits square::collisions_along(ray const& object_ray) const {
    hits ts;
    // is the ray parallel to the plane?
    vector const& N = unormal(); // .normalized();
    vector const& V = object_ray.direction();
    precision proj = dot(V, N);       // if so the projection is zero
    if (not basal::nearly_zero(proj)) {  // they collide *somewhere*
        // get the vector of the center to the ray initial
        vector C = position() - object_ray.location();
        // t is the ratio of the projection of the arbitrary center vector divided by the projection ray
        precision t0 = dot(C, N) / proj;
        point D = object_ray.distance_along(t0);
        // if point D is contained within the 3D points it's in the square.
        if (m_points[0].x <= D.x and D.x <= m_points[1].x and m_points[0].y <= D.y and D.y <= m_points[1].y) {
            ts.emplace_back(intersection{D}, t0, normal_(D), this);
        }
    }
    return ts;
}

bool square::is_surface_point(point const& world_point) const {
    point object_point = reverse_transform(world_point);
    precision x = object_point.x;
    precision y = object_point.y;
    return linalg::within(m_points[0].x, x, m_points[1].x) and linalg::within(m_points[0].y, y, m_points[1].y);
}

void square::print(char const str[]) const {
    std::cout << str << " square @" << this << " " << object_<3>::position() << " " << m_normal << std::endl;
}

image::point square::map(point const& object_surface_point) const {
    return image::point(object_surface_point.x / m_surface_scale.u, object_surface_point.y / m_surface_scale.v);
}

precision square::get_object_extent(void) const {
    return (m_points[1] - R3::origin).magnitude();
}

}  // namespace objects
}  // namespace raytrace
