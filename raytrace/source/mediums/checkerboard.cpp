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
    m_pal[2] = dark;
    m_pal[3] = light;
    m_pal[4] = dark;
    m_pal[5] = light;
    m_pal[6] = dark;
    m_pal[7] = light;
}

checkerboard::checkerboard(element_type repeat,
                           color q1_dark, color q1_light,
                           color q2_dark, color q2_light,
                           color q3_dark, color q3_light,
                           color q4_dark, color q4_light)
    : opaque()
    , m_repeat(repeat)
    , m_pal(8)
    {
        m_ambient = colors::white;
        m_ambient_scale = ambient::none;
        m_smoothness = smoothness::barely;
        m_pal[0] = q1_dark;
        m_pal[1] = q1_light;
        m_pal[2] = q2_dark;
        m_pal[3] = q2_light;
        m_pal[4] = q3_dark;
        m_pal[5] = q3_light;
        m_pal[6] = q4_dark;
        m_pal[7] = q4_light;

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
