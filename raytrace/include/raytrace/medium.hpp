#pragma once

#include <functional>
#include <linalg/linalg.hpp>
#include <noise/noise.hpp>

#include "raytrace/color.hpp"
#include "raytrace/image.hpp"
#include "raytrace/types.hpp"
#include "raytrace/mapping.hpp"
#include "raytrace/mediums/functions.hpp"

namespace raytrace {

/** A special mixin interface for mediums which are used to make objects */
class medium {
public:
    /** The default constructor */
    medium();

    /** The default destructor */
    virtual ~medium() = default;

    /** Returns the overall ambient color, post-scaled. This is a stand-in for light in a scene the bounces around diffusely */
    virtual color ambient(const raytrace::point& volumetric_point) const;

    /** Within reflection, given the uv point of the surface, returns a diffuse color */
    virtual color diffuse(const raytrace::point& volumetric_point) const;
    /** Within reflection, given the uv point of the surface, returns a specular color */
    virtual color specular(const raytrace::point& volumetric_point, element_type scaling, const color& light_color) const;
    /** Returns the overall tightness the specular highlight (K) */
    virtual element_type specular_tightness(const raytrace::point& volumetric_point) const;
    /**
     * HACK "Smoothness"
     * Determines the proportion of reflected light which is coherently reflected from the surface like a mirror
     * 0.0 means no coherent reflected light, it's all too diffuse.
     * 1.0 means like a perfect mirror. No diffuse surface colors at all.
     */
    virtual element_type smoothness(const raytrace::point& volumetric_point) const;

    /** Computes the emissive, reflected and transmitted components of the medium.
     * This replaces the reflectivity, transparency components.
     * @note Typically transmitted + reflected = 1.0.
     * @param[in] volumetric_point The volumetric point
     * @param[in] refractive_index The other medium's eta or wave length impedance (aka refractive index)
     * @param[in] incident_angle The angle of incident to the normal of the surface at the given point.
     * @param[in] transmitted_angle The angle of transmission (from the inverted normal).
     * @param[out] emitted The scalar value of emitted light from the surface
     * @param[out] reflected The scalar value of the reflectivity component of the surface.
     * @param[out] transmitted The scalar value of the transmitted component of the surface.
     */
    virtual void radiosity(const raytrace::point& volumetric_point,
                           element_type refractive_index,
                           const iso::radians& incident_angle,
                           const iso::radians& transmitted_angle,
                           element_type& emitted,
                           element_type& reflected,
                           element_type& transmitted) const;

    /**
     * Return the refractive index of the medium at a point
     * @param volumetric_point The point to determine the refractive index
     */
    virtual element_type refractive_index(const raytrace::point& volumetric_point) const;

    /** Returns how much light is absorbed by the medium during transmission for a given (unitless) distance. */
    virtual element_type absorbance(element_type distance) const;

    /** Returns the mapping function */
    mapping::reducer mapper() const;

    /** Allows the mapping mechanism to be set */
    void mapper(mapping::reducer m);

protected:
    /** How bright the ambient color is in unit scale */
    element_type m_ambient_scale;
    /** The color of the materials under ambient conditions */
    color m_ambient;
    /** The color of the diffuse light reflected from the material */
    color m_diffuse;
    /** The tightness of the specular highlight (lower is larger, higher is smaller) on a power scale with regards to specular highlights on the phong model */
    element_type m_tightness;
    /**
     * The smoothness of the surface, i.e. how much of a coherent mirror does it form?
     * 0.0 means bounced light is not allowed at all.
     * 1.0 means it's a perfect mirror
     */
    element_type m_smoothness;
    /** The proportion of incoming light which is reflected versus transmitted 1 = (R + T). If 0, then all is reflected, if 1 then all is transmitted */
    element_type m_transmissivity;
    /** This material's refractive index */
    element_type m_refractive_index;
    /** Electrical Permissivity */
    //element_type m_permissivity;
    /** Magnetic Permeability */
    //element_type m_permeability;
    mapping::reducer m_reducing_map;
};

/** This is a namespace of constants to use on mediums for smoothness to get a sense of what to expect */
namespace smoothness {
    /** This medium will not reflect *any* light and will only have diffuse and ambient components */
    constexpr element_type none = 0.0;
    /** There's barely any reflections */
    constexpr element_type barely = 0.0625;
    /** The medium will have a small bit of reflections in it */
    constexpr element_type small = 0.2;
    /** The medium will be very reflective like a polished metal */
    constexpr element_type polished = 0.7;
    /** The medium will *only* reflect light and will not have any ambient or diffuse components */
    constexpr element_type mirror = 1.0;
}

/**
 * A namespace of constants to describe the scale of ambient emissions from a medium
 * Ambient light is itself a cheat to get light in places which would otherwise not
 * have any light from the scene but we need to see details.
 */
namespace ambient {
    constexpr element_type none = 0.0;
    constexpr element_type dim = 0.1;
    constexpr element_type glowy = 0.4;
}

/**
 * A namespace of constants to describe the "roughness" of the surface in regards to specular highlights.
 * Lower values result in larger spots. Higher values result in tighter spots.
 */
namespace roughness {
    constexpr element_type tight = 100.0;
    constexpr element_type loose = 20.0;
}

} // namespace raytrace
