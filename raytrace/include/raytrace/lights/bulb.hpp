#pragma once

#include "raytrace/lights/light.hpp"
#include "raytrace/entity.hpp"
#include <basal/printable.hpp>
namespace raytrace {

/** A uniform diffuse light surface with fall-off obeying the inverse square law */
class bulb : public light, public entity {
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

/** Helper to print bulbs of light */
std::ostream& operator<<(std::ostream& os, const bulb& l);

}
