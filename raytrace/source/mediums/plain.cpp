#include "raytrace/mediums/plain.hpp"

namespace raytrace {

namespace mediums {

plain::plain(const color& amb, element_type amb_scale, const color& dif, element_type ref, element_type rough)
    : opaque()
    {
    m_ambient = amb;
    m_ambient_scale = amb_scale;
    m_diffuse = dif;
    m_smoothness = ref;
    m_tightness = rough;
}

} // namespace mediums

} // namespace raytrace
