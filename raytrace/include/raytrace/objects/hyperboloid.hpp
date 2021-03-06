#pragma once

#include "raytrace/objects/quadratic.hpp"

namespace raytrace {
namespace objects {
/** Creates Hyperbolic surface */
class hyperboloid : public quadratic {
public:
    hyperboloid(const point &center, element_type a, element_type b, element_type c);
    ~hyperboloid() = default;
};
}  // namespace objects
}  // namespace raytrace
