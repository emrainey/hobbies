#pragma once

#include <array>
#include <linalg/linalg.hpp>

#include "raytrace/objects/plane.hpp"

namespace raytrace {
namespace objects {
/// A three point polygon which uses the plane intersection rules then the convex "inner" rules to find the intersection
template <size_t N>
class polygon : public object {
public:
    /// Constructs a polygon from some number of points (where N >=3)
    /// The second point is considered the middle point for computation of the normal.
    /// The origin of the polygon is considered to be the the R3::origin at first but will be computed as the centroid
    /// of the given points. All the points will be adjusted so that the center is at the R3::origin and the
    /// entire polygon has been translated by the centroid
    /// @throw basal::exception if the number of points is not equal to N
    polygon(std::initializer_list<R3::point> points);
    /// Constructs a polygon from some number of points (where N >=3)
    /// The second point is considered the middle point for computation of the normal.
    /// The origin of the polygon is considered to be the the R3::origin at first but will be computed as the centroid
    /// of the given points. All the points will be adjusted so that the center is at the R3::origin and the
    /// entire polygon has been translated by the centroid
    polygon(std::array<R3::point, N> const& points);
    /// Destructors
    virtual ~polygon() = default;

    // ┌─────────────────────────┐
    // │raytrace::objects::object│
    // └─────────────────────────┘
    hits collisions_along(ray const& object_ray) const override;
    bool is_surface_point(point const& world_point) const override;
    precision get_object_extent(void) const override;
    void print(std::ostream& os, char const str[]) const override;
    image::point map(point const& object_surface_point) const override;

    /// Determines if a point on the plane is contained within the array of three points
    bool is_contained(point const& object_point) const;

    /// Returns a read-only reference to an array of N points
    const std::array<R3::point, N>& points() const;

protected:
    std::array<R3::point, N> m_points;
    R3::vector normal_(point const& object_surface_point) const override;
    /// The squared maximum radius from object center.
    precision m_radius2;
};

using triangle = polygon<3U>;

/// Factory Template Method to make the points of a polygon of N sides
/// @param radius The radius of the polygon
/// @param center The center of the polygon
template <size_t N>
std::array<raytrace::point, N> make_polygon_points(precision radius, R3::point const& center) {
    static_assert(N >= 3, "A polygon must have at least three sides.");
    std::array<raytrace::point, N> points;
    R3::vector const offset = center - R3::origin;
    for (size_t i = 0; i < N; ++i) {
        precision angle = -iso::tau * static_cast<precision>(i) / static_cast<precision>(N);
        points[i] = raytrace::point{radius * cos(angle), radius * sin(angle), 0} + offset;
    }
    return points;
}

}  // namespace objects

}  // namespace raytrace
