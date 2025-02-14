#pragma once
/// @file
/// Definitions for the sphere object.
/// @copyright Copyright 2019 (C) Erik Rainey.

#include <geometry/line.hpp>
#include <geometry/point.hpp>
#include <geometry/types.hpp>

namespace geometry {

namespace R3 {
/// Follows the form of the equation (in 3D space):
/// (x - x0)^2 + (y - y0)^2 + (z - z0)^2 = r^2
/// with the notion that the center is at the origin
class sphere {
protected:
    /// The radius of the sphere
    precision m_radius;

public:
    precision const &radius = m_radius;

    sphere() = delete;
    /// Generic Constructor
    sphere(precision r);
    /// Copy Constructor
    sphere(sphere const& s);
    /// Move Constructor
    sphere(sphere &&s);
    /// D-tor
    virtual ~sphere() = default;
    /// Copy Assign
    sphere &operator=(sphere const& s);
    /// Move Assign
    sphere &operator=(sphere &&s);

    /// Indicates if the point is contained in or on the sphere
    virtual bool contains(R3::point const& object_point) const;

    /// Returns a surface normal at a surface point (in world space)
    virtual R3::vector normal_at(R3::point const& object_point) const;

    /// Indicates if a object_point is on the surface of the sphere
    virtual bool on_surface(R3::point const& object_point) const;

    /// Returns a point in polar representation of the object point
    R3::point cart_to_polar(R3::point const& object_point) const;
};

}  // namespace R3

}  // namespace geometry
