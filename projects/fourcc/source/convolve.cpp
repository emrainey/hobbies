/// @file
/// Definitions for image processing
/// @copyright Copyright 2022 (C) Erik Rainey.
#include <cmath>

#include <basal/ieee754.hpp>
#include <fourcc/image.hpp>
#include <fourcc/convolve.hpp>
#include <fourcc/convert.hpp>

namespace fourcc {

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

void sobel_mask(image<PixelFormat::RGB8> const& rgb_image, image<PixelFormat::Y8>& mask) {
    size_t height = rgb_image.height;
    size_t width = rgb_image.width;
    image<PixelFormat::IYU2> iyu2_image(height, width);
    convert(rgb_image, iyu2_image);
    sobel_mask(iyu2_image, mask);
}

void sobel_mask(image<PixelFormat::IYU2> const& iyu2_image, image<PixelFormat::Y8>& mask) {
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

    image<PixelFormat::Y16> grad_x(height, width);
    image<PixelFormat::Y16> grad_y(height, width);

    convolve(grad_x, sobel_x, iyu2_image, ChannelName::Y);
    convolve(grad_y, sobel_y, iyu2_image, ChannelName::Y);

    mask.for_each([&](size_t y, size_t x, uint8_t& pixel) {
        uint16_t a = grad_x.at(y, x);
        uint16_t b = grad_y.at(y, x);
        int32_t c = static_cast<int32_t>(std::sqrt((a * a) + (b * b)));
        pixel = static_cast<uint8_t>(std::min(c, 255));
    });
}

void convolve(image<PixelFormat::Y16>& out, int16_t const (&kernel)[3][3],
              image<PixelFormat::RGB8> const& input, ChannelName channel) {
    for (size_t y = 1; y < (input.height - 1); y++) {
        for (size_t x = 1; x < (input.width - 1); x++) {
            int32_t sum = 0;
            int32_t div = 0;
            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    div += kernel[j + 1][i + 1];
                    uint8_t v = 0;
                    size_t h = static_cast<size_t>(static_cast<int>(x) + i);
                    size_t k = static_cast<size_t>(static_cast<int>(y) + j);
                    if (channel == ChannelName::R) {
                        v = input.at(k, h).components.r;
                    } else if (channel == ChannelName::G) {
                        v = input.at(k, h).components.g;
                    } else if (channel == ChannelName::B) {
                        v = input.at(k, h).components.b;
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
            out.at(y, x) = static_cast<uint16_t>(value);
        }
    }
}

void convolve(image<PixelFormat::Y16>& out, int16_t const (&kernel)[3][3],
              image<PixelFormat::IYU2> const& input, ChannelName channel) {
    for (size_t y = 1; y < (input.height - 1); y++) {
        for (size_t x = 1; x < (input.width - 1); x++) {
            int32_t sum = 0;
            int32_t div = 0;
            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    div += kernel[j + 1][i + 1];
                    uint8_t v = 0;
                    size_t h = static_cast<size_t>(static_cast<int>(x) + i);
                    size_t k = static_cast<size_t>(static_cast<int>(y) + j);
                    if (channel == ChannelName::Y) {
                        v = input.at(k, h).components.y;
                    } else if (channel == ChannelName::U) {
                        v = input.at(k, h).components.u;
                    } else if (channel == ChannelName::V) {
                        v = input.at(k, h).components.v;
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
            out.at(y, x) = static_cast<uint16_t>(value);
        }
    }
}

// use a simple kernel like -1, 2, -1 or if you use one which does not sum to 0.
void filter(image<PixelFormat::RGB8>& output, image<PixelFormat::RGB8> const& input,
            int16_t const kernel[3]) {
    basal::exception::throw_unless(output.width == input.width, __FILE__, __LINE__);
    basal::exception::throw_unless(output.height == input.height, __FILE__, __LINE__);
    int16_t sum = kernel[0] + kernel[1] + kernel[2];
    sum = (sum == 0 ? 1 : sum);
    for (size_t y = 0; y < input.height; y++) {
        output.at(y, 0) = input.at(y, 0);
        for (size_t x = 1; x < (input.width - 1); x++) {
            int16_t r = (kernel[0] * input.at(y, x - 1).components.r) + (kernel[1] * input.at(y, x - 0).components.r)
                        + (kernel[2] * input.at(y, x + 1).components.r);
            int16_t g = (kernel[0] * input.at(y, x - 1).components.g) + (kernel[1] * input.at(y, x - 0).components.g)
                        + (kernel[2] * input.at(y, x + 1).components.g);
            int16_t b = (kernel[0] * input.at(y, x - 1).components.b) + (kernel[1] * input.at(y, x - 0).components.b)
                        + (kernel[2] * input.at(y, x + 1).components.b);
            output.at(y, x).components.r = uint8_t(clamp<int16_t>(r / sum, 0, 255));
            output.at(y, x).components.g = uint8_t(clamp<int16_t>(g / sum, 0, 255));
            output.at(y, x).components.b = uint8_t(clamp<int16_t>(b / sum, 0, 255));
        }
        output.at(y, input.width - 1) = input.at(y, input.width - 1);
    }
}

void filter(image<PixelFormat::RGB8>& output, image<PixelFormat::RGB8> const& input,
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
                    size_t h = static_cast<size_t>(static_cast<int>(x) + i);
                    size_t k = static_cast<size_t>(static_cast<int>(y) + j);
                    r += kernel[j + 1][i + 1] * input.at(k, h).components.r;
                    g += kernel[j + 1][i + 1] * input.at(k, h).components.g;
                    b += kernel[j + 1][i + 1] * input.at(k, h).components.b;
                }
            }
            output.at(y, x).components.r = uint8_t(clamp<int16_t>(r / sum, 0, 255));
            output.at(y, x).components.g = uint8_t(clamp<int16_t>(g / sum, 0, 255));
            output.at(y, x).components.b = uint8_t(clamp<int16_t>(b / sum, 0, 255));
        }
        output.at(y, input.width - 1) = input.at(y, input.width - 1);
    }
}

void box(image<PixelFormat::RGB8>& output, image<PixelFormat::RGB8> const& input) {
    basal::exception::throw_unless(output.width == input.width, __FILE__, __LINE__);
    basal::exception::throw_unless(output.height == input.height, __FILE__, __LINE__);
    int16_t kernel[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
    filter(output, input, kernel);
}

void gaussian(image<PixelFormat::RGB8>& output, image<PixelFormat::RGB8> const& input) {
    basal::exception::throw_unless(output.width == input.width, __FILE__, __LINE__);
    basal::exception::throw_unless(output.height == input.height, __FILE__, __LINE__);
    int16_t kernel[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
    filter(output, input, kernel);
}

}  // namespace fourcc
