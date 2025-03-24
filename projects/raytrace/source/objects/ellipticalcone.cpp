#include <raytrace/objects/ellipticalcone.hpp>

#include "basal/ieee754.hpp"
using namespace basal::literals;

namespace raytrace {
namespace objects {
ellipticalcone::ellipticalcone(point const& center, precision a, precision b) : quadratic{center} {
    m_coefficients = linalg::matrix{{{1.0_p / (a * a), 0.0_p, 0.0_p, 0.0_p},
                                     {0.0_p, 1.0_p / (b * b), 0.0_p, 0.0_p},
                                     {0.0_p, 0.0_p, -1.0_p, 0.0_p},
                                     {0.0_p, 0.0_p, 0.0_p, -1.0_p}}};
    m_type = Type::EllipticalCone;
    m_max_collisions = 2;           // up to 2 collisions
    m_has_definite_volume = false;  // not a closed surface
}

}  // namespace objects
}  // namespace raytrace
