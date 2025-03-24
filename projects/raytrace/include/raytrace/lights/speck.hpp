#pragma once

#include "raytrace/entity.hpp"
#include "raytrace/lights/light.hpp"

namespace raytrace {

namespace lights {
/// A point light with fall-off obeying the inverse square law
class speck : public light {
public:
    speck(point const& P, raytrace::color const& C, precision intensity);
    speck(point&& P, raytrace::color const& C, precision intensity);
    virtual ~speck() = default;

    ray incident(point const& world_point, size_t sample_index) const override;
    void print(std::ostream& os, char const str[]) const override;
};

}  // namespace lights
}  // namespace raytrace
