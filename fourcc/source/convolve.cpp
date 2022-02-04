/**
 * @file
 * Definitions for image processing
 * @copyright Copyright 2022 (C) Erik Rainey.
 */
#include "fourcc/image.hpp"

#include <cmath>

namespace fourcc {

void sobel_mask(const image<rgb8, pixel_format::RGB8>& rgb_image,
                image<uint8_t, pixel_format::Y8>& mask) {
    size_t height = rgb_image.height;
    size_t width = rgb_image.width;
    image<iyu2, pixel_format::IYU2> iyu2_image(height, width);
    convert(rgb_image, iyu2_image);
    sobel_mask(iyu2_image, mask);
}

void sobel_mask(const image<iyu2, pixel_format::IYU2>& iyu2_image,
                image<uint8_t, pixel_format::Y8>& mask) {
    size_t height = iyu2_image.height;
    size_t width = iyu2_image.width;

    // create a x and Y gradient image and then sum them and down scale to the mask
    const int16_t sobel_x[3][3] = {
        {-1,  0, +1},
        {-2,  0, +2},
        {-1,  0, +1},
    };
    const int16_t sobel_y[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        {+1, +2, +1},
    };

    image<int16_t, pixel_format::Y16> grad_x(height, width);
    image<int16_t, pixel_format::Y16> grad_y(height, width);

    convolve(grad_x, sobel_x, iyu2_image, channel::Y);
    convolve(grad_y, sobel_y, iyu2_image, channel::Y);

    mask.for_each([&](size_t y, size_t x, uint8_t& pixel) {
        int16_t a = grad_x.at(y, x);
        int16_t b = grad_y.at(y, x);
        int32_t c = std::sqrt(a*a + b*b);
        pixel = std::min(c, 255);
    });
}

void convert(const image<rgb8, pixel_format::RGB8>& in,
             image<iyu2, pixel_format::IYU2>& out) {
    throw_exception_unless(in.height == out.height, "Must be the same height %zu != %zu", in.height, out.height);
    throw_exception_unless(in.width == out.width, "Must be the same width %zu != %zu", in.width, out.width);
    for (size_t y = 0; y < in.height; y++) {
        for (size_t x = 0; x < in.width; x++) {
            double R = in.at(y, x).r;
            double G = in.at(y, x).g;
            double B = in.at(y, x).b;
            double Y = 0 + 0.2215f*R + 0.7154f*G + 0.0721f*B;
            double Cb = 0 - 0.1145f*R - 0.3855f*G + 0.5000f*B;
            double Cr = 0 + 0.5016f*R - 0.4556f*G - 0.0459f*B;
            int32_t y_ = (int32_t)Y;
            int32_t u_ = (int32_t)Cb + 128;
            int32_t v_ = (int32_t)Cr + 128;
            uint8_t _u = (u_ > 255 ? 255 : (u_ < 0 ? 0 : u_));
            uint8_t _y = (y_ > 255 ? 255 : (y_ < 0 ? 0 : y_));
            uint8_t _v = (v_ > 255 ? 255 : (v_ < 0 ? 0 : v_));
            iyu2 pixel;
            pixel.u = u_;
            pixel.y = y_;
            pixel.v = v_;
            out.at(y, x) = pixel;
        }
    }
}

void convolve(image<int16_t, pixel_format::Y16>& out,
              const int16_t (&kernel)[3][3],
              const image<rgb8, pixel_format::RGB8>& input,
              channel channel) {

    for (int y = 1; y < input.height-1; y++) {
        for (int x = 1; x < input.width-1; x++) {
            int32_t sum = 0;
            int32_t div = 0;
            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    div += kernel[j+1][i+1];
                    uint8_t v = 0;
                    if (channel == channel::R) {
                        v = input.at(y + j, x + i).r;
                    } else if (channel == channel::G) {
                        v = input.at(y + j, x + i).g;
                    } else if (channel == channel::B) {
                        v = input.at(y + j, x + i).b;
                    }
                    sum += v * kernel[j+1][i+1];
                }
            }
            int32_t value = sum / (div == 0 ? 1 : div);
            if (value > INT16_MAX) {
                value = INT16_MAX;
            } else if (value < INT16_MIN) {
                value = INT16_MIN;
            }
            sum = 0;
            div = 0;
            out.at(y, x) = value;
        }
    }
}


void convolve(image<int16_t, pixel_format::Y16>& out,
              const int16_t (&kernel)[3][3],
              const image<iyu2, pixel_format::IYU2>& input,
              channel channel) {

    for (int y = 1; y < input.height-1; y++) {
        for (int x = 1; x < input.width-1; x++) {
            int32_t sum = 0;
            int32_t div = 0;
            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    div += kernel[j+1][i+1];
                    uint8_t v = 0;
                    if (channel == channel::Y) {
                        v = input.at(y + j, x + i).y;
                    } else if (channel == channel::U) {
                        v = input.at(y + j, x + i).u;
                    } else if (channel == channel::V) {
                        v = input.at(y + j, x + i).v;
                    }
                    sum += v * kernel[j+1][i+1];
                }
            }
            int32_t value = sum / (div == 0 ? 1 : div);
            if (value > INT16_MAX) {
                value = INT16_MAX;
            } else if (value < INT16_MIN) {
                value = INT16_MIN;
            }
            sum = 0;
            div = 0;
            out.at(y, x) = value;
        }
    }
}

} // namespace fourcc
