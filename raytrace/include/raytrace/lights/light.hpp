#pragma once

#include "raytrace/types.hpp"
#include "raytrace/color.hpp"
#include <basal/printable.hpp>
namespace raytrace {

/** The namespace to contain all light objects */
namespace lights {
/**
 * An interface to all light types in the system which is used to determine the color contribution at some surface point.
 * @note Color is being used as a stand-in for a power-spectrum graph.
 */
class light : public basal::printable {
public:
    /** Parameter Constructor
     * @param C the point in world space to place the light
     * @param intensity The unbounded (non-unit) intensity value of the light
     * @param number_of_samples The number of samples to request to be averaged in order to get a good sampling of the light.
     */
    light(const raytrace::color& C, element_type intensity, size_t number_of_samples);

    /** Default Destructor */
    virtual ~light() = default;

    /** Returns the intensity scaled by the appropriate dropoff at the point */
    virtual element_type intensity_at(const point &world_point) const = 0;

    /** Returns the incident ray back to the light from a given point in the world.
     * @param world_point The point in world coordinates to form the ray to. The ray must be based at the world_point.
     * @param sample_index The index of the sample (less than or equal to the number of samples)
     */
    virtual ray incident(const point& world_point, size_t sample_index) const = 0;

    /** Returns intensity scaled color at some point in the world */
    color color_at(const point& world_point) const;

    /** Returns the number of samples which the light uses. */
    inline size_t number_of_samples() const { return m_samples; }

protected:
    /** The color of the light source */
    color m_color;

    /** The intensity at the source (each light type may compute dropoff differently) */
    element_type m_intensity;

    /** The number of samples that this light will need to produce */
    size_t m_samples;
};

} // namespace lights
} // namespace raytrace
