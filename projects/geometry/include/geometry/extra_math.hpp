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
template <typename DATA_TYPE, size_t DIMS>
matrix rowjoin(matrix& a, vector_<DATA_TYPE, DIMS>& b) noexcept(false) {
    basal::exception::throw_unless(a.rows == b.dimensions, __FILE__, __LINE__,
                                   "To join, the number of rows must be equal");
    matrix::ref_coord_iterator iter = [&](size_t r, size_t c, DATA_TYPE& v) {
        if (c < a.cols) {
            v = a[r][c];
        } else {
            v = b[r];
        }
    };
    return matrix(a.rows, a.cols + b.cols).for_each (iter);
}

/// Creates a matrix from an array of column vectors
template <typename DATA_TYPE, size_t DIM>
matrix span(std::vector<geometry::vector_<DATA_TYPE, DIM>> const& set) noexcept(false) {
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
}  // namespace geometry
