#pragma once

#include <basal/printable.hpp>

#include "raytrace/color.hpp"
#include "raytrace/types.hpp"
#include "raytrace/entity.hpp"
namespace raytrace {

/// @brief The types of falloff in the calculation of intensity
enum class Falloff : int {
    None,
    Linear,
    InverseSquare
};
/// The namespace to contain all light objects
namespace lights {

namespace intensities {
/// @brief Used for dim lighting (under 1.0)
constexpr static precision dark{0.1};
constexpr static precision dim{0.25};
constexpr static precision moderate{0.5};
constexpr static precision full{1.0};
constexpr static precision bright{1E1};
constexpr static precision intense{1E2};
constexpr static precision radiant{1E3};
constexpr static precision blinding{1E11};
constexpr static precision searing{1E20};
}  // namespace intensities

/// An interface to all light types in the system which is used to determine the color contribution at some surface
/// point.
/// @note Color is being used as a stand-in for a power-spectrum graph.
class light
    : public entity
    , public basal::printable {
public:
    /// Parameter Constructor
    /// @param P the point in world space to place the light
    /// @param C the point in world space to place the light
    /// @param intensity The unbounded (non-unit) intensity value of the light
    /// @param number_of_samples The number of samples to request to be averaged in order to get a good sampling of the
    /// light.
    /// @param falloff The type of falloff to use in intensity calculations
    ///
    light(raytrace::point const& P, raytrace::color const& C, precision intensity, size_t number_of_samples,
          Falloff falloff);

    /// Default Destructor
    virtual ~light() = default;

    /// Returns the intensity scaled by the appropriate dropoff at the point
    virtual precision intensity_at(point const& world_point) const;

    /// Returns the incident ray back to the light from a given point in the world.
    /// @param world_point The point in world coordinates to form the ray to. The ray must be based at the world_point.
    /// @param sample_index The index of the sample (less than or equal to the number of samples)
    ///
    virtual ray incident(point const& world_point, size_t sample_index) const = 0;

    /// Returns intensity scaled color at some point in the world
    virtual color color_at(point const& world_point) const;

    /// Returns the number of samples which the light uses.
    inline size_t number_of_samples() const {
        return m_samples;
    }

    /// @brief The base class stream operator
    /// @param os The output stream to write to
    /// @param l The light reference
    /// @return
    friend std::ostream& operator<<(std::ostream& os, light const& l);

    /// @brief Emit a ray from the light source. Each subtype will
    /// emit rays according to its own methodology.
    /// @return A ray emitted from the light source.
    virtual ray emit() = 0;

protected:
    /// The color of the light source
    color m_color;

    /// The intensity at the source (each light type may compute dropoff differently)
    precision m_intensity;

    /// The number of samples that this light will need to produce
    size_t m_samples;

    /// Falloff type
    Falloff m_falloff{Falloff::InverseSquare};
};

}  // namespace lights
}  // namespace raytrace
