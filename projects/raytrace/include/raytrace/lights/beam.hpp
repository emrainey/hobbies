#pragma once

#include "raytrace/entity.hpp"
#include "raytrace/lights/light.hpp"

namespace raytrace {
namespace lights {
/// A beam of light, directionalized light to simulate parallel rays from the sun.
class beam : public light {
public:
    /// Constructs a beam of light given the vector of the beam, the color and the intensity
    beam(const raytrace::vector& v, const raytrace::color& C, precision intensity);
    /// Constructs a beam of light given the vector of the beam, the color and the intensity
    beam(raytrace::vector&& v, const raytrace::color& C, precision intensity);
    virtual ~beam() = default;

    /// @copydoc raytrace::light::intensity_at()
    precision intensity_at(const point& world_point) const override;

    /// @copydoc raytrace::light::incident()
    ray incident(const point& world_point, size_t sample_index) const override;

    /// @copydoc basal::printable::print()
    void print(const char str[]) const override;

protected:
    /// The direction which the light is coming from
    raytrace::vector m_world_source;
    /// The emulated distance of the light source
    precision m_distance;
};

}  // namespace lights

/// Helper to print specks of light
std::ostream& operator<<(std::ostream& os, const lights::beam& l);

}  // namespace raytrace
