#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

opaque::opaque() : dielectric{} {
    m_ambient_scale = mediums::ambient::none;  // no glow
    m_smoothness = mediums::smoothness::none;  // all surface color, no reflections
    m_tightness = mediums::roughness::tight;   // tight specular blob
    m_reflectivity = 1.0;                      // all incident light is reflected
    m_transmissivity = 0.0;                    // no light is transmitted
}

}  // namespace mediums

}  // namespace raytrace
