#pragma once

#include <basal/printable.hpp>
#include <geometry/geometry.hpp>
#include <iostream>
#include <linalg/linalg.hpp>

#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/// A six sided rectangular (in a 3D sense) object (cuboid).
class cuboid : public object {
public:
    /// Constructs a cuboid at a point of a given set of half widths. When the half-width are the same,
/// it is a cube.
    cuboid(point const& center, precision x_half_width, precision y_half_width, precision z_half_width);
    /// Uses the point move syntax
    cuboid(point&& center, precision x_half_width, precision y_half_width, precision z_half_width);

    virtual ~cuboid() = default;

    /// @copydoc raytrace::object::intersect
    // geometry::intersection intersect(ray const& world_ray) const override;
    /// @copydoc raytrace::object::collision_along
    hits collisions_along(ray const& object_ray) const override;
    /// @copydoc raytrace::object::map
    image::point map(point const& object_surface_point) const override;
    /// @copydoc basal::printable::print
    void print(char const str[]) const override;
    precision get_object_extent(void) const override;

    precision const& x_half_width;
    precision const& y_half_width;
    precision const& z_half_width;

protected:
    vector normal_(point const& object_surface_point) const override;

    // The half-width sizes for x,y,z
    precision m_half_widths[3];
    point m_faces[6];
    // These don't change with any particular instance
    static vector const m_normals[6];
};

}  // namespace objects

/// Print in stream for cuboid
std::ostream& operator<<(std::ostream& os, objects::cuboid const& c);

}  // namespace raytrace