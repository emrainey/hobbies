#include <raytrace/objects/hyperboloid.hpp>

#include "basal/ieee754.hpp"
using namespace basal::literals;

namespace raytrace {
namespace objects {
hyperboloid::hyperboloid(point const& center, precision a, precision b, precision c) : quadratic{center} {
    m_coefficients = linalg::matrix{{{1.0_p / (a * a), 0.0_p, 0.0_p, 0.0_p},
                                     {0.0_p, -1.0_p / (b * b), 0.0_p, 0.0_p},
                                     {0.0_p, 0.0_p, 1.0_p / (c * c), 0.0_p},
                                     {0.0_p, 0.0_p, 0.0_p, -1.0_p}}};
    m_type = Type::Hyperboloid;
    m_max_collisions = 2;           // up to 2 collisions
    m_has_definite_volume = false;  // not a closed surface
}

}  // namespace objects
}  // namespace raytrace
