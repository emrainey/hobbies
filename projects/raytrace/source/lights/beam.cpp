#include "raytrace/lights/beam.hpp"

namespace raytrace {
namespace lights {

using namespace linalg::operators;

static constexpr precision default_distance{1E16};

beam::beam(vector const& v, color const& C, precision intensity)
    : light{R3::origin, C, intensity, 1u, Falloff::None}, m_world_source{v}, m_distance{default_distance} {
}

beam::beam(raytrace::vector&& v, color const& C, precision intensity)
    : light{R3::origin, C, intensity, 1u, Falloff::None}, m_world_source{std::move(v)}, m_distance{default_distance} {
}

ray beam::incident(point const& world_point, size_t sample_index __attribute__((unused))) const {
    // since the beam source is considered to be ridiculously far away (like the Sun) we have to give the vector large
    // components
    return ray(world_point, -m_distance * m_world_source);
}

void beam::print(std::ostream& os, char const str[]) const {
    os << str << " beam @" << this << " from: " << m_world_source << ", distance: " << m_distance << " " << m_color
       << std::endl;
}

}  // namespace lights
}  // namespace raytrace
