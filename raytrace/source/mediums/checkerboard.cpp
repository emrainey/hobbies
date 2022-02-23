#include "raytrace/mediums/checkerboard.hpp"

namespace raytrace {

namespace mediums {

checkerboard::checkerboard(element_type repeat, color dark, color light)
    : opaque()
    , m_repeat(repeat)
    , m_pal(2)
    {
    m_ambient = colors::white;
    m_ambient_scale = ambient::none;
    m_smoothness = smoothness::barely;
    m_pal[0] = dark;
    m_pal[1] = light;
}

color checkerboard::diffuse(const raytrace::point& volumetric_point) const {
    if (m_reducing_map) {
        image::point texture_point = m_reducing_map(volumetric_point) * m_repeat;
        return functions::checkerboard(texture_point, m_pal);
    } else {
        raytrace::point voxel_point = volumetric_point * m_repeat;
        return functions::checkerboard(voxel_point, m_pal);
    }
}

} // namespace mediums

} // namespace raytrace
