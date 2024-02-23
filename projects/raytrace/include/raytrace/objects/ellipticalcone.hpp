#pragma once

#include "raytrace/objects/quadratic.hpp"

namespace raytrace {
namespace objects {
class ellipticalcone : public quadratic {
public:
    ellipticalcone(point const& center, precision a, precision b);
    ~ellipticalcone() = default;
};
}  // namespace objects
}  // namespace raytrace
