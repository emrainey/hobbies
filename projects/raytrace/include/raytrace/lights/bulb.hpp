#pragma once

#include "raytrace/entity.hpp"
#include "raytrace/lights/light.hpp"

namespace raytrace {

namespace lights {

/// A uniform diffuse light surface with fall-off obeying the inverse square law
class bulb : public light {
public:
    bulb(point const& P, precision radius, raytrace::color const& C, precision intensity, size_t samples);
    bulb(point&& P, precision radius, raytrace::color const& C, precision intensity, size_t samples);
    virtual ~bulb() = default;

    ray incident(point const& world_point, size_t sample_index) const override;
    void print(std::ostream& os, char const str[]) const override;

protected:
    /// The radius of the bulb
    precision const m_radius;
};

}  // namespace lights
}  // namespace raytrace
