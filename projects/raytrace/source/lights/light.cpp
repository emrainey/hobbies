#include "raytrace/lights/light.hpp"
#include "raytrace/laws.hpp"

namespace raytrace {
namespace lights {
using namespace linalg::operators;

constexpr static bool use_true_intensity{true};

light::light(point const& position, color const& color, precision intensity, size_t number_of_samples, Falloff falloff)
    : entity{position}, m_color{color}, m_intensity{intensity}, m_samples{number_of_samples}, m_falloff{falloff} {
}

color light::color_at(point const& world_point) const {
    using namespace operators;
    color C;
    precision intensity = intensity_at(world_point);
    if (use_true_intensity) {
        C = m_color;
        // this doesn't clamp
        C.scale(intensity, false);
    } else {
        // caution! this is a clamped intensity, not true intensity
        C = m_color * intensity;
    }
    return C;
}

precision light::intensity_at(point const& world_point) const {
    using namespace geometry::operators;
    if (m_falloff == Falloff::None) {
        return m_intensity;
    } else if (m_falloff == Falloff::Linear) {
        vector direction = position() - world_point;
        precision d = direction.magnitude();
        return m_intensity / (d + 1.0_p); // this prevents division by zero
    } else if (m_falloff == Falloff::InverseSquare) {
        vector direction = position() - world_point;
        precision d = direction.magnitude();
        return m_intensity * laws::inverse_square(d);
    } else {
        return 0.0_p;
    }
}

}  // namespace lights
}  // namespace raytrace
