#include <iostream>
#include "raytrace/objects/square.hpp"

namespace raytrace {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

square::square(const point& C, const vector& N, element_type hh, element_type hw)
    : raytrace::plane(C, N, 1.0)
    , m_points()
    {
    m_points[0] = raytrace::point(-hw, -hh, 0);
    m_points[1] = raytrace::point(+hw, +hh, 0);
}

hits square::collisions_along(const ray& object_ray) const {
    hits ts;
    // is the ray parallel to the plane?
    const vector& N = m_normal;
    const vector& V = object_ray.direction();
    element_type proj = dot(V, N); // if so the projection is zero
    if (not basal::equals_zero(proj)) { // they collide *somewhere*
        // get the vector of the center to the ray initial
        vector C = position() - object_ray.location();
        // t is the ratio of the projection of the arbitrary center vector divided by the projection ray
        element_type t0 = dot(C, N) / proj;
        point D = object_ray.distance_along(t0);
        // if point D is contained within the 3D points it's in the square.
        if (m_points[0].x <= D.x and D.x <= m_points[1].x and
            m_points[0].y <= D.y and D.y <= m_points[1].y) {
            ts.push_back(t0);
        }
    }
    return ts;
}

void square::print(const char str[]) const {
    std::cout << str << " square @" << this << " " << object_<3>::position() << " " << m_normal << std::endl;
}

image::point square::map(const point& object_surface_point) const {
    return image::point(object_surface_point.x / m_surface_scale, object_surface_point.y / m_surface_scale);
}

} // namespace raytrace
