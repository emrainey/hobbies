#pragma once

#include "raytrace/lights/light.hpp"
#include "raytrace/entity.hpp"
#include <basal/printable.hpp>
namespace raytrace {

/** A Spot light is a speck (point light) with a limiting angle of incoming ray angle. This still obeys the inverse square law for fall-off. */
class spot : public light, public entity {
public:
    spot(const raytrace::ray& r, const raytrace::color& C, element_type intensity, const iso::degrees& incoming_angle);
    spot(raytrace::ray&& r, const raytrace::color& C, element_type intensity, const iso::degrees& incoming_angle);
    virtual ~spot() = default;

    /** @copydoc raytrace::light::intensity_at() */
    element_type intensity_at(const point& world_point) const override;

    /** @copydoc raytrace::light::incident() */
    ray incident(const point& world_point, size_t sample_index) const override;

    /** @copydoc basal::printable::print */
    void print(const char str[]) const override;

protected:
    /** The direction which the light is facing from the location */
    raytrace::vector m_direction;
    /** The allowed angles under this limit */
    iso::degrees m_incoming_angle;
};

/** Helper to print specks of light */
std::ostream& operator<<(std::ostream& os, const spot& l);

}
