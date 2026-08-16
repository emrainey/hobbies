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
void chroma_replace(cv::Mat const& image, cv::Mat const& background, std::string const& type, cv::Vec3b key,
                    cv::Mat& result, float clip_black = 0.0f, float clip_white = 1.0f, float fg_keep = 0.01f,
                    float bg_keep = 0.05f, cv::Mat const& protect = cv::Mat());

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
void key_out(cv::Mat const& image, std::string const& type, cv::Vec3b key, cv::Mat& result, float clip_black = 0.0f,
             float clip_white = 1.0f, float fg_keep = 0.01f, float bg_keep = 0.05f, cv::Mat const& protect = cv::Mat());

}  // namespace vision