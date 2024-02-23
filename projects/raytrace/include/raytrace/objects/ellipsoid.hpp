#pragma once

#include "raytrace/objects/quadratic.hpp"

namespace raytrace {
namespace objects {
class ellipsoid : public quadratic {
public:
    ellipsoid(point const& center, precision a, precision b, precision c);
    ~ellipsoid() = default;
    precision get_object_extent(void) const override;
};
}  // namespace objects
}  // namespace raytrace
