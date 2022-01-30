#pragma once
#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

/** A perlin noise generated is used to simulate (poorly) a marble pattern */
class marble : public opaque {
public:
    marble(element_type seed, element_type scale, element_type gain, color dark, color light);
    virtual ~marble() = default;

    color diffuse(const raytrace::point& volumetric_point) const final;
protected:
    color m_dark, m_light;
    noise::vector m_seed;
    element_type m_gain;
    element_type m_scale;
};

} // namespace mediums

} // namespace raytrace
