
#include <iostream>
#include "raytrace/objects/pyramid.hpp"

namespace raytrace {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

pyramid::pyramid(const point& base, double height)
    : object(base, 2)
    {}

vector pyramid::normal(const point& world_surface_point) const {
    point object_surface_point = reverse_transform(world_surface_point);
    vector object_surface_normal{{
        object_surface_point.x >= 0 ? 1.0 : -1.0,
        object_surface_point.y >= 0 ? 1.0 : -1.0,
        1.0
    }};
    return forward_transform(object_surface_normal.normalized());
}

inline bool is_positive(element_type p, element_type d, element_type t) {
    return ((p + d*t) > 0);
}

hits pyramid::collisions_along(const ray& object_ray) const {
    hits ts;
    element_type h = m_height;
    element_type px = object_ray.location().x;
    element_type py = object_ray.location().y;
    element_type pz = object_ray.location().z;
    element_type dx = object_ray.direction()[0];
    element_type dy = object_ray.direction()[1];
    element_type dz = object_ray.direction()[2];
    // we know if x or y components positive or negative but we don't know
    // t yet so we don't know when the ray equation (p + dt) will be > 0
    // so we solve for 4 different scenarios to find the answer. Generally we subsitute
    // |p + d*t| = a*(p + d*t) = a*p + a*d*t where a can be 1.0 or -1.0.
    // when p > 0 and d > 0 then a =  1.0
    // when p < 0 and d < 0 then a = -1.0
    // when p > 0 and d < 0 then when t < |p|/|d| a = 1.0, else -1.0
    // when p < 0 and d > 0 then when t > |p|/|d| a = 1.0, else -1.0
    // the problem is that we don't know t yet! However, a can only be 2 values, and we have
    // a and b (associated with x and y) respectively.
    // z = h - |x| - |y|
    // 0 = h - |x| - |y| - z
    // 0 = h - a(px + dx*t) - b(py + dy*t) - (pz + dz*t) and t > 0
    // 0 = h - a*px - a*dx*t - b*py - b*dy*t - pz - dz*t
    // 0 = h - a*px - b*py - pz - t*(a*dx + b+dy + dz)
    // t = (h - a*px - b*py - pz) / (a*dx + b+dy + dz)
    element_type denoms[4] = {
        dx + dy + dz, // a = 1, b = 1
        dy - dx + dz, // a =-1, b = 1
        dx - dy + dz, // a = 1, b =-1
        dz - dx - dy, // a =-1, b =-1
    };
    element_type t[] = {
        // a = 1, b = 1
        basal::equals_zero(denoms[0]) ? std::nan("") : (h - px - py - pz) / (denoms[0]),
        // a =-1, b = 1
        basal::equals_zero(denoms[1]) ? std::nan("") : (h + px - py - pz) / (denoms[1]),
        // a = 1, b =-1
        basal::equals_zero(denoms[2]) ? std::nan("") : (h - px + py - pz) / (denoms[2]),
        // a =-1, b =-1
        basal::equals_zero(denoms[3]) ? std::nan("") : (h + px + py - pz) / (denoms[3]),
    };
    bool usable[4] = {
            is_positive(px, dx, t[0]) and     is_positive(py, dy, t[0]),
        not is_positive(px, dx, t[1]) and     is_positive(py, dy, t[1]),
            is_positive(px, dx, t[2]) and not is_positive(py, dy, t[2]),
        not is_positive(px, dx, t[3]) and not is_positive(py, dy, t[3]),
    };
    for (size_t i = 0; i < 4; i++) {
        if (not std::isnan(t[i])) {
            if (usable[i] and t[i] >= 0) {
                ts.push_back(t[i]);
            }
        }
    }
    return ts;
}

image::point pyramid::map(const point& object_surface_point) const {
    return image::point(0, 0); // no mapping for now
}

void pyramid::print(const char str[]) const  {
    std::cout  << str << " Pyramid @" << this << " " << position() << " Height " << m_height << std::endl;
}

} // namespace raytrace
