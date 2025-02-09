#include "raytrace/mediums/stripes.hpp"

namespace raytrace {

namespace mediums {

stripes::stripes(precision repeat, color dark, color light) : opaque{}, m_repeat{repeat}, m_pal{2} {
    m_ambient = colors::white;
    m_ambient_scale = mediums::ambient::none;
    m_smoothness = mediums::smoothness::barely;
    m_pal[0] = dark;
    m_pal[1] = light;
}

color stripes::diffuse(raytrace::point const& volumetric_point) const {
    if (m_reducing_map) {
        image::point texture_point = m_reducing_map(volumetric_point) * m_repeat;
        return functions::diagonal(texture_point, m_pal);
    } else {
        raytrace::point voxel_point = volumetric_point * m_repeat;
        return functions::diagonal(voxel_point, m_pal);
    }
}

}  // namespace mediums

}  // namespace raytrace
