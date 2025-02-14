#pragma once

#include <array>
#include <geometry/geometry.hpp>
#include <linalg/linalg.hpp>

#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/// A cylinder with an outward facing normal
class cylinder : public object {
public:
    /// Constructs a cylinder which is up and down in Z.
    /// The point C is in the center of the cylinder between the top and bottom.
    cylinder(point const& C, precision half_height, precision radius);
    /// Constructs a cylinder in which the base and apex are defined by points
    /// The height and rotation are defined by the point orientations.
    cylinder(point const& base, point const& apex, precision radius);

    virtual ~cylinder() = default;

    /// @copydoc raytrace::object::intersect
    // geometry::intersection intersect(ray const& world_ray) const override;
    /// @copydoc raytrace::object::collision_along
    hits collisions_along(ray const& object_ray) const override;
    /// @copydoc raytrace::object::map
    image::point map(point const& object_surface_point) const override;
    /// @copydoc basal::printable::print
    void print(char const str[]) const override;
    bool is_surface_point(point const& world_point) const override;
    precision get_object_extent(void) const override;
protected:
    vector normal_(point const& object_surface_point) const override;
private:
    precision m_half_height;
    precision m_radius;
};

}  // namespace objects
}  // namespace raytrace
