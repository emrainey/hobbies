#pragma once

#include <raytrace/types.hpp>

namespace raytrace {

/// An Bounds aligned bounding box for computing the BSP
struct Bounds {
    point min;                                 //!< The lower inclusive set of values.
    point max;                                 //!< The higher exclusive set of values.
    static constexpr size_t NumSubBounds{8U};  //!< The number of sub-bounds (octants) for splitting the bounds.

    /// Default Constructor
    Bounds();

    /// Constructs a bounds from two points
    Bounds(point const& min, point const& max);

    /// Computes the center of the bounds in world space
    /// @note If the min/max are both infinite, then the center is the origin, otherwise at least 1 dimension is
    /// infinite it is undefined.
    point center() const;

    /// Determines if a point is within the bounds
    bool contained(point const& p) const;

    /// Determines if a ray intersects the bounds
    /// @param r The ray to check for intersection
    /// @return true if the ray intersects the bounds, false otherwise
    /// @note Does NOT return the intersection location
    bool intersects(ray const& r) const;

    /// Determines if another bounds intersects with this bounds.
    bool intersects(Bounds const& b) const;

    /// Determines if the bounds are infinite
    bool is_infinite() const;

    /// Grows the bounds to include another bounds
    void grow(Bounds const& b);

    /// @brief Splits the bounds into 8 sub-bounds (octants)
    /// @return A vector of the 8 octants that make up the bounds
    std::vector<Bounds> split() const;
};

std::ostream& operator<<(std::ostream& os, Bounds const& b);

}  // namespace raytrace
