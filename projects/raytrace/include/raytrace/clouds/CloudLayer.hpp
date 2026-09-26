#pragma once

#include <functional>
#include <noise/noise.hpp>

#include "raytrace/bounds.hpp"
#include "raytrace/types.hpp"

namespace raytrace {

namespace clouds {

/// The box axis treated as the vertical (up-down) direction for the density profile.
/// The vertical profile tapers to zero at the +/- faces along this axis; the other two
/// axes feed the horizontal envelope. This lets a cloud bank lie flat along a scene's
/// up axis rather than always along the local y axis.
enum class axis : int {
    X,
    Y,
    Z
};

/// A bounded volumetric cloud. Rays passing through the bounding box integrate the
/// continuous density field rho(p):
///
///     rho(p) = fbm3(p * scale) * verticalProfile(vertical) * horizontalEnvelope(lateral)
///
/// The FBM places the cloud clumps, the vertical profile peels the density from the
/// top and bottom faces (lit tops / soft undersides) and the horizontal envelope
/// feathers the lateral edges into open air. The material extinction coefficient
/// sigma and single-scattering albedo characterize how an integrating ray is
/// attenuated (Beer-Lambert). The sun-facing brightening is applied per-light by the
/// scene, so this object carries no single resident sun direction.
class CloudLayer {
public:
    /// Default Constructor
    /// @param center The center of the bounding box
    /// @param xhw The half-width in the x axis
    /// @param yhw The half-width in the y axis (the "vertical" thickness)
    /// @param zhw The half-width in the z axis
    CloudLayer(point const& center, precision xhw, precision yhw, precision zhw);

    /// Destructor
    virtual ~CloudLayer() = default;

    /// ------------------------------------------------------------------
    /// Geometry
    /// ------------------------------------------------------------------

    /// @returns The center of the bounding box
    point center() const;
    /// @returns The x half-width
    precision x_half_width() const;
    /// @returns The y (vertical thickness) half-width
    precision y_half_width() const;
    /// @returns The z half-width
    precision z_half_width() const;
    /// @returns An axis aligned bounding box of the cloud
    Bounds bounds() const;
    /// @returns True if the world point lies within the bounding box (inclusive)
    bool contains(point const& world_point) const;
    /// @returns The point expressed in normalized [-1, 1] box coordinates
    point normalized(point const& world_point) const;

    /// ------------------------------------------------------------------
    /// Material configuration
    /// ------------------------------------------------------------------

    /// Sets the extinction coefficient sigma (optical depth per unit density path)
    CloudLayer& extinction(precision sigma);
    /// Returns the extinction coefficient
    precision extinction() const;

    /// Sets the single-scattering albedo (fraction of interacting light that scatters)
    CloudLayer& albedo(precision scattering);
    /// Returns the single-scattering albedo
    precision albedo() const;

    /// ------------------------------------------------------------------
    /// Density field configuration
    /// ------------------------------------------------------------------

    /// Sets the FBM seed used to place the cloud clumps
    CloudLayer& density_seed(noise::vector3 const& seed);
    /// Sets the number of FBM octaves (higher = finer structure)
    CloudLayer& density_octaves(size_t octaves);
    /// Sets how many cloud features span the box (higher = smaller, denser detail)
    CloudLayer& density_scale(precision features_across);
    /// Sets the strength of the lateral feathering (0 = a broad bank, larger = a blob)
    CloudLayer& horizontal_strength(precision strength);
    /// Sets which box axis is the vertical (up-down) direction of the density profile
    CloudLayer& vertical_axis(axis vertical);
    /// Replaces the whole model with an arbitrary density function (used for tests
    /// and bespoke fields). When set, the FBM model is ignored.
    CloudLayer& density_callback(std::function<precision(point const&)> function);

    /// ------------------------------------------------------------------
    /// Density evaluation
    /// ------------------------------------------------------------------

    /// Evaluates the continuous density field at a world space point, clamped to [0, 1].
    /// @note Non-negative by construction and zero at the box faces via the vertical profile.
    precision density(point const& world_point) const;

    /// The vertical profile only. @param ny normalized y in [-1, 1]
    /// @returns 1 at the mid-plane, tapering to 0 at the top and bottom faces
    precision vertical_profile(precision ny) const;

    /// The horizontal feathering only. @param nx, nz normalized coordinates in [-1, 1]
    /// @returns A value in (0, 1] fading with distance from the box center
    precision horizontal_envelope(precision nx, precision nz) const;

protected:
    /// The geometric center of the cloud
    point m_center;
    /// The three half-widths of the bounding box
    precision m_x_half_width;
    precision m_y_half_width;
    precision m_z_half_width;
    /// The extinction coefficient (optical depth per unit density-length)
    precision m_extinction;
    /// The single-scattering albedo
    precision m_albedo;
    /// The density field decomposition
    noise::vector3 m_seed;
    size_t m_octaves;
    precision m_features_across;
    precision m_horizontal_strength;
    /// The box axis treated as up for the density profile
    axis m_vertical;
    /// An optional user supplied density function
    std::function<precision(point const&)> m_density_callback;
};

}  // namespace clouds

}  // namespace raytrace