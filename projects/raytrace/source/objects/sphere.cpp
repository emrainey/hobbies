
#include "raytrace/objects/sphere.hpp"

#include <iostream>

namespace raytrace {
namespace objects {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

sphere::sphere(point const& c, precision r)
    : geometry::R3::sphere(r)
    , object(c, 2, Type::Sphere, true)  // up to 2 collisions, closed surface
{
}

bool sphere::is_surface_point(point const& world_surface_point) const {
    point object_surface_point = reverse_transform(world_surface_point);
    return geometry::R3::sphere::on_surface(object_surface_point);
}

vector sphere::normal_(point const& object_surface_point) const {
    return geometry::R3::sphere::normal_at(object_surface_point);
}

hits sphere::collisions_along(ray const& object_ray) const {
    hits ts;
    precision px = object_ray.location().x();
    precision py = object_ray.location().y();
    precision pz = object_ray.location().z();
    precision dx = object_ray.direction()[0];
    precision dy = object_ray.direction()[1];
    precision dz = object_ray.direction()[2];
    precision a = (dx * dx + dy * dy + dz * dz);
    precision b = 2.0_p * (dx * px + dy * py + dz * pz);
    precision c = (px * px + py * py + pz * pz) - (m_radius * m_radius);
    auto roots = linalg::quadratic_roots(a, b, c);
    precision t0 = std::get<0>(roots);
    precision t1 = std::get<1>(roots);
    if (not basal::is_nan(t0)) {
        point R = object_ray.distance_along(t0);
        ts.emplace_back(intersection{R}, t0, normal_(R), this);
    }
    if (not basal::is_nan(t1)) {
        point R = object_ray.distance_along(t1);
        ts.emplace_back(intersection{R}, t1, normal_(R), this);
    }
    return ts;
}

image::point sphere::map(point const& object_surface_point) const {
    return mapping::spherical(object_surface_point);
}

void sphere::print(std::ostream& os, char const str[]) const {
    os << str << " Sphere @" << this << " " << position() << " Radius " << radius << std::endl;
}

precision sphere::get_object_extent(void) const {
    return m_radius;
}

}  // namespace objects
}  // namespace raytrace
