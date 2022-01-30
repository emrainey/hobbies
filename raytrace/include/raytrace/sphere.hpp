#pragma once

#include <linalg/linalg.hpp>
#include <geometry/geometry.hpp>
#include "raytrace/object.hpp"

namespace raytrace {

/** The Classical Roundy Boi */
class sphere : public geometry::R3::sphere, public object {
public:
    /**
     * Constructs a sphere at a point of a given radius.
     */
    sphere(const point& center, double radius);
    virtual ~sphere() = default;

    /** @copydoc geometry::sphere::surface */
    bool surface(const point &world_surface_point) const override;

    /** @copydoc raytrace::object::normal */
    vector normal(const point& world_surface_point) const override;
    /** @copydoc raytrace::object::intersect */
    //geometry::intersection intersect(const ray& world_ray) const override;
    /** @copydoc raytrace::object::collision_along */
    hits collisions_along(const ray& object_ray) const override;
    /** @copydoc raytrace::object::map */
    image::point map(const point& object_surface_point) const override;
    /** @copydoc basal::printable::print */
    void print(const char str[]) const override;
};

}