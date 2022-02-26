#pragma once
#include "raytrace/mediums/conductor.hpp"

namespace raytrace {

namespace mediums {

/** A sub class of conductors */
class metal : public conductor {
public:
    metal(color diffuse, element_type smoothness, element_type tightness, element_type fuzz_scale = 0.0);
    virtual ~metal() = default;

    // metals can alter the specular light given off from them.
    color specular(const raytrace::point& volumetric_point, element_type scaling, const color& light_color) const override;

    vector perturbation(const raytrace::point& volumetric_point) const override;
protected:
    /** The color filter of the specular highlight (used only with metals) */
    color m_specularity;
    /** The scale o perturbations in the surface */
    element_type m_fuzz_scale;
};

namespace metals {
    const metal aluminum(colors::aluminum, smoothness::small, roughness::loose);
    const metal brass(colors::brass, smoothness::polished - 0.1, roughness::tight);
    const metal bronze(colors::bronze, smoothness::small, roughness::loose);
    const metal chrome(colors::chrome, smoothness::polished + 0.2, roughness::tight);
    const metal copper(colors::copper, smoothness::polished + 0.2, roughness::tight);
    const metal gold(colors::gold, smoothness::polished, roughness::tight);
    const metal silver(colors::silver, smoothness::polished + 0.2, roughness::tight);
    const metal stainless(colors::stainless, smoothness::polished + 0.2, roughness::tight);
    const metal steel(colors::steel, smoothness::polished, roughness::tight);
    const metal tin(colors::tin, smoothness::barely, roughness::loose);
}

} // namespace mediums

} // namespace raytrace
