#pragma once

#include <geometry/geometry.hpp>
#include <linalg/linalg.hpp>

#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/// A Cone object with an outward facing normal
class cone : public object {
public:
    /// Constructs a cone which is up and down in Z. The point C is on the apex pinch.
    /// This cone is infinite and closed. The angle must be < pi/2
    cone(point const& C, iso::radians angle);
    /// Constructs a cone which is up and down in Z. The point C is on the base in the center of the ring.
    /// The cone with a height is not closed and can't be used in intersections.
    cone(point const& C, precision bottom_radius, precision height);
    virtual ~cone() = default;

    /// @copydoc raytrace::object::intersect
    /// geometry::intersection intersect(ray const& world_ray) const override;
    /// @copydoc raytrace::object::collision_along
    hits collisions_along(ray const& object_ray) const override;
    /// @copydoc raytrace::object::map
    image::point map(point const& object_surface_point) const override;
    /// @copydoc basal::printable::print
    void print(char const str[]) const override;
    bool is_surface_point(raytrace::point const& world_point) const override;
    precision get_object_extent(void) const override;

protected:
    vector normal_(point const& object_surface_point) const override;

private:
    precision m_bottom_radius;
    precision m_height;
    iso::radians m_angle;
};

}  // namespace objects
}  // namespace raytrace
