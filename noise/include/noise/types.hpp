#pragma once

#include <geometry/geometry.hpp>
#include <linalg/linalg.hpp>
#include <xmmt/xmmt.hpp>

namespace noise {
using element_type = linalg::element_type;
using point = geometry::point_<2>;
using vector = geometry::vector_<element_type, 2>;
}  // namespace noise

namespace noise {
using array = intel::xmmt2;
}
