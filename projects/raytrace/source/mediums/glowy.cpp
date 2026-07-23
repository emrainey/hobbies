#include "raytrace/mediums/glowy.hpp"

namespace raytrace {
namespace mediums {

glowy::glowy(color const& ambient, color const& emissive, precision smoothness, precision roughness) : opaque{} {
    m_ambient = ambient;
    m_ambient_scale = mediums::ambient::none;
    m_diffuse = colors::black;
    m_smoothness = smoothness;
    m_tightness = roughness;
    m_emissive_color = emissive;
}

color glowy::emissive(raytrace::point const& volumetric_point __attribute__((unused))) const {
    return m_emissive_color;
}

}  // namespace mediums
}  // namespace raytrace
