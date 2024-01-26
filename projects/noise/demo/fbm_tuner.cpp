/**
 * @file
 * @author Erik Rainey (erik.rainey@gmail.com)
 * @brief Allows you to tune a perlin noise generator.
 * @version 0.1
 * @date 2020-05-24
 * @copyright Copyright (c) 2020
 * @note Creates a noise image in a fourcc::image, then transforms to a cv::Mat to render. Installs a callback for
 * keystrokes to allow tuning of the perlin noise seeds.
 */

#include <fourcc/image.hpp>
#include <noise/noise.hpp>
#include <opencv2/opencv.hpp>

constexpr int width = 1024;
constexpr int height = 1024;
constexpr int scale = 1024;

// noise image
fourcc::image<fourcc::yf, fourcc::pixel_format::YF> noise_image(height, width);
// the render image
cv::Mat render_image(height, width, CV_32FC1);  // CV_8UC3);
size_t octaves = 6;
noise::precision lacunarity = 2.0;
noise::precision gain = 0.5;
noise::precision amplitude = 0.5;
noise::precision frequency = 1.0;
iso::degrees angle{81};
noise::precision magnitude = 78.0;
noise::vector seed = noise::convert_to_seed(angle);
constexpr static bool debug = false;

void generate_noise_image(void) {
    printf("octaves: %zu lacunarity: %lf, gain: %lf, amplitude: %lf, frequency: %lf\r\n", octaves, lacunarity, gain,
           amplitude, frequency);
    noise_image.for_each ([&](size_t y, size_t x, fourcc::yf &pixel) {
        noise::point pnt{(noise::precision)x / scale, (noise::precision)y / scale};
        pixel.y = (float)noise::fractal_brownian(pnt, seed, octaves, lacunarity, gain, amplitude, frequency);
    });
    noise_image.for_each ([&](size_t y, size_t x, fourcc::yf &pixel) {
        float n = pixel.y;  // the intensity, not the cartesian y
        // if min is negative, slide it up to be zero to some larger value
        render_image.at<float>(y, x) = n;
    });
}

void on_octaves_update(int value, void *cookie __attribute__((unused))) {
    octaves = size_t(value);
}
void on_seed_angle_update(int value, void *cookie __attribute__((unused))) {
    angle = iso::degrees{(noise::precision)value};
    seed = noise::convert_to_seed(angle);
    seed *= magnitude;
    printf("angle %lf, seed: %lf, %lf\r\n", angle.value, seed[0], seed[1]);
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    bool should_quit = false;
    bool should_render = true;
    int default_value = 0;
    std::string windowName{"Fractal Brownian Motion Image"};
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
    default_value = static_cast<size_t>(std::floor(octaves));
    cv::createTrackbar(std::string("Octaves"), windowName, &default_value, 20, on_octaves_update, nullptr);
    default_value = static_cast<size_t>(std::floor(angle.value));
    cv::createTrackbar(std::string("Seed Angle"), windowName, &default_value, 360, on_seed_angle_update, nullptr);
    do {
        if (should_render) {
            generate_noise_image();
            should_render = false;
            printf("Render Complete!\r\n");
        }
        cv::imshow(windowName, render_image);
        int key = cv::waitKey(0) & 0x00FFFFFF;  // wait for key-press
        printf("Pressed key %d\n", key);
        switch (key) {
            case 27:  // [fall-through]
            case 'q':
                should_quit = true;
                break;  // ESC or q
            case 13:
                should_render = true;
                break;  // (CR) ENTER
            default:
                break;
        }
    } while (not should_quit);
    return 0;
}
