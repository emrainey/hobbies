#pragma once

/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Chroma key replacement algorithms: replace a key color in an image with a
///        background image.
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
    Keylight,           ///< Screen-balanced soft keyer (see keylight_alpha)
    Mishima,            ///< Two-state hidden Markov model estimated with forward-backward along each scanline
    ClosedFormMatting,  ///< Matting: solve the matting Laplacian (Levin et al. 2008) on a rough trimap
    BayesianMatting,    ///< Matting: per-pixel Bayesian color-line estimate on a rough trimap (Chuang et al. 2001)
    KnnMatting,         ///< Matting: nonlocal KNN affinities solved as a graph-Laplacian matte (Chen et al. 2013)
    GlobalMatting,      ///< Matting: OpenCV alphamat information-flow global matting on a rough trimap
    SharedSampling,     ///< Matting: shared-sampling matting (Gastal & Oliveira 2010), per-pixel solve
    FusedMatting,       ///< Keying + closed-form matting fusion: keying pins what is not the screen, matting
                        ///< soft-keys the rest
};

/// Parses the --type string into a ChromaType
ChromaType parse_chroma_type(std::string const& type);

/// Resolves a named color (e.g. "green") to BGR to match OpenCV's default layout
cv::Vec3b named_color(std::string const& name);

/// Computes a per-pixel alpha matte (CV_32FC1, 1.0 = fully replace with background)
cv::Mat vlahos_alpha(cv::Mat const& image, cv::Vec3b key);

// Keylight-style keyer. Balances the frame to the key luminance (so gradient,
// vignetted and off-hue screens key uniformly) then soft-keys on the colour
// difference in that balanced space.
//
// @param image CV_8UC3 input
// @param key Key color in BGR (typically the estimated screen color)
// @param softness Soft factor ramps the matte from 0 to 1 as the balanced colour
//                 excess goes from 0 up to this fraction; small = crisp, default
// @return CV_32FC1 key-convention matte in [0,1] (1.0 = replace with background)
cv::Mat keylight_alpha(cv::Mat const& image, cv::Vec3b key, float softness = 0.1f);

/// Computes a per-pixel alpha matte (CV_32FC1, 1.0 = fully replace with background)
cv::Mat mishima_alpha(cv::Mat const& image, cv::Vec3b key);

/// Computes the key-convention alpha matte for a given ChromaType (1.0 = replace)
cv::Mat compute_alpha(ChromaType type, cv::Mat const& image, cv::Vec3b key, float fg_keep = 0.01f,
                      float bg_keep = 0.05f, cv::Mat const& protect = cv::Mat(), float softness = -1.0f,
                      int trimap_clean = 0);

/// Removes key-color spill from foreground pixels (Ultimatte/Keylight style).
cv::Mat despill(cv::Mat const& image, cv::Mat const& alpha, cv::Vec3b key, float strength = 1.0f, float floor = 0.2f);

/// Estimates the actual screen color from the input instead of relying on a pure named key.
cv::Vec3b estimate_screen_color(cv::Mat const& image, cv::Vec3b key);

/// Refines an alpha matte in place with a guided filter guided by the image luminance.
void refine_alpha(cv::Mat& alpha, cv::Mat const& image, int radius = 3, float eps = 1.0e-4f);

/// Remaps an alpha matte in place using clip black/white (Keylight style).
void remap_alpha(cv::Mat& alpha, float clip_black = 0.0f, float clip_white = 1.0f);

/// Temporal EMA matte smoothing. Blends the current frame's alpha toward the previous
/// with a factor that fades out at near-solid pixels, so per-frame flicker in the soft
/// fringe is damped without trailing a moving subject's hard edge.
///
/// @param[in,out] alpha Current-frame matte (CV_32FC1 in [0,1]); replaced in place
/// @param[in,out] state Persistent accumulator, empty on the first frame
/// @param smooth         EMA weight toward the previous frame in [0,1]; 0 disables
void smooth_alpha(cv::Mat& alpha, cv::Mat& state, float smooth);

/// Replaces the key color in an image with a background, producing a composited result.
void chroma_replace(cv::Mat const& image, cv::Mat const& background, std::string const& type, cv::Vec3b key,
                    cv::Mat& result, float clip_black = 0.0f, float clip_white = 1.0f, float fg_keep = 0.01f,
                    float bg_keep = 0.05f, cv::Mat const& protect = cv::Mat(), float despill_strength = 0.0f,
                    int refine_radius = 0, float softness = -1.0f, float despill_floor = 0.2f, int trimap_clean = 0,
                    float matte_smooth = 0.0f, cv::Mat* matte_state = nullptr);

/// "Keys out" the subject: replaces the key color with a solid, pure version of the key
/// color itself.
void key_out(cv::Mat const& image, std::string const& type, cv::Vec3b key, cv::Mat& result, float clip_black = 0.0f,
             float clip_white = 1.0f, float fg_keep = 0.01f, float bg_keep = 0.05f, cv::Mat const& protect = cv::Mat(),
             float despill_strength = 0.0f, int refine_radius = 0, float softness = -1.0f, float despill_floor = 0.2f,
             int trimap_clean = 0, float matte_smooth = 0.0f, cv::Mat* matte_state = nullptr);

}  // namespace vision