#pragma once
#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

/** A perlin noise generated is used to simulate (poorly) a perlin pattern */
class turbsin : public opaque {
public:
    turbsin(element_type xs, element_type ys, element_type power, element_type size, color dark, color light);
    virtual ~turbsin() = default;

    color diffuse(const raytrace::point& volumetric_point) const final;

protected:
    noise::pad m_pad;
    color m_dark, m_light;
    element_type m_xs, m_ys;
    element_type m_power, m_size;
};

}  // namespace mediums

}  // namespace raytrace
