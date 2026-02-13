#pragma once
/// @file
/// Definitions for the plane object.
/// @copyright Copyright 2019 (C) Erik Rainey.

#include "geometry/line.hpp"
#include "geometry/point.hpp"
#include "geometry/types.hpp"
#include "geometry/vector.hpp"

namespace geometry {
/// The structure for expressing a plane as 4 coefficients.
/// These are related by the equation \code ax + by + cz + d = 0 \endcode
typedef struct coefficients_t {
    /// First coefficient
    precision a;
    /// Second coefficient
    precision b;
    /// Third coefficient
    precision c;
    /// Fourth coefficient
    precision d;
    /// Solves the value of the x intercept
    /// \returns precision
    precision x(void) const;
    /// Solves the value of the y intercept
    /// \returns precision
    precision y(void) const;
    /// Solves the value of the z intercept
    /// \returns precision
    precision z(void) const;
} coefficients_t;

/// A 3-dimensional plane
class plane : public basal::printable {
protected:
    /// Normal of the plane in normalized form for hessians representations
    R3::vector m_normal;
    /// The point at which the normal is placed
    R3::point m_center_point;
    /// The terms of the equation of a plane in 3d: ax+by+cz+d=0
    coefficients_t eq;
    /// The scaling factor between the equation variables and the normal (magnitude)
    precision m_magnitude;

public:
    /// no default constructor
    plane() = delete;

    /// Generic constructors which take a point and a normal
    plane(R3::vector const& on, R3::point const& p);

    /// Generic constructors which take a point and a normal
    plane(R3::point const& p, R3::vector const& on);

    /// Constructs a plane from 3 points.
    plane(R3::point const& A, R3::point const& B, R3::point const& C);

    plane(plane const& other);

    plane(plane&& other);

    virtual ~plane() = default;

    plane& operator=(plane const& other);

    plane& operator=(plane&& other);

    /// List initialization constructor
    explicit plane(std::vector<precision> const& list);

    /// Of the form ax + by + cz + d = 0
    explicit plane(precision a, precision b, precision c, precision d);

    /// The distance from some point to this plane. Negative values indicates
    /// that the normal is pointing away from the point.
    precision distance(R3::point const& a) const;

    /// Determines if the planes are parallel
    bool parallel(plane const& a) const;

    /// Determines if the planes are perpendicular
    bool perpendicular(plane const& a) const;

    /// Returns the unnormalized normal
    R3::vector unormal() const;

    /// Returns the center point of the plane
    R3::point center() const;

    /// Returns a const reference to the coefficients
    coefficients_t const& coefficient() const;

    virtual void print(std::ostream&, char const name[]) const;

    bool operator==(plane const& other) const;
    bool operator!=(plane const& other) const;

    bool contains(R3::point const& b) const;
    bool contains(R3::line const& l) const;

    /// Returns the dihedral angle
    precision angle(plane const& P) const;

    friend std::ostream& operator<<(std::ostream& os, plane const& p);
};

inline bool parallel(plane const& a, plane const& b) {
    return a.parallel(b);
}

namespace operators {
/// Determines if the planes are parallel
inline bool operator||(plane const& a, plane const& b) {
    return parallel(a, b);
}
}  // namespace operators

std::ostream& operator<<(std::ostream& os, plane const& p);

}  // namespace geometry
