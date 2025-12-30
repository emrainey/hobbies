#pragma once

#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

/// The stripes surface lets users define the size and color of the stripes.
class stripes : public opaque {
public:
    /// @param repeat repeat value (values under 1.0_p scale up pattern, values over 1.0_p scale down pattern)
    stripes(precision repeat, color dark, color light);
    virtual ~stripes() = default;

    color diffuse(raytrace::point const& volumetric_point) const final;

protected:
    precision m_repeat;
    palette m_pal;
};

}  // namespace mediums

}  // namespace raytrace
