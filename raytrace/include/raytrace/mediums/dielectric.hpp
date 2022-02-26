#pragma once
#include "raytrace/mediums/medium.hpp"

namespace raytrace {

namespace mediums {

/** A dielectric is a non conductive material that follows either a binn-phong model or a transparent object */
class dielectric : public medium {
public:
    dielectric();
    virtual ~dielectric() = default;
};

} // namespace mediums

} // namespace raytrace
