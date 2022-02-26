#include "raytrace/lights/speck.hpp"

namespace raytrace {
namespace lights {
using namespace linalg::operators;

speck::speck(const point& P, const color& C, element_type intensity)
    : light(C, intensity, 1)
    , entity(P)
    {}

speck::speck(point&& P, const color& C, element_type intensity)
    : light(C, intensity, 1)
    , entity(std::move(P))
    {}

element_type speck::intensity_at(const point& pnt) const {
    using namespace geometry::operators;
    vector direction = position() - pnt;
    element_type d = direction.magnitude();
    if (basal::equals_zero(d)) {
        return m_intensity;
    } else {
        return m_intensity / (d * d);
    }
}

ray speck::incident(const point& world_point, size_t sample_index __attribute__((unused))) const {
    return ray(world_point, position() - world_point);
}

void speck::print(const char str[]) const {
    std::cout << str << " speck @" << this << " " << position() << ", " << m_color << std::endl;
}

} // namespace lights

std::ostream& operator<<(std::ostream& os, const lights::speck& l) {
    os << " speck " << l.position() << ", " << l.color_at(l.position());
    return os;
}

} // namespace raytrace
