#pragma once
#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

/** A perlin noise generated is used to simulate (poorly) a perlin pattern */
class perlin : public opaque {
public:
    perlin(element_type seed, element_type scale, element_type gain, color dark, color light);
    virtual ~perlin() = default;

    color diffuse(const raytrace::point& volumetric_point) const final;

protected:
    color m_dark, m_light;
    noise::vector m_seed;
    element_type m_gain;
    element_type m_scale;
};

}  // namespace mediums

}  // namespace raytrace
