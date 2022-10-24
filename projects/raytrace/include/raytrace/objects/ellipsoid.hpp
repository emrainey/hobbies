#pragma once

#include "raytrace/objects/quadratic.hpp"

namespace raytrace {
namespace objects {
class ellipsoid : public quadratic {
public:
    ellipsoid(const point &center, element_type a, element_type b, element_type c);
    ~ellipsoid() = default;
};
}  // namespace objects
}  // namespace raytrace
