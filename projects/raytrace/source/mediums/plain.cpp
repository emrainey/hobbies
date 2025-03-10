#include "raytrace/mediums/plain.hpp"

namespace raytrace {

namespace mediums {

plain::plain(color const& amb, precision amb_scale, color const& dif, precision ref, precision rough) : opaque{} {
    m_ambient = amb;
    m_ambient_scale = basal::clamp(0.0_p, amb_scale, 1.0_p);
    m_diffuse = dif;
    m_smoothness = basal::clamp(0.0_p, ref, 1.0_p);
    m_tightness = rough;
}

}  // namespace mediums

}  // namespace raytrace
