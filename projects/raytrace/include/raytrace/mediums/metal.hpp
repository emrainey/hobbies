#pragma once
#include "raytrace/mediums/conductor.hpp"

namespace raytrace {

namespace mediums {

/// A sub class of conductors
class metal : public conductor {
public:
    metal(color diffuse, precision smoothness, precision tightness, precision fuzz_scale = 0.0_p);
    virtual ~metal() = default;

    // metals can alter the specular light given off from them.
    color specular(raytrace::point const& volumetric_point, precision scaling, color const& light_color) const override;

    vector perturbation(raytrace::point const& volumetric_point) const override;

protected:
    /// The color filter of the specular highlight (used only with metals)
    color m_specularity;
    /// The scale o perturbations in the surface
    precision m_fuzz_scale;
};

namespace metals {
metal const aluminum(colors::aluminum, smoothness::small, roughness::loose);
metal const brass(colors::brass, smoothness::polished - 0.1_p, roughness::tight);
metal const bronze(colors::bronze, smoothness::small, roughness::loose);
metal const chrome(colors::chrome, smoothness::polished + 0.2_p, roughness::tight);
metal const copper(colors::copper, smoothness::polished + 0.2_p, roughness::tight);
metal const gold(colors::gold, smoothness::polished, roughness::tight);
metal const silver(colors::silver, smoothness::polished + 0.2_p, roughness::tight);
metal const stainless(colors::stainless, smoothness::polished + 0.2_p, roughness::tight);
metal const steel(colors::steel, smoothness::polished, roughness::tight);
metal const tin(colors::tin, smoothness::barely, roughness::loose);
}  // namespace metals

}  // namespace mediums

}  // namespace raytrace
