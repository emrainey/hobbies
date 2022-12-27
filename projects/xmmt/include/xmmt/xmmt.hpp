#pragma once

/**
 * @file
 * Main Interface for the AVX/AVX2 points and vectors
 */

#include "xmmt/point.hpp"
#include "xmmt/vector.hpp"

namespace intel {

// DEFINE POINT / VECTOR MATH

/****** FRIENDS **************************************************************/

/** A Point minus a point is a vector from the last to the first. */
template <typename pack_type, size_t dimensions>
vector_<pack_type, dimensions> operator-(const point_<pack_type, dimensions>& a,
                                         const point_<pack_type, dimensions>& b) {
    vector_<pack_type, dimensions> c{};
    if constexpr (point_<pack_type, dimensions>::number_of_elements == 2) {
        c.data = _mm_sub_pd(a.data, b.data);
    } else {
        c.data = _mm256_sub_pd(a.data, b.data);
    }
    return c;
}

/** A Point plus a vector is a point. */
template <typename pack_type, size_t dimensions>
point_<pack_type, dimensions> operator+(const point_<pack_type, dimensions>& a,
                                        const vector_<pack_type, dimensions>& b) {
    point_<pack_type, dimensions> c{};
    if constexpr (point_<pack_type, dimensions>::number_of_elements == 2) {
        c.data = _mm_add_pd(a.data, b.data);
    } else {
        c.data = _mm256_add_pd(a.data, b.data);
    }
    return c;
}

using point2 = point_<double2, 2>;
using point3 = point_<double3, 3>;
using point4 = point_<double4, 4>;

using vector2 = vector_<double2, 2>;
using vector3 = vector_<double3, 3>;
using vector4 = vector_<double4, 4>;

}  // namespace intel
