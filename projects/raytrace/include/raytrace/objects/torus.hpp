#pragma once

#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/// The Classical Donut
class torus : public object {
public:
    /// Constructs a torus which is up and down in Z
    torus(point const& C, precision ring_radius, precision tube_radius);
    virtual ~torus() = default;

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
    precision m_ring_radius;
    precision m_tube_radius;
};
}  // namespace objects
}  // namespace raytrace
