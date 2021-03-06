#pragma once

#include <array>

#include "raytrace/objects/plane.hpp"

namespace raytrace {
namespace objects {
/** A plane bounded by an inner and outer radius */
class ring : public raytrace::objects::plane {
public:
    /**
     * Constructs a ring
     */
    ring(const point& C, const vector& N, element_type inner, element_type outer);
    virtual ~ring() = default;

    /** @copydoc raytrace::object::collisions_along */
    hits collisions_along(const ray& object_ray) const override;
    /** @copydoc basal::printable::print */
    void print(const char str[]) const override;
    bool is_surface_point(const point& world_point) const override;

private:
    element_type m_inner_radius2;  ///< Squared Inner Radius
    element_type m_outer_radius2;  ///< Squared Outer Radius
};
}  // namespace objects
/** Returns the plane in which the three points we co-planar */
geometry::plane as_plane(const objects::ring& tri);

}  // namespace raytrace
