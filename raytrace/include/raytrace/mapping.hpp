#pragma once

#include <cstdint>
#include <cmath>

#include "raytrace/types.hpp"

namespace raytrace {

namespace mapping {
    /** Defines function pointer to a R3=>R2 reducing mapping function */
    using reducer = std::function<geometry::R2::point(const geometry::R3::point&)>;

    /** Normalizes the R3 point to a unit sphere while converting to R2 coordinates on the surface of the sphere */
    geometry::R2::point spherical(const geometry::R3::point& p);

    /**
     * Maps on to an ideal cylinderical fields along the Z axis
     * @param scale The Z height scale size
     * @note Use std::bind(scale, _1) to fix a certain scale height when creating a mapper
     */
    geometry::R2::point cylindrical(element_type scale, const geometry::R3::point& p);

    // FIXME create a toriodal mapping from a single larger radius value
    geometry::R2::point toroidal(element_type r1, const geometry::R3::point& p);

    // FIXME create a planar polar mapping given a Normal ray and the "X" unit point
    geometry::R2::point planar_polar(const raytrace::ray& N, const raytrace::point& X, const geometry::R3::point& p);

    /** Defines a method which can map a value within a unit range given a integer ratio, to a 3D surface */
    using expander = std::function<geometry::R3::point(const size_t numerator, const size_t denominator)>;

    /** Uses the golden ratio to map a range (from an integer ratio) to a point on a unit sphere
     * @param numerator must be less than or equal to the denominator
     * @param denominator must be > 0.
     */
    geometry::R3::point golden_ratio_mapper(const size_t numerator, const size_t denominator);
}

}