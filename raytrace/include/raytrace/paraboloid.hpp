#pragma once

#include "raytrace/quadratic.hpp"

namespace raytrace {

/** Creates a parabolic surface */
class paraboloid : public quadratic {
public:
    paraboloid(const point &center, element_type a, element_type b, element_type f);
    ~paraboloid() = default;
};

} // namespace raytrace