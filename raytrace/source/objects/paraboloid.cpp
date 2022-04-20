#include <raytrace/objects/paraboloid.hpp>

namespace raytrace {
namespace objects {
paraboloid::paraboloid(const point &center, element_type a, element_type b, element_type f) : quadratic{center} {
    m_coefficients = linalg::matrix{{{1.0 / (a * a), 0.0, 0.0, 0.0},
                                     {0.0, 1.0 / (b * b), 0.0, 0.0},
                                     {0.0, 0.0, 0.0, -2.0 * f},
                                     {0.0, 0.0, -2.0 * f, 0.0}}};
}

}  // namespace objects
}  // namespace raytrace
