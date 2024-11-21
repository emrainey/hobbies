#pragma once

/// @file
/// Definitions for the point object.
/// @copyright Copyright 2019 (C) Erik Rainey.

// #include <xmmt/xmmt.hpp>

#include "geometry/types.hpp"
#include "geometry/vector.hpp"

namespace geometry {

/// A N-dimensional point in space
template <size_t DIMS>
class point : public basal::printable {
protected:
    /// The storage of the data
    precision m_data[DIMS];

public:
    /// The dimensionality of the point
    constexpr static size_t const dimensions{DIMS};

    /// Default
    point();

    /// Constructor from a pointer to an array
    explicit point(precision const a[], size_t len) noexcept(false);

    /// Constructor from a reference to an array of values
    explicit point(precision const (&a)[DIMS]) noexcept(false);

    /// Copy Constructor`
    point(point const& other);

    /// Move Constructor
    point(point&& other);

    /// Deconstructor
    virtual ~point() = default;

    /// Copy Assignment
    point& operator=(point const& other) noexcept(false);

    /// Move Assignment
    point& operator=(point&& other) noexcept(false);

    /// Accumulate a vector into the point (moves the point by the vector)
    point& operator+=(vector_<DIMS> const& a) noexcept(false) {
        for (size_t i = 0; i < DIMS; i++) {
            m_data[i] += a[i];
        };
        return (*this);
    }

    /// De-accumulate a vector into the point (moves the point by the vector)
    point& operator-=(vector_<DIMS> const& a) noexcept(false) {
        for (size_t i = 0; i < DIMS; i++) {
            m_data[i] -= a[i];
        };
        return (*this);
    }

    /// Scale a point
    point& operator*=(precision s) noexcept(false);

    /// Indexer
    precision& operator[](int i);

    /// Indexer for Const objects
    precision operator[](int i) const;

    /// Clears the point to zero values
    void zero();

    /// @copydoc basal::printable::print
    void print(char const name[]) const override;
};

template <size_t DIMS>
point<DIMS> multiply(point<DIMS> const& a, precision s);

template <size_t DIMS>
point<DIMS> multiply(precision s, point<DIMS> const& a);

namespace operators {

/// Equality Operator
template <size_t DIMS>
bool operator==(point<DIMS> const& a, point<DIMS> const& b);

/// Inequality Operator
template <size_t DIMS>
bool operator!=(point<DIMS> const& a, point<DIMS> const& b) noexcept(false);

template <size_t DIMS>
inline point<DIMS> operator*(point<DIMS> const& a, precision s) noexcept(false) {
    return multiply(a, s);
}

template <size_t DIMS>
inline point<DIMS> operator*(precision s, point<DIMS> const& a) noexcept(false) {
    return multiply(s, a);
}

///
/// Multiples a point by a matrix to get another point.
/// This could be used to transform a point
/// @param m The input matrix.
/// @param p The input point.
/// @throws basal::exception if the dimensions are incorrect.
///
template <size_t DIMS>
point<DIMS> operator*(linalg::matrix const& m, point<DIMS> const& p);
}  // namespace operators

namespace pairwise {
/// Does a pairwise multiply
template <size_t DIMS>
point<DIMS> multiply(point<DIMS> const& a, point<DIMS> const& b);
/// Does a pairwise divide
template <size_t DIMS>
point<DIMS> divide(point<DIMS> const& a, point<DIMS> const& b);

namespace operators {
template <size_t DIMS>
inline point<DIMS> operator*(point<DIMS> const& a, point<DIMS> const& b) noexcept(false) {
    return multiply(a, b);
}
template <size_t DIMS>
inline point<DIMS> operator/(point<DIMS> const& a, point<DIMS> const& b) noexcept(false) {
    return divide(a, b);
}
}  // namespace operators
}  // namespace pairwise

/// Generic template class wrapper
template <size_t N>
class point_ : public point<N> {
    // No move constructor
    point_(point_&&) = delete;
    // No move assign
    point_& operator=(point_&&) = delete;
};

/// Specific 2d point template wrapper to define easy access reference to \ref x and \ref y
template <>
class point_<2> : public point<2> {
public:
    precision& x; ///< First dimensional reference
    precision& y; ///< Second dimensional reference
    /// Custom empty constructor
    point_() : point<2>{}, x(m_data[0]), y(m_data[1]) {
    }
    /// Base type Constructor
    point_(point<2> const& p) : point_{} {
        x = p[0];
        y = p[1];
    }
    /// Copy Constructor
    point_(point_ const& other) : point_{} {
        x = other.x;
        y = other.y;
    }

    /// Custom precision input constructor
    explicit point_(precision a, precision b) : point_{} {
        x = a;
        y = b;
    }
    /// Assignment Operator
    point_& operator=(point_ const& other) {
        x = other.x;
        y = other.y;
        return (*this);
    }

    /// Adding points creates a vector
    friend vector_<2> operator+(point_ const& a, point_ const& b) noexcept(false) {
        vector_<2> c;
        for (size_t i = 0; i < c.dimensions; i++) {
            c[i] = a[i] + b[i];
        }
        return c;
    }

    /// Subtracting points creates a vector
    friend vector_<2> operator-(point_ const& a, point_ const& b) noexcept(false) {
        vector_<2> c;
        for (size_t i = 0; i < c.dimensions; i++) {
            c[i] = a[i] - b[i];
        }
        return c;
    }

