
#include "raytrace/objects/cuboid.hpp"

namespace raytrace {
namespace objects {
const vector cuboid::m_normals[6] = {
    R3::basis::X, -R3::basis::X, R3::basis::Y, -R3::basis::Y, R3::basis::Z, -R3::basis::Z,
};

cuboid::cuboid(const point& center, precision xhw, precision yhw, precision zhw)
    : object{center, 2, true}  // max 2 collisions, closed surface
    , x_half_width{m_half_widths[0]}
    , y_half_width{m_half_widths[1]}
    , z_half_width{m_half_widths[2]}
    , m_half_widths{} {
    m_half_widths[0] = xhw;
    m_half_widths[1] = yhw;
    m_half_widths[2] = zhw;
    for (auto width : m_half_widths) {
        basal::exception::throw_if(width == 0.0, __FILE__, __LINE__, "Half-width can't be zero");
    }
    m_faces[0] = point(x_half_width, 0, 0);
    m_faces[1] = point(-x_half_width, 0, 0);
    m_faces[2] = point(0, y_half_width, 0);
    m_faces[3] = point(0, -y_half_width, 0);
    m_faces[4] = point(0, 0, z_half_width);
    m_faces[5] = point(0, 0, -z_half_width);
}

cuboid::cuboid(point&& center, precision xhw, precision yhw, precision zhw)
    : object{std::move(center), 2, true}  // 2 collisions, closed surface
    , x_half_width{m_half_widths[0]}
    , y_half_width{m_half_widths[1]}
    , z_half_width{m_half_widths[2]}
    , m_half_widths{} {
    m_half_widths[0] = xhw;
    m_half_widths[1] = yhw;
    m_half_widths[2] = zhw;
    for (auto width : m_half_widths) {
        basal::exception::throw_if(width == 0.0, __FILE__, __LINE__, "Half-width can't be zero");
    }
    m_faces[0] = point(x_half_width, 0, 0);
    m_faces[1] = point(-x_half_width, 0, 0);
    m_faces[2] = point(0, y_half_width, 0);
    m_faces[3] = point(0, -y_half_width, 0);
    m_faces[4] = point(0, 0, z_half_width);
    m_faces[5] = point(0, 0, -z_half_width);
}

vector cuboid::normal(const point& world_surface_point) const {
    point object_surface_point = reverse_transform(world_surface_point);
    vector object_normal = R3::null;
    // isolate which axis this is on and return the forward_transformed normal
    if (basal::nearly_equals(object_surface_point.x, x_half_width)) {
        // positive yz plane
        object_normal = m_normals[0];
    } else if (basal::nearly_equals(object_surface_point.x, -x_half_width)) {
        // negative yz plane
        object_normal = m_normals[1];
    } else if (basal::nearly_equals(object_surface_point.y, y_half_width)) {
        // positive xz plane
        object_normal = m_normals[2];
    } else if (basal::nearly_equals(object_surface_point.y, -y_half_width)) {
        // negative xz plane
        object_normal = m_normals[3];
    } else if (basal::nearly_equals(object_surface_point.z, z_half_width)) {
        // positive xy plane
        object_normal = m_normals[4];
    } else if (basal::nearly_equals(object_surface_point.z, -z_half_width)) {
        // negative xy plane
        object_normal = m_normals[5];
    }
    return forward_transform(object_normal);
}

hits cuboid::collisions_along(const ray& object_ray) const {
    hits ts;
    const bool disable_optimization = true;  // to allow cuboids to be overlap objects, this has to be turned off.

    // given the direction of the vector, only certain sides of the AABB are colliable.
    // the sign of the vector component is the opposite of the side that may be collidable.
    const precision& x = object_ray.location().x;
    const precision& y = object_ray.location().y;
    const precision& z = object_ray.location().z;
    const precision& i = object_ray.direction()[0];
    const precision& j = object_ray.direction()[1];
    const precision& k = object_ray.direction()[2];

    precision t;

    precision p_xmax = m_half_widths[0];
    precision p_xmin = -m_half_widths[0];
    precision p_ymax = m_half_widths[1];
    precision p_ymin = -m_half_widths[1];
    precision p_zmax = m_half_widths[2];
    precision p_zmin = -m_half_widths[2];

    point p_max(p_xmax, p_ymax, p_zmax);
    point p_min(p_xmin, p_ymin, p_zmin);

    point surface_point;

/// uses t, object_point, and p_min, p_max
#define FACET_TEST(p, v, m, op)                                   \
    if (v op 0 or disable_optimization) {                         \
        t = (m - p) / v;                                          \
        surface_point = object_ray.distance_along(t);             \
        if (contained_within_aabb(surface_point, p_min, p_max)) { \
            ts.push_back(t);                                      \
        }                                                         \
    }

    FACET_TEST(x, i, p_xmin, >);
    FACET_TEST(x, i, p_xmax, <);
    FACET_TEST(y, j, p_ymin, >);
    FACET_TEST(y, j, p_ymax, <);
    FACET_TEST(z, k, p_zmin, >);
    FACET_TEST(z, k, p_zmax, <);
#undef FACET_TEST
    return ts;
}

image::point cuboid::map(const point& object_surface_point) const {
    // map some range of object 3D points to some 2D u,v pair
    // isolate which axis this is on and return the forward_transformed normal
    precision u = 0.0, v = 0.0;

    if (basal::nearly_equals(object_surface_point.x, x_half_width)) {
        // positive yz plane
        // u = -y -> +y
        u = ((object_surface_point.y / (2.0 * y_half_width)) + 0.5);
        // v = +z -> -z
        v = -((object_surface_point.z / (2.0 * z_half_width)) - 0.5);
    } else if (basal::nearly_equals(object_surface_point.x, -x_half_width)) {
        // negative yz plane
        // u = +y -> -y
        u = -((object_surface_point.y / (2.0 * y_half_width)) - 0.5);
        // v = +z -> -z
        v = -((object_surface_point.z / (2.0 * z_half_width)) - 0.5);
    } else if (basal::nearly_equals(object_surface_point.y, y_half_width)) {
        // positive xz plane
        // u = +x -> -x
        u = -((object_surface_point.x / (2.0 * x_half_width)) - 0.5);
        // v = +z -> -z
        v = -((object_surface_point.z / (2.0 * z_half_width)) - 0.5);
    } else if (basal::nearly_equals(object_surface_point.y, -y_half_width)) {
        // negative xz plane
        // u = -x -> +x
        u = ((object_surface_point.x / (2.0 * x_half_width)) + 0.5);
        // v = +z -> -z
        v = -((object_surface_point.z / (2.0 * z_half_width)) - 0.5);
    } else if (basal::nearly_equals(object_surface_point.z, z_half_width)) {
        // positive xy plane
        // u = +x -> -x
        u = -((object_surface_point.x / (2.0 * x_half_width)) - 0.5);
        // v = +y -> -y
        v = -((object_surface_point.y / (2.0 * y_half_width)) - 0.5);
    } else if (basal::nearly_equals(object_surface_point.z, -z_half_width)) {
        // negative xy plane
        // u = -x -> +x
        u = ((object_surface_point.x / (2.0 * x_half_width)) + 0.5);
        // v = +y -> -y
        v = -((object_surface_point.y / (2.0 * y_half_width)) - 0.5);
    }

    // if all the clauses miss, then it returns 0,0
    return image::point(u, v);
}

void cuboid::print(const char name[]) const {
    std::cout << name << " " << *this << std::endl;
}

precision cuboid::get_object_extent(void) const {
    return vector{x_half_width, y_half_width, z_half_width}.magnitude();
}

}  // namespace objects

std::ostream& operator<<(std::ostream& os, const objects::cuboid& c) {
    os << " Cube " << c.position() << ", Half-Widths {" << c.x_half_width << "," << c.y_half_width << ","
       << c.z_half_width << "}";
    return os;
}

}  // namespace raytrace
