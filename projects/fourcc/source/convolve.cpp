/// @file
/// Definitions for image processing
/// @copyright Copyright 2022 (C) Erik Rainey.
#include <cmath>

#include "basal/ieee754.hpp"
#include "fourcc/image.hpp"

namespace fourcc {

using namespace basal::literals;
using precision = basal::precision;

template <typename TYPE>
TYPE clamp(TYPE value, TYPE _min, TYPE _max) {
    return std::max(_min, std::min(value, _max));
}

template <size_t HEIGHT, size_t WIDTH>
int16_t kernel_sum(int16_t const (&kernel)[HEIGHT][WIDTH]) {
    int16_t sum = 0;
    for (size_t y = 0; y < HEIGHT; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            if (kernel[y][x] < 0) {
                sum -= kernel[y][x];  // subtract the negative value to make a positive
            } else {
                sum += kernel[y][x];
            }
        }
    }
    return sum == 0 ? 1 : sum;
}

void sobel_mask(image<rgb8, pixel_format::RGB8> const& rgb_image, image<uint8_t, pixel_format::Y8>& mask) {
    size_t height = rgb_image.height;
    size_t width = rgb_image.width;
    image<iyu2, pixel_format::IYU2> iyu2_image(height, width);
    convert(rgb_image, iyu2_image);
    sobel_mask(iyu2_image, mask);
}

void sobel_mask(image<iyu2, pixel_format::IYU2> const& iyu2_image, image<uint8_t, pixel_format::Y8>& mask) {
    size_t height = iyu2_image.height;
    size_t width = iyu2_image.width;

    // create a x and Y gradient image and then sum them and down scale to the mask
    int16_t const sobel_x[3][3] = {
        {-1, 0, +1},
        {-2, 0, +2},
        {-1, 0, +1},
    };
    int16_t const sobel_y[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {+1, +2, +1},
    };

    image<int16_t, pixel_format::Y16> grad_x(height, width);
    image<int16_t, pixel_format::Y16> grad_y(height, width);

    convolve(grad_x, sobel_x, iyu2_image, channel::Y);
    convolve(grad_y, sobel_y, iyu2_image, channel::Y);

    mask.for_each([&](size_t y, size_t x, uint8_t& pixel) {
        int16_t a = grad_x.at(y, x);
        int16_t b = grad_y.at(y, x);
        int32_t c = std::sqrt(a * a + b * b);
        pixel = std::min(c, 255);
    });
}

void convert(image<rgb8, pixel_format::RGB8> const& in, image<iyu2, pixel_format::IYU2>& out) {
    throw_exception_unless(in.height == out.height, "Must be the same height %zu != %zu", in.height, out.height);
    throw_exception_unless(in.width == out.width, "Must be the same width %zu != %zu", in.width, out.width);
    for (size_t y = 0; y < in.height; y++) {
        for (size_t x = 0; x < in.width; x++) {
            precision R = in.at(y, x).r;
            precision G = in.at(y, x).g;
            precision B = in.at(y, x).b;
            precision Y = 0 + 0.2215_p * R + 0.7154_p * G + 0.0721_p * B;
            precision Cb = 0 - 0.1145_p * R - 0.3855_p * G + 0.5000_p * B;
            precision Cr = 0 + 0.5016_p * R - 0.4556_p * G - 0.0459_p * B;
            int32_t y_ = (int32_t)Y;
            int32_t u_ = (int32_t)Cb + 128;
            int32_t v_ = (int32_t)Cr + 128;
            uint8_t _u = (u_ > 255 ? 255 : (u_ < 0 ? 0 : u_));
            uint8_t _y = (y_ > 255 ? 255 : (y_ < 0 ? 0 : y_));
            uint8_t _v = (v_ > 255 ? 255 : (v_ < 0 ? 0 : v_));
            iyu2 pixel;
            pixel.u = _u;
            pixel.y = _y;
            pixel.v = _v;
            out.at(y, x) = pixel;
        }
    }
}

