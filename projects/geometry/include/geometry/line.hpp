#pragma once
/// @file
/// Definitions for the line object.
/// @copyright Copyright 2019 (C) Erik Rainey.clude <array>
#include <functional>
#include <initializer_list>
#include <utility>
#include <vector>

#include "geometry/point.hpp"
#include "geometry/types.hpp"
#include "geometry/vector.hpp"

namespace geometry {

namespace R3 {
/// A 3-dimensional line of the \ref point + \ref vector form
class line : public basal::printable {
protected:
    /// Unit vector of the line
    R3::vector m_udir;
    /// The point form which the vector is based
    R3::point m_zero;

public:
    /// Default Constructor is not supported
    line() = delete;
    /// Semi-ray construction
    explicit line(R3::vector const& ov, R3::point const& op);
    /// Semi-ray construction
    explicit line(R3::point const& op, R3::vector const& ov);
    /// Construction form points. b is considered the "zero" point.
    explicit line(R3::point const& a, R3::point const& b);

    /// list initialization, first vector tuple, then point tuple. Specialized 3D constructor
    explicit line(std::vector<precision> const& list);
    /// Copy Constructor
    line(line const& other);
    /// Move Constructor
    line(line&& other);
    /// Copy Assign
    line& operator=(line const& other);
    /// Move Assign
    line& operator=(line&& other);
    /// Default Destructor
    virtual ~line() = default;

    /// Find the point at some value t along the line.
    /// Orientation and scale of t is determined by \ref direction().
    ///
    R3::point solve(precision t) const;

    /// Determines if a point is on the line and returns the t value if it is.
    bool solve(point const& n, precision& t) const;

    /// Normalizes the direction and returns a point at a distance along the line
    R3::point distance_along(precision t) const;

    /// Returns the distance of a point from the line
    precision distance(R3::point const& p) const;

    /// Returns the direction of the line from the zero point
    R3::vector const& direction() const;

    /// Returns the zero point of the line
    R3::point const& position() const;

    /// Returns the closest point on the line to the point provided
    R3::point closest(R3::point const& p) const;

    virtual void print(char const[]) const;
};

/// Equality operator for lines
bool operator==(line const& a, line const& b);
/// Inequality operator for lines
bool operator!=(line const& a, line const& b);
/// Determines if two lines are parallel
bool parallel(line const& a, line const& b);
/// Determines if two lines are parallel
inline bool operator||(line const& a, line const& b) {
    return parallel(a, b);
}
/// Determines if two lines are skew, ie. not intersected and not parallel
bool skew(R3::line const& i, R3::line const& j);

}  // namespace R3

}  // namespace geometry
