#include <raytrace/objects/ellipsoid.hpp>

namespace raytrace {
namespace objects {
ellipsoid::ellipsoid(const point &center, element_type a, element_type b, element_type c) : quadratic{center} {
    m_coefficients = linalg::matrix{{{1.0 / (a * a), 0.0, 0.0, 0.0},
                                     {0.0, 1.0 / (b * b), 0.0, 0.0},
                                     {0.0, 0.0, 1.0 / (c * c), 0.0},
                                     {0.0, 0.0, 0.0, -1.0}}};
}

}  // namespace objects
}  // namespace raytrace
