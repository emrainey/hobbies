#pragma once

/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Image matting for chroma keying: solve for a soft alpha matte in the unknown
///        band of a rough trimap instead of making a hard per-pixel keying decision.
/// @copyright Copyright (c) 2026
///
/// Matting reframes the chroma key problem as a "matting" rather than "keying" problem:
/// a rough trimap partitions the image into definite background (the key screen),
/// definite foreground (the subject) and an unknown band (shadows, spill, soft edges),
/// and a solver fills in fractional alpha for the unknown pixels. This produces soft,
/// continuous boundaries which is exactly what "clip-into-submission" keying misses.
///
/// All matte-producing functions here return the *foreground* alpha in [0,1]
/// (1.0 = subject), matching the alpha-matting literature. The chroma key compositing
/// convention in @ref vision::compute_alpha inverts this (1.0 = replace with background).

#include <opencv2/opencv.hpp>

#include <cstdint>
#include <stdexcept>

namespace vision {
namespace matting {

/// The three classes of a rough trimap (OpenCV/alpha-matting convention).
enum class TrimapClass : std::uint8_t {
    Background = 0,    ///< Definite key screen: foreground alpha is 0
    Unknown = 128,     ///< Band to be solved by matting
    Foreground = 255,  ///< Definite subject: foreground alpha is 1
};

/// Builds a rough trimap from a key color using HSV thresholds.
///
/// Pixels close to the key hue and reasonably saturated/not-black are the definite
/// background (the key screen). Pixels far from the key hue, or desaturated pixels
/// far from the key hue, are the definite foreground. Everything in between - the
/// shadow/spill/soft-boundary band - becomes unknown so a matting solver can produce
/// a soft alpha instead of a hard key threshold.
///
/// @param image CV_8UC3 input
/// @param key Key color in BGR
/// @param bg_hue_tol Normalized circular hue distance (of the 180° hue range) within
///        which a saturated pixel is definite background, default 0.12 (~22°)
/// @param sat_min Minimum saturation in [0,1] for a pixel to count as the key screen,
///        default 0.25
/// @param fg_hue_tol Normalized hue distance beyond which a pixel is definite
///        foreground, default 0.30 (~54°)
/// @return CV_8UC1 trimap in {0, 128, 255}
cv::Mat build_trimap(cv::Mat const& image, cv::Vec3b key, float bg_hue_tol = 0.12f, float sat_min = 0.25f,
                     float fg_hue_tol = 0.30f);

/// Closed-form matting (Levin, Lischinski and Weiss 2008).
///
/// Builds the matting Laplacian from color-line statistics of every 3x3 window and
/// solves (L + lambda*D) alpha = lambda*b with the trimap as hard constraints using a
/// preconditioned conjugate-gradient solver over the implicit operator (no explicit
/// sparse matrix is materialized). Excellent at recovering soft, low-contrast
/// boundaries such as shadow edges. @p lambda defaults to 100.
///
/// @warning O(pixels) memory and a few hundred CG iterations; slow on large frames.
///
/// @return CV_32FC1 foreground alpha in [0,1]
cv::Mat closed_form_matting(cv::Mat const& image, cv::Mat const& trimap, float lambda = 100.0f);

/// Bayesian matting (Chuang et al. 2001), color-line variant.
///
/// Per unknown pixel, foreground and background Gaussian color models are fit from the
/// nearest definite samples (spatially weighted). Alpha is then the maximum-likelihood
/// projection of the pixel color onto the foreground/background color line. This is
/// the classic per-pixel Bayesian estimate, without the full iterative mixture walk.
///
/// @return CV_32FC1 foreground alpha in [0,1]
cv::Mat bayesian_matting(cv::Mat const& image, cv::Mat const& trimap);

/// KNN matting (Chen, Li and Tang 2013).
///
/// Nonlocal nearest-neighbour affinities are found in a 5-D feature space
/// (x, y, R, G, B) with a KD-tree, giving a large sparse graph which is solved by the
/// same preconditioned conjugate-gradient matting solver. The nonlocal relations
/// propagate alpha across the whole image, which suits soft shadow boundaries.
///
/// @return CV_32FC1 foreground alpha in [0,1]
cv::Mat knn_matting(cv::Mat const& image, cv::Mat const& trimap);

/// Global matting through OpenCV's alphamat module (Aksoy et al. 2017 "Information
/// Flow"). Requires an OpenCV build with the alphamat module (4.7+). A thin wrapper
/// around cv::alphamat::infoFlow that converts the result to the common scale.
///
/// @throw std::runtime_error if the OpenCV build lacks the alphamat module
/// @return CV_32FC1 foreground alpha in [0,1]
cv::Mat global_matting(cv::Mat const& image, cv::Mat const& trimap);

/// Builds a rough trimap from the *keying* decision instead of HSV hue windows.
///
/// The per-pixel "green excess" of the key channel over the other two (the same signal
/// used by @ref vision::vlahos_alpha) pins the definite regions and leaves the genuine
/// shadow/spill band unknown:
///
/// - excess >= @p bg_keep -> definite background (the key screen),
/// - excess <= @p fg_keep -> definite foreground (the subject and anything not-green),
/// - otherwise             -> unknown, solved softly by a matting solver.
///
/// This fuses the two families of keying: the cheap, per-pixel keying signal is trusted
/// to say what is definitively *not* the screen, so the matting solver cannot bleed key
/// alpha into the subject the way HSV build_trimap can for weakly-keyed footage.
///
/// @param fg_keep Green excess at or below this is definite foreground, default 0.01
/// @param bg_keep Green excess at or above this is definite background, default 0.05
/// @param protect Optional CV_8UC1 mask (same size as image): pixels >= 128 are forced to
///        definite foreground no matter what the keying signal says, e.g. a user-marked
///        subject rectangle; pixels in 1..127 are treated as an unknown band (see
///        feather_protect_mask) so the solver blends the matte at the edge. Empty passes
///        through.
/// @return CV_8UC1 trimap with TrimapClass values
cv::Mat build_trimap_from_keying(cv::Mat const& image, cv::Vec3b key, float fg_keep = 0.01f, float bg_keep = 0.05f,
                                 cv::Mat const& protect = cv::Mat());

/// Fuses keying with matting (the "fused"/"hybrid" chroma type).
///
/// Wraps @ref closed_form_matting over a trimap from @ref build_trimap_from_keying: the
/// per-pixel keying decision already trusts what is definitely not the screen (so subject
/// pixels are pinned to foreground and cannot be eaten by the solver), while the matting
/// Laplacian still produces a soft fractional alpha across the genuine shadow/spill band.
///
/// @param fg_keep Green excess at or below this is kept as foreground, default 0.01
/// @param bg_keep Green excess at or above this is keyed as background, default 0.05
/// @param protect Optional CV_8UC1 mask (same size as image): pixels >= 128 are forced to
///        definite foreground, protecting reflective subjects from being keyed; pixels in
///        1..127 become an unknown band (see feather_protect_mask) so the edge is solved
///        softly instead of a hard step.
/// @return CV_32FC1 foreground alpha in [0,1]
cv::Mat fused_matting(cv::Mat const& image, cv::Vec3b key, float fg_keep = 0.01f, float bg_keep = 0.05f,
                      cv::Mat const& protect = cv::Mat());

}  // namespace matting
}  // namespace vision