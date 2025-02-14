#pragma once

#include <geometry/geometry.hpp>
#include <linalg/linalg.hpp>

#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/// The simple pyramid which follows z = height - |x| - |y|
class pyramid : public object {
public:
    /// Constructs a pyramid at a point of a given height. The slope is fixed
    pyramid(point const& base, precision height);
    virtual ~pyramid() = default;

    /// @copydoc raytrace::object::collision_along
    hits collisions_along(ray const& object_ray) const override;
    /// @copydoc raytrace::object::map
    image::point map(point const& object_surface_point) const override;
    /// @copydoc basal::printable::print
    void print(char const str[]) const override;
    bool is_surface_point(point const& world_point) const override;
    precision get_object_extent(void) const override;

protected:
    vector normal_(point const& object_surface_point) const override;

    precision m_height;  //!< The height of the pyramid above 0 in Z.
};
}  // namespace objects
}  // namespace raytrace