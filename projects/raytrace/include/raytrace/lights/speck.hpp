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
    speck(const point& P, const raytrace::color& C, precision intensity);
    speck(point&& P, const raytrace::color& C, precision intensity);
    virtual ~speck() = default;

    /// @copydoc raytrace::light::intensity_at()
    precision intensity_at(const point& world_point) const override;

    /// @copydoc raytrace::light::incident()
    ray incident(const point& world_point, size_t sample_index) const override;

    /// @copydoc basal::printable::print
    void print(const char str[]) const override;
};

}  // namespace lights

/// Helper to print specks of light
std::ostream& operator<<(std::ostream& os, const lights::speck& l);

}  // namespace raytrace
