#pragma once

#include <linalg/linalg.hpp>
#include "geometry/geometry.hpp"
#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/** Constructs a 1 sided infinite plane */
class plane : public geometry::plane, public object {
public:
    /**
     * Constructs a plane from a point and a normal.
     */
    plane(const point& point, const vector& normal, element_type surface_scale);
    virtual ~plane() = default;

    /** @copydoc raytrace::object::normal */
    vector normal(const point& world_surface_point) const override;
    /** @copydoc raytrace::object::intersect */
    intersection intersect(const ray& world_ray) const override;
    /** @copydoc raytrace::object::collisions_along */
    hits collisions_along(const ray& object_ray) const override;
    /** @copydoc raytrace::object::map */
    image::point map(const point& object_surface_point) const override;
    /** @copydoc basal::printable::print */
    void print(const char str[]) const override;
protected:
    element_type m_surface_scale;
};
}
}