#pragma once

#include <array>
#include <linalg/linalg.hpp>

#include "raytrace/objects/plane.hpp"

namespace raytrace {
namespace objects {
/// A three point polygon which uses the convex "inner" rules to find the intersection
/// and not the plane intersection rules
class triangle : public plane {
public:
    /// Constructs a triangle from three points.
    /// Point B is considered the middle point for computation of the normal.
    triangle(point const& A, point const& B, point const& C);
    virtual ~triangle() = default;

    /// @copydoc raytrace::object::intersect
    hit intersect(ray const& world_ray) const override;

    /// @copydoc basal::printable::print
    void print(char const str[]) const override;

    bool is_surface_point(point const& world_point) const override;

    /// Determines if a point on the plane is contained within the array of three points
    bool is_contained(point const& object_point) const;

    /// Returns a read-only reference to an array of 3 points
    const std::array<point, raytrace::dimensions>& points() const;

    precision get_object_extent(void) const override;

protected:
    std::array<point, raytrace::dimensions> m_points;
    /// The squared maximum radius from object center.
    precision m_radius2;
};
}  // namespace objects

/// Returns the plane in which the three points are co-planar
geometry::plane as_plane(objects::triangle const& tri);

}  // namespace raytrace
