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

TEST(FourccTest, RGBh) {
    image<rgbh, pixel_format::RGBh> img(480, 640);
    img.for_each([](size_t sy, size_t sx, rgbh& pixel) {
        // each corner is a different color
        // white -> red
        // |          |
        // blue -> green
        basal::precision c = static_cast<basal::precision>(sy)/static_cast<basal::precision>(sx);
        if (c > 1.0f) { c = 1 / c; }
        pixel.r = 1.0f - (basal::precision(sy) / 480.0f);
        pixel.g = c;
        pixel.b = 1.0f - (basal::precision(sx) / 640.0f);
    });
    img.save("gradient.exr");
}