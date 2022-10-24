#pragma once
#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

/** A standard binn-phong model diffuse surface with specular highlights */
class plain : public opaque {
public:
    plain(const color& ambient, element_type ambient_scale, const color& diffuse, element_type smoothness,
          element_type rough);
    virtual ~plain() = default;
};

/** The default boring surface for all objects */
static plain dull(colors::white, 0.0, colors::white, 0.01, 10.0);

}  // namespace mediums

}  // namespace raytrace
