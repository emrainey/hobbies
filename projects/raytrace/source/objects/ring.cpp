#include "raytrace/objects/ring.hpp"

#include <iostream>

namespace raytrace {
namespace objects {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

ring::ring(precision inner, precision outer) : ring(R3::origin, R3::identity, inner, outer) {
}

ring::ring(point const& C, matrix const& R, precision inner, precision outer)
    : raytrace::objects::plane(C, R), m_inner_radius2(inner * inner), m_outer_radius2(outer * outer) {
    m_type = Type::Ring;
    basal::exception::throw_unless(m_inner_radius2 < m_outer_radius2, __FILE__, __LINE__,
                                   "Inner radius must be less than outer radius");
}

hits ring::collisions_along(ray const& object_ray) const {
    hits ts;
    // is the ray parallel to the plane?
    vector const& N = normal_(R3::origin);  // same as plane()
    vector const& V = object_ray.direction();
    precision const proj = dot(V, N);                     // if so the projection is zero
    if (not basal::nearly_zero(proj) and proj < 0.0_p) {  // they collide *somewhere*
        point const& P = object_ray.location();
        // get the vector of the center to the ray initial
        vector const C = geometry::R3::origin - P;
        // t is the ratio of the projection of the arbitrary center vector divided by the projection ray
        precision const t0 = dot(C, N) / proj;
        // D is the point on the line
        point const D = object_ray.distance_along(t0);
        // E is the vector from the center to that point
        vector const E = D - geometry::R3::origin;
        // the distance to the center squared
        precision distance_to_center2 = E.quadrance();
        // do we need to do a more in-depth test?
        if (m_inner_radius2 <= distance_to_center2 and distance_to_center2 <= m_outer_radius2) {
            ts.emplace_back(intersection{D}, t0, N, this);
        }
    }
    return ts;
}

bool ring::is_surface_point(point const& world_point) const {
    point object_point = reverse_transform(world_point);
    precision x = object_point.x();
    precision y = object_point.y();
    precision dd = x * x + y * y;
    return m_inner_radius2 <= dd and dd <= m_outer_radius2;
}

void ring::print(std::ostream& os, char const str[]) const {
    os << str << " ring @" << this << " " << object_<3>::position() << " Radii (Squared):" << m_inner_radius2 << ", "
       << m_outer_radius2 << std::endl;
}

precision ring::get_object_extent(void) const {
    return sqrt(m_outer_radius2);
}

}  // namespace objects
}  // namespace raytrace
