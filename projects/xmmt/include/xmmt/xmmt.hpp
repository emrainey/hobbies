#ifndef XMMT_XMMT_HPP_
#define XMMT_XMMT_HPP_

#include "xmmt/point.hpp"
#include "xmmt/vector.hpp"
#include "xmmt/pixel.hpp"

namespace intel {

// +================================================================================================+
// Cross-Type operations
// +================================================================================================+

/// A Point minus a point is a vector from the last to the first.
template <typename pack_type, size_t dimensions>
vector_<pack_type, dimensions> operator-(const point_<pack_type, dimensions>& a,
                                         const point_<pack_type, dimensions>& b) {
    vector_<pack_type, dimensions> c{};
    if constexpr (point_<pack_type, dimensions>::number_of_elements == 2) {
        c.data = _mm_sub_pd(a.data, b.data);
    } else {
        if constexpr (std::is_same_v<typename pack_type::element_type, float>) {
            c.data = _mm_sub_ps(a.data, b.data);
        } else {
            c.data = _mm256_sub_pd(a.data, b.data);
        }
    }
    return c;
}

}  // namespace intel

#endif // XMMT_XMMT_HPP_INCLUDED

