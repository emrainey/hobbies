#pragma once

#include <geometry/geometry.hpp>
#include <linalg/linalg.hpp>
#if defined(USE_XMMT)
#include <xmmt/xmmt.hpp>
#endif

namespace noise {
using precision = linalg::precision;
#if defined(USE_XMMT)
using point = xmmt::point_<xmmt::double2, 2ul>;
using vector = xmmt::vector_<xmmt::double2, 2ul>;
#else
using point = geometry::point_<2>;
using vector = geometry::vector_<2>;
#endif

#if defined(USE_XMMT)
using point3 = xmmt::point_<xmmt::double3, 3ul>;
using vector3 = xmmt::vector_<xmmt::double3, 3ul>;
#else
using point3 = geometry::point_<3>;
using vector3 = geometry::vector_<3>;
#endif

#if defined(USE_XMMT)
using array = xmmt::point_<xmmt::double2, 2ul>;
using array3 = xmmt::point_<xmmt::double3, 3ul>;
#else
using array = precision[2];
using array3 = precision[3];
#endif
}  // namespace noise
