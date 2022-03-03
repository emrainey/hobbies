#include "raytrace/objects/ring.hpp"

#include <iostream>

namespace raytrace {
namespace objects {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

ring::ring(const point& C, const vector& N, element_type inner, element_type outer)
    : raytrace::objects::plane(C, N, outer), m_inner_radius2(inner * inner), m_outer_radius2(outer * outer) {
}

hits ring::collisions_along(const ray& object_ray) const {
    hits ts;
    // is the ray parallel to the plane?
    const vector& N = m_normal;
    const vector& V = object_ray.direction();
    element_type proj = dot(V, N);       // if so the projection is zero
    if (not basal::equals_zero(proj)) {  // they collide *somewhere*
        // get the vector of the center to the ray initial
        vector C = position() - object_ray.location();
        // t is the ratio of the projection of the arbitrary center vector divided by the projection ray
        element_type t0 = dot(C, N) / proj;
        point D = object_ray.distance_along(t0);
        element_type distance_to_center2 = (position() - D).quadrance();
        // do we need to do a more in-depth test?
        if (m_inner_radius2 <= distance_to_center2 and distance_to_center2 <= m_outer_radius2) {
            ts.push_back(t0);
        }
    }
    return ts;
}

void ring::print(const char str[]) const {
    std::cout << str << " ring @" << this << " " << object_<3>::position() << " " << m_normal
              << " Radii (Squared):" << m_inner_radius2 << ", " << m_outer_radius2 << std::endl;
}

}  // namespace objects
}  // namespace raytrace
