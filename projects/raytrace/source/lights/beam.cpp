#include "raytrace/lights/beam.hpp"

namespace raytrace {
namespace lights {

using namespace linalg::operators;

beam::beam(const vector& v, const color& C, precision distance)
    : light{C, 1.0, 1}, m_world_source{v}, m_distance{distance} {
}

beam::beam(raytrace::vector&& v, const color& C, precision distance)
    : light{C, 1.0, 1}, m_world_source{std::move(v)}, m_distance{distance} {
}

precision beam::intensity_at(const point& pnt __attribute__((unused))) const {
    return m_intensity;
}

ray beam::incident(const point& world_point, size_t sample_index __attribute__((unused))) const {
    // since the beam source is considered to be ridiculously far away (like the Sun) we have to give the vector large
    // components
    return ray(world_point, -m_distance * m_world_source);
}

void beam::print(const char str[]) const {
    std::cout << str << " beam @" << this << " from: " << m_world_source << ", distance: " << m_distance << " "
              << m_color << std::endl;
}

}  // namespace lights

std::ostream& operator<<(std::ostream& os, const lights::beam& l) {
    os << " beam " << l.incident(geometry::R3::origin, 0) << " " << l.color_at(geometry::R3::origin);
    return os;
}

}  // namespace raytrace
