#pragma once

#include <array>

#include "raytrace/objects/plane.hpp"

namespace raytrace {
namespace objects {
/// A plane bounded by an inner and outer radius
class ring : public raytrace::objects::plane {
public:
    /// Constructs a ring
    ring(point const& C, vector const& N, precision inner, precision outer);
    virtual ~ring() = default;

    /// @copydoc raytrace::object::collisions_along
    hits collisions_along(ray const& object_ray) const override;
    /// @copydoc basal::printable::print
    void print(char const str[]) const override;
    bool is_surface_point(point const& world_point) const override;
    precision get_object_extent(void) const override;

private:
    precision m_inner_radius2;  ///< Squared Inner Radius
    precision m_outer_radius2;  ///< Squared Outer Radius
};
}  // namespace objects
/// Returns the plane in which the three points we co-planar
geometry::plane as_plane(objects::ring const& tri);

}  // namespace raytrace
