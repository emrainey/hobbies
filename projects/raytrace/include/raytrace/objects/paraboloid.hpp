#pragma once

#include "raytrace/objects/quadratic.hpp"

namespace raytrace {
namespace objects {
/// Creates a parabolic surface
class paraboloid : public quadratic {
public:
    paraboloid(const point &center, precision a, precision b, precision f);
    ~paraboloid() = default;
};
}  // namespace objects
}  // namespace raytrace
