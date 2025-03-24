#include <raytrace/objects/paraboloid.hpp>

namespace raytrace {
namespace objects {
paraboloid::paraboloid(point const& center, precision a, precision b, precision f) : quadratic{center} {
    m_coefficients = linalg::matrix{{{1.0_p / (a * a), 0.0_p, 0.0_p, 0.0_p},
                                     {0.0_p, 1.0_p / (b * b), 0.0_p, 0.0_p},
                                     {0.0_p, 0.0_p, 0.0_p, -2.0_p * f},
                                     {0.0_p, 0.0_p, -2.0_p * f, 0.0_p}}};
    m_type = Type::Paraboloid;
    m_max_collisions = 2;           // up to 2 collisions
    m_has_infinite_extent = false;  // not a closed surface
}

}  // namespace objects
}  // namespace raytrace
