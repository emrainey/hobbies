#include <raytrace/objects/ellipsoid.hpp>

namespace raytrace {
namespace objects {
ellipsoid::ellipsoid(const point &center, element_type a, element_type b, element_type c) : quadratic{center} {
    m_coefficients = linalg::matrix{{{1.0 / (a * a), 0.0, 0.0, 0.0},
                                     {0.0, 1.0 / (b * b), 0.0, 0.0},
                                     {0.0, 0.0, 1.0 / (c * c), 0.0},
                                     {0.0, 0.0, 0.0, -1.0}}};
}

element_type ellipsoid::get_object_extant(void) const {
    element_type a = 1.0 / m_coefficients[0][0];
    element_type b = 1.0 / m_coefficients[1][1];
    element_type c = 1.0 / m_coefficients[2][2];
    return std::max(a, std::max(b, c));
}

}  // namespace objects
}  // namespace raytrace
