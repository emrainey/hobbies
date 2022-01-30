#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

opaque::opaque() : dielectric() {
    m_ambient_scale = ambient::none; // no glow
    m_smoothness = smoothness::none; // all surface color, no reflections
    m_tightness = roughness::tight; // tightish specular blob
    m_transmissivity = 0.0; // all incident light is reflected
}

} // namespace mediums

} // namespace raytrace
