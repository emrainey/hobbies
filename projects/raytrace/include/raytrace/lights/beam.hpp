#pragma once

#include "raytrace/entity.hpp"
#include "raytrace/lights/light.hpp"

namespace raytrace {
namespace lights {
/// A beam of light, directionalized light to simulate parallel rays from the sun.
class beam : public light {
public:
    /// Constructs a beam of light given the vector of the beam, the color and the intensity
    beam(raytrace::vector const& v, raytrace::color const& C, precision intensity);
    /// Constructs a beam of light given the vector of the beam, the color and the intensity
    beam(raytrace::vector&& v, raytrace::color const& C, precision intensity);
    virtual ~beam() = default;

    ray incident(point const& world_point, size_t sample_index) const override;
    void print(std::ostream& os, char const str[]) const override;

protected:
    /// The direction which the light is coming from
    raytrace::vector m_world_source;
    /// The emulated distance of the light source
    precision m_distance;
};

}  // namespace lights
}  // namespace raytrace
