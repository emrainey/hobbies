#pragma once
#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

/// A perlin noise generated is used to simulate (poorly) a perlin pattern
class turbsin : public opaque {
public:
    /// @brief The turbulent sin constructor
    /// @param xs X Scale factor
    /// @param ys Y Scale Factor
    /// @param power The power of the turbulence (iterations)
    /// @param scale The final output is multiplied by this scale for output. Use carefully for pixels.
    /// @param size The power of 2 of the size of features in the turbulence.
    /// @param dark The dark color
    /// @param light The light color
    turbsin(precision map2d_scale, precision xs, precision ys, precision power, precision scale, precision size, color dark, color light);
    virtual ~turbsin() = default;

    color diffuse(raytrace::point const& volumetric_point) const final;

protected:
    noise::pad m_pad;
    color m_dark, m_light;
    precision m_xs, m_ys;
    precision m_power, m_scale, m_size;
    precision m_2dscale;
};

}  // namespace mediums

}  // namespace raytrace
