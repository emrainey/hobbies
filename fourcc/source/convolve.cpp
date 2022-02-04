/**
 * @file
 * Definitions for image processing
 * @copyright Copyright 2022 (C) Erik Rainey.
 */
#include "fourcc/image.hpp"

namespace fourcc {

void convert(const fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8>& in,
             fourcc::image<fourcc::iyu2, fourcc::pixel_format::IYU2>& out) {
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
            fourcc::iyu2 pixel;
            pixel.u = u_;
            pixel.y = y_;
            pixel.v = v_;
            out.at(y, x) = pixel;
        }
    }
}

void convolve(fourcc::image<int16_t, fourcc::pixel_format::Y16>& out,
              const int16_t (&kernel)[3][3],
              const fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8>& input,
              Channel channel) {

    for (int y = 1; y < input.height-1; y++) {
        for (int x = 1; x < input.width-1; x++) {
            int32_t sum = 0;
            int32_t div = 0;
            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    div += kernel[j+1][i+1];
                    uint8_t v = 0;
                    if (channel == Channel::R) {
                        v = input.at(y + j, x + i).r;
                    } else if (channel == Channel::G) {
                        v = input.at(y + j, x + i).g;
                    } else if (channel == Channel::B) {
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


void convolve(fourcc::image<int16_t, fourcc::pixel_format::Y16>& out,
              const int16_t (&kernel)[3][3],
              const fourcc::image<fourcc::iyu2, fourcc::pixel_format::IYU2>& input,
              Channel channel) {

    for (int y = 1; y < input.height-1; y++) {
        for (int x = 1; x < input.width-1; x++) {
            int32_t sum = 0;
            int32_t div = 0;
            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    div += kernel[j+1][i+1];
                    uint8_t v = 0;
                    if (channel == Channel::Y) {
                        v = input.at(y + j, x + i).y;
                    } else if (channel == Channel::U) {
                        v = input.at(y + j, x + i).u;
                    } else if (channel == Channel::V) {
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
