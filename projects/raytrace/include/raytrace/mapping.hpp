#pragma once

#include <cmath>
#include <cstdint>

#include "raytrace/types.hpp"

namespace raytrace {

namespace mapping {
/// Defines function pointer to a R3=>R2 reducing mapping function
using reducer = std::function<geometry::R2::point(geometry::R3::point const&)>;

/// Normalizes the R3 point to a unit sphere while converting to R2 coordinates on the surface of the sphere
/// @note Values are in unit TURNS not RADS
geometry::R2::point spherical(geometry::R3::point const& p);

/// Normalizes the R3 vector to a unit sphere while converting to R2 coordinates on the surface of the sphere.
/// @note Values are in unit TURNS not RADS
geometry::R2::point spherical(geometry::R3::vector const& q);

/// Maps on to an ideal cylinderical fields along the Z axis
/// @param scale The Z height scale size
/// @note Use std::bind(scale, _1) to fix a certain scale height when creating a mapper
geometry::R2::point cylindrical(precision scale, geometry::R3::point const& p);

// FIXME create a toriodal mapping from a single larger radius value
geometry::R2::point toroidal(precision r1, geometry::R3::point const& p);

// FIXME create a planar polar mapping given a Normal ray and the "X" unit point
geometry::R2::point planar_polar(raytrace::ray const& N, raytrace::point const& X, geometry::R3::point const& p);

/// Defines a method which can map a value within a unit range given a integer ratio, to a 3D surface
using expander = std::function<geometry::R3::point(size_t const numerator, size_t const denominator)>;

/// Uses the golden ratio to map a range (from an integer ratio) to a point on a unit sphere
/// @param numerator must be less than or equal to the denominator
/// @param denominator must be > 0.
/// @return A point on the unit sphere
geometry::R3::point golden_ratio_mapper(size_t const numerator, size_t const denominator);
}  // namespace mapping

}  // namespace raytrace