#pragma once
#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

/** The checkerboard surface lets users define the size and color of the squares. */
class checkerboard : public opaque {
public:
    /**
     * @param r repeat value
     */
    checkerboard(element_type r, color dark, color light);
    virtual ~checkerboard() = default;

    color diffuse(const raytrace::point& volumetric_point) const final;
protected:
    element_type m_repeat;
    color m_dark, m_light;
};

} // namespace mediums

} // namespace raytrace
