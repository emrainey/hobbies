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
using vector = geometry::vector_<2>;
#endif
}  // namespace noise

namespace noise {
#if defined(USE_XMMT)
using array = intel::point_<intel::double2, 2ul>;
#else
using array = precision[2];
#endif
}  // namespace noise
