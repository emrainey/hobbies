#include "raytrace/mediums/turbsin.hpp"

namespace raytrace {

namespace mediums {

turbsin::turbsin(precision map2d_scale, precision xs, precision ys, precision power, precision size, precision scale, color dark, color light)
    : opaque{}, m_pad{}, m_dark{dark}, m_light{light}, m_xs{xs}, m_ys{ys}, m_power{power}, m_scale{scale}, m_size{size}, m_2dscale{map2d_scale} {
    m_ambient = colors::white;
    m_ambient_scale = mediums::ambient::none;
    m_smoothness = mediums::smoothness::small;  // moderate polish
    m_tightness = mediums::roughness::tight;
}

color turbsin::diffuse(raytrace::point const& volumetric_point) const {
    if (m_reducing_map) {
        image::point pnt = m_reducing_map(volumetric_point);
        // the reduced map may be normal space (0 - 1) and may need some stretching to look good
        pnt.x *= m_2dscale;
        pnt.y *= m_2dscale;
        precision alpha = noise::turbulentsin(pnt, m_xs, m_ys, m_power, m_size, m_scale, m_pad);
        return interpolate(m_light, m_dark, alpha);
    } else {
        // FIXME implement a real volumetric turbulentsin noise function.
        image::point pnt(volumetric_point.x, volumetric_point.y);
        precision alpha = noise::turbulentsin(pnt, m_xs, m_ys, m_power, m_size, m_scale, m_pad);
        return interpolate(m_light, m_dark, alpha);
    }
}

}  // namespace mediums

}  // namespace raytrace
