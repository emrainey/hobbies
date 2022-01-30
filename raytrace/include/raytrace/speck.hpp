#pragma once

#include "raytrace/light.hpp"
#include "raytrace/entity.hpp"
#include <basal/printable.hpp>
namespace raytrace {

/** A point light with fall-off obeying the inverse square law */
class speck : public light, public entity {
public:
    speck(const point& P, const raytrace::color& C, element_type intensity);
    speck(point&& P, const raytrace::color& C, element_type intensity);
    virtual ~speck() = default;

    /** @copydoc raytrace::light::intensity_at() */
    element_type intensity_at(const point& world_point) const override;

    /** @copydoc raytrace::light::incident() */
    ray incident(const point& world_point, size_t sample_index) const override;

    /** @copydoc basal::printable::print */
    void print(const char str[]) const override;
};

/** Helper to print specks of light */
std::ostream& operator<<(std::ostream& os, const speck& l);

}
