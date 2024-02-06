#pragma once
#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

/// A standard binn-phong model diffuse surface with specular highlights
class plain : public opaque {
public:
    plain(const color& ambient, precision ambient_scale, const color& diffuse, precision smoothness,
          precision rough);
    virtual ~plain() = default;
};

/// The default boring surface for all objects
static plain dull(colors::white, 0.0_p, colors::white, 0.01_p, 10.0_p);

}  // namespace mediums

}  // namespace raytrace
