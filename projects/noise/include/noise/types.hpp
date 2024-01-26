#pragma once

#include <geometry/geometry.hpp>
#include <linalg/linalg.hpp>
#include <xmmt/xmmt.hpp>

namespace noise {
using precision = linalg::precision;
#if defined(USE_XMMT)
using point = intel::point_<intel::double2, 2ul>;
using vector = intel::vector_<intel::double2, 2ul>;
#else
using point = geometry::point_<2>;
using vector = geometry::vector_<precision, 2>;
#endif
}  // namespace noise

namespace noise {
using array = intel::point_<intel::double2, 2ul>;
}
