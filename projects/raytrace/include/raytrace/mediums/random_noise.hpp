#pragma once
#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

/// Generates seemingly pure random noise as colors per pixel
class random_noise : public opaque {
public:
    random_noise();
    virtual ~random_noise() = default;

    color diffuse(raytrace::point const& volumetric_point) const final;
};

}  // namespace mediums

}  // namespace raytrace
