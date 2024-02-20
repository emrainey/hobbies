#pragma once
#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

/// The grid surface lets users define the size and color of the squares.
class grid : public opaque {
public:
    /// @param scale The scaling factor
    grid(precision scale, color dark, color light);
    virtual ~grid() = default;

    color diffuse(raytrace::point const& volumetric_point) const final;

protected:
    precision m_scale;
    color m_dark, m_light;
};

}  // namespace mediums

}  // namespace raytrace
