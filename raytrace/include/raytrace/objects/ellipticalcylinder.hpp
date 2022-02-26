#pragma once

#include "raytrace/objects/quadratic.hpp"

namespace raytrace {
namespace objects {
class ellipticalcylinder : public quadratic {
public:
    ellipticalcylinder(const point &center, element_type a, element_type b);
    ~ellipticalcylinder() = default;
};
}
} // namespace raytrace
