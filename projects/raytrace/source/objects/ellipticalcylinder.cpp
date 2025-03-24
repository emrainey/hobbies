#include <raytrace/objects/ellipticalcylinder.hpp>

namespace raytrace {
namespace objects {
ellipticalcylinder::ellipticalcylinder(point const& center, precision a, precision b) : quadratic{center} {
    basal::exception::throw_if(a <= 0.0_p, __FILE__, __LINE__, "Semi-axis a must be positive");
    basal::exception::throw_if(b <= 0.0_p, __FILE__, __LINE__, "Semi-axis b must be positive");
    m_coefficients = linalg::matrix{{
        {1.0_p / (a * a), 0.0_p, 0.0_p, 0.0_p},
        {0.0_p, 1.0_p / (b * b), 0.0_p, 0.0_p},
        {0.0_p, 0.0_p, 0.0_p, 0.0_p},
        {0.0_p, 0.0_p, 0.0_p, -1.0_p},
    }};
    m_type = Type::EllipticalCylinder;
    m_max_collisions = 2;           // up to 2 collisions
    m_has_infinite_extent = false;  // not a closed surface
}

}  // namespace objects
}  // namespace raytrace
