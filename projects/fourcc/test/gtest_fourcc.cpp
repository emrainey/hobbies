#include <gtest/gtest.h>

#include <fourcc/fourcc.hpp>

using namespace fourcc;

TEST(FourccTest, Bars) {
    image<PixelFormat::RGB8> img(480, 640);
    img.for_each([](size_t y, size_t x, rgb8& pixel) {
        if (x < 60) {
            pixel.components.r = 255;
            pixel.components.g = 0;
            pixel.components.b = 0;
        } else if (60 < x and x < 120) {
            pixel.components.r = 0;
            pixel.components.g = 255;
            pixel.components.b = 0;
        } else if (120 < x and x < 180) {
            pixel.components.r = 0;
            pixel.components.g = 0;
            pixel.components.b = 255;
        } else if (180 < x and x < 240) {
            pixel.components.r = 255;
            pixel.components.g = 255;
            pixel.components.b = 0;
        } else if (240 < x and x < 300) {
            pixel.components.r = 0;
            pixel.components.g = 255;
            pixel.components.b = 255;
        } else if (300 < x and x < 360) {
            pixel.components.r = 255;
            pixel.components.g = 0;
            pixel.components.b = 255;
        } else if (360 < x and x < 420) {
            pixel.components.r = 255;
            pixel.components.g = 255;
            pixel.components.b = 255;
        } else if (420 < x and x < 480) {
            pixel.components.r = 0;
            pixel.components.g = 0;
            pixel.components.b = 0;
        } else {
            pixel.components.r = 128;
            pixel.components.g = 128;
            pixel.components.b = 128;
        }
    });
    img.save("bars.ppm");
    img.save("bars.tga");
}

TEST(FourccTest, RGBh) {
    image<PixelFormat::RGBh> img(480, 640);
    img.for_each([](size_t sy, size_t sx, rgbh& pixel) {
        // each corner is a different color
        // white -> red
        // |          |
        // blue -> green
        precision c = static_cast<precision>(sy) / static_cast<precision>(sx);
        if (c > 1.0f) {
            c = 1 / c;
        }
        pixel.components.r = 1.0f - (precision(sy) / 480.0f);
        pixel.components.g = c;
        pixel.components.b = 1.0f - (precision(sx) / 640.0f);
    });
    img.save("gradient.exr");
}


TEST(FourccTest, RGBf) {
    image<PixelFormat::RGBf> image{480, 640};
    image.for_each([](size_t sy, size_t sx, rgbf& pixel) -> void {
        // each corner is a different color
        // white -> red
        // |          |
        // blue -> green
        float c = static_cast<float>(sy) / static_cast<float>(sx);
        if (c > 1.0f) {
            c = 1 / c;
        }
        pixel.components.r = 1.0f - (static_cast<float>(sy) / 480.0f);
        pixel.components.g = c;
        pixel.components.b = 1.0f - (static_cast<float>(sx) / 640.0f);
    });
    image.save("gradient.pfm");
}

TEST(FourccTest, FilterRGBf) {
    image<PixelFormat::RGBf> img(480, 640);
    img.for_each([](size_t y, size_t x, rgbf& pixel) {
        if (((x / 2) % 2) == 0) {
            pixel.components.r = static_cast<float>((x + y + 137) % 256) / 255.0f;
            pixel.components.g = static_cast<float>((x + y + 11) % 256) / 255.0f;
            pixel.components.b = static_cast<float>((x + y + 89) % 256) / 255.0f;
        } else {
            pixel.components.r = 0.0f;
            pixel.components.g = 0.0f;
            pixel.components.b = 0.0f;
        }
    });
    img.save("pre-filter.pfm");
    image<PixelFormat::RGBf> filtered(480, 640);
    float kernel[3] = {-1.0, 2.0, 1.0};
    filter(filtered, img, kernel);
    filtered.save("post-filter.pfm");
}

