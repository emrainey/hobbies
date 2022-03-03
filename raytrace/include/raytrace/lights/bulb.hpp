#pragma once

#include "raytrace/entity.hpp"
#include "raytrace/lights/light.hpp"

namespace raytrace {

namespace lights {

/** A uniform diffuse light surface with fall-off obeying the inverse square law */
class bulb
    : public light
    , public entity {
public:
    bulb(const point& P, element_type radius, const raytrace::color& C, element_type intensity, size_t samples);
    bulb(point&& P, element_type radius, const raytrace::color& C, element_type intensity, size_t samples);
    virtual ~bulb() = default;

    /** @copydoc raytrace::light::intensity_at() */
    element_type intensity_at(const point& world_point) const override;

    /** @copydoc raytrace::light::incident() */
    ray incident(const point& world_point, size_t sample_index) const override;

    /** @copydoc basal::printable::print */
    void print(const char str[]) const override;

protected:
    const element_type m_radius;
};

}  // namespace lights

/** Helper to print bulbs of light */
std::ostream& operator<<(std::ostream& os, const lights::bulb& l);

}  // namespace raytrace
