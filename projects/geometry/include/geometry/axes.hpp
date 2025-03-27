#pragma once
/// @file
/// Definitions for the axes object.
/// @copyright Copyright 2025 (C) Erik Rainey
#include <array>
#include <functional>
#include <initializer_list>
#include <utility>
#include <vector>

#include "geometry/point.hpp"
#include "geometry/types.hpp"
#include "geometry/vector.hpp"

namespace geometry {

namespace R2 {

/// A structure which allows for creating a translated and rotated coordinate system. The values of the origin and axes
/// are in Standard 2D Cartesian coordinates.
class axes {
public:
    axes() = default;
    axes(R2::point const& origin, R2::vector const& abscissa, R2::vector const& ordinate);
    R2::point const& origin() const;
    R2::vector const& abscissa() const;
    R2::vector const& ordinate() const;
    R2::point const& origin(R2::point const&);
    R2::vector const& abscissa(R2::vector const&);
    R2::vector const& ordinate(R2::vector const&);
    bool is_basis() const;
    matrix forward() const;
    matrix backwards() const;

protected:
    R2::point origin_{0.0_p, 0.0_p};
    R2::vector abscissa_{R2::basis::X};
    R2::vector ordinate_{R2::basis::Y};
};
}  // namespace R2

namespace R3 {
/// A structure which allows for creating a translated and rotated coordinate system. The values of the origin and axes
/// are in Standard 3D Cartesian coordinates.
class axes {
public:
    axes() = default;
    axes(R3::point const& origin, R3::vector const& abscissa, R3::vector const& ordinate, R3::vector const& applicate);

    R3::point const& origin() const;
    R3::vector const& abscissa() const;
    R3::vector const& ordinate() const;
    R3::vector const& applicate() const;
    R3::point const& origin(R3::point const&);
    R3::vector const& abscissa(R3::vector const&);
    R3::vector const& ordinate(R3::vector const&);
    R3::vector const& applicate(R3::vector const&);
    bool is_basis() const;
    matrix forward() const;
    matrix backwards() const;

protected:
    R3::point origin_{0.0_p, 0.0_p, 0.0_p};
    R3::vector abscissa_{R3::basis::X};
    R3::vector ordinate_{R3::basis::Y};
    R3::vector applicate_{R3::basis::Z};
};
}  // namespace R3

// no R4::axes as it is not needed yet in my projects

}  // namespace geometry