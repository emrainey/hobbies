/**
 * @file
 * @author Erik Rainey (erik.rainey@gmail.com)
 * @brief Allows you to tune a perlin noise generator.
 * @version 0.1
 * @date 2020-05-24
 * @copyright Copyright (c) 2020
 * @note Creates a noise image in a fourcc::image, then transfoms to a cv::Mat to render. Installs a callback for
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
cv::Mat render_image(height, width, CV_8UC3);
size_t octaves = 2;
double lacunarity = 1.0;
double gain = 0.5;
double amplitude = 152;
double frequency = 1.0;
iso::degrees angle{172};

constexpr static bool debug = false;

void generate_noise_image(void) {
    noise::vector seed = noise::convert_to_seed(angle);
    printf(
        "octaves: %zu lacunarity: %lf, gain: %lf, amplitude: %lf, frequency: %lf angle %lf, seed: %lf, "
        "%lf\r\n",
        octaves, lacunarity, gain, amplitude, frequency, angle.value, seed[0], seed[1]);
    // initialize as swapped to allow logic to work
    float min_value = std::numeric_limits<float>::max();
    float max_value = std::numeric_limits<float>::min();
    noise_image.for_each ([&](size_t y, size_t x, fourcc::yf &pixel) {
        noise::point pnt{(double)x / scale, (double)y / scale};
        float n = (float)noise::fractal_brownian(pnt, seed, octaves, lacunarity, gain, amplitude, frequency);
        if constexpr (debug) {
            printf("x,y={%lf, %lf} = %lf\n", pnt.x, pnt.y, n);
        }
        if (min_value > n) {
            min_value = n;
        }
        if (max_value < n) {
            max_value = n;
        }
        pixel.y = n;
    });
    float range = max_value - min_value;
    printf("Min: %lf, Max: %lf, Range: %lf\r\n", min_value, max_value, range);
    noise_image.for_each ([&](size_t y, size_t x, fourcc::yf &pixel) {
        float n = pixel.y;
        // if min is negative, slide it up to be zero to some larger value
        if (min_value < 0.0) {
            n -= min_value;
        }
        // if range > 255.0, expect banding! (wood grain?)
        uint8_t m = uint8_t(std::floor(n));
        render_image.at<cv::Vec3b>(y, x)[0] = m;
        render_image.at<cv::Vec3b>(y, x)[1] = m;
        render_image.at<cv::Vec3b>(y, x)[2] = m;
    });
}

void on_octaves_update(int value, void *cookie __attribute__((unused))) {
    octaves = size_t(value);
}

void on_lacunarity_update(int value, void *cookie __attribute__((unused))) {
    lacunarity = double(value + 1) / 32.0;
}

void on_gain_update(int value, void *cookie __attribute__((unused))) {
    gain = double(value + 1) / 32.0;
}

void on_amplitude_update(int value, void *cookie __attribute__((unused))) {
    amplitude = double(value + 1);
}

void on_frequency_update(int value, void *cookie __attribute__((unused))) {
    frequency = double(value + 1) / 32.0;
}

void on_seed_angle_update(int value, void *cookie __attribute__((unused))) {
    angle = iso::degrees{(double)value};
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    bool should_quit = false;
    bool should_render = true;
    int default_value = 0;
    std::string windowName{"Fractal Brownian Motion Image"};
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
    default_value = static_cast<size_t>(std::floor(octaves));
    cv::createTrackbar(std::string("Octaves"), windowName, &default_value, 20, on_octaves_update, nullptr);
    default_value = static_cast<size_t>(std::floor((lacunarity * 32.0) - 1.0));  // 0-128 are converted to 0-4 in Q.4
    cv::createTrackbar(std::string("Lucunarity"), windowName, &default_value, 128, on_lacunarity_update, nullptr);
    default_value = static_cast<size_t>(std::floor((gain * 32.0) - 1.0));  // 0-128 are converted to 0-4 in Q.4
    cv::createTrackbar(std::string("Gain"), windowName, &default_value, 128, on_gain_update, nullptr);
    default_value = static_cast<size_t>(std::floor(amplitude - 1));  // 0-255
    cv::createTrackbar(std::string("Amplitude"), windowName, &default_value, 255, on_amplitude_update, nullptr);
    default_value = static_cast<size_t>(std::floor((frequency * 32.0) - 1));  // 0-128 are then
    cv::createTrackbar(std::string("Frequency"), windowName, &default_value, 128, on_frequency_update, nullptr);
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
