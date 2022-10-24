#include "raytrace/mediums/perlin.hpp"

namespace raytrace {

namespace mediums {

perlin::perlin(element_type seed, element_type scale, element_type gain, color dark, color light)
    : opaque{}, m_dark{dark}, m_light{light}, m_seed{}, m_gain{gain}, m_scale{scale} {
    m_ambient = colors::white;
    m_ambient_scale = mediums::ambient::none;
    m_smoothness = mediums::smoothness::small;  // moderate polish
    m_tightness = mediums::roughness::tight;
    m_seed = noise::convert_to_seed(iso::radians{seed});
}

color perlin::diffuse(const raytrace::point& volumetric_point) const {
    if (m_reducing_map) {
        image::point pnt = m_reducing_map(volumetric_point);
        element_type alpha = noise::perlin(pnt, m_scale, m_seed, m_gain);
        return interpolate(m_dark, m_light, alpha);
    } else {
        // FIXME implement a real volumetric perlin noise function.
        image::point pnt(volumetric_point.x, volumetric_point.y);
        element_type alpha = noise::perlin(pnt, m_scale, m_seed, m_gain);
        return interpolate(m_dark, m_light, alpha);
    }
}

}  // namespace mediums

}  // namespace raytrace
