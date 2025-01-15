#include <gtest/gtest.h>

#include <fourcc/image.hpp>

using namespace fourcc;

TEST(FourccTest, Bars) {
    image<rgb8, pixel_format::RGB8> img(480, 640);
    img.for_each([](size_t y, size_t x, rgb8& pixel) {
        if (x < 60) {
            pixel.r = 255;
            pixel.g = 0;
            pixel.b = 0;
        } else if (60 < x and x < 120) {
            pixel.r = 0;
            pixel.g = 255;
            pixel.b = 0;
        } else if (120 < x and x < 180) {
            pixel.r = 0;
            pixel.g = 0;
            pixel.b = 255;
        } else if (180 < x and x < 240) {
            pixel.r = 255;
            pixel.g = 255;
            pixel.b = 0;
        } else if (240 < x and x < 300) {
            pixel.r = 0;
            pixel.g = 255;
            pixel.b = 255;
        } else if (300 < x and x < 360) {
            pixel.r = 255;
            pixel.g = 0;
            pixel.b = 255;
        } else if (360 < x and x < 420) {
            pixel.r = 255;
            pixel.g = 255;
            pixel.b = 255;
        } else if (420 < x and x < 480) {
            pixel.r = 0;
            pixel.g = 0;
            pixel.b = 0;
        } else {
            pixel.r = 128;
            pixel.g = 128;
            pixel.b = 128;
        }
    });
    img.save("bars.ppm");
    img.save("bars.tga");
}

TEST(FourccTest, RGBf) {
    image<rgbf, pixel_format::RGBf> img(480, 640);
    img.for_each([](size_t y, size_t x, rgbf& pixel) {
        pixel.r = y / 480.0f;
        pixel.g = x / 640.0f;
        pixel.b = 1.0f;
    });
    img.save("gradient.exr");
}