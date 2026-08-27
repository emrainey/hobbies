#pragma once

/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Apple Vision subject segmentation: detects people/subjects in a frame and
///        rasterizes them into a foreground protection mask for keying/matting.
/// @copyright Copyright (c) 2026
///
/// Uses macOS' Vision framework (VNGenerateForegroundInstanceMaskRequest) - the same
/// segmentation behind Preview's "the subject" tool - so keys like figure-alignment or
/// a reflective (silver) suit that reflects the screen can be pinned as kept subject
/// automatically instead of by hand-placed --protect rectangles.
///
/// The header is plain C++: the implementation lives in an Objective-C++ translation
/// unit (subject_mask.mm) that only exists on Apple platforms. Busy frames cost on the
/// order of tens to a few hundred milliseconds; run per frame when the cast moves.

#include <opencv2/core.hpp>

namespace vision {

/// Detects foreground subjects (people, etc.) using Apple Vision and returns a CV_8UC1
/// mask where non-zero pixels are known foreground.
///
/// @param bgr Image to segment (CV_8UC3, BGR layout as OpenCV reads frames)
/// @param dilate_px Grow the mask by this many pixels in each direction, so a few
///        subject-colored boundary pixels cannot be keyed; default 3
/// @return CV_8UC1 mask sized like @p bgr (255 = subject) with @p dilate_px dilation, or
///         an empty Mat when detection fails, no request is supported, or no subject was
///         found (callers should treat an empty mask as "protect nothing")
cv::Mat detect_subject_mask(cv::Mat const& bgr, int dilate_px = 3);

}  // namespace vision