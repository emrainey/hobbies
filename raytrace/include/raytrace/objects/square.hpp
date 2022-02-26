#pragma once

#include <array>
#include <linalg/linalg.hpp>
#include "raytrace/objects/plane.hpp"
namespace raytrace {
namespace objects {
/** A square limit on a plane */
class square : public plane {
public:
    /**
     * Constructs a square from two half width/heights and a normal
     */
    square(const point& C,  const raytrace::vector& N, element_type half_height, element_type half_width);
    virtual ~square() = default;

    /** @copydoc raytrace::object::collisions_along */
    hits collisions_along(const ray& object_ray) const override;

    /** @copydoc basal::printable::print */
    void print(const char str[]) const override;

    /** @copydoc raytrace::object::map */
    image::point map(const point& object_surface_point) const override;
private:
    std::array<raytrace::point, 2> m_points;
};
}
/** Returns the plane in which the three points we co-planar */
geometry::plane as_plane(const objects::square& sq);

}
