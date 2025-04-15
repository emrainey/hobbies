#pragma once

#include <cmath>
#include <cstdint>

#include "raytrace/types.hpp"

namespace raytrace {

namespace mapping {
/// Defines functor to a R3=>R2 reducing mapping function
using reducer = std::function<geometry::R2::point(geometry::R3::point const&)>;

/// Normalizes the R3 point to a unit sphere while converting to R2 coordinates on the surface of the sphere
/// @note Values are in unit TURNS not RADS
geometry::R2::point spherical(geometry::R3::point const& p);

/// Normalizes the R3 vector to a unit sphere while converting to R2 coordinates on the surface of the sphere.
/// @note Values are in unit TURNS not RADS
geometry::R2::point spherical(geometry::R3::vector const& q);

/// Maps on to an ideal cylindrical fields along the Z axis
/// @param scale The Z height scale size
/// @note Use std::bind(scale, _1) to fix a certain scale height when creating a mapper
geometry::R2::point cylindrical(precision scale, geometry::R3::point const& p);

/// Maps on to an ideal toroidal shape of a given radius. The toriod is surrounding the Z axis.
/// @param r1 The given inner radius of the toroid
/// @param p The point in 3 space to map
/// @return The surface point in 2 space (phi, theta)
geometry::R2::point toroidal(precision r1, geometry::R3::point const& p);

/// Maps a point on to a plane in polar coordinates
/// @param N The normal vector to the plane
/// @param X The x basis in the plane.
/// @param C The center unit point on the plane
/// @param p The point in 3 space to map
/// @return The surface point in 2 space (theta, radius)
geometry::R2::point planar_polar(raytrace::vector const& N, raytrace::vector const& X, raytrace::point const& C,
                                 geometry::R3::point const& p);

/// Defines a functor which can map a value within a unit range given a integer ratio, to a 3D surface
using expander = std::function<geometry::R3::point(size_t const numerator, size_t const denominator)>;

/// Uses the golden ratio to map a range (from an integer ratio) to a point on a unit sphere
/// @param numerator must be less than or equal to the denominator
/// @param denominator must be > 0.
/// @return A point on the unit sphere
geometry::R3::point golden_ratio_mapper(size_t const numerator, size_t const denominator);
}  // namespace mapping

}  // namespace raytrace