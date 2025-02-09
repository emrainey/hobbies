#pragma once
#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

/// A perlin noise generated is used to simulate (poorly) a perlin pattern
class turbsin : public opaque {
public:
    turbsin(precision xs, precision ys, precision power, precision scale, precision size, color dark, color light);
    virtual ~turbsin() = default;

    color diffuse(raytrace::point const& volumetric_point) const final;

protected:
    noise::pad m_pad;
    color m_dark, m_light;
    precision m_xs, m_ys;
    precision m_power, m_scale, m_size;
};

}  // namespace mediums

}  // namespace raytrace
