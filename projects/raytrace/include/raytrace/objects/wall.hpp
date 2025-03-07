#pragma once

#include <geometry/geometry.hpp>
#include <linalg/linalg.hpp>

#include "raytrace/objects/plane.hpp"

namespace raytrace {
namespace objects {
/// A pair of opposing planes with a thickness
class wall : public object {
public:
    /// Constructs a sphere at a point of a given radius.
    wall(point const& center, vector const& normal, precision thickness);
    virtual ~wall() = default;

    /// @copydoc raytrace::object::intersect
    // intersection intersect(ray const& world_ray) const override;
    /// @copydoc raytrace::object::collisions_along
    hits collisions_along(ray const& object_ray) const override;
    /// @copydoc raytrace::object::map
    image::point map(point const& object_surface_point) const override;
    /// @copydoc basal::printable::print
    void print(char const str[]) const override;
    /// @copydoc raytrace::object::is_surface_point
    bool is_surface_point(point const& world_point) const override;
    /// @copydoc raytrace::object::get_object_extent
    precision get_object_extent(void) const override;
    /// @copydoc raytrace::object::is_outside
    bool is_outside(point const& world_point) const override;
    /// @copydoc raytrace::object::is_along_infinite_extent
    bool is_along_infinite_extent(ray const& world_ray) const override;

protected:
    vector normal_(point const& object_surface_point) const override;

    raytrace::objects::plane m_front_;
    raytrace::objects::plane m_back_;
};
}  // namespace objects
}  // namespace raytrace