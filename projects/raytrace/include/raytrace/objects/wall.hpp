#pragma once

#include <geometry/geometry.hpp>
#include <linalg/linalg.hpp>

#include "raytrace/objects/plane.hpp"

namespace raytrace {
namespace objects {
/// A pair of opposing planes with a thickness
class wall : public object {
public:
    /// Constructs a sphere at a point of a given radius.
    wall(point const& center, vector const& normal, precision thickness);
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