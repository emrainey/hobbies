#pragma once

#include "raytrace/quadratic.hpp"

namespace raytrace {

class ellipticalcone : public quadratic {
public:
    ellipticalcone(const point &center, element_type a, element_type b);
    ~ellipticalcone() = default;
};

} // namespace raytrace
