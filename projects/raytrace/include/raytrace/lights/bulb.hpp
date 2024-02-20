#pragma once

#include "raytrace/entity.hpp"
#include "raytrace/lights/light.hpp"

namespace raytrace {

namespace lights {

/// A uniform diffuse light surface with fall-off obeying the inverse square law
class bulb
    : public light
    , public entity {
public:
    bulb(point const& P, precision radius, raytrace::color const& C, precision intensity, size_t samples);
    bulb(point&& P, precision radius, raytrace::color const& C, precision intensity, size_t samples);
    virtual ~bulb() = default;

    /// @copydoc raytrace::light::intensity_at()
    precision intensity_at(point const& world_point) const override;

    /// @copydoc raytrace::light::incident()
    ray incident(point const& world_point, size_t sample_index) const override;

    /// @copydoc basal::printable::print
    void print(char const str[]) const override;

protected:
    precision const m_radius;
};

}  // namespace lights

/// Helper to print bulbs of light
std::ostream& operator<<(std::ostream& os, lights::bulb const& l);

}  // namespace raytrace
