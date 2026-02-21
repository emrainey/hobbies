#pragma once

/// @file
/// Convolution definitions

#include <fourcc/image.hpp>

namespace fourcc {

/// Convolves a specific channel of the input image and the kernel into the output gradient image
void convolve(fourcc::image<fourcc::PixelFormat::Y16>& out, int16_t const (&kernel)[3][3],
              fourcc::image<fourcc::PixelFormat::RGB8> const& input, ChannelName channel);

/// Convolves a specific channel of the input image and the kernel into the output gradient image
void convolve(fourcc::image<fourcc::PixelFormat::Y16>& out, int16_t const (&kernel)[3][3],
              fourcc::image<fourcc::PixelFormat::IYU2> const& input, ChannelName channel);

/// Produces a Sobel Mask image
void sobel_mask(fourcc::image<fourcc::PixelFormat::IYU2> const& iyu2_image,
                fourcc::image<fourcc::PixelFormat::Y8>& mask);

/// Produces a Sobel Mask image
void sobel_mask(fourcc::image<fourcc::PixelFormat::RGB8> const& rgb_image,
                fourcc::image<fourcc::PixelFormat::Y8>& mask);

/// Runs a 1D filter across a linear uncompressed image
/// @warning no gamma correction!
void filter(fourcc::image<fourcc::PixelFormat::RGBf>& output,
            fourcc::image<fourcc::PixelFormat::RGBf> const& input, float const kernel[3]);

/// Runs a 1D filter across a linear uncompressed image
/// @warning no gamma correction!
void filter(fourcc::image<fourcc::PixelFormat::RGBId>& output,
            fourcc::image<fourcc::PixelFormat::RGBId> const& input, double const kernel[3]);

/// Runs a 1D filter across an encoded image
/// @warning no gamma correction!
void filter(fourcc::image<fourcc::PixelFormat::RGB8>& output,
            fourcc::image<fourcc::PixelFormat::RGB8> const& input, int16_t const kernel[3]);

/// Runs a 2D 3x3 Filter across an image
/// @warning no gamma correction!
void filter(image<PixelFormat::RGB8>& output, image<PixelFormat::RGB8> const& input,
            int16_t (&kernel)[3][3]);

/// @brief Box 2D filter
/// @param output The output image
/// @param input The input image
void box(image<PixelFormat::RGB8>& output, image<PixelFormat::RGB8> const& input);

/// @brief Gaussian 2D filter
/// @param output The output image
/// @param input The input image
void gaussian(image<PixelFormat::RGB8>& output, image<PixelFormat::RGB8> const& input);

}  // namespace fourcc