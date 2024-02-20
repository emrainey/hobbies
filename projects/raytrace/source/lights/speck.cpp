#include "raytrace/lights/speck.hpp"

namespace raytrace {
namespace lights {
using namespace linalg::operators;

speck::speck(point const& P, color const& C, precision intensity) : light(C, intensity, 1), entity(P) {
}

speck::speck(point&& P, color const& C, precision intensity) : light(C, intensity, 1), entity(std::move(P)) {
}

precision speck::intensity_at(point const& pnt) const {
    using namespace geometry::operators;
    vector direction = position() - pnt;
    precision d = direction.magnitude();
    if (basal::nearly_zero(d)) {
        return m_intensity;
    } else {
        return m_intensity / (d * d);
    }
}

ray speck::incident(point const& world_point, size_t sample_index __attribute__((unused))) const {
    return ray(world_point, position() - world_point);
}

void speck::print(char const str[]) const {
    std::cout << str << " speck @" << this << " " << position() << ", " << m_color << std::endl;
}

}  // namespace lights

std::ostream& operator<<(std::ostream& os, lights::speck const& l) {
    os << " speck " << l.position() << ", " << l.color_at(l.position());
    return os;
}

}  // namespace raytrace
