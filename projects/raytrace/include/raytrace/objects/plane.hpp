#pragma once

#include <linalg/linalg.hpp>

#include "geometry/geometry.hpp"
#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/// Constructs a 1 sided infinite plane
class plane
    : public geometry::plane
    , public object {
public:
    /// Constructs a plane from a point and a normal.
    plane(point const& point, vector const& normal);
    virtual ~plane() = default;

    // ┌─────────────────────────┐
    // │raytrace::objects::object│
    // └─────────────────────────┘
    hits collisions_along(ray const& object_ray) const override;
    image::point map(point const& object_surface_point) const override;
    void print(std::ostream& os, char const str[]) const override;
    bool is_surface_point(point const& world_point) const override;
    precision get_object_extent(void) const override;
    bool is_along_infinite_extent(ray const& ray) const override;

protected:
    vector normal_(point const& object_surface_point) const override;
};
}  // namespace objects
}  // namespace raytrace