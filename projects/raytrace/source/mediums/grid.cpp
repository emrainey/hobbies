#include "raytrace/mediums/grid.hpp"

namespace raytrace {

namespace mediums {

grid::grid(precision scale, color dark, color light) : opaque{}, m_scale{scale}, m_dark{dark}, m_light{light} {
    m_ambient = colors::white;
    m_ambient_scale = mediums::ambient::dim;
}

color grid::diffuse(raytrace::point const& volumetric_point) const {
    raytrace::point pnt(volumetric_point.x / m_scale, volumetric_point.y / m_scale, volumetric_point.z / m_scale);
    palette pal = {m_dark, m_light};
    if (m_reducing_map) {
        image::point texture_point = m_reducing_map(pnt);
        return functions::grid(texture_point, pal);
    } else {
        return functions::grid(pnt, pal);
    }
}

}  // namespace mediums

}  // namespace raytrace
