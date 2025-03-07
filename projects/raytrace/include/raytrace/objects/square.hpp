#pragma once

#include <array>
#include <linalg/linalg.hpp>

#include "raytrace/objects/plane.hpp"
namespace raytrace {
namespace objects {
/// A square limit on a plane
class square : public plane {
public:
    /// Constructs a square from two half width/heights and a normal
    square(point const& C, raytrace::vector const& N, precision half_height, precision half_width);
    virtual ~square() = default;

    /// @copydoc raytrace::object::collisions_along
    hits collisions_along(ray const& object_ray) const override;

    /// @copydoc basal::printable::print
    void print(char const str[]) const override;

    /// @copydoc raytrace::object::map
    image::point map(point const& object_surface_point) const override;

    bool is_surface_point(point const& world_point) const override;
    precision get_object_extent(void) const override;

private:
    std::array<raytrace::point, 2> m_points;
};
}  // namespace objects
/// Returns the plane in which the three points we co-planar
geometry::plane as_plane(objects::square const& sq);

}  // namespace raytrace
