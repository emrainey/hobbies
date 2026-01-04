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

    precision intensity_at(point const& world_point) const override;
    ray incident(point const& world_point, size_t sample_index) const override;
    void print(std::ostream& os, char const str[]) const override;
    ray emit() override;

protected:
    /// The direction which the light is facing from the location
    raytrace::vector m_direction;
    /// The allowed angles under this limit
    iso::degrees m_incoming_angle;

    std::random_device m_random_device;
    std::mt19937 m_generator;
    constexpr static size_t limit{1024u};
    std::uniform_int_distribution<> m_distribution;
};

}  // namespace lights
}  // namespace raytrace
