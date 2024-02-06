#pragma once
#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

/// A perlin noise generated is used to simulate (poorly) a perlin pattern
class perlin : public opaque {
public:
    perlin(precision seed, precision scale, precision gain, color dark, color light);
    virtual ~perlin() = default;

    color diffuse(const raytrace::point& volumetric_point) const final;

protected:
    color m_dark, m_light;
    noise::vector m_seed;
    precision m_gain;
    precision m_scale;
};

}  // namespace mediums

}  // namespace raytrace
