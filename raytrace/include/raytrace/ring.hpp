#pragma once

#include <array>

#include "raytrace/object.hpp"
#include "raytrace/plane.hpp"

namespace raytrace {
/** A plane bounded by an inner and outer radius */
class ring : public raytrace::plane {
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
private:
    element_type m_inner_radius2; ///< Squared Inner Radius
    element_type m_outer_radius2; ///< Squared Outer Radius
};

/** Returns the plane in which the three points we co-planar */
geometry::plane as_plane(const ring& tri);

}
