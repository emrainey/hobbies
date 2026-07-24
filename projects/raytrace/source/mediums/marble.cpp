#include "raytrace/mediums/marble.hpp"

namespace raytrace {

namespace mediums {

marble::marble(precision seed, precision scale, precision gain, color dark, color light)
    : opaque{}, m_dark{dark}, m_light{light}, m_seed{}, m_gain{gain}, m_scale{scale}, m_pad{} {
    m_ambient = colors::white;
    m_ambient_scale = mediums::ambient::none;
    m_smoothness = mediums::smoothness::small;
    m_tightness = mediums::roughness::tight;
    m_seed = noise::convert_to_seed(iso::radians{seed});
}

color marble::diffuse(raytrace::point const& volumetric_point) const {
    precision xs = 4.0_p;
    precision ys = 4.0_p;
    precision power = m_gain;
    precision size = 64.0_p;
    if (m_reducing_map) {
        auto map_pnt = m_reducing_map(volumetric_point);
        noise::point pnt = noise::point{map_pnt.x(), map_pnt.y()};
        pnt.x() *= m_scale;
        pnt.y() *= m_scale;
        precision alpha = noise::turbulentsin(pnt, xs, ys, power, size, 1.0_p, m_pad);
        return fourcc::linear::interpolate(m_dark, m_light, alpha);
    } else {
        noise::point pnt(volumetric_point.x(), volumetric_point.y());
        pnt.x() *= m_scale;
        pnt.y() *= m_scale;
        precision alpha = noise::turbulentsin(pnt, xs, ys, power, size, 1.0_p, m_pad);
        return fourcc::linear::interpolate(m_dark, m_light, alpha);
    }
}

}  // namespace mediums

}  // namespace raytrace
