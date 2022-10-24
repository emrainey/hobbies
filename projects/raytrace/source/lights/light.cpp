#include "raytrace/lights/light.hpp"

namespace raytrace {
namespace lights {
using namespace linalg::operators;

light::light(const color& C, element_type intensity, size_t number_of_samples)
    : m_color{C}, m_intensity{intensity}, m_samples{number_of_samples} {
}

color light::color_at(const point& world_point) const {
    using namespace operators;
    color C = m_color * intensity_at(world_point);
    return C;
}

}  // namespace lights
}  // namespace raytrace
