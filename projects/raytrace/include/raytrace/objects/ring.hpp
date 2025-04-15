#pragma once

#include <array>

#include "raytrace/objects/plane.hpp"

namespace raytrace {
namespace objects {
/// A plane bounded by an inner and outer radius
class ring : public raytrace::objects::plane {
public:
    /// Constructs a ring at the default location
    /// The ring is idealized as the XY plane with the normal as +Z.
    /// @param inner The inner radius of the ring in object space
    /// @param outer The outer radius of the ring in object space
    ring(precision inner, precision outer);

    /// Constructs a ring
    /// The ring is idealized as the XY plane with the normal as +Z.
    /// @param C The center of the ring in world space
    /// @param R The rotation matrix of the ring in world space
    /// @param inner The inner radius of the ring in object space
    /// @param outer The outer radius of the ring in object space
    ring(point const& C, matrix const& R, precision inner, precision outer);

    /// Destructor
    virtual ~ring() = default;

    // ┌─────────────────────────┐
    // │raytrace::objects::object│
    // └─────────────────────────┘
    hits collisions_along(ray const& object_ray) const override;
    void print(std::ostream& os, char const str[]) const override;
    bool is_surface_point(point const& world_point) const override;
    precision get_object_extent(void) const override;

private:
    precision m_inner_radius2;  ///< Squared Inner Radius
    precision m_outer_radius2;  ///< Squared Outer Radius
};
}  // namespace objects
}  // namespace raytrace
