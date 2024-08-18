#include "raytrace/lights/speck.hpp"
#include "raytrace/laws.hpp"

namespace raytrace {
namespace lights {
using namespace linalg::operators;

speck::speck(point const& P, color const& C, precision intensity)
    : light(P, C, intensity, 1u, Falloff::InverseSquare) {
}

speck::speck(point&& P, color const& C, precision intensity)
    : light(std::move(P), C, intensity, 1u, Falloff::InverseSquare) {
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
