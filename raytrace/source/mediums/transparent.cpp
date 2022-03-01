#include "raytrace/laws.hpp"
#include "raytrace/mediums/transparent.hpp"

namespace raytrace {

namespace mediums {

transparent::transparent(element_type eta, element_type fade)
    : dielectric()
    , m_fade(basal::clamp(0.0, fade, 1.0))
    {
    m_smoothness = mediums::smoothness::mirror; // no "surface colors"
    m_refractive_index = eta;
}

void transparent::radiosity(
        const raytrace::point& volumetric_point __attribute__((unused)),
        element_type refractive_index,
        const iso::radians& incident_angle,
        const iso::radians& transmitted_angle,
        element_type& emitted,
        element_type& reflected,
        element_type& transmitted
    ) const {
    emitted = 0.0;
    reflected = laws::fresnel(refractive_index, m_refractive_index, incident_angle, transmitted_angle);
    transmitted = 1.0 - reflected;
}

element_type transparent::absorbance(element_type distance) const {
    return std::exp(-distance * m_fade);
}

} // namespace mediums

} // namespace raytrace
