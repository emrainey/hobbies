#include "raytrace/mediums/metal.hpp"

namespace raytrace {

namespace mediums {

metal::metal(color diffuse, precision smoothness, precision tightness, precision fuzz_scale)
    : conductor{}
    , m_specularity{diffuse}  // metals (conductors} can alter the color of the light
    , m_fuzz_scale{basal::clamp(0.0_p, fuzz_scale, 1.0_p)} {
    m_smoothness = basal::clamp(0.0_p, smoothness, 1.0_p);
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
    if (m_fuzz_scale > 0.0_p) {
        size_t denom = 79;
        int r = rand();  // [0, 32767]
        size_t num = static_cast<size_t>(r) % denom;
        auto u = mapping::golden_ratio_mapper(num, denom);
        auto v = u - geometry::R3::origin;
        auto p = v * m_fuzz_scale;
        return p;
    } else {
        return geometry::R3::null;
    }
}

}  // namespace mediums

}  // namespace raytrace
