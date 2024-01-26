#pragma once
/**
 * @file
 * Definitions for the plane object.
 * @copyright Copyright 2019 (C) Erik Rainey.
 */

#include "geometry/line.hpp"
#include "geometry/point.hpp"
#include "geometry/types.hpp"
#include "geometry/vector.hpp"

namespace geometry {
/**
 * The structure for expressing a plane as 4 coefficients.
 * These are related by the equation \code ax + by + cz + d = 0 \endcode
 */
typedef struct coefficients_t {
    /** First coefficient */
    precision a;
    /** Second coefficient */
    precision b;
    /** Third coefficient */
    precision c;
    /** Fourth coefficient */
    precision d;
    /** Solves the value of x
     * \returns precision
     */
    precision x(void);
    /** Solves the value of y
     * \returns precision
     */
    precision y(void);
    /** Solves the value of z
     * \returns precision
     */
    precision z(void);
} coefficients_t;

/** A N-dimensional plane */
class plane : public basal::printable {
protected:
    /** Normal of the plane in normalized form for hessians representations */
    R3::vector m_normal;
    /** The point at which the normal is placed */
    R3::point m_center_point;
    /** The terms of the equation of a plane in 3d: ax+by+cz+d=0 */
    coefficients_t eq;
    /** The scaling factor between the equation variables and the normal (magnitude) */
    precision m_magnitude;

public:
    /** Constant reference to the internal \ref vector normal */
    const R3::vector &normal = m_normal;

    /** no default constructor */
    plane() = delete;

    /** Generic constructors which take a point and a normal */
    plane(const R3::vector &on, const R3::point &p);

    /** Generic constructors which take a point and a normal */
    plane(const R3::point &p, const R3::vector &on);

    /** Constructs a plane from 3 points. */
    plane(const R3::point &A, const R3::point &B, const R3::point &C);

    plane(const plane &other);

    plane(plane &&other);

    virtual ~plane() = default;

    plane &operator=(const plane &other);

    plane &operator=(plane &&other);

    /** List initialization constructor */
    explicit plane(const std::vector<precision> &list);

    /** Of the form ax + by + cz + d = 0 */
    explicit plane(precision a, precision b, precision c, precision d);

    /**
     * The distance from some point to this plane. Negative values indicates
     * that the normal is pointing away from the point.
     */
    precision distance(const R3::point &a) const;

    /** Determines if the planes are parallel */
    bool parallel(const plane &a) const;

    /** Determines if the planes are perpendicular */
    bool perpendicular(const plane &a) const;

    /** Returns the unnormalized vector */
    R3::vector unormal() const;

    /** Returns a const reference to the coefficients */
    const coefficients_t &coefficient() const;

    virtual void print(const char name[]) const;

    bool operator==(const plane &other) const;
    bool operator!=(const plane &other) const;

    bool contains(const R3::point &b) const;
    bool contains(const R3::line &l) const;

    /** Returns the dihedral angle */
    precision angle(const plane &P) const;

    friend std::ostream &operator<<(std::ostream &os, const plane &p);
};

inline bool parallel(const plane &a, const plane &b) {
    return a.parallel(b);
}

namespace operators {
/** Determines if the planes are parallel */
inline bool operator||(const plane &a, const plane &b) {
    return parallel(a, b);
}
}  // namespace operators

std::ostream &operator<<(std::ostream &os, const plane &p);

}  // namespace geometry
