#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

opaque::opaque() : dielectric{} {
    m_ambient_scale = mediums::ambient::none;  // no glow
    m_smoothness = mediums::smoothness::none;  // all surface color, no reflections
    m_tightness = mediums::roughness::tight;   // tightish specular blob
    m_transmissivity = 0.0;                    // all incident light is reflected
}

}  // namespace mediums

}  // namespace raytrace
