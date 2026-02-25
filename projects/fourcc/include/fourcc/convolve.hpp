#pragma once

/// @file
/// Convolution definitions

#include <fourcc/image.hpp>

namespace fourcc {

/// Convolves a specific channel of the input image and the kernel into the output gradient image
void convolve(image<PixelFormat::Y16>& out, int16_t const (&kernel)[3][3], image<PixelFormat::RGB8> const& input,
              ChannelName channel);

/// Convolves a specific channel of the input image and the kernel into the output gradient image
void convolve(image<PixelFormat::Y16>& out, int16_t const (&kernel)[3][3], image<PixelFormat::IYU2> const& input,
              ChannelName channel);

/// Produces a Sobel Mask image
void sobel_mask(image<PixelFormat::IYU2> const& iyu2_image, image<PixelFormat::Y8>& mask);

/// Produces a Sobel Mask image
void sobel_mask(image<PixelFormat::RGB8> const& rgb_image, image<PixelFormat::Y8>& mask);

/// Runs a 1D filter across a linear uncompressed image
/// @warning no gamma correction!
void filter(image<PixelFormat::RGBf>& output, image<PixelFormat::RGBf> const& input, float const kernel[3]);

/// Runs a 1D filter across a linear uncompressed image
/// @warning no gamma correction!
void filter(image<PixelFormat::RGBId>& output, image<PixelFormat::RGBId> const& input, double const kernel[3]);

/// Runs a 1D filter across an encoded image
/// @warning no gamma correction!
void filter(image<PixelFormat::RGB8>& output, image<PixelFormat::RGB8> const& input, int16_t const kernel[3]);

/// Runs a 2D 3x3 Filter across an image
/// @warning no gamma correction!
void filter(image<PixelFormat::RGB8>& output, image<PixelFormat::RGB8> const& input, int16_t (&kernel)[3][3]);

/// Runs a 2D 3x3 Filter across an image
/// @warning no gamma correction!
void filter(image<PixelFormat::RGBId>& output, image<PixelFormat::RGBId> const& input, double (&kernel)[3][3]);

/// @brief Box 2D filter
/// @param output The output image
/// @param input The input image
void box(image<PixelFormat::RGB8>& output, image<PixelFormat::RGB8> const& input);

/// @brief Gaussian 2D filter
/// @param output The output image
/// @param input The input image
void gaussian(image<PixelFormat::RGB8>& output, image<PixelFormat::RGB8> const& input);

/// The bilateral filter for a given input image using an NxN kernel.
template <size_t N>
void bilateral(image<PixelFormat::RGBId>& output, image<PixelFormat::RGBId> const& input);

}  // namespace fourcc