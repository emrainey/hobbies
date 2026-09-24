#pragma once

#include "raytrace/clouds/CloudLayer.hpp"
#include "raytrace/types.hpp"

namespace raytrace {

namespace clouds {

/// Integrates a ray through a @ref CloudLayer's density field, following Beer-Lambert
/// extinction with a single-scattering sun-facing brightening term.
///
/// For a ray clipped to the box we accumulate the optical depth
///
///     tau = sum over samples of rho(p_i) * ds
///
/// with the sampling step aligned so a constant density recovers Beer-Lambert exactly,
/// and report the transmittance T = exp(-sigma * tau). The brightening term averages,
/// over the same samples, the albedo attenuated by the cloud lying between each sample
/// and a given light: albedo * rho * exp(-sigma * tau_toward_light(p_i)). Passing the
/// light direction in per light lets a single integrator serve many light sources;
/// points near a lit face see a short path to that light so they glow, deeper points
/// are shaded.
class CloudIntegrator {
public:
    /// Constructor
    /// @param layer The cloud to integrate against (held by reference, must outlive this)
    /// @param step_size The world-space step length used to sample the density
    CloudIntegrator(CloudLayer const& layer, precision step_size = 0.25_p);

    /// Destructor
    virtual ~CloudIntegrator() = default;

    /// @returns The sampling step length
    precision step_size() const;

    /// Clips a ray to the bounding box. On a hit fills t0 (entry) and t1 (exit), the
    /// ray parameters along the travelling direction.
    /// @returns True if the ray intersects the box, false otherwise
    bool clip(ray const& r, precision& t0, precision& t1) const;

    /// @returns True if the ray intersects the cloud's bounding box
    bool intersects(ray const& r) const;

    /// The accumulated optical depth of a ray through the cloud (0 when it misses).
    precision optical_depth(ray const& r) const;

    /// The Kuhn-Lambert / Beer-Lambert transmittance: exp(-sigma * optical_depth).
    /// @returns A value in (0, 1]; 1 when the cloud is empty or the ray misses.
    precision transmittance(ray const& r) const;

    /// The average sun-facing single-scattering brightening factor.
    /// @param r The ray to brighten
    /// @param light_direction The normalized direction the light travels as it enters
    ///        the cloud (from the sun). Passing a direction per light lets one
    ///        integrator serve multiple light sources.
    /// @returns A value in [0, albedo]; 0 when the cloud is empty or the ray misses.
    precision brightening(ray const& r, vector const& light_direction) const;

protected:
    /// The cloud under integration
    CloudLayer const& m_layer;
    /// The world-space sampling step
    precision m_step;

    /// The optical depth between two world points, sampled at the configured step.
    precision optical_depth_between(point const& a, point const& b) const;
};

}  // namespace clouds

}  // namespace raytrace