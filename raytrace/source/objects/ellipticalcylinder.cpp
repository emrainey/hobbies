#include <raytrace/objects/ellipticalcylinder.hpp>

namespace raytrace {
namespace objects {
ellipticalcylinder::ellipticalcylinder(const point &center, element_type a, element_type b) : quadratic{center} {
    m_coefficients = linalg::matrix{{
        {1.0 / (a * a), 0.0, 0.0, 0.0},
        {0.0, 1.0 / (b * b), 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, -1.0},
    }};
}

}  // namespace objects
}  // namespace raytrace
