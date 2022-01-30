#include "raytrace/mediums/metal.hpp"

namespace raytrace {

namespace mediums {

metal::metal(color diffuse, element_type smoothness, element_type tightness)
    : conductor()
    , m_specularity(colors::white) {
    m_smoothness = smoothness;
    m_diffuse = diffuse;
    m_tightness = tightness;
}

color metal::specular(const raytrace::point& volumetric_point, element_type scaling, const color& light_color) const {
    using namespace operators;
    // metals can alter the incoming light in specular highlights
    // we multiply by the color to emulate surfaces which would reject the light color by being a different color.
    // A red light on a blue metal surface should not have *any* specular highlights!
    color new_spec = m_specularity * light_color;
    return std::pow(scaling, specular_tightness(volumetric_point)) * new_spec;
}

} // namespace mediums

} // namespace raytrace
