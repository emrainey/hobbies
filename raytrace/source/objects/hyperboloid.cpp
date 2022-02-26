#include <raytrace/objects/hyperboloid.hpp>

namespace raytrace {
namespace objects {
hyperboloid::hyperboloid(const point &center, element_type a, element_type b, element_type c)
    : quadratic(center)
    {
    m_coefficients = linalg::matrix{{
        {1.0/(a*a), 0.0, 0.0, 0.0},
        {0.0,-1.0/(b*b), 0.0, 0.0},
        {0.0, 0.0, -1.0, 0.0},
        {0.0, 0.0, 0.0, -1.0}
    }};
}

} // namespace objects
} // namespace raytrace
