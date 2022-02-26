#pragma once

#include "raytrace/objects/quadratic.hpp"

namespace raytrace {
namespace objects {
/** Creates a parabolic surface */
class paraboloid : public quadratic {
public:
    paraboloid(const point &center, element_type a, element_type b, element_type f);
    ~paraboloid() = default;
};
}
} // namespace raytrace
