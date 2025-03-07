#pragma once

#include "raytrace/entity.hpp"
#include "raytrace/lights/light.hpp"

namespace raytrace {

namespace lights {
/// A Spot light is a speck (point light) with a limiting angle of incoming ray angle. This still obeys the inverse
/// square law for fall-off. */
class spot : public light {
public:
    spot(raytrace::ray const& r, raytrace::color const& C, precision intensity, iso::degrees const& incoming_angle);
    spot(raytrace::ray&& r, raytrace::color const& C, precision intensity, iso::degrees const& incoming_angle);
    virtual ~spot() = default;

    /// @copydoc raytrace::light::intensity_at()
    precision intensity_at(point const& world_point) const override;

    /// @copydoc raytrace::light::incident()
    ray incident(point const& world_point, size_t sample_index) const override;

    /// @copydoc basal::printable::print
    void print(char const str[]) const override;

protected:
    /// The direction which the light is facing from the location
    raytrace::vector m_direction;
    /// The allowed angles under this limit
    iso::degrees m_incoming_angle;
};

}  // namespace lights

/// Helper to print specks of light
std::ostream& operator<<(std::ostream& os, lights::spot const& l);

}  // namespace raytrace
