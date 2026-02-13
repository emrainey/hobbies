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
class point_ : public basal::printable {
public:
public:
    static_assert(2 <= DIMS and DIMS <= 4, "Must have between (inclusive) 2 and 4 dimensions");

    /// The dimensionality of the vector
    constexpr static size_t const dimensions{DIMS};

    /// Determines if the point should be sorted by distance
    /// Used in @ref operator< for sorting in std::set and std::map
    constexpr static bool use_distance_sort{false};

    /// Determines if the point should be sorted lexically (by x, then y, then z, then w)
    /// Used in @ref operator< for sorting in std::set and std::map
    constexpr static bool use_lexical_sort{true};

protected:
    // the data type array (prealigned for optimizations with SIMD instructions)
    alignas(16) precision data_[dimensions];

public:
    /// Default
    point_();

    /// Constructor from a pointer to an array
    explicit point_(precision const a[], size_t len) noexcept(false);

    /// Constructor from a reference to an array of values
    explicit point_(precision const (&a)[DIMS]) noexcept(false);

    /// Copy Constructor`
    point_(point_ const& other);

    /// Move Constructor
    point_(point_&& other);

    /// Constructor with 2 parameters (enabled for DIMS == 2)
    template <size_t D = DIMS, typename = std::enable_if_t<D == 2>>
    explicit point_(precision a, precision b);

    /// Constructor with 3 parameters (enabled for DIMS == 3)
    template <size_t D = DIMS, typename = std::enable_if_t<D == 3>>
    explicit point_(precision a, precision b, precision c);

    /// Constructor with 4 parameters (enabled for DIMS == 4)
    template <size_t D = DIMS, typename = std::enable_if_t<D == 4>>
    explicit point_(precision a, precision b, precision c, precision d);

    /// Homogenizing constructor from 2D to 3D with z=1 (enabled for DIMS == 3)
    template <size_t D = DIMS, typename = std::enable_if_t<D == 3>>
    point_(point_<2> const& p);

    /// Homogenizing constructor from 3D to 4D with w=1 (enabled for DIMS == 4)
    template <size_t D = DIMS, typename = std::enable_if_t<D == 4>>
    point_(point_<3> const& p);

    /// Deconstructor
    virtual ~point_() = default;

    /// Copy Assignment
    point_& operator=(point_ const& other) noexcept(false);

    /// Move Assignment
    point_& operator=(point_&& other) noexcept(false);

    /// Accumulate a vector into the point (moves the point by the vector)
    point_& operator+=(vector_<DIMS> const& a) noexcept(false);

    /// De-accumulate a vector into the point (moves the point by the vector)
    point_& operator-=(vector_<DIMS> const& a) noexcept(false);

    /// Scale a point_
    point_& operator*=(precision s) noexcept(false);

    /// Indexer
    precision& operator[](size_t i);

    /// Indexer for Const objects
    precision operator[](size_t i) const;

    /// Named accessor for x component (always available for DIMS >= 2)
    constexpr precision& x() noexcept {
        static_assert(DIMS >= 2, "x() requires at least 2 dimensions");
        return data_[0];
    }

    /// Named accessor for x component (const)
    constexpr precision const& x() const noexcept {
        static_assert(DIMS >= 2, "x() requires at least 2 dimensions");
        return data_[0];
    }

    /// Named accessor for y component (always available for DIMS >= 2)
    constexpr precision& y() noexcept {
        static_assert(DIMS >= 2, "y() requires at least 2 dimensions");
        return data_[1];
    }

    /// Named accessor for y component (const)
    constexpr precision const& y() const noexcept {
        static_assert(DIMS >= 2, "y() requires at least 2 dimensions");
        return data_[1];
    }

    /// Named accessor for z component (only available for DIMS >= 3)
    template <size_t D = DIMS, typename = std::enable_if_t<D >= 3>>
    constexpr precision& z() noexcept {
        return data_[2];
    }

    /// Named accessor for z component (const, only available for DIMS >= 3)
    template <size_t D = DIMS, typename = std::enable_if_t<D >= 3>>
    constexpr precision const& z() const noexcept {
        return data_[2];
    }

    /// Named accessor for w component (only available for DIMS >= 4)
    template <size_t D = DIMS, typename = std::enable_if_t<D >= 4>>
    constexpr precision& w() noexcept {
        return data_[3];
    }

    /// Named accessor for w component (const, only available for DIMS >= 4)
    template <size_t D = DIMS, typename = std::enable_if_t<D >= 4>>
    constexpr precision const& w() const noexcept {
        return data_[3];
    }

    /// Clears the point_ to zero values
    void zero();

    /// @copydoc basal::printable::print
    void print(std::ostream&, char const[]) const override;
};

template <size_t DIMS>
point_<DIMS> multiply(point_<DIMS> const& a, precision s);

template <size_t DIMS>
point_<DIMS> multiply(precision s, point_<DIMS> const& a);

namespace operators {

/// Equality Operator
template <size_t DIMS>
bool operator==(point_<DIMS> const& a, point_<DIMS> const& b);

/// Inequality Operator
template <size_t DIMS>
bool operator!=(point_<DIMS> const& a, point_<DIMS> const& b);

template <size_t DIMS>
bool operator<(point_<DIMS> const& a, point_<DIMS> const& b) noexcept(false);

template <size_t DIMS>
inline point_<DIMS> operator*(point_<DIMS> const& a, precision s) noexcept(false) {
    return multiply(a, s);
}

template <size_t DIMS>
inline point_<DIMS> operator*(precision s, point_<DIMS> const& a) noexcept(false) {
    return multiply(s, a);
}

template <size_t DIMS>
vector_<DIMS> operator+(point_<DIMS> const& a, point_<DIMS> const& b) noexcept(false);

template <size_t DIMS>
vector_<DIMS> operator-(point_<DIMS> const& a, point_<DIMS> const& b) noexcept(false);

template <size_t DIMS>
point_<DIMS> operator+(point_<DIMS> const& a, const vector_<DIMS>& b) noexcept(false);

template <size_t DIMS>
point_<DIMS> operator-(point_<DIMS> const& a, const vector_<DIMS>& b) noexcept(false);

///
/// Multiples a point_ by a matrix to get another point_.
/// This could be used to transform a point_
/// @param m The input matrix.
/// @param p The input point_.
/// @throws basal::exception if the dimensions are incorrect.
///
template <size_t DIMS>
point_<DIMS> operator*(linalg::matrix const& m, point_<DIMS> const& p);
}  // namespace operators

namespace pairwise {
/// Does a pairwise multiply
template <size_t DIMS>
point_<DIMS> multiply(point_<DIMS> const& a, point_<DIMS> const& b);
/// Does a pairwise divide
template <size_t DIMS>
point_<DIMS> divide(point_<DIMS> const& a, point_<DIMS> const& b);

namespace operators {
template <size_t DIMS>
inline point_<DIMS> operator*(point_<DIMS> const& a, point_<DIMS> const& b) noexcept(false) {
    return multiply(a, b);
}
template <size_t DIMS>
inline point_<DIMS> operator/(point_<DIMS> const& a, point_<DIMS> const& b) noexcept(false) {
    return divide(a, b);
}

}  // namespace operators
}  // namespace pairwise

template <size_t DIMS>
std::ostream& operator<<(std::ostream& os, point_<DIMS> const& p);

/// Converts a vector to a point_
template <size_t DIM>
point_<DIM> as_point(vector_<DIM> const& v);

using geometry::operators::operator<;  // This has to be promoted to allow usage in std::set and std::map

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
