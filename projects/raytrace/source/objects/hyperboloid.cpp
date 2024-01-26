#include <raytrace/objects/hyperboloid.hpp>

#include "basal/ieee754.hpp"
using namespace basal::literals;

namespace raytrace {
namespace objects {
hyperboloid::hyperboloid(const point &center, precision a, precision b, precision c) : quadratic{center} {
    m_coefficients = linalg::matrix{{{1.0_p / (a * a), 0.0_p, 0.0_p, 0.0_p},
                                     {0.0_p, -1.0_p / (b * b), 0.0_p, 0.0_p},
                                     {0.0_p, 0.0_p, -1.0_p, 0.0_p},
                                     {0.0_p, 0.0_p, 0.0_p, -1.0_p}}};
}

}  // namespace objects
}  // namespace raytrace
