#pragma once

#include "raytrace/mediums/opaque.hpp"

namespace raytrace {
namespace mediums {

/// A medium that emits light (glows) with a configurable color
class glowy : public opaque {
public:
    glowy(color const& ambient, color const& emissive, precision smoothness = smoothness::none,
          precision roughness = roughness::tight);
    virtual ~glowy() = default;

    color emissive(raytrace::point const& volumetric_point) const override;
};

}  // namespace mediums
}  // namespace raytrace
