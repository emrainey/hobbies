
#include <iostream>
#include "raytrace/objects/sphere.hpp"

namespace raytrace {
namespace objects {

using namespace linalg;
using namespace linalg::operators;
using namespace geometry;
using namespace geometry::operators;

sphere::sphere(const point& c, double r)
    : geometry::R3::sphere(r)
    , object(c, 2, true) // up to 2 collisions, closed surface
    {}

bool sphere::surface(const point& world_surface_point) const {
    point object_surface_point = reverse_transform(world_surface_point);
    return geometry::R3::sphere::surface(object_surface_point);
}

vector sphere::normal(const point& world_surface_point) const {
    point object_surface_point = reverse_transform(world_surface_point);
    return forward_transform(geometry::R3::sphere::normal(object_surface_point));
}

hits sphere::collisions_along(const ray& object_ray) const {
    hits ts;
    element_type px = object_ray.location().x;
    element_type py = object_ray.location().y;
    element_type pz = object_ray.location().z;
    element_type dx = object_ray.direction()[0];
    element_type dy = object_ray.direction()[1];
    element_type dz = object_ray.direction()[2];
    element_type a = (dx*dx + dy*dy + dz*dz);
    element_type b = 2.0*(dx*px + dy*py + dz*pz);
    element_type c = (px*px + py*py + pz*pz) - (m_radius * m_radius);
    auto roots = linalg::quadratic_roots(a, b, c);
    element_type t0 = std::get<0>(roots);
    element_type t1 = std::get<1>(roots);
    if (not std::isnan(t0)) ts.push_back(t0);
    if (not std::isnan(t1)) ts.push_back(t1);
    return ts;
}

image::point sphere::map(const point& object_surface_point) const {
    // in object space in a sphere, it's a unit sphere.
    // get the polar coordinates
    point pol = geometry::R3::sphere::cart_to_polar(object_surface_point);
    element_type u = pol[1]  / iso::tau; // 0-theta-2pi
    u = (u < 0 ? 1.0 + u : u);
    element_type v = pol[2] / iso::pi; // 0-phi-pi
    image::point uv(u, v);
    // std::cout << "R3: " << object_surface_point << " => R2: " << uv;
    return uv;
}

void sphere::print(const char str[]) const  {
    std::cout  << str << " Sphere @" << this << " " << position() << " Radius " << radius << std::endl;
}

} // namespace objects
} // namespace raytrace
