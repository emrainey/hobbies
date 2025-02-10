#pragma once

#include <functional>
#include <linalg/linalg.hpp>
#include <noise/noise.hpp>

#include "raytrace/color.hpp"
#include "raytrace/image.hpp"
#include "raytrace/mapping.hpp"
#include "raytrace/mediums/functions.hpp"
#include "raytrace/types.hpp"

namespace raytrace {

/// The namespace that contains all surface/volumetric mediums
namespace mediums {

/// A special mixin interface for mediums which are used to make objects
class medium {
public:
    /// The default constructor
    medium();

    /// The default destructor
    virtual ~medium() = default;

    /// Returns the overall ambient color, post-scaled. This is a stand-in for light in a scene the bounces around
    /// diffusely
    /// @param volumetric_point A point on the surface in object space (not world space)
    virtual color ambient(raytrace::point const& volumetric_point) const;

    /// Within reflection, given the surface point, returns a diffuse color
    /// @param volumetric_point A point on the surface in object space (not world space)
    virtual color diffuse(raytrace::point const& volumetric_point) const;

    /// Within reflection, given the surface point, returns a specular color
    /// @param volumetric_point A point on the surface in object space (not world space)
    virtual color specular(raytrace::point const& volumetric_point, precision scaling,
                           color const& light_color) const;

    /// Returns the overall tightness the specular highlight (K)
    /// @param volumetric_point A point on the surface in object space (not world space)
    virtual precision specular_tightness(raytrace::point const& volumetric_point) const;

    /// Returns the emitted light at the volumetric point
    /// @param volumetric_point A point on the surface in object space (not world space)
    virtual color emissive(raytrace::point const& volumetric_point) const;

    /// Returns the color bounced from the medium at a point, given an input color
    /// @param volumetric_point A point on the surface in object space (not world space)
    /// @param incoming The incoming light color
    virtual color bounced(raytrace::point const& volumetric_point, color const& incoming) const;

    /// HACK "Smoothness"
    /// Determines the proportion of reflected light which is coherently reflected from the surface like a mirror
    /// 0.0_p means no coherent reflected light, it's all too diffuse.
    /// 1.0_p means like a perfect mirror. No diffuse surface colors at all.
    /// @param volumetric_point A point on the surface in object space (not world space)
    virtual precision smoothness(raytrace::point const& volumetric_point) const;

    /// Computes the emissive, reflected and transmitted components of the medium.
    /// This replaces the reflectivity, transparency components.
    /// @note Typically transmitted + reflected = 1.0_p.
    /// @param[in] volumetric_point The volumetric point in object space
    /// @param[in] refractive_index The other medium's eta or wave length impedance (aka refractive index)
    /// @param[in] incident_angle The angle of incident to the normal of the surface at the given point.
    /// @param[in] transmitted_angle The angle of transmission (from the inverted normal).
    /// @param[out] emitted The scalar value of emitted light from the surface
    /// @param[out] reflected The scalar value of the reflectivity component of the surface.
    /// @param[out] transmitted The scalar value of the transmitted component of the surface.
    virtual void radiosity(raytrace::point const& volumetric_point, precision refractive_index,
                           iso::radians const& incident_angle, iso::radians const& transmitted_angle,
                           precision& emitted, precision& reflected, precision& transmitted) const;

    /// Return the refractive index of the medium at a point
    /// @param volumetric_point The point to determine the refractive index
    virtual precision refractive_index(raytrace::point const& volumetric_point) const;

    /// Returns the filtered color of the light as absorbed by the medium during
    /// transmission for a given (unit-less) distance.
    virtual color absorbance(precision distance, color const& given_color) const;

    /// Returns the mapping function
    mapping::reducer mapper() const;

    /// Allows the mapping mechanism to be set
    void mapper(mapping::reducer m);

    /// Returns the perturbation of the normal at a given surface point
    /// @param volumetric_point A point on the surface in object space (not world space)
    virtual raytrace::vector perturbation(raytrace::point const& volumetric_point) const;

protected:
    /// How bright the ambient color is in unit scale
    precision m_ambient_scale;
    /// The color of the materials under ambient conditions
    color m_ambient;
    /// The color of the diffuse light reflected from the material
    color m_diffuse;
    /// The tightness of the specular highlight (lower is larger, higher is smaller) on a power scale with regards to
    /// specular highlights on the phong model
    precision m_tightness;
    /// The smoothness of the surface, i.e. how much of a coherent mirror does it form?
    /// 0.0_p means bounced light is not allowed at all.
    /// 1.0_p means it's a perfect mirror
    precision m_smoothness;
    /// The proportion of incoming light which is reflected versus transmitted 1 = (R + T). If 1.0, then all is reflected,
    /// if 0.0 then all is transmitted
    precision m_reflectivity;
    /// The proportion of light which is absorbed when transmitted through a mediums.
    /// A vacuum is 1.0, air is nearly 1.0 and glass is high too. Metal would be nearly zero.
    precision m_transmissivity;
    /// This material's refractive index
    precision m_refractive_index;
    /// Electrical Permissivity
    // precision m_permissivity;
    /// Magnetic Permeability
    // precision m_permeability;
    /// The mapping reduction function (if supplied) maps R3 to R2 for "surface mapping"
    mapping::reducer m_reducing_map;
};

/// This is a namespace of constants to use on mediums for smoothness to get a sense of what to expect
namespace smoothness {
/// This medium will not reflect *any* light and will only have diffuse and ambient components
constexpr precision none = 0.0_p;
/// There's barely any reflections
constexpr precision barely = 0.0625_p;
/// The medium will have a small bit of reflections in it
constexpr precision small = 0.2_p;
/// The medium will be very reflective like a polished metal
constexpr precision polished = 0.7_p;
/// The medium will be very reflective like a good silver mirror, very mild ambient and diffuse components
constexpr precision mirror = 0.95_p;
/// The medium will *only* reflect light and will not have any ambient or diffuse components
constexpr precision perfect_mirror = 1.0_p;
}  // namespace smoothness

/// A namespace of constants to describe the scale of ambient emissions from a medium
/// Ambient light is itself a cheat to get light in places which would otherwise not
/// have any light from the scene but we need to see details.
namespace ambient {
constexpr precision none = 0.0_p;
constexpr precision dim = 0.1_p;
constexpr precision glowy = 0.4_p;
}  // namespace ambient

/// A namespace of constants to describe the "roughness" of the surface in regards to specular highlights.
/// Lower values result in larger spots. Higher values result in tighter spots.
namespace roughness {
constexpr precision tight = 100.0_p;
constexpr precision medium = 50.0_p;
constexpr precision loose = 20.0_p;
}  // namespace roughness

}  // namespace mediums
}  // namespace raytrace
