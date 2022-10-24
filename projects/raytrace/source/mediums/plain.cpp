#include "raytrace/mediums/plain.hpp"

namespace raytrace {

namespace mediums {

plain::plain(const color& amb, element_type amb_scale, const color& dif, element_type ref, element_type rough)
    : opaque{} {
    m_ambient = amb;
    m_ambient_scale = basal::clamp(0.0, amb_scale, 1.0);
    m_diffuse = dif;
    m_smoothness = basal::clamp(0.0, ref, 1.0);
    m_tightness = rough;
}

}  // namespace mediums

}  // namespace raytrace