    /// Adding a vector to a point creates a new point
    friend point_ operator+(point_ const& a, const vector_<2>& b) noexcept(false) {
        point_ c{a};
        c += b;
        return point_{c};
    }
};

/// Specific 3d point template wrapper to define easy access reference to \ref x, \ref y and \ref z
template <>
class point_<3> : public point<3> {
public:
    precision& x; ///< First dimensional reference
    precision& y; ///< Second dimensional reference
    precision& z; ///< Third dimensional reference
    /// Custom empty constructor
    point_() : point<3>{}, x(m_data[0]), y(m_data[1]), z(m_data[2]) {
    }
    /// Custom Homogenizing Constructor
    point_(point_<2> const& p) : point_{} {
        x = p.x;
        y = p.y;
        z = 1.0_p;
    }
    /// Base type Constructor
    point_(point<3> const& p) : point_{} {
        x = p[0];
        y = p[1];
        z = p[2];
    }
    /// Copy Constructor
    point_(point_ const& other) : point_{} {
        x = other.x;
        y = other.y;
        z = other.z;
    }
    /// Custom triple input constructor
    explicit point_(precision a, precision b, precision c) : point_{} {
        x = a;
        y = b;
        z = c;
    }
    /// Assignment from another template
    point_& operator=(point_ const& other) {
        x = other.x;
        y = other.y;
        z = other.z;
        return (*this);
    }

    /// Adding points creates a vector
    friend vector_<3> operator+(point_ const& a, point_ const& b) noexcept(false) {
        vector_<3> c;
        for (size_t i = 0; i < c.dimensions; i++) {
            c[i] = a[i] + b[i];
        }
        return c;
    }

    /// Subtracting points creates a vector
    friend vector_<3> operator-(point_ const& a, point_ const& b) noexcept(false) {
        vector_<3> c;
        for (size_t i = 0; i < c.dimensions; i++) {
            c[i] = a[i] - b[i];
        }
        return c;
    }

    /// Adding a vector to a point creates a new point
    friend point_ operator+(point_ const& a, const vector_<3>& b) noexcept(false) {
        point_ c{a};
        c += b;
        return point_{c};
    }

    /// Adding a vector to a point creates a new point
    friend point_ operator-(point_ const& a, const vector_<3>& b) noexcept(false) {
        point_ c{a};
        c -= b;
        return point_{c};
    }
};

/// Specific 4d point template wrapper to define easy access reference to \ref x, \ref y and \ref z
template <>
class point_<4> : public point<4> {
public:
    precision& x; ///< First dimensional reference
    precision& y; ///< Second dimensional reference
    precision& z; ///< Third dimensional reference
    precision& w; ///< Fourth dimensional reference
    /// Custom empty constructor
    point_() : point<4>{}, x(m_data[0]), y(m_data[1]), z(m_data[2]), w(m_data[3]) {
    }
    /// Custom Homogenizing Constructor
    point_(point_<3> const& p) : point_{} {
        x = p.x;
        y = p.y;
        z = p.z;
        w = 1.0_p;
    }
    /// Base type Constructor
    point_(point<4> const& p) : point_{} {
        x = p[0];
        y = p[1];
        z = p[2];
        w = p[3];
    }
    /// Copy Constructor
    point_(point_ const& other) : point_{} {
        x = other.x;
        y = other.y;
        z = other.z;
        w = other.w;
    }
    /// Custom triple input constructor
    explicit point_(precision a, precision b, precision c, precision d) : point_{} {
        x = a;
        y = b;
        z = c;
        w = d;
    }
    /// Assignment from another template
    point_& operator=(point_ const& other) {
        x = other.x;
        y = other.y;
        z = other.z;
        w = other.w;
        return (*this);
    }

    /// Adding points creates a vector
    friend vector_<4> operator+(point_ const& a, point_ const& b) noexcept(false) {
        vector_<4> c;
        for (size_t i = 0; i < c.dimensions; i++) {
            c[i] = a[i] + b[i];
        }
        return c;
    }

    /// Subtracting points creates a vector
    friend vector_<4> operator-(point_ const& a, point_ const& b) noexcept(false) {
        vector_<4> c;
        for (size_t i = 0; i < c.dimensions; i++) {
            c[i] = a[i] - b[i];
        }
        return c;
    }

    /// Adding a vector to a point creates a new point
    friend point operator+(point_ const& a, const vector_<4>& b) noexcept(false) {
        point c{a};
        c += b;
        return c;
    }
};

template <size_t DIMS>
std::ostream& operator<<(std::ostream& os, point<DIMS> const& p);

/// Converts a vector to a point
template <size_t DIM>
point_<DIM> as_point(vector_<DIM> const& v) {
    point_<DIM> P;
    for (size_t i = 0; i < P.dimensions; i++) {
        P[i] = v[i];
    }
    return P;
}

namespace R2 {
using point = point_<2>;
static point const origin{0.0_p, 0.0_p};
using points = std::vector<point>;
}  // namespace R2

namespace R3 {
using point = point_<3>;
static point const origin{0.0_p, 0.0_p, 0.0_p};
using points = std::vector<point>;
}  // namespace R3

namespace R4 {
using point = point_<4>;
static point const origin{0.0_p, 0.0_p, 0.0_p, 0.0_p};
using points = std::vector<point>;
}  // namespace R4

}  // namespace geometry
