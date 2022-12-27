#pragma once

#include <geometry/geometry.hpp>
#include <linalg/linalg.hpp>
#include <xmmt/xmmt.hpp>

namespace noise {
using element_type = linalg::element_type;
#if defined(USE_XMMT)
using point = intel::point2;
using vector = intel::vector2;
#else
using point = geometry::point_<2>;
using vector = geometry::vector_<element_type, 2>;
#endif
}  // namespace noise

namespace noise {
using array = intel::point2;
}
