#pragma once

/// @file
/// The Raytrace library configuration header

#include <geometry/geometry.hpp>

//
// Configuration Variables
//
namespace raytrace {
namespace debug {
/// Enables generic debug statements
static constexpr bool generic{false};
/// Enables debug for the ray casting calls
static constexpr bool cast{false};
/// Enables debug for the ray tracing calls
static constexpr bool hit{false};
/// Enables debug for the camera setup
static constexpr bool camera{false};
/// Enables debug for the scene setup
static constexpr bool scene{false};
/// Enables debug for the objparser calls
static constexpr bool objparser{false};
/// Enables debug for the polygon
static constexpr bool polygon{false};
/// Enables debug for the model loader
static constexpr bool model{false};
}  // namespace debug

/// Enforces range checking on some calls
static constexpr bool enforce_contracts{true};

/// Enables using true intensity for lights or a clamped intensity
static constexpr bool use_true_intensity{true};

/// If true, use random sample points, otherwise use fixed sample points for samplers
static constexpr bool use_random_sample_points{false};

/// uses fixed color scheme for shadows, light, near-zero values
static constexpr bool debug_shadows_and_light{false};
static constexpr bool use_incident_scaling{true};
static constexpr bool use_specular_scaling{true};

/// displays the distance to a light in shadow as a grayscale value (black is still shadow, white is very close)
static constexpr bool use_grayscale_distance{false};

/// Determines if points are on surfaces, thus having a normal
static constexpr bool check_on_surface = geometry::check_on_surface;

/// Determine the number of objects in the scene to switch from brute force to a bounding box search
static constexpr size_t brute_force_to_bounding_box{10U};

/// A flag to control if origin collisions are counted
static constexpr bool can_ray_origin_be_collision{true};

/// A flag to control is polygon collisions should use the triple product or not.
static constexpr bool use_triple = true;

namespace gamma {
/// Control if the full correction or the fast correction is used
static constexpr bool use_full = true;

/// @brief If true, use the adaptive threshold for raytracing
static constexpr bool enforce_contract = false;
}  // namespace gamma

}  // namespace raytrace
