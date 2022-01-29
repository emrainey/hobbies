#pragma once
/**
 * @file
 * Definitions for the line object.
 * @copyright Copyright 2019 (C) Erik Rainey.
 */

#include <array>
#include <vector>
#include <utility>
#include <functional>
#include <initializer_list>

#include "geometry/types.hpp"
#include "geometry/vector.hpp"
#include "geometry/point.hpp"

namespace geometry {

namespace R3 {
/** A 3-dimensional line of the \ref point + \ref vector form */
class line : public basal::printable {
protected:
    /** Unit vector of the line */
    R3::vector m_udir;
    /** The point form which the vector is based */
    R3::point m_zero;
public:
    /** Default Constructor is not supported */
    line() = delete;
    /** Semi-ray construction */
    explicit line(const R3::vector &ov, const R3::point &op);
    /** Semi-ray construction */
    explicit line(const R3::point &op, const R3::vector &ov);
    /** Construction form points. b is considered the "zero" point. */
    explicit line(const R3::point &a, const R3::point &b);

    /** list initialization, first vector tuple, then point tuple. Specialized 3D constructor */
    explicit line(const std::vector<element_type> &list);
    /** Copy Constructor */
    line(const line &other);
    /** Move Constructor */
    line(line &&other);
    /** Copy Assign */
    line &operator=(const line &other);
    /** Move Assign */
    line &operator=(line &&other);
    /** Default Destructor */
    virtual ~line() = default;

    /** Find the point at some value t along the line.
     * Orientation and scale of t is determined by \ref direction().
     */
    R3::point solve(element_type t) const;

    /** Determines if a point is on the line and returns the t value if it is. */
    bool solve(const point &n, element_type &t) const;

    /** Normalizes the direction and returns a point at a distance along the line */
    R3::point distance_along(element_type t) const;

    /** Returns the distance of a point from the line */
    element_type distance(const R3::point &p) const;

    /** Returns the direction of the line from the zero point */
    const R3::vector& direction() const;

    /** Returns the zero point of the line */
    const R3::point& position() const;

    /** Returns the closest point on the line to the point provided */
    R3::point closest(const R3::point &p) const;

    virtual void print(const char[]) const;
};

/** Equality operator for lines */
bool operator==(const line &a, const line &b);
/** Inequality operator for lines */
bool operator!=(const line &a, const line &b);
/** Determines if two lines are parallel  */
bool parallel(const line &a, const line &b);
/** Determines if two lines are parallel */
inline bool operator||(const line &a, const line &b) {
    return parallel(a, b);
}
/** Determines if two lines are skew, ie. not intersected and not parallel */
bool skew(const R3::line &i, const R3::line &j);

}

}
