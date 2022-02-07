#pragma once
#include "raytrace/mediums/conductor.hpp"

namespace raytrace {

namespace mediums {

/** A sub class of conductors */
class metal : public conductor {
public:
    metal(color diffuse, element_type smoothness, element_type tightness);
    virtual ~metal() = default;

    // metals can alter the specular light given off from them.
    color specular(const raytrace::point& volumetric_point, element_type scaling, const color& light_color) const override;

protected:
    /** The color filter of the specular highlight (used only with metals) */
    color m_specularity;
};

} // namespace mediums

namespace metals {
    const mediums::metal aluminum(colors::aluminum, smoothness::small, roughness::loose);
    const mediums::metal brass(colors::brass, smoothness::polished, roughness::tight);
    const mediums::metal chrome(colors::chrome, smoothness::polished, roughness::tight);
    const mediums::metal copper(colors::copper, smoothness::polished, roughness::tight);
    const mediums::metal steel(colors::steel, smoothness::polished, roughness::tight);
    const mediums::metal tin(colors::tin, smoothness::barely, roughness::loose);
}

} // namespace raytrace
