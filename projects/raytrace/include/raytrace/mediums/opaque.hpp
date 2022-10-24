#pragma once
#include "raytrace/mediums/dielectric.hpp"

namespace raytrace {

namespace mediums {

/** A class of mediums which are dominated by largely diffuse coloring with few specular hightlights */
class opaque : public dielectric {
public:
    opaque();
    virtual ~opaque() = default;
};
}  // namespace mediums

}  // namespace raytrace
