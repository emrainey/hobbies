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

    constexpr static bool use_distance_sort{false};
    constexpr static bool use_lexical_sort{true};

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
    precision& operator[](size_t i);

    /// Indexer for Const objects
    precision operator[](size_t i) const;

    /// Named accessor for x component (always available for DIMS >= 2)
    constexpr precision& x() noexcept {
        static_assert(DIMS >= 2, "x() requires at least 2 dimensions");
        return m_data[0];
    }

    /// Named accessor for x component (const)
    constexpr precision const& x() const noexcept {
        static_assert(DIMS >= 2, "x() requires at least 2 dimensions");
        return m_data[0];
    }

    /// Named accessor for y component (always available for DIMS >= 2)
    constexpr precision& y() noexcept {
        static_assert(DIMS >= 2, "y() requires at least 2 dimensions");
        return m_data[1];
    }

    /// Named accessor for y component (const)
    constexpr precision const& y() const noexcept {
        static_assert(DIMS >= 2, "y() requires at least 2 dimensions");
        return m_data[1];
    }

    /// Named accessor for z component (only available for DIMS >= 3)
    template <size_t D = DIMS, typename = std::enable_if_t<D >= 3>>
    constexpr precision& z() noexcept {
        return m_data[2];
    }

    /// Named accessor for z component (const, only available for DIMS >= 3)
    template <size_t D = DIMS, typename = std::enable_if_t<D >= 3>>
    constexpr precision const& z() const noexcept {
        return m_data[2];
    }

    /// Named accessor for w component (only available for DIMS >= 4)
    template <size_t D = DIMS, typename = std::enable_if_t<D >= 4>>
    constexpr precision& w() noexcept {
        return m_data[3];
    }

    /// Named accessor for w component (const, only available for DIMS >= 4)
    template <size_t D = DIMS, typename = std::enable_if_t<D >= 4>>
    constexpr precision const& w() const noexcept {
        return m_data[3];
    }

    /// Clears the point to zero values
    void zero();

    /// @copydoc basal::printable::print
    void print(std::ostream&, char const[]) const override;
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
bool operator!=(point<DIMS> const& a, point<DIMS> const& b);

template <size_t DIMS>
bool operator<(point<DIMS> const& a, point<DIMS> const& b) noexcept(false);

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

// Forward declarations for operators used by point_<N> specializations
template <size_t DIMS>
class point_;

template <size_t DIMS>
vector_<DIMS> operator+(point_<DIMS> const& a, point_<DIMS> const& b) noexcept(false);

template <size_t DIMS>
vector_<DIMS> operator-(point_<DIMS> const& a, point_<DIMS> const& b) noexcept(false);

template <size_t DIMS>
point_<DIMS> operator+(point_<DIMS> const& a, const vector_<DIMS>& b) noexcept(false);

template <size_t DIMS>
point_<DIMS> operator-(point_<DIMS> const& a, const vector_<DIMS>& b) noexcept(false);

/// Generic template class wrapper
template <size_t N>
class point_ : public point<N> {
public:
    static_assert(2 <= N && N <= 4, "point_ must have between 2 and 4 dimensions");

    /// Default constructor
    point_() : point<N>{} {
    }

    /// Base type Constructor
    point_(point<N> const& p) : point<N>{p} {
    }

    /// Copy Constructor
    point_(point_ const& other) : point<N>{other} {
    }

    /// Move Constructor
    point_(point_&& other) : point<N>{std::move(other)} {
    }

    /// Assignment Operator
    point_& operator=(point_ const& other) {
        point<N>::operator=(other);
        return (*this);
    }

    /// Move assignment
    point_& operator=(point_&& other) {
        point<N>::operator=(std::move(other));
        return (*this);
    }
};

/// Specialization for 2D points with custom constructor
template <>
class point_<2> : public point<2> {
public:
    /// Default constructor
    point_() : point<2>{} {
    }

    /// Base type Constructor
    point_(point<2> const& p) : point<2>{p} {
    }

    /// Copy Constructor
    point_(point_ const& other) : point<2>{other} {
    }

    /// Move Constructor
    point_(point_&& other) : point<2>{std::move(other)} {
    }

    /// Custom precision input constructor
    explicit point_(precision a, precision b) : point<2>{} {
        x() = a;
        y() = b;
    }

    /// Assignment Operator
    point_& operator=(point_ const& other) {
        point<2>::operator=(other);
        return (*this);
    }

    /// Move assignment
    point_& operator=(point_&& other) {
        point<2>::operator=(std::move(other));
        return (*this);
    }

    /// Uses distance from origin to sort a pair of points.
    bool operator<(point_<2> const& other) const noexcept(false) {
        if constexpr (point::use_distance_sort) {
            vector_<2> a0 = (*this) - point_<2>{};
            vector_<2> b0 = other - point_<2>{};
            return (a0.quadrance() < b0.quadrance());
        }
        if constexpr (point::use_lexical_sort) {
            if (x() < other.x()) {
                return true;
            } else if (x() > other.x()) {
                return false;
            }
            return (y() < other.y());
        }
        return false;
    }
};

