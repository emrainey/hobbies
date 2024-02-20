
#pragma once
/// @file
/// Definitions for the intersection of two objects.
/// @copyright Copyright 2019 (C) Erik Rainey.

#include <iostream>
#include <variant>

#include "geometry/geometry.hpp"
#include "linalg/linalg.hpp"

namespace geometry {

/// The types of intersections available
enum class IntersectionType : int
{
    None = 0, ///< No intersection was possible
    Point,    ///< The objects intersected as a single \ref point
    Points,   ///< The object intersected as a set_of_points
    Line,     ///< The objects intersected as a \ref line
    Plane     ///< The objects intersected as a \ref plane
};

/// An intersection can be a set of points on a simple solid (think of a line/torus)
using set_of_points = std::vector<R3::point>;

/// The intersection itself is an object variant.
using intersection = std::variant<std::monostate, R3::point, set_of_points, R3::line, plane>;

/// Returns the type of an intersection
IntersectionType get_type(const intersection &var);

/// Converts an intersection type enum to a string
/// \return a pointer to a constant "c" string.
char const *get_type_as_str(IntersectionType obj);

inline const R3::point &as_point(const intersection &intersector) {
    return std::get<R3::point>(intersector);
}

inline const set_of_points &as_points(const intersection &intersector) {
    return std::get<set_of_points>(intersector);
}

inline const R3::line &as_line(const intersection &intersector) {
    return std::get<R3::line>(intersector);
}

inline const plane &as_plane(const intersection &intersector) {
    return std::get<plane>(intersector);
}

/// Allow printing of an intersection
std::ostream &operator<<(std::ostream &os, const intersection &intersector);

/// Determines if a point is on a line.
bool intersects(const point &p, const R3::line &l);

/// Determines if a point is on a line.
inline bool intersects(const R3::line &l, const R3::point &p) {
    return intersects(p, l);
}

/// Determines if two lines intersect.
/// \returns Either a NONE, or a POINT, or a LINE.
///
intersection intersects(const R3::line &a, const R3::line &b);

/// Determines if a line and a plane intersect.
/// \returns Either a NONE, or a POINT, or a LINE.
///
intersection intersects(const R3::line &l, const plane &P);

/// Determines if a line and a plane intersect.
/// \returns Either a NONE, or a POINT, or a LINE.
///
inline intersection intersects(const plane &P, const R3::line &l) {
    return intersects(l, P);
}

/// Determines if the two planes are parallel (NONE) or the same
/// plane (PLANE) or a line (LINE).
intersection intersects(const plane &A, const plane &B);

/// Determines if the line and the sphere intersect
/// \returns Either a NONE, a POINT or POINTS.
///
intersection intersects(const R3::sphere &S, const R3::line &l) noexcept(false);

/// Determines if the line and the sphere intersect.
/// \returns Either a NONE, a POINT or POINTS.
///
inline intersection intersects(const R3::line &l, const R3::sphere &S) noexcept(false) {
    return intersects(S, l);
}
}  // namespace geometry
