///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Allows you to tune a perlin noise generator.
/// @version 0.1
/// @date 2020-05-24
/// @copyright Copyright (c) 2020
/// @note Creates a noise image in a fourcc::image, then transfoms to a cv::Mat to render. Installs a callback for
/// keystrokes to allow tuning of the perlin noise seeds.
///

#include <fourcc/image.hpp>
#include <noise/noise.hpp>
#include <opencv2/opencv.hpp>

constexpr int width = 320;
constexpr int height = 240;

// noise image
fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> noise_image(height, width);
fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> pad_image(noise::pad::dimensions, noise::pad::dimensions);
// the render image
cv::Mat render_image(height, width, CV_8UC3);
cv::Mat render_pad_image(noise::pad::dimensions, noise::pad::dimensions, CV_8UC3);
noise::precision x_scale = 1.0_p;
noise::precision y_scale = 1.0_p;
noise::precision power = 1.0_p;
noise::precision size = 32.0_p;
// The initialize scale value
noise::precision scale = 256.0_p;
// the map of noise values (generate once)
noise::pad map;

void generate_pad_image(void) {
    pad_image.for_each([&](size_t y, size_t x, fourcc::rgb8 &pixel) {
        noise::precision v = map.at(y, x);
        pixel.r = uint8_t(255u * v);
        pixel.g = uint8_t(255u * v);
        pixel.b = uint8_t(255u * v);
    });
}

void generate_noise_image(void) {
    printf("xs: %lf, ys: %lf, power: %lf, size: %lf, scale: %lf\r\n", x_scale, y_scale, power, size, scale);
    noise_image.for_each([&](size_t y, size_t x, fourcc::rgb8 &pixel) {
        noise::point pnt{(noise::precision)x, (noise::precision)y};
        noise::precision n = noise::turbulentsin(pnt, x_scale, y_scale, power, size, scale, map);
        if constexpr (noise::debug::turbulentsin) {
            printf("x,y={%lf, %lf} = %lf\n", pnt.x, pnt.y, n);
        }
        pixel.r = uint8_t(std::floor(n));
        pixel.g = uint8_t(std::floor(n));
        pixel.b = uint8_t(std::floor(n));
    });
}

void copy_to_cv_image(void) {
    noise_image.for_each([&](size_t y, size_t x, fourcc::rgb8 &pixel) {
        render_image.at<cv::Vec3b>(y, x)[0] = pixel.r;
        render_image.at<cv::Vec3b>(y, x)[1] = pixel.g;
        render_image.at<cv::Vec3b>(y, x)[2] = pixel.b;
    });
}

void copy_to_pad_image(void) {
    pad_image.for_each([&](size_t y, size_t x, fourcc::rgb8 &pixel) {
        render_pad_image.at<cv::Vec3b>(y, x)[0] = pixel.r;
        render_pad_image.at<cv::Vec3b>(y, x)[1] = pixel.g;
        render_pad_image.at<cv::Vec3b>(y, x)[2] = pixel.b;
    });
}

void on_xscale_update(int value, void *cookie __attribute__((unused))) {
    x_scale = noise::precision(value + 1);
}

void on_yscale_update(int value, void *cookie __attribute__((unused))) {
    y_scale = noise::precision(value + 1);
}

void on_power_update(int value, void *cookie __attribute__((unused))) {
    power = noise::precision(value + 1);
}

void on_size_update(int value, void *cookie __attribute__((unused))) {
    size = noise::precision(value + 1);
}

void on_scale_update(int value, void *cookie __attribute__((unused))) {
    scale = noise::precision(value + 1);
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    bool should_quit = false;
    bool should_render = true;
    int default_value = 0;
    std::string windowName{"Turbulent Sine Wave Image"};
    std::string padWindowName{"Pad"};
    std::string xscaleName{"X Scale"};
    std::string yscaleName{"Y Scale"};
    std::string powerName{"Power"};
    std::string sizeName{"Size"};
    std::string scaleName("Scale");
    cv::namedWindow(padWindowName, cv::WINDOW_AUTOSIZE);
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
    default_value = static_cast<size_t>(std::floor(x_scale));
    cv::createTrackbar(xscaleName, windowName, &default_value, 128, on_xscale_update, nullptr);
    default_value = static_cast<size_t>(std::floor(y_scale));
    cv::createTrackbar(yscaleName, windowName, &default_value, 128, on_yscale_update, nullptr);
    default_value = static_cast<size_t>(std::floor(power));
    cv::createTrackbar(powerName, windowName, &default_value, 128, on_power_update, nullptr);
    default_value = static_cast<size_t>(std::floor(size));
    cv::createTrackbar(sizeName, windowName, &default_value, 128, on_size_update, nullptr);
    default_value = static_cast<size_t>(std::floor(scale));
    cv::createTrackbar(scaleName, windowName, &default_value, 1024, on_scale_update, nullptr);

    do {
        if (should_render) {
            generate_pad_image();
            generate_noise_image();
            copy_to_pad_image();
            copy_to_cv_image();
            should_render = false;
        }
        cv::imshow(windowName, render_image);
        cv::imshow(padWindowName, render_pad_image);
        int key = cv::waitKey(0) & 0x00FFFFFF;  // wait for key-press
        printf("Pressed key %d\n", key);
        switch (key) {
            case 'r':
                map.generate();
                break;
            case 27:  // [fall-through]
            case 'q':
                should_quit = true;
                break;  // ESC or q
            case 13:
                should_render = true;
                break;  // (CR) ENTER
            case '[':
                scale -= 1.0_p;
                break;
            case ']':
                scale += 1.0_p;
                break;
            default:
                break;
        }
    } while (not should_quit);
    return 0;
}
