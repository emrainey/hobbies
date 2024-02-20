#pragma once

#include "raytrace/entity.hpp"
#include "raytrace/lights/light.hpp"

namespace raytrace {

namespace lights {
/// A point light with fall-off obeying the inverse square law
class speck
    : public light
    , public entity {
public:
    speck(point const& P, raytrace::color const& C, precision intensity);
    speck(point&& P, raytrace::color const& C, precision intensity);
    virtual ~speck() = default;

    /// @copydoc raytrace::light::intensity_at()
    precision intensity_at(point const& world_point) const override;

    /// @copydoc raytrace::light::incident()
    ray incident(point const& world_point, size_t sample_index) const override;

    /// @copydoc basal::printable::print
    void print(char const str[]) const override;
};

}  // namespace lights

/// Helper to print specks of light
std::ostream& operator<<(std::ostream& os, lights::speck const& l);

}  // namespace raytrace
