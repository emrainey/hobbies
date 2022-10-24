#include "raytrace/mediums/turbsin.hpp"

namespace raytrace {

namespace mediums {

turbsin::turbsin(element_type xs, element_type ys, element_type power, element_type size, color dark, color light)
    : opaque{}, m_pad{}, m_dark{dark}, m_light{light}, m_xs{xs}, m_ys{ys}, m_power{power}, m_size{size} {
    m_ambient = colors::white;
    m_ambient_scale = mediums::ambient::none;
    m_smoothness = mediums::smoothness::small;  // moderate polish
    m_tightness = mediums::roughness::tight;
}

color turbsin::diffuse(const raytrace::point& volumetric_point) const {
    if (m_reducing_map) {
        image::point pnt = m_reducing_map(volumetric_point);
        element_type alpha = noise::turbulentsin(pnt, m_xs, m_ys, m_power, m_size, 1.0, m_pad);
        return interpolate(m_light, m_dark, alpha);
    } else {
        // FIXME implement a real volumetric turbulentsin noise function.
        image::point pnt(volumetric_point.x, volumetric_point.y);
        element_type alpha = noise::turbulentsin(pnt, m_xs, m_ys, m_power, m_size, 1.0, m_pad);
        return interpolate(m_light, m_dark, alpha);
    }
}

}  // namespace mediums

}  // namespace raytrace
