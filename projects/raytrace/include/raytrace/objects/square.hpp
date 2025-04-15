#pragma once

#include <array>
#include <linalg/linalg.hpp>

#include "raytrace/objects/plane.hpp"
namespace raytrace {
namespace objects {
/// A square limit on a plane
class square : public plane {
public:
    /// Constructs a square in the default orientation
    /// Since it is a square, the sides will be the same.
    square(precision side);

    /// Constructs a square from a point, a rotation and the length of a side
    /// Since it is a square, the sides will be the same.
    square(point const& C, matrix const& rotation, precision side);

    virtual ~square() = default;

    // ┌─────────────────────────┐
    // │raytrace::objects::object│
    // └─────────────────────────┘
    hits collisions_along(ray const& object_ray) const override;
    image::point map(point const& object_surface_point) const override;
    bool is_surface_point(point const& world_point) const override;
    precision get_object_extent(void) const override;
    void print(std::ostream& os, char const str[]) const override;

private:
    image::point min_;
    image::point max_;
};
}  // namespace objects
/// Returns the plane in which the three points we co-planar
geometry::plane as_plane(objects::square const& sq);

}  // namespace raytrace
