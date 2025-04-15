#pragma once

#include <geometry/geometry.hpp>
#include <linalg/linalg.hpp>

#include "raytrace/objects/plane.hpp"

namespace raytrace {
namespace objects {
/// A pair of opposing planes with a thickness
class wall : public object {
public:
    /// Constructs a wall (a pair of planes facing opposite directions) at the default location of a given thickness.
    wall(precision thickness);

    /// Constructs a wall (a pair of planes facing opposite directions) at given location and orientation of a given
    /// thickness.
    wall(point const& center, matrix const& rotation, precision thickness);
    /// Destructor
    virtual ~wall() = default;

    // ┌─────────────────────────┐
    // │raytrace::objects::object│
    // └─────────────────────────┘
    hits collisions_along(ray const& object_ray) const override;
    image::point map(point const& object_surface_point) const override;
    bool is_surface_point(point const& world_point) const override;
    precision get_object_extent(void) const override;
    bool is_outside(point const& world_point) const override;
    bool is_along_infinite_extent(ray const& world_ray) const override;
    void print(std::ostream& os, char const str[]) const override;

protected:
    vector normal_(point const& object_surface_point) const override;

    raytrace::objects::plane m_front_;
    raytrace::objects::plane m_back_;
};
}  // namespace objects
}  // namespace raytrace