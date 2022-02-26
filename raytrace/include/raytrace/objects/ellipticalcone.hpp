#pragma once

#include "raytrace/objects/quadratic.hpp"

namespace raytrace {
namespace objects {
class ellipticalcone : public quadratic {
public:
    ellipticalcone(const point &center, element_type a, element_type b);
    ~ellipticalcone() = default;
};
}
} // namespace raytrace
