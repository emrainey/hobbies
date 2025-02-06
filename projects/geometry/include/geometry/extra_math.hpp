#pragma once
/// @file
/// This header is to declare cross-object math operations
/// @copyright Copyright 2019 (C) Erik Rainey.

#include "geometry/geometry.hpp"

namespace geometry {
/// Computes the dot product of a vector and a point
precision dot(R3::vector const& a, R3::point const& b);

/// Computes the dot product of a vector and a point
inline precision dot(R3::point const& b, R3::vector const& a) {
    return dot(a, b);
}

/// Finds the centroid of a set of points
R3::point centroid(std::vector<R3::point> const& points);

/// Finds the centroid of 3 points.
R3::point centroid(R3::point const& A, R3::point const& B, R3::point const& C);

/// Finds the centroid of 2 points.
R3::point centroid(R3::point const& A, R3::point const& B);

///
/// Returns a point on each line of skew lines which is closest to the other line.
/// @param first First line
/// @param second Second line
/// @return std::pair of points. The first and second attributes map back to the first and second lines.
std::pair<R3::point, R3::point> closest_points_from_skew_lines(R3::line const& first, R3::line const& second);

/// Returns a point on the XY plane which is addressed by 2d polar
/// coordinates within the plane, centered around the axis of
/// the normal which is based of the point closed to the origin.
///
R2::point polar_to_cartesian(R2::point const& cart);

/// Returns a polar point on the unit plane given the 2d cartesian coordinates
R2::point cartesian_to_polar(R2::point const& polar);

/// Converts a Cartesian 3D point to a 3D spherical coordinate.
/// @see https://keisan.casio.com/exec/system/1359533867
///
R3::point cartesian_to_spherical(R3::point const& cartesian_point);

/// Converts a 3D Spherical Coordinate to a Cartesian 3D Coordinate
/// @return R3::point
R3::point spherical_to_cartesian(R3::point const& spherical_point);

/// Returns a 3x3 rotation matrix.
/// @param in axis The axis to rotate about
/// @param in theta The amount in radians (following the right hand rule) to rotate around the axis
linalg::matrix rotation(R3::vector const& axis, iso::radians const theta);

/// Joins the matricies horizontally, mxn and mxk to make a mx(n+k) matrix
template <size_t DIMS>
matrix rowjoin(matrix& a, vector_<DIMS>& b) noexcept(false) {
    basal::exception::throw_unless(a.rows == b.dimensions, __FILE__, __LINE__,
                                   "To join, the number of rows must be equal");
    matrix::ref_coord_iterator iter = [&](size_t r, size_t c, precision& v) {
        if (c < a.cols) {
            v = a[r][c];
        } else {
            v = b[r];
        }
    };
    return matrix(a.rows, a.cols + b.cols).for_each (iter);
}

/// Creates a matrix from an array of column vectors
template <size_t DIM>
matrix span(std::vector<geometry::vector_<DIM>> const& set) noexcept(false) {
    matrix m{set[0].dimensions, set.size()};
    for (auto& v : set) {
        basal::exception::throw_if(v.dimensions == m.rows, __FILE__, __LINE__,
                                   "Each vector must match dimensionality for the first vector.");
    }
    for (size_t j = 0; j < m.cols; j++) {
        for (size_t i = 0; i < m.rows; i++) {
            m(i, j) = set[j](i);
        }
    }
    return m;
}

/// Determines if a point is within an AABB (Axis Aligned Bounding Box) defined by 2 points, min and max
bool contained_within_aabb(R3::point const& P, R3::point const& min, R3::point const& max);

/// @brief A mapping function that takes a precision between 0 and 1 and returns a precision between 0 and 1.
/// Values outside this range are undefined.
/// @note It is required that when the input is zero, the output is zero and when the input is one, the output is one.
using mapper = std::function<precision(precision)>;

namespace mapping {
constexpr precision linear(precision x) {
    return x;
}

constexpr precision quadratic(precision x) {
    return x * x;
}

constexpr precision cubic(precision x) {
    return x * x * x;
}

constexpr precision sqrt(precision x) {
    return std::sqrt(x);
}

constexpr precision sin(precision x) {
    return std::sin(x * (iso::pi / 2.0_p));
}

constexpr precision ease(precision x) {
    return (std::sin((x * iso::pi) - (iso::pi/2.0_p)) / 2.0_p) + 0.5_p;
}

/// x^(1/x)
constexpr precision curb(precision x) {
    return std::pow(x, 1.0_p / x);
}

/// x^x^sqrt(x)
constexpr precision root_spindle(precision x) {
    return std::pow(x, std::pow(x, sqrt(x)));
}

/// x^x^x
constexpr precision spindle(precision x) {
    return std::pow(x, std::pow(x, x));
}

} // namespace mapping



namespace R2 {
using interpolator = std::function<point(point const&, point const&, mapper, precision)>;
}  // namespace R2

namespace R3 {
using interpolator = std::function<point(point const&, point const&, mapper, precision)>;
}  // namespace R3

namespace R4 {
using interpolator = std::function<point(point const&, point const&, mapper, precision)>;
}  // namespace R4

/// Linearly interpolates between two points using a mapper function to slightly change the interpolation
template <typename POINT>
POINT lerp(POINT const& a, POINT const& b, mapper map, precision t) {
    return a + ((b - a) * map(t));
}

/// Linearly interpolates between two points using a linear mapper
template <typename POINT>
POINT lerp(POINT const& a, POINT const& b, precision t) {
    return lerp(a, b, mapping::linear, t);
}

}  // namespace geometry
