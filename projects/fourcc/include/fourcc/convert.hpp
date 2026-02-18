#pragma once

/// @file
/// Conversion Functions for images and pixels

#include <fourcc/image.hpp>

namespace fourcc {

/// Converts RGB8 to IYU2
void convert(fourcc::image<fourcc::PixelFormat::RGB8> const& in,
             fourcc::image<fourcc::PixelFormat::IYU2>& out);

/// Converts RGBid to RGB8
void convert(fourcc::image<fourcc::PixelFormat::RGBId> const& in,
             fourcc::image<fourcc::PixelFormat::RGB8>& out);

}