#pragma once

#include <raytrace/types.hpp>

namespace raytrace {


/// An Bounds aligned bounding box for computing the BSP
struct Bounds {
    point min;  //!< The lower inclusive set of values.
    point max;  //!< The higher inclusive set of values.

    point center() const;

    /// Determines if a point is within the bounds
    bool contained(point const& p) const;

    /// Determines if a ray intersects the bounds
    bool intersects(ray const& r) const ;

    /// Determines if another bounds intersects with this bounds.
    bool intersects(Bounds const& b) const;

    /// Determines if the bounds are infinite
    bool is_infinite() const;

    /// Grows the bounds to include another bounds
    void grow(Bounds const& b);
};

} // namespace raytrace
