#pragma once

#include "raytrace/objects/quadratic.hpp"

namespace raytrace {
namespace objects {
class ellipticalcylinder : public quadratic {
public:
    ellipticalcylinder(const point &center, precision a, precision b);
    ~ellipticalcylinder() = default;
};
}  // namespace objects
}  // namespace raytrace
