#include "raytrace/mediums/transparent.hpp"

#include "raytrace/laws.hpp"

namespace raytrace {

namespace mediums {

transparent::transparent(element_type eta, element_type fade, const color& diffuse)
    : dielectric(), m_fade(basal::clamp(0.0, fade, 1.0)) {
    m_diffuse = diffuse;
    m_smoothness = mediums::smoothness::mirror;  // no "surface colors"
    m_refractive_index = eta;
}

void transparent::radiosity(const raytrace::point& volumetric_point __attribute__((unused)),
                            element_type refractive_index, const iso::radians& incident_angle,
                            const iso::radians& transmitted_angle, element_type& emitted, element_type& reflected,
                            element_type& transmitted) const {
    emitted = 0.0;
    reflected = laws::fresnel(refractive_index, m_refractive_index, incident_angle, transmitted_angle);
    transmitted = 1.0 - reflected;
}

color transparent::absorbance(element_type distance, const color& given_color) const {
    using namespace operators;
    if (std::isinf(distance)) {
        return given_color * m_diffuse;
    }
    element_type dropoff = std::exp(-distance * m_fade);
    return given_color * interpolate(colors::white, m_diffuse, dropoff);
}

}  // namespace mediums

}  // namespace raytrace
