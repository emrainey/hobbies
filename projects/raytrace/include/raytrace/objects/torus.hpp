#pragma once

#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/// The Classical Donut
class torus : public object {
public:
    /// Constructs a torus which is up and down in Z
    torus(point const& C, precision ring_radius, precision tube_radius);
    virtual ~torus() = default;

    // ┌─────────────────────────┐
    // │raytrace::objects::object│
    // └─────────────────────────┘
    hits collisions_along(ray const& object_ray) const override;
    image::point map(point const& object_surface_point) const override;
    bool is_surface_point(point const& world_point) const override;
    precision get_object_extent(void) const override;
    void print(std::ostream& os, char const str[]) const override;

protected:
    vector normal_(point const& object_surface_point) const override;

private:
    precision m_ring_radius;
    precision m_tube_radius;
};
}  // namespace objects
}  // namespace raytrace
