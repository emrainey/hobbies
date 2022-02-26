#include "raytrace/mediums/dots.hpp"

namespace raytrace {

namespace mediums {

dots::dots(element_type r, color dot, color background)
    : opaque()
    , m_repeat(r)
    , m_dot(dot)
    , m_background(background)
    {
    m_ambient = colors::white;
    m_ambient_scale = mediums::ambient::dim;
}

color dots::diffuse(const raytrace::point& volumetric_point) const {
    palette pal = {m_background, m_dot};
    if (m_reducing_map) {
        image::point texture_point = m_reducing_map(volumetric_point);
        texture_point.x *= m_repeat;
        texture_point.y *= m_repeat;
        return functions::dots(texture_point, pal);
    } else {
        return functions::dots(volumetric_point, pal);
    }
}

} // namespace mediums

} // namespace raytrace
