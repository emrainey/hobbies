#pragma once
/**
 * @file
 * Definitions for the plane object.
 * @copyright Copyright 2019 (C) Erik Rainey.
 */

#include "geometry/types.hpp"
#include "geometry/vector.hpp"
#include "geometry/point.hpp"
#include "geometry/line.hpp"

namespace geometry {
/**
 * The structure for expressing a plane as 4 coefficients.
 * These are related by the equation \code ax + by + cz + d = 0 \endcode
 */
typedef struct coefficients_t {
    /** First coefficient */
    element_type a;
    /** Second coefficient */
    element_type b;
    /** Third coefficient */
    element_type c;
    /** Fourth coefficient */
    element_type d;
    /** Solves the value of x
     * \returns element_type
     */
    element_type x();
    /** Solves the value of y
     * \returns element_type
     */
    element_type y();
    /** Solves the value of z
     * \returns element_type
     */
    element_type z();
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
    element_type m_magnitude;
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
    explicit plane(const std::vector<element_type> &list);

    /** Of the form ax + by + cz + d = 0 */
    plane(element_type a, element_type b, element_type c, element_type d);

    /**
     * The distance from some point to this plane. Negative values indicates
     * that the normal is pointing away from the point.
     */
    element_type distance(const R3::point &a) const;

    /** Determines if the planes are parallel */
    bool parallel(const plane &a) const;

    /** Determines if the planes are perpendicular */
    bool perpendicular(const plane &a) const;

    /** Returns the unnormalized vector */
    R3::vector unormal() const;

    /** Returns a const reference to the coefficients */
    const coefficients_t & coefficient() const;

    virtual void print(const char name[]) const;

    bool operator==(const plane &other) const;
    bool operator!=(const plane &other) const;

    bool contains(const R3::point &b) const;
    bool contains(const R3::line &l) const;

    /** Returns the dihedral angle */
    element_type angle(const plane &P) const;

    friend std::ostream& operator<<(std::ostream& os, const plane& p);
};

inline bool parallel(const plane &a, const plane &b) {
    return a.parallel(b);
}

namespace operators {
    /** Determines if the planes are parallel */
    inline bool operator||(const plane &a, const plane& b) {
        return parallel(a, b);
    }
}

std::ostream& operator<<(std::ostream& os, const plane& p);

} // namespace geometry
