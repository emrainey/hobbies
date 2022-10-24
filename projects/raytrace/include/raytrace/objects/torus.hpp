#pragma once

#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/** The Classical Donut */
class torus : public object {
public:
    /**
     * Constructs a torus which is up and down in Z
     */
    torus(const point& C, element_type ring_radius, element_type tube_radius);
    virtual ~torus() = default;

    /** @copydoc raytrace::object::normal */
    vector normal(const point& world_surface_point) const override;
    /** @copydoc raytrace::object::intersect */
    // geometry::intersection intersect(const ray& world_ray) const override;
    /** @copydoc raytrace::object::collision_along */
    hits collisions_along(const ray& object_ray) const override;
    /** @copydoc raytrace::object::map */
    image::point map(const point& object_surface_point) const override;
    /** @copydoc basal::printable::print */
    void print(const char str[]) const override;
    bool is_surface_point(const point& world_point) const override;

private:
    element_type m_ring_radius;
    element_type m_tube_radius;
};
}  // namespace objects
}  // namespace raytrace
