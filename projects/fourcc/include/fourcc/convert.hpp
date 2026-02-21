#pragma once

/// @file
/// Conversion Functions for images and pixels

#include <fourcc/image.hpp>

namespace fourcc {

/// Converts RGB8 to IYU2
void convert(image<PixelFormat::RGB8> const& in,
             image<PixelFormat::IYU2>& out);

/// Converts RGBid to RGB8
void convert(image<PixelFormat::RGBId> const& in,
             image<PixelFormat::RGB8>& out);

/// Converts RGBid to RGBf
void convert(image<PixelFormat::RGBId> const& in,
             image<PixelFormat::RGBf>& out);

/// Converts RGBid to RGBh
void convert(image<PixelFormat::RGBId> const& in,
             image<PixelFormat::RGBh>& out);

}