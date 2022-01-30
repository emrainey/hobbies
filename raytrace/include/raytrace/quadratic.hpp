#pragma once

#include <iostream>
#include <vector>

#include <basal/printable.hpp>
#include <linalg/linalg.hpp>
#include <geometry/geometry.hpp>

#include "raytrace/object.hpp"

namespace raytrace {

/** A user defined object which uses a 4x4 matrix to solve the surface normal and intersection interface
 * using a quadratic solver. Can be used to make spheres, elliptoids, hyperboloids (saddles), etc.
 */
class quadratic : public object {
public:
    /**
     * Constructs a quadratic at a point of a given set of enough doubles to form a 4x4 matrix
     */
    quadratic(const point& center, raytrace::matrix& C);

    virtual ~quadratic() = default;

    /** @copydoc raytrace::object::normal */
    vector normal(const point& world_surface_point) const override;
    /** @copydoc raytrace::object::intersect */
    //geometry::intersection intersect(const ray& world_ray) const override;
    /** @copydoc raytrace::object::collisions_along */
    hits collisions_along(const ray& object_ray) const override;
    /** @copydoc raytrace::object::map */
    image::point map(const point& object_surface_point) const override;
    /** @copydoc basal::printable::print */
    void print(const char str[]) const override;
protected:
    /** Print in stream for cuboid */
    friend std::ostream& operator<<(std::ostream& os, const quadratic& c);

    // The Coefficient Matrix form of Q from Graphic Gems Vol 3 for Quadratic Surfaces.
    raytrace::matrix m_coefficients;
};


}