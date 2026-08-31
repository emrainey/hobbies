/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Portable fallback for Apple Vision subject segmentation.
/// @copyright Copyright (c) 2026
///
/// Non-Apple platforms have no Vision framework, so no subject can be detected. This
/// translation unit mirrors the early-return contract of the Objective-C++ implementation
/// (subject_mask.mm) and otherwise reports "protect nothing" so keyers and their tests
/// build and run on Linux/Windows CI.

#include <vision/subject_mask.hpp>

#include <opencv2/core.hpp>

namespace vision {

cv::Mat detect_subject_mask(cv::Mat const& bgr, int dilate_px) {
    (void)dilate_px;
    if (bgr.empty() || bgr.type() != CV_8UC3) {
        return {};
    }
    // No subject segmentation available on this platform; treat every frame as
    // "protect nothing" so callers degrade gracefully.
    return {};
}

}  // namespace vision