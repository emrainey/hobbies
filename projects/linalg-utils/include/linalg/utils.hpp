/// @file
/// Definitions for linalg utility library
/// @copyright Copyright 2021 (C) Erik Rainey.

#pragma once

#include <fourcc/image.hpp>

#include "linalg/opencv.hpp"
#include "linalg/plot.hpp"

namespace linalg {
/// Debugging tool to export a matrix into a color coded image based on some precision to pixel converter
template <typename PIXEL_TYPE, fourcc::PixelFormat PixelFormat>
void convert(linalg::matrix const& mat, fourcc::image<PixelFormat>& image,
             std::function<PIXEL_TYPE(precision)> converter) {
    basal::exception::throw_unless(
        PixelFormat == fourcc::PixelFormat::GREY8 or PixelFormat == fourcc::PixelFormat::Y8
            or PixelFormat == fourcc::PixelFormat::Y16 or PixelFormat == fourcc::PixelFormat::Y32,
        __FILE__, __LINE__);
    image.for_each([mat, image, converter](size_t y, size_t x, PIXEL_TYPE& pixel) -> void {
        pixel = converter(mat[(y * image.width) + x][0]);  // assuming Y*W,1
    });
}

}  // namespace linalg
