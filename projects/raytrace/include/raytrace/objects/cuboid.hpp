#pragma once

#include <basal/printable.hpp>
#include <geometry/geometry.hpp>
#include <iostream>
#include <linalg/linalg.hpp>

#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/** A six sided rectangar (in a 3D sense) object (cuboid). */
class cuboid : public object {
public:
    /**
     * Constructs a cuboid at a point of a given set of half widths. When the half-width are the same,
     * it is a cube.
     */
    cuboid(const point& center, double x_half_width, double y_half_width, double z_half_width);
    /** Uses the point move syntax */
    cuboid(point&& center, double x_half_width, double y_half_width, double z_half_width);
    virtual ~cuboid() = default;

    /** @copydoc raytrace::object::normal */
    vector normal(const point& world_surface_point) const override;
    /** @copydoc raytrace::object::intersect */
    // geometry::intersection intersect(const ray& world_ray) const override;
    /** @copydoc raytrace::object::collision_along */
    hits collisions_along(const ray& object_ray) const override;
    /** @copydoc raytrace::object::map */
    image::point map(const point& object_surface_point) const override;
    /** @copydoc basal::printable::print */
    void print(const char str[]) const override;

    const element_type& x_half_width;
    const element_type& y_half_width;
    const element_type& z_half_width;

protected:
    // The half-width sizes for x,y,z
    element_type m_half_widths[3];
    point m_faces[6];
    // These don't change with any particular instance
    static const vector m_normals[6];
};

}  // namespace objects

/** Print in stream for cuboid */
std::ostream& operator<<(std::ostream& os, const objects::cuboid& c);

}  // namespace raytrace