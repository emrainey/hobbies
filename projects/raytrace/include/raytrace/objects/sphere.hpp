#pragma once

#include <geometry/geometry.hpp>
#include <linalg/linalg.hpp>

#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/// The Classical Roundy-Boi
class sphere
    : public geometry::R3::sphere
    , public object {
public:
    /// Constructs a sphere at a point of a given radius.
    sphere(point const& center, precision radius);
    virtual ~sphere() = default;

    // ┌─────────────────────────┐
    // │raytrace::objects::object│
    // └─────────────────────────┘
    bool is_surface_point(point const& world_surface_point) const override;
    hits collisions_along(ray const& object_ray) const override;
    image::point map(point const& object_surface_point) const override;
    void print(std::ostream& os, char const str[]) const override;
    precision get_object_extent(void) const override;

protected:
    vector normal_(point const& object_surface_point) const override;
};
}  // namespace objects
}  // namespace raytrace