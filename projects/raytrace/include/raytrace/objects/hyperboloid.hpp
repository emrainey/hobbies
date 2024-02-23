#pragma once

#include "raytrace/objects/quadratic.hpp"

namespace raytrace {
namespace objects {
/// Creates Hyperbolic surface
class hyperboloid : public quadratic {
public:
    hyperboloid(point const& center, precision a, precision b, precision c);
    ~hyperboloid() = default;
};
}  // namespace objects
}  // namespace raytrace
