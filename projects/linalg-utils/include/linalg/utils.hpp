/// @file
/// Definitions for linalg utility library
/// @copyright Copyright 2021 (C) Erik Rainey.

#pragma once

#include <fourcc/image.hpp>

#include "linalg/opencv.hpp"
#include "linalg/plot.hpp"

namespace linalg {
/// Debugging tool to export a matrix into a color coded image based on some precision to pixel converter
template <typename PIXEL_TYPE, fourcc::pixel_format PIXEL_FORMAT>
void convert(const linalg::matrix& mat, fourcc::image<PIXEL_TYPE, PIXEL_FORMAT>& image,
             std::function<PIXEL_TYPE(precision)> converter) {
    basal::exception::throw_unless(
        PIXEL_FORMAT == fourcc::pixel_format::GREY8 or PIXEL_FORMAT == fourcc::pixel_format::Y8
            or PIXEL_FORMAT == fourcc::pixel_format::Y16 or PIXEL_FORMAT == fourcc::pixel_format::Y32,
        __FILE__, __LINE__);
    image.for_each([mat, image, converter](size_t y, size_t x, PIXEL_TYPE& pixel) -> void {
        pixel = converter(mat[(y * image.width) + x][0]);  // assuming Y*W,1
    });
}

}  // namespace linalg
