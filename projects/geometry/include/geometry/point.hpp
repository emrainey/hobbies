#pragma once

/**
 * @file
 * Definitions for the point object.
 * @copyright Copyright 2019 (C) Erik Rainey.
 */

#include <xmmt/xmmt.hpp>

#include "geometry/types.hpp"
#include "geometry/vector.hpp"

namespace geometry {

/** A N-dimensional point in space */
class point : public basal::printable {
protected:
    /** The storage of the data */
    std::unique_ptr<element_type[]> m_data;

public:
    /** The dimensionality of the point */
    const size_t dimensions;

    /** Default */
    point(const size_t dim = 3);

    /** Constructor from a pointer to an array */
    explicit point(const element_type a[], size_t len) noexcept(false);

    /** Constructor from an initialization list {{}}; */
    explicit point(const std::vector<element_type>& list) noexcept(false);

    /** Copy Constructor  */
    point(const point& other);

    /** Move Constructor */
    point(point&& other);

    /** Deconstructor */
    virtual ~point();

    /** Copy Assignment */
    point& operator=(const point& other) noexcept(false);

    /** Move Assignment */
    point& operator=(point&& other) noexcept(false);

    /** Accumulate a vector into the point (moves the point by the vector) */
    template <typename DATA_TYPE, size_t DIM>
    point& operator+=(const vector_<DATA_TYPE, DIM>& a) noexcept(false) {
        basal::exception::throw_if(a.dimensions != dimensions, __FILE__, __LINE__,
                                   "Point/Vector must have same dimensionality");
        for (size_t i = 0; i < a.dimensions; i++) {
            m_data[i] += a[i];
        };
        return (*this);
    }

    /** De-accumulate a vector into the point (moves the point by the vector) */
    template <typename DATA_TYPE, size_t DIM>
    point& operator-=(const vector_<DATA_TYPE, DIM>& a) noexcept(false) {
        basal::exception::throw_if(a.dimensions != dimensions, __FILE__, __LINE__,
                                   "Point/Vector must have same dimensionality");
        for (size_t i = 0; i < a.dimensions; i++) {
            m_data[i] -= a[i];
        };
        return (*this);
    }

    /** Scale a point */
    point& operator*=(element_type s) noexcept(false);

    /** Indexer */
    element_type& operator[](int i);

    /** Indexer for Const objects */
    element_type operator[](int i) const;

    /** Clears the point to zero values */
    void zero();

    /** @copydoc basal::printable::print */
    void print(const char name[]) const override;
};

point multiply(const point& a, element_type s) noexcept(false);

point multiply(element_type s, const point& a) noexcept(false);

namespace operators {

/** Equality Operator */
bool operator==(const point& a, const point& b) noexcept(false);

/** Comparing two points */
bool operator==(point& a, point& b) noexcept(false);

/** Inequality Operator */
bool operator!=(const point& a, const point& b) noexcept(false);

inline point operator*(const point& a, element_type s) noexcept(false) {
    return multiply(a, s);
}

inline point operator*(element_type s, const point& a) noexcept(false) {
    return multiply(s, a);
}

/**
 * Multiples a point by a matrix to get another point.
 * This could be used to transform a point
 * @param m The input matrix.
 * @param p The input point.
 * @throws basal::exception if the dimensions are incorrect.
 */
point operator*(const linalg::matrix& m, const point& p);
}  // namespace operators

namespace pairwise {
/** Does a pairwise multiply */
point multiply(const point& a, const point& b) noexcept(false);
/** Does a pairwise divide */
point divide(const point& a, const point& b) noexcept(false);

namespace operators {
inline point operator*(const point& a, const point& b) noexcept(false) {
    return multiply(a, b);
}
inline point operator/(const point& a, const point& b) noexcept(false) {
    return divide(a, b);
}
}  // namespace operators
}  // namespace pairwise

/** Generic template class wrapper */
template <size_t N>
class point_ : public point {
    // No move constructor
    point_(point_&&) = delete;
    // No move assign
    point_& operator=(point_&&) = delete;
};

/** Specific 2d point template wrapper to define easy access reference to \ref x and \ref y */
template <>
class point_<2> : public point {
public:
    element_type& x; /**< First dimensional reference */
    element_type& y; /**< Second dimensional reference */
    /** Custom empty constructor */
    point_() : point{2}, x(m_data[0]), y(m_data[1]) {
    }
    /** Base type Constructor */
    point_(const point& p) : point_{} {
        basal::exception::throw_unless(p.dimensions == 2, __FILE__, __LINE__, "Must match the number of dimensions");
        x = p[0];
        y = p[1];
    }
    /** Copy Constructor */
    point_(const point_& other) : point_{} {
        x = other.x;
        y = other.y;
    }

    /** Custom element_type input constructor */
    explicit point_(element_type a, element_type b) : point_{} {
        x = a;
        y = b;
    }
    /** Assignment Operator */
    point_& operator=(const point_& other) {
        x = other.x;
        y = other.y;
        return (*this);
    }

    /** Adding points creates a vector */
    friend vector_<element_type, 2> operator+(const point_& a, const point_& b) noexcept(false) {
        vector_<element_type, 2> c;
        for (size_t i = 0; i < c.dimensions; i++) {
            c[i] = a[i] + b[i];
        }
        return vector_<element_type, 2>(c);
    }

    /** Subtracting points creates a vector */
    friend vector_<element_type, 2> operator-(const point_& a, const point_& b) noexcept(false) {
        vector_<element_type, 2> c;
        for (size_t i = 0; i < c.dimensions; i++) {
            c[i] = a[i] - b[i];
        }
        return vector_<element_type, 2>(c);
    }

