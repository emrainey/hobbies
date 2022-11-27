#pragma once

#include <array>
#include <geometry/geometry.hpp>
#include <linalg/linalg.hpp>

#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/** A cylinder with an outward facing normal */
class cylinder : public object {
public:
    /**
     * Constructs a cylinder which is up and down in Z.
     * The point C is in the center of the cylinder between the top and bottom.
     */
    cylinder(const point& C, element_type half_height, element_type radius);
    /**
     * Constructs a cylinder in which the base and apex are defined by points
     * The height and rotation are defined by the point orientations.
     */
    cylinder(const point& base, const point& apex, element_type radius);

    virtual ~cylinder() = default;

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
    element_type get_object_extant(void) const override;

private:
    element_type m_half_height;
    element_type m_radius;
};

}  // namespace objects
}  // namespace raytrace
