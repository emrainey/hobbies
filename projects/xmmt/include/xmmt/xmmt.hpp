#pragma once

#include "xmmt/point.hpp"
#include "xmmt/vector.hpp"
#include "xmmt/pixel.hpp"

namespace xmmt {

// +================================================================================================+
// Cross-Type operations
// +================================================================================================+

/// A Point minus a point is a vector from the last to the first.
template <typename pack_type, size_t dimensions>
vector_<pack_type, dimensions> operator-(point_<pack_type, dimensions> const& a,
                                         point_<pack_type, dimensions> const& b) {
    vector_<pack_type, dimensions> c{};
    if constexpr (point_<pack_type, dimensions>::number_of_elements == 2) {
        c.data = simde_mm_sub_pd(a.data, b.data);
    } else {
        if constexpr (std::is_same_v<typename pack_type::element_type, float>) {
            c.data = simde_mm_sub_ps(a.data, b.data);
        } else {
            c.data = simde_mm256_sub_pd(a.data, b.data);
        }
    }
    return c;
}

}  // namespace xmmt

