#include "raytrace/mediums/checkerboard.hpp"

namespace raytrace {

namespace mediums {

checkerboard::checkerboard(element_type r, color dark, color light)
    : opaque()
    , m_repeat(r)
    , m_dark(dark)
    , m_light(light)
    {
    m_ambient = colors::white;
    m_ambient_scale = ambient::none;
    m_smoothness = smoothness::barely;
}

color checkerboard::diffuse(const raytrace::point& volumetric_point) const {
    palette pal = {m_dark, m_light};
    if (m_reducing_map) {
        image::point texture_point = m_reducing_map(volumetric_point);
        texture_point.x *= m_repeat;
        texture_point.y *= m_repeat;
        return functions::checkerboard(texture_point, pal);
    } else {
        return functions::checkerboard(volumetric_point, pal);
    }
}

} // namespace mediums

} // namespace raytrace
