#pragma once
#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

/** A polka dot styled surface */
class dots : public opaque {
public:
    /**
     * @param r The number of repeats of the pattern.
     * @param dot The color of the dot
     * @param background The background color of the dot
     */
    dots(element_type r, color dot, color background);
    virtual ~dots() = default;

    color diffuse(const raytrace::point& volumetric_point) const final;

protected:
    element_type m_repeat;
    color m_dot, m_background;
};

}  // namespace mediums

}  // namespace raytrace
