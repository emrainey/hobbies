
#include "raytrace/objects/pyramid.hpp"
#include "basal/ieee754.hpp"
#include <iostream>

using namespace basal::literals;

namespace raytrace {
namespace objects {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

pyramid::pyramid(point const& base, precision height) : object(base, 2, Type::Pyramid, false), m_height{height} {
}

vector pyramid::normal_(point const& object_surface_point) const {
    // Return the normal in object space. The callers (object::normal() and object::intersect())
    // transform it to world space themselves, so transforming here would double-rotate.
    bool positive_x = (object_surface_point.x() > 0.0_p);
    bool positive_y = (object_surface_point.y() > 0.0_p);
    precision x = positive_x ? basal::inv_sqrt_3 : -basal::inv_sqrt_3;
    precision y = positive_y ? basal::inv_sqrt_3 : -basal::inv_sqrt_3;
    precision z = basal::inv_sqrt_3;
    return vector{{x, y, z}};
}

inline bool is_positive(precision p, precision d, precision t) {
    return ((p + d * t) > 0);
}

hits pyramid::collisions_along(ray const& object_ray) const {
    hits ts;
    precision h = m_height;
    precision px = object_ray.location().x();
    precision py = object_ray.location().y();
    precision pz = object_ray.location().z();
    precision dx = object_ray.direction()[0];
    precision dy = object_ray.direction()[1];
    precision dz = object_ray.direction()[2];
    // we know if x or y components positive or negative but we don't know
    // t yet so we don't know when the ray equation (p + dt) will be > 0
    // so we solve for 4 different scenarios to find the answer. Generally we substitute
    // |p + d*t| = a*(p + d*t) = a*p + a*d*t where a can be 1.0_p or -1.0_p.
    // when p > 0 and d > 0 then a =  1.0_p
    // when p < 0 and d < 0 then a = -1.0_p
    // when p > 0 and d < 0 then when t < |p|/|d| a = 1.0_p, else -1.0_p
    // when p < 0 and d > 0 then when t > |p|/|d| a = 1.0_p, else -1.0_p
    // the problem is that we don't know t yet! However, it can only be 2 values, and we have
    // a and b (associated with x and y) respectively.
    // z = h - |x| - |y|
    // 0 = h - |x| - |y| - z
    // 0 = h - a(px + dx*t) - b(py + dy*t) - (pz + dz*t) and t > 0
    // 0 = h - a*px - a*dx*t - b*py - b*dy*t - pz - dz*t
    // 0 = h - a*px - b*py - pz - t*(a*dx + b+dy + dz)
    // t = (h - a*px - b*py - pz) / (a*dx + b+dy + dz)
    precision denoms[4] = {
        dx + dy + dz,  // a = 1, b = 1
        dy - dx + dz,  // a =-1, b = 1
        dx - dy + dz,  // a = 1, b =-1
        dz - dx - dy,  // a =-1, b =-1
    };
    precision t[] = {
        // a = 1, b = 1
        precision(basal::nearly_zero(denoms[0]) ? basal::nan : (h - px - py - pz) / (denoms[0])),
        // a =-1, b = 1
        precision(basal::nearly_zero(denoms[1]) ? basal::nan : (h + px - py - pz) / (denoms[1])),
        // a = 1, b =-1
        precision(basal::nearly_zero(denoms[2]) ? basal::nan : (h - px + py - pz) / (denoms[2])),
        // a =-1, b =-1
        precision(basal::nearly_zero(denoms[3]) ? basal::nan : (h + px + py - pz) / (denoms[3])),
    };
    bool usable[4] = {
        is_positive(px, dx, t[0]) and is_positive(py, dy, t[0]),
        not is_positive(px, dx, t[1]) and is_positive(py, dy, t[1]),
        is_positive(px, dx, t[2]) and not is_positive(py, dy, t[2]),
        not is_positive(px, dx, t[3]) and not is_positive(py, dy, t[3]),
    };
    for (size_t i = 0; i < 4; i++) {
        if (not basal::is_nan(t[i])) {
            if (usable[i]) {
                point R = object_ray.distance_along(t[i]);
                // The side planes extend infinitely downward; only keep hits within the
                // pyramid's vertical extent (0, h).
                if (linalg::within(0, R.z(), h)) {
                    ts.emplace_back(intersection{R}, t[i], normal_(R), this);
                }
            }
        }
    }
    return ts;
}

bool pyramid::is_surface_point(point const& world_point) const {
    point object_point = reverse_transform(world_point);
    precision x = object_point.x();
    precision y = object_point.y();
    precision z = object_point.z();
    // The equation also holds on the infinite extensions of the side planes, so the
    // point must additionally lie within the pyramid's vertical extent [0, h].
    if (z < 0.0_p or z > m_height) {
        return false;
    }
    return basal::nearly_equals(z, m_height - std::abs(x) - std::abs(y));
}

image::point pyramid::map(point const& object_surface_point __attribute__((unused))) const {
    return image::point(0, 0);  // no mapping for now
}

void pyramid::print(std::ostream& os, char const str[]) const {
    os << str << " Pyramid @" << this << " " << position() << " Height " << m_height << std::endl;
}

precision pyramid::get_object_extent(void) const {
    // The apex and all four base corners lie at distance m_height from the
    // base center, so a sphere of radius m_height encloses the pyramid.
    return m_height;
}

}  // namespace objects
}  // namespace raytrace
