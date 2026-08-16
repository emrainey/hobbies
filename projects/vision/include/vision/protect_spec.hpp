#pragma once

/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Parses user-supplied "known foreground" rectangle specs (e.g. "100,50,640,400")
///        into rectangles and rasterizes them into a protection mask for keying/matting.
/// @copyright Copyright (c) 2026
///
/// A protect spec is a comma-separated list of pixel rectangles written as `x,y,w,h`,
/// for example `100,50,640,400` or `100,50,640,400,900,300,300,500`. The rectangles are
/// clamped to the image bounds. The mask marks every pixel inside any rectangle as a
/// definite foreground region, so a chroma solver cannot key it even when it reflects
/// the key screen's color (e.g. a reflective suit on a green screen).

#include <opencv2/core.hpp>

#include <vector>

namespace vision {

/// Parses a protect spec into rectangles of (x, y, width, height) in pixels.
///
/// @throw basal::exception on malformed numbers or non-positive tag dimensions
std::vector<cv::Rect> parse_protect_rects(std::string const& spec);

/// Rasterizes the rectangles into a CV_8UC1 mask (255 inside any rectangle, 0 outside).
///
/// Rectangles are clamped to [0, rows) x [0, cols); a clipped-away rectangle simply
/// contributes nothing. An empty rect list yields an all-zero mask.
cv::Mat build_protect_mask(int rows, int cols, std::vector<cv::Rect> const& rects);

/// Softens the edge of a protection mask so the matte blends instead of stepping.
///
/// Blurs @p mask with a @p radius-pixel Gaussian and rescales back to 0..255, leaving
/// the interior solid 255 (definite subject) and the boundary a grey fringe. In a
/// keying trimap that grey fringe is solved as an unknown band, giving a smooth alpha
/// ramp at the edge of a protected region. A @p radius of 0 returns @p mask unchanged.
cv::Mat feather_protect_mask(cv::Mat const& mask, int radius);

}  // namespace vision