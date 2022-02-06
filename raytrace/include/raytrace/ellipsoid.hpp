#pragma once

#include "raytrace/quadratic.hpp"

namespace raytrace {

class ellipsoid : public quadratic {
public:
    ellipsoid(const point &center, element_type a, element_type b, element_type c);
    ~ellipsoid() = default;
};

} // namespace raytrace