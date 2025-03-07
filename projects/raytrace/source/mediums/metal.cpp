#include "raytrace/mediums/metal.hpp"

namespace raytrace {

namespace mediums {

metal::metal(color diffuse, precision smoothness, precision tightness, precision fuzz_scale)
    : conductor{},
      m_specularity{diffuse}  // metals (conductors} can alter the color of the light
      ,
      m_fuzz_scale{basal::clamp(0.0, fuzz_scale, 1.0)} {
    m_smoothness = basal::clamp(0.0, smoothness, 1.0);
    m_diffuse = diffuse;
    m_tightness = tightness;
}

color metal::specular(raytrace::point const& volumetric_point, precision scaling, color const& light_color) const {
    using namespace operators;
    // metals can alter the incoming light in specular highlights
    // we multiply by the color to emulate surfaces which would reject the light color by being a different color.
    // A red light on a blue metal surface should not have *any* specular highlights!
    color new_spec = m_specularity * light_color;
    return std::pow(scaling, specular_tightness(volumetric_point)) * new_spec;
}

raytrace::vector metal::perturbation(raytrace::point const& volumetric_point __attribute__((unused))) const {
    if (m_fuzz_scale > 0) {
        size_t denom = 79;
        size_t num = rand() % denom;
        return (mapping::golden_ratio_mapper(num, denom) - geometry::R3::origin) * m_fuzz_scale;
    } else {
        return geometry::R3::null;
    }
}

}  // namespace mediums

}  // namespace raytrace