    /** Adding a vector to a point creates a new point */
    friend point operator+(const point_& a, const vector_<element_type, 2>& b) noexcept(false) {
        point c{a};
        c += b;
        return point(c);
    }
};

/** Specific 3d point template wrapper to define easy access reference to \ref x, \ref y and \ref z */
template <>
class point_<3> : public point {
public:
    element_type& x; /**< First dimensional reference */
    element_type& y; /**< Second dimensional reference */
    element_type& z; /**< Third dimensional reference */
    /** Custom empty constructor */
    point_() : point{3}, x(m_data[0]), y(m_data[1]), z(m_data[2]) {
    }
    /** Custom Homogenizing Constructor */
    point_(const point_<2>& p) : point_{} {
        x = p.x;
        y = p.y;
        z = 1.0;
    }
    /** Base type Constructor */
    point_(const point& p) : point_{} {
        basal::exception::throw_unless(p.dimensions == 3, __FILE__, __LINE__, "Must match the number of dimensions");
        x = p[0];
        y = p[1];
        z = p[2];
    }
    /** Copy Constructor */
    point_(const point_& other) : point_{} {
        x = other.x;
        y = other.y;
        z = other.z;
    }
    /** Custom triple input constructor */
    explicit point_(element_type a, element_type b, element_type c) : point_{} {
        x = a;
        y = b;
        z = c;
    }
    /** Assignment from another template */
    point_& operator=(const point_& other) {
        x = other.x;
        y = other.y;
        z = other.z;
        return (*this);
    }

    /** Adding points creates a vector */
    friend vector_<element_type, 3> operator+(const point_& a, const point_& b) noexcept(false) {
        vector_<element_type, 3> c;
        for (size_t i = 0; i < c.dimensions; i++) {
            c[i] = a[i] + b[i];
        }
        return vector_<element_type, 3>(c);
    }

    /** Subtracting points creates a vector */
    friend vector_<element_type, 3> operator-(const point_& a, const point_& b) noexcept(false) {
        vector_<element_type, 3> c;
        for (size_t i = 0; i < c.dimensions; i++) {
            c[i] = a[i] - b[i];
        }
        return vector_<element_type, 3>(c);
    }

    /** Adding a vector to a point creates a new point */
    friend point operator+(const point_& a, const vector_<element_type, 3>& b) noexcept(false) {
        point c{a};
        c += b;
        return point(c);
    }

    /** Adding a vector to a point creates a new point */
    friend point operator-(const point_& a, const vector_<element_type, 3>& b) noexcept(false) {
        point c{a};
        c -= b;
        return point(c);
    }
};

/** Specific 4d point template wrapper to define easy access reference to \ref x, \ref y and \ref z */
template <>
class point_<4> : public point {
public:
    element_type& x; /**< First dimensional reference */
    element_type& y; /**< Second dimensional reference */
    element_type& z; /**< Third dimensional reference */
    element_type& w; /**< Fourth dimensional reference */
    /** Custom empty constructor */
    point_() : point{4}, x(m_data[0]), y(m_data[1]), z(m_data[2]), w(m_data[3]) {
    }
    /** Custom Homogenizing Constructor */
    point_(const point_<3>& p) : point_{} {
        x = p.x;
        y = p.y;
        z = p.z;
        w = 1.0;
    }
    /** Base type Constructor */
    point_(const point& p) : point_{} {
        basal::exception::throw_unless(p.dimensions == 4, __FILE__, __LINE__, "Must match the number of dimensions");
        x = p[0];
        y = p[1];
        z = p[2];
        w = p[3];
    }
    /** Copy Constructor */
    point_(const point_& other) : point_{} {
        x = other.x;
        y = other.y;
        z = other.z;
        w = other.w;
    }
    /** Custom triple input constructor */
    explicit point_(element_type a, element_type b, element_type c, element_type d) : point_{} {
        x = a;
        y = b;
        z = c;
        w = d;
    }
    /** Assignment from another template */
    point_& operator=(const point_& other) {
        x = other.x;
        y = other.y;
        z = other.z;
        w = other.w;
        return (*this);
    }

    /** Adding points creates a vector */
    friend vector_<element_type, 4> operator+(const point_& a, const point_& b) noexcept(false) {
        vector_<element_type, 4> c;
        for (size_t i = 0; i < c.dimensions; i++) {
            c[i] = a[i] + b[i];
        }
        return vector_<element_type, 4>(c);
    }

    /** Subtracting points creates a vector */
    friend vector_<element_type, 4> operator-(const point_& a, const point_& b) noexcept(false) {
        vector_<element_type, 4> c;
        for (size_t i = 0; i < c.dimensions; i++) {
            c[i] = a[i] - b[i];
        }
        return vector_<element_type, 4>(c);
    }

    /** Adding a vector to a point creates a new point */
    friend point operator+(const point_& a, const vector_<element_type, 4>& b) noexcept(false) {
        point c{a};
        c += b;
        return point(c);
    }
};

std::ostream& operator<<(std::ostream& os, const point& p);

/** Converts a vector to a point */
template <typename DATA_TYPE, size_t DIM>
point_<DIM> as_point(const vector_<DATA_TYPE, DIM>& v) {
    point_<DIM> P;
    for (size_t i = 0; i < P.dimensions; i++) {
        P[i] = v[i];
    }
    return point_<DIM>(P);
}

namespace R2 {
using point = point_<2>;
static const point origin(0.0, 0.0);
}  // namespace R2

namespace R3 {
using point = point_<3>;
static const point origin(0.0, 0.0, 0.0);
}  // namespace R3

namespace R4 {
using point = point_<4>;
static const point origin(0.0, 0.0, 0.0, 0.0);
}  // namespace R4

}  // namespace geometry
