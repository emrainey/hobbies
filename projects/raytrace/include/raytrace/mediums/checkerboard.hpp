#pragma once
#include "raytrace/mediums/opaque.hpp"

namespace raytrace {

namespace mediums {

/** The checkerboard surface lets users define the size and color of the squares. */
class checkerboard : public opaque {
public:
    /**
     * @param repeat repeat value (values under 1.0 scale up pattern, values over 1.0 scale down pattern)
     */
    checkerboard(element_type repeat, color dark, color light);

    /**
     * Creates a per planar quadrant color set
     */
    checkerboard(element_type repeat, color q1_dark, color q1_light, color q2_dark, color q2_light, color q3_dark,
                 color q3_light, color q4_dark, color q4_light);
    virtual ~checkerboard() = default;

    color diffuse(const raytrace::point& volumetric_point) const final;

protected:
    element_type m_repeat;
    palette m_pal;
};

}  // namespace mediums

}  // namespace raytrace
