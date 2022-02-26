#pragma once

#include <linalg/linalg.hpp>
#include <geometry/geometry.hpp>
#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/** The simple pyramid which follows z = height - |x| - |y| */
class pyramid : public object {
public:
    /**
     * Constructs a pyramid at a point of a given height. The slope is fixed
     */
    pyramid(const point& base, double height);
    virtual ~pyramid() = default;

    /** @copydoc raytrace::object::normal */
    vector normal(const point& world_surface_point) const override;
    /** @copydoc raytrace::object::collision_along */
    hits collisions_along(const ray& object_ray) const override;
    /** @copydoc raytrace::object::map */
    image::point map(const point& object_surface_point) const override;
    /** @copydoc basal::printable::print */
    void print(const char str[]) const override;
protected:
    double m_height; //!< The height of the pyramid above 0 in Z.
};
}
}