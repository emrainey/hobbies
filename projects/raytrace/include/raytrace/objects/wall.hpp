#pragma once

#include <geometry/geometry.hpp>
#include <linalg/linalg.hpp>

#include "raytrace/objects/plane.hpp"

namespace raytrace {
namespace objects {
/// A pair of opposing planes with a thickness
class wall
    : public object {
public:
    /// Constructs a sphere at a point of a given radius.
    wall(const point& center, const vector& normal, precision thickness);
    virtual ~wall() = default;

    /// @copydoc raytrace::object::normal
    vector normal(const point& world_surface_point) const override;
    /// @copydoc raytrace::object::intersect
    // intersection intersect(const ray& world_ray) const override;
    /// @copydoc raytrace::object::collisions_along
    hits collisions_along(const ray& object_ray) const override;
    /// @copydoc raytrace::object::map
    image::point map(const point& object_surface_point) const override;
    /// @copydoc basal::printable::print
    void print(const char str[]) const override;
    /// @copydoc raytrace::object::is_surface_point
    bool is_surface_point(const point& world_point) const override;
    /// @copydoc raytrace::object::get_object_extent
    precision get_object_extent(void) const override;
    bool is_outside(point const& world_point) const override;
protected:
    raytrace::objects::plane m_front_;
    raytrace::objects::plane m_back_;
};
}  // namespace objects
}  // namespace raytrace