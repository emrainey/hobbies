#include "raytrace/mediums/transparent.hpp"

#include "raytrace/laws.hpp"

namespace raytrace {

namespace mediums {

transparent::transparent(precision eta, precision fade, color const& diffuse)
    : dielectric{}, m_fade{basal::clamp(0.0_p, fade, 1.0_p)} {
    m_diffuse = diffuse;
    m_smoothness = mediums::smoothness::perfect_mirror;  // no "surface colors"
    m_refractive_index = eta;
}

void transparent::radiosity(raytrace::point const& volumetric_point __attribute__((unused)), precision refractive_index,
                            iso::radians const& incident_angle, iso::radians const& transmitted_angle,
                            precision& emitted, precision& reflected, precision& transmitted) const {
    emitted = 0.0_p;
    reflected = laws::fresnel(refractive_index, m_refractive_index, incident_angle, transmitted_angle);
    transmitted = 1.0_p - reflected;
}

color transparent::absorbance(precision distance, color const& given_color) const {
    using namespace operators;
    if (std::isinf(distance)) {
        return given_color * m_diffuse;
    }
    precision dropoff = std::exp(-distance * m_fade);
    return given_color * fourcc::linear::interpolate(colors::white, m_diffuse, dropoff);
}

}  // namespace mediums

}  // namespace raytrace