void convolve(image<int16_t, pixel_format::Y16>& out, int16_t const (&kernel)[3][3],
              image<rgb8, pixel_format::RGB8> const& input, channel channel) {
    for (size_t y = 1; y < (input.height - 1); y++) {
        for (size_t x = 1; x < (input.width - 1); x++) {
            int32_t sum = 0;
            int32_t div = 0;
            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    div += kernel[j + 1][i + 1];
                    uint8_t v = 0;
                    if (channel == channel::R) {
                        v = input.at(y + j, x + i).r;
                    } else if (channel == channel::G) {
                        v = input.at(y + j, x + i).g;
                    } else if (channel == channel::B) {
                        v = input.at(y + j, x + i).b;
                    }
                    sum += v * kernel[j + 1][i + 1];
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

void convolve(image<int16_t, pixel_format::Y16>& out, int16_t const (&kernel)[3][3],
              image<iyu2, pixel_format::IYU2> const& input, channel channel) {
    for (size_t y = 1; y < (input.height - 1); y++) {
        for (size_t x = 1; x < (input.width - 1); x++) {
            int32_t sum = 0;
            int32_t div = 0;
            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    div += kernel[j + 1][i + 1];
                    uint8_t v = 0;
                    if (channel == channel::Y) {
                        v = input.at(y + j, x + i).y;
                    } else if (channel == channel::U) {
                        v = input.at(y + j, x + i).u;
                    } else if (channel == channel::V) {
                        v = input.at(y + j, x + i).v;
                    }
                    sum += v * kernel[j + 1][i + 1];
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

// use a simple kernel like -1, 2, -1 or if you use one which does not sum to 0.
void filter(image<rgb8, pixel_format::RGB8>& output, image<rgb8, pixel_format::RGB8> const& input,
            int16_t const kernel[3]) {
    basal::exception::throw_unless(output.width == input.width, __FILE__, __LINE__);
    basal::exception::throw_unless(output.height == input.height, __FILE__, __LINE__);
    int16_t sum = kernel[0] + kernel[1] + kernel[2];
    sum = (sum == 0 ? 1 : sum);
    for (size_t y = 0; y < input.height; y++) {
        output.at(y, 0) = input.at(y, 0);
        for (size_t x = 1; x < (input.width - 1); x++) {
            int16_t r = (kernel[0] * input.at(y, x - 1).r) + (kernel[1] * input.at(y, x - 0).r)
                        + (kernel[2] * input.at(y, x + 1).r);
            int16_t g = (kernel[0] * input.at(y, x - 1).g) + (kernel[1] * input.at(y, x - 0).g)
                        + (kernel[2] * input.at(y, x + 1).g);
            int16_t b = (kernel[0] * input.at(y, x - 1).b) + (kernel[1] * input.at(y, x - 0).b)
                        + (kernel[2] * input.at(y, x + 1).b);
            output.at(y, x).r = uint8_t(clamp<int16_t>(r / sum, 0, 255));
            output.at(y, x).g = uint8_t(clamp<int16_t>(g / sum, 0, 255));
            output.at(y, x).b = uint8_t(clamp<int16_t>(b / sum, 0, 255));
        }
        output.at(y, input.width - 1) = input.at(y, input.width - 1);
    }
}

void filter(image<rgb8, pixel_format::RGB8>& output, image<rgb8, pixel_format::RGB8> const& input,
            int16_t (&kernel)[3][3]) {
    int16_t const sum = kernel_sum(kernel);
    for (size_t y = 1; y < input.height; y++) {
        if (y == 0 or y == (input.height - 1)) {
            for (size_t x = 0; x < input.width; x++) {
                output.at(y, x) = input.at(y, x);
            }
            continue;
        }
        output.at(y, 0) = input.at(y, 0);
        for (size_t x = 1; x < (input.width - 1); x++) {
            int16_t r = 0;
            int16_t g = 0;
            int16_t b = 0;
            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    r += kernel[j + 1][i + 1] * input.at(y + j, x + i).r;
                    g += kernel[j + 1][i + 1] * input.at(y + j, x + i).g;
                    b += kernel[j + 1][i + 1] * input.at(y + j, x + i).b;
                }
            }
            output.at(y, x).r = uint8_t(clamp<int16_t>(r / sum, 0, 255));
            output.at(y, x).g = uint8_t(clamp<int16_t>(g / sum, 0, 255));
            output.at(y, x).b = uint8_t(clamp<int16_t>(b / sum, 0, 255));
        }
        output.at(y, input.width - 1) = input.at(y, input.width - 1);
    }
}

void box(image<rgb8, pixel_format::RGB8>& output, image<rgb8, pixel_format::RGB8> const& input) {
    basal::exception::throw_unless(output.width == input.width, __FILE__, __LINE__);
    basal::exception::throw_unless(output.height == input.height, __FILE__, __LINE__);
    int16_t kernel[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
    filter(output, input, kernel);
}

void gaussian(image<rgb8, pixel_format::RGB8>& output, image<rgb8, pixel_format::RGB8> const& input) {
    basal::exception::throw_unless(output.width == input.width, __FILE__, __LINE__);
    basal::exception::throw_unless(output.height == input.height, __FILE__, __LINE__);
    int16_t kernel[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
    filter(output, input, kernel);
}

}  // namespace fourcc
