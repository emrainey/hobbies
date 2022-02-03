#pragma once

#include "raytrace/quadratic.hpp"

namespace raytrace {

class ellipticalcylinder : public quadratic {
public:
    ellipticalcylinder(const point &center, element_type a, element_type b);
    ~ellipticalcylinder() = default;
};

} // namespace raytrace
