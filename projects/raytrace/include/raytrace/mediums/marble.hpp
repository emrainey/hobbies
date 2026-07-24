#pragma once

#include "raytrace/mediums/opaque.hpp"
#include "noise/noise.hpp"

namespace raytrace {

namespace mediums {

/// A marble-like texture using turbulent sine noise
class marble : public opaque {
public:
    marble(precision seed, precision scale, precision gain, color dark, color light);
    virtual ~marble() = default;

    color diffuse(raytrace::point const& volumetric_point) const final;

protected:
    color m_dark, m_light;
    noise::vector m_seed;
    precision m_gain;
    precision m_scale;
    noise::pad m_pad;
};

}  // namespace mediums

}  // namespace raytrace
