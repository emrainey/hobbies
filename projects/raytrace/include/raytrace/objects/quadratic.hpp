#pragma once

#include <basal/printable.hpp>
#include <geometry/geometry.hpp>
#include <iostream>
#include <linalg/linalg.hpp>
#include <vector>

#include "raytrace/objects/object.hpp"

namespace raytrace {
namespace objects {
/// A user defined object which uses a 4x4 matrix to solve the surface normal and intersection interface
/// using a quadratic solver. Can be used to make spheres, ellipsoids, hyperboloids (saddles), etc.
///
class quadratic : public object {
public:
    /// Constructs a quadratic at a point of a given set of enough precisions to form a 4x4 matrix
    quadratic(point const& center, raytrace::matrix& C);

    virtual ~quadratic() = default;

    // ┌─────────────────────────┐
    // │raytrace::objects::object│
    // └─────────────────────────┘
    hits collisions_along(ray const& object_ray) const override;
    image::point map(point const& object_surface_point) const override;
    void print(std::ostream& os, char const str[]) const override;
    bool is_surface_point(point const& world_point) const override;
    precision get_object_extent(void) const override;

protected:
    vector normal_(point const& object_surface_point) const override;
    /// Subclass constructor
    quadratic(point const& center);
    // The Coefficient Matrix form of Q from Graphic Gems Vol 3 for Quadratic Surfaces.
    raytrace::matrix m_coefficients;
};
}  // namespace objects
}  // namespace raytrace
