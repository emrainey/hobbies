#include <raytrace/objects/ellipsoid.hpp>

namespace raytrace {
namespace objects {
ellipsoid::ellipsoid(point const& center, precision a, precision b, precision c) : quadratic{center} {
    m_coefficients = linalg::matrix{{{1.0_p / (a * a), 0.0_p, 0.0_p, 0.0_p},
                                     {0.0_p, 1.0_p / (b * b), 0.0_p, 0.0_p},
                                     {0.0_p, 0.0_p, 1.0_p / (c * c), 0.0_p},
                                     {0.0_p, 0.0_p, 0.0_p, -1.0_p}}};
    m_type = Type::Ellipsoid;
    m_max_collisions = 2;          // up to 2 collisions
    m_has_infinite_extent = true;  // closed surface
}

precision ellipsoid::get_object_extent(void) const {
    precision a = 1.0_p / m_coefficients[0][0];
    precision b = 1.0_p / m_coefficients[1][1];
    precision c = 1.0_p / m_coefficients[2][2];
    return std::max(a, std::max(b, c));
}

}  // namespace objects
}  // namespace raytrace