/// Specialization for 3D points with custom constructor and homogenizing constructor
template <>
class point_<3> : public point<3> {
public:
    /// Default constructor
    point_() : point<3>{} {
    }

    /// Homogenizing Constructor from 2D
    point_(point_<2> const& p) : point<3>{} {
        x() = p.x();
        y() = p.y();
        z() = 1.0_p;
    }

    /// Base type Constructor
    point_(point<3> const& p) : point<3>{p} {
    }

    /// Copy Constructor
    point_(point_ const& other) : point<3>{other} {
    }

    /// Move Constructor
    point_(point_&& other) : point<3>{std::move(other)} {
    }

    /// Custom triple input constructor
    explicit point_(precision a, precision b, precision c) : point<3>{} {
        x() = a;
        y() = b;
        z() = c;
    }

    /// Assignment Operator
    point_& operator=(point_ const& other) {
        point<3>::operator=(other);
        return (*this);
    }

    /// Move assignment
    point_& operator=(point_&& other) {
        point<3>::operator=(std::move(other));
        return (*this);
    }

    /// Uses distance from origin to sort a pair of points.
    bool operator<(point_<3> const& other) const noexcept(false) {
        if constexpr (point::use_distance_sort) {
            vector_<3> a0 = (*this) - point_<3>{};
            vector_<3> b0 = other - point_<3>{};
            return (a0.quadrance() < b0.quadrance());
        }
        if constexpr (point::use_lexical_sort) {
            if (x() < other.x()) {
                return true;
            } else if (x() > other.x()) {
                return false;
            }
            if (y() < other.y()) {
                return true;
            } else if (y() > other.y()) {
                return false;
            }
            return (z() < other.z());
        }
        return false;
    }
};

/// Specialization for 4D points with custom constructor and homogenizing constructor
template <>
class point_<4> : public point<4> {
public:
    /// Default constructor
    point_() : point<4>{} {
    }

    /// Homogenizing Constructor from 3D
    point_(point_<3> const& p) : point<4>{} {
        x() = p.x();
        y() = p.y();
        z() = p.z();
        w() = 1.0_p;
    }

    /// Base type Constructor
    point_(point<4> const& p) : point<4>{p} {
    }

    /// Copy Constructor
    point_(point_ const& other) : point<4>{other} {
    }

    /// Move Constructor
    point_(point_&& other) : point<4>{std::move(other)} {
    }

    /// Custom four input constructor
    explicit point_(precision a, precision b, precision c, precision d) : point<4>{} {
        x() = a;
        y() = b;
        z() = c;
        w() = d;
    }

    /// Assignment Operator
    point_& operator=(point_ const& other) {
        point<4>::operator=(other);
        return (*this);
    }

    /// Move assignment
    point_& operator=(point_&& other) {
        point<4>::operator=(std::move(other));
        return (*this);
    }

    /// Uses distance from origin to sort a pair of points.
    bool operator<(point_<4> const& other) const noexcept(false) {
        if constexpr (point::use_distance_sort) {
            vector_<4> a0 = (*this) - point_<4>{};
            vector_<4> b0 = other - point_<4>{};
            return (a0.quadrance() < b0.quadrance());
        }
        if constexpr (point::use_lexical_sort) {
            if (x() < other.x()) {
                return true;
            } else if (x() > other.x()) {
                return false;
            }
            if (y() < other.y()) {
                return true;
            } else if (y() > other.y()) {
                return false;
            }
            if (z() < other.z()) {
                return true;
            } else if (z() > other.z()) {
                return false;
            }
            return (w() < other.w());
        }
        return false;
    }
};

// Template friend operators (moved outside specializations)

/// Adding points creates a vector
template <size_t DIMS>
vector_<DIMS> operator+(point_<DIMS> const& a, point_<DIMS> const& b) noexcept(false) {
    vector_<DIMS> c;
    for (size_t i = 0; i < c.dimensions; i++) {
        c[i] = a[i] + b[i];
    }
    return c;
}

/// Subtracting points creates a vector
template <size_t DIMS>
vector_<DIMS> operator-(point_<DIMS> const& a, point_<DIMS> const& b) noexcept(false) {
    vector_<DIMS> c;
    for (size_t i = 0; i < c.dimensions; i++) {
        c[i] = a[i] - b[i];
    }
    return c;
}

/// Adding a vector to a point creates a new point
template <size_t DIMS>
point_<DIMS> operator+(point_<DIMS> const& a, const vector_<DIMS>& b) noexcept(false) {
    point_<DIMS> c{a};
    c += b;
    return c;
}

/// Subtracting a vector from a point creates a new point
template <size_t DIMS>
point_<DIMS> operator-(point_<DIMS> const& a, const vector_<DIMS>& b) noexcept(false) {
    point_<DIMS> c{a};
    c -= b;
    return c;
}

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
