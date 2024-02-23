#include <raytrace/objects/paraboloid.hpp>

namespace raytrace {
namespace objects {
paraboloid::paraboloid(point const& center, precision a, precision b, precision f) : quadratic{center} {
    m_coefficients = linalg::matrix{{{1.0_p / (a * a), 0.0_p, 0.0_p, 0.0_p},
                                     {0.0_p, 1.0_p / (b * b), 0.0_p, 0.0_p},
                                     {0.0_p, 0.0_p, 0.0_p, -2.0_p * f},
                                     {0.0_p, 0.0_p, -2.0_p * f, 0.0_p}}};
}

}  // namespace objects
}  // namespace raytrace