TEST(FourccTest, FilterRGB8) {
    image<PixelFormat::RGB8> img(480, 640);
    img.for_each([](size_t y, size_t x, rgb8& pixel) {
        if (((x / 2) % 2) == 0) {
            pixel.components.r = (x + y + 137) % 256;
            pixel.components.g = (x + y + 11) % 256;
            pixel.components.b = (x + y + 89) % 256;
        } else {
            pixel.components.r = 0;
            pixel.components.g = 0;
            pixel.components.b = 0;
        }
    });
    img.save("pre-filter.ppm");
    image<PixelFormat::RGB8> filtered(480, 640);
    int16_t kernel[3] = {-1, 2, 1};
    filter(filtered, img, kernel);
    filtered.save("post-filter.ppm");
}

TEST(FourccTest, SobelEdge) {
    image<PixelFormat::RGB8> img(480, 640);
    img.for_each([](size_t y, size_t x, rgb8& pixel) {
        // find the distance from the center
        int dx = x - 320;
        int dy = y - 240;
        int d = sqrt(dx * dx + dy * dy);
        // make a circle
        if (d < 100) {
            pixel.components.r = (x + y + 48) % 256;
            pixel.components.g = (x + y + 111) % 256;
            pixel.components.b = (x + y + 27) % 256;
        } else {
            pixel.components.r = 0;
            pixel.components.g = 0;
            pixel.components.b = 0;
        }
    });
    img.save("pre-convolve.ppm");
    image<PixelFormat::Y8> edge(480, 640);
    sobel_mask(img, edge);
    edge.save("sobel_mask.ppm");
}

TEST(FourccTest, BoxFilter) {
    image<PixelFormat::RGB8> img(480, 640);
    img.for_each([](size_t y, size_t x, rgb8& pixel) {
        // find the distance from the center
        int dx = x - 320;
        int dy = y - 240;
        int d = sqrt(dx * dx + dy * dy);
        // make a circle
        if (d < 100) {
            pixel.components.r = (x + y + 48) % 256;
            pixel.components.g = (x + y + 111) % 256;
            pixel.components.b = (x + y + 27) % 256;
        } else {
            pixel.components.r = 0;
            pixel.components.g = 0;
            pixel.components.b = 0;
        }
    });
    img.save("pre-box.ppm");
    image<PixelFormat::RGB8> boxed(480, 640);
    box(boxed, img);
    boxed.save("post-box.ppm");
}

TEST(FourccTest, GaussianFilter) {
    image<PixelFormat::RGB8> img(480, 640);
    img.for_each([](size_t y, size_t x, rgb8& pixel) {
        // find the distance from the center
        int dx = x - 320;
        int dy = y - 240;
        int d = sqrt(dx * dx + dy * dy);
        // make a circle
        if (d < 100) {
            pixel.components.r = (x + y + 48) % 256;
            pixel.components.g = (x + y + 111) % 256;
            pixel.components.b = (x + y + 27) % 256;
        } else {
            pixel.components.r = 0;
            pixel.components.g = 0;
            pixel.components.b = 0;
        }
    });
    img.save("pre-gaussian.ppm");
    image<PixelFormat::RGB8> output(480, 640);
    gaussian(output, img);
    output.save("post-gaussian.ppm");
}

template <size_t N>
void bilateral_test() {
    size_t W = (N + 1) * 6;
    image<PixelFormat::RGBId> img(W, W);
    img.for_each([](size_t y, size_t x, rgbid& pixel) {
        // find the distance from the center
        size_t j = y % (N + 1);
        size_t i = x % (N + 1);
        size_t k = (x + y) % (N + 1);
        pixel.components.r = static_cast<double>(j / static_cast<double>(N));
        pixel.components.g = static_cast<double>(k / static_cast<double>(N));
        pixel.components.b = static_cast<double>(i / static_cast<double>(N));
    });
    img.save("pre-bilateral-" + std::to_string(N) + ".pfm");
    image<PixelFormat::RGBId> output(W, W);
    fourcc::bilateral<N>(output, img);
    output.save("post-bilateral-" + std::to_string(N) + ".pfm");
}

TEST(FourccTest, BilateralFilter) {
    bilateral_test<3>();
    bilateral_test<5>();
    bilateral_test<7>();
}