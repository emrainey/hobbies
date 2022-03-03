#pragma once

#include <array>
#include <linalg/linalg.hpp>

#include "raytrace/objects/plane.hpp"

namespace raytrace {
namespace objects {
/** A three point polygon which uses the convex "inner" rules to find the intersection and not the plane intersection
 * rules */
class triangle : public plane {
public:
    /**
     * Constructs a triangle from three points.
     * Point B is considered the middle point for computation of the normal.
     */
    triangle(const point& A, const point& B, const point& C);
    virtual ~triangle() = default;

    /** @copydoc raytrace::object::intersect */
    geometry::intersection intersect(const ray& world_ray) const override;

    /** @copydoc basal::printable::print */
    void print(const char str[]) const override;

    /** Returns a read-only reference to an array of 3 points */
    const std::array<point, 3>& points() const;

private:
    std::array<point, 3> m_points;
    /** The squared maximum radius */
    element_type m_radius2;
};
}  // namespace objects
/** Returns the plane in which the three points we co-planar */
geometry::plane as_plane(const objects::triangle& tri);

}  // namespace raytrace
