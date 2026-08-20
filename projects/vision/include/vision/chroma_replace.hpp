#pragma once

/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Chroma key replacement algorithms: replace a key color in an image with a background image
/// @copyright Copyright (c) 2026
///

#include <opencv2/opencv.hpp>

#include <basal/exception.hpp>

#include <stdexcept>
#include <string>

namespace vision {

/// Selects which chroma key algorithm to use
enum class ChromaType {
    Vlahos,             ///< alpha = key channel - max(other channels), clamped to [0,1]
    Mishima,            ///< Two-state hidden Markov model estimated with forward-backward along each scanline
    ClosedFormMatting,  ///< Matting: solve the matting Laplacian (Levin et al. 2008) on a rough trimap
    BayesianMatting,    ///< Matting: per-pixel Bayesian color-line estimate on a rough trimap (Chuang et al. 2001)
    KnnMatting,         ///< Matting: nonlocal KNN affinities solved as a graph-Laplacian matte (Chen et al. 2013)
    GlobalMatting,      ///< Matting: OpenCV alphamat information-flow global matting on a rough trimap
    FusedMatting,  ///< Keying + closed-form matting fusion: keying pins what is not the screen, matting soft-keys the
                   ///< rest
};

/// Parses the --type string into a ChromaType
ChromaType parse_chroma_type(std::string const& type);

/// Resolves a named color (e.g. "green") to BGR to match OpenCV's default layout
cv::Vec3b named_color(std::string const& name);

/// Computes a per-pixel alpha matte (CV_32FC1, 1.0 = fully replace with background)
cv::Mat vlahos_alpha(cv::Mat const& image, cv::Vec3b key);

/// Computes a per-pixel alpha matte (CV_32FC1, 1.0 = fully replace with background)
cv::Mat mishima_alpha(cv::Mat const& image, cv::Vec3b key);

/// Computes the alpha matte for the selected algorithm
///
/// @param type Selected algorithm
/// @param image 8UC3 input image
/// @param key Key color in BGR
/// @param fg_keep FusedMatting only: green excess at or below this is definite foreground, default 0.01
/// @param bg_keep FusedMatting only: green excess at or above this is definite background, default 0.05
/// @param protect FusedMatting only: optional CV_8UC1 mask where non-zero forces the pixel foreground
cv::Mat compute_alpha(ChromaType type, cv::Mat const& image, cv::Vec3b key, float fg_keep = 0.01f,
                      float bg_keep = 0.05f, cv::Mat const& protect = cv::Mat());

/// Removes key-color spill from foreground pixels (Ultimatte/Keylight style despill).
///
/// Where the key channel dominates the other two (e.g. green fringe on a silver suit
/// reflecting the green screen), the excess key color is removed. Despill is weighted
/// by how much each pixel is kept as subject: fully replaced screen pixels (alpha = 1.0)
/// are untouched, while subject and fringe pixels (low alpha) have their green cast
/// pushed down toward the level of the other two channels. Non-key pixels are unchanged.
///
/// @param image CV_8UC3 input
/// @param alpha CV_32FC1 key-convention matte in [0,1] (1.0 = replaced with background)
/// @param key Key color in BGR
/// @param strength Spill suppression amount in [0,1]; 0 = off, 1 = fully remove the excess
/// @return CV_8UC3 copy of @p image with key-channel spill reduced on foreground pixels
cv::Mat despill(cv::Mat const& image, cv::Mat const& alpha, cv::Vec3b key, float strength = 1.0f);

/// Estimates the actual screen color from an image instead of relying on a pure named key.
///
/// Real key screens are gradient, vignetted and slightly off-hue. This samples pixels
/// whose key channel clearly dominates (strong green excess and reasonably bright - the
/// confident screen region) and returns the mean color of the dominant histogram bin, so
/// occasional outliers (edges, spill on the subject) don't pull the estimate. Falls back
/// to @p key when no confident screen pixels are found.
///
/// @param image CV_8UC3 input
/// @param key Key color in BGR, used only to pick the dominant channel and as a fallback
/// @return CV_8UC3-estimated screen color in BGR
cv::Vec3b estimate_screen_color(cv::Mat const& image, cv::Vec3b key);

/// Refines an alpha matte in place with a guided filter guided by the image luminance.
///
/// After any matte is produced, snapping it to the color edges of the input sharpens
/// soft/hard boundaries onto the actual object outline and removes speckle, without
/// introducing a hard step (the filter preserves the overall alpha profile).
///
/// @param alpha CV_32FC1 matte in [0,1], modified in place
/// @param image CV_8UC3 guide image (same size as @p alpha)
/// @param radius Guided-filter local window radius in pixels (must be >= 1)
/// @param eps Regularization; smaller keeps color edges sharper
void refine_alpha(cv::Mat& alpha, cv::Mat const& image, int radius = 3, float eps = 1.0e-4f);

/// Remaps an alpha matte in place using clip black/white (Keylight style).
///
/// Values at or below @p clip_black become 0.0 (fully foreground), values at or
/// above @p clip_white become 1.0 (fully replaced), and the range in between is
/// linearly stretched. This lifts weakly-keyed colors such as the dark greens in
/// shadows toward a full replacement (e.g. a bright green).
///
/// @param alpha CV_32FC1 matte in [0,1], modified in place
/// @param clip_black Lower clip threshold in [0,1], default 0.0
/// @param clip_white Upper clip threshold in [0,1], default 1.0
void remap_alpha(cv::Mat& alpha, float clip_black = 0.0f, float clip_white = 1.0f);

/// Composites the background over the image where the key color is found
///
/// @param image 8UC3 input frame
/// @param background 8UC3 replacement image (resized to the input size if it differs)
/// @param type Algorithm name: "vlahos" or "mishima"
/// @param key Key color to replace in BGR
/// @param result Filled with the CV_8UC3 composited output
/// @param clip_black Lower alpha clip threshold (see remap_alpha), default 0.0
/// @param clip_white Upper alpha clip threshold (see remap_alpha), default 1.0
/// @param fg_keep FusedMatting only: green excess at or below this is definite foreground, default 0.01
/// @param bg_keep FusedMatting only: green excess at or above this is definite background, default 0.05
/// @param protect FusedMatting only: optional CV_8UC1 mask where non-zero forces the pixel foreground
/// @param despill_strength Spill suppression applied to the foreground fringe, 0 = off
/// @param refine_radius Guided-filter alpha refinement radius in pixels; 0 = off
void chroma_replace(cv::Mat const& image, cv::Mat const& background, std::string const& type, cv::Vec3b key,
                    cv::Mat& result, float clip_black = 0.0f, float clip_white = 1.0f, float fg_keep = 0.01f,
                    float bg_keep = 0.05f, cv::Mat const& protect = cv::Mat(), float despill_strength = 0.0f,
                    int refine_radius = 0);

/// "Keys out" the subject: replaces the key color with a solid, pure version of the
/// key color itself. A flat pure key color is brighter and more consistent than a
/// physical key screen, which is handy when no background image is available.
///
/// @param image 8UC3 input frame
/// @param type Algorithm name: "vlahos" or "mishima"
/// @param key Key color to replace in BGR
/// @param result Filled with the CV_8UC3 keyed-out output
/// @param clip_black Lower alpha clip threshold (see remap_alpha), default 0.0
/// @param clip_white Upper alpha clip threshold (see remap_alpha), default 1.0
/// @param fg_keep FusedMatting only: green excess at or below this is definite foreground, default 0.01
/// @param bg_keep FusedMatting only: green excess at or above this is definite background, default 0.05
/// @param protect FusedMatting only: optional CV_8UC1 mask where non-zero forces the pixel foreground
/// @param despill_strength Spill suppression applied to the foreground fringe, 0 = off
/// @param refine_radius Guided-filter alpha refinement radius in pixels; 0 = off
void key_out(cv::Mat const& image, std::string const& type, cv::Vec3b key, cv::Mat& result, float clip_black = 0.0f,
             float clip_white = 1.0f, float fg_keep = 0.01f, float bg_keep = 0.05f, cv::Mat const& protect = cv::Mat(),
             float despill_strength = 0.0f, int refine_radius = 0);

}  // namespace vision