/**
 * @file
 * @author Erik Rainey (erik.rainey@gmail.com)
 * @brief Allows you to tune a perlin noise generator.
 * @version 0.1
 * @date 2020-05-24
 * @copyright Copyright (c) 2020
 * @note Creates a noise image in a fourcc::image, then transfoms to a cv::Mat to render. Installs a callback for keystrokes to allow tuning of the perlin noise seeds.
 */

#include <opencv2/opencv.hpp>
#include <noise/noise.hpp>
#include <fourcc/image.hpp>

constexpr int width = 1024;
constexpr int height = 1024;

int angle_pos = 0;
// random starting angle
iso::radians angle(-11*iso::pi/57);
// The perlin noise seed for the pattern
noise::vector seed = noise::convert_to_seed(angle);
// not sure how this effects the pattern yet
double gain = 1.0;
// scale of the pattern
double scale = 16.0;
// noise image
fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> noise_image(height, width);
// the render image
cv::Mat render_image(height, width, CV_8UC3);

void generate_noise_image(void) {
    noise_image.for_each([&](int y, int x, fourcc::rgb8& pixel) {
        noise::point pnt(x, y);
        double n = noise::perlin(pnt, scale, seed, gain);
        pixel.r = n * 255;
        pixel.g = n * 255;
        pixel.b = n * 255;
    });
}

void copy_to_cv_image(void) {
    noise_image.for_each([&](int y, int x, fourcc::rgb8& pixel) {
        render_image.at<cv::Vec3b>(y, x)[0] = pixel.r;
        render_image.at<cv::Vec3b>(y, x)[1] = pixel.g;
        render_image.at<cv::Vec3b>(y, x)[2] = pixel.b;
    });
}

void on_gain_update(int value, void *cookie __attribute__((unused))) {
    gain = M_PI * value / 1E2;
}

void on_seed_angle_update(int value, void *cookie __attribute__((unused))) {
    if (0 <= value and value <= 1023) {
        // save the position
        angle_pos = value;
        // value from 0 - 1024
        // convert to 0 - 2.0
        double u = (double)value / 512.0;
        // converts to 0 - 2pi
        double v = u * iso::pi;
        // then to -pi to +pi
        angle = iso::radians(v - iso::pi);
        seed = noise::convert_to_seed(angle);
    }
}

void on_scale_update(int value, void *cookie __attribute__((unused))) {
    scale = double(value + 1);
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    bool should_quit = false;
    bool should_render = true;
    char buffer[1024];
    int default_value = 0;
    std::string windowName("Perlin Image");
    std::string gainName("Gain");
    std::string seedName("Seed Angle");
    std::string scaleName("Feature Scale");
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
    default_value = static_cast<size_t>(std::floor(gain * 1E2 / M_PI));
    cv::createTrackbar(gainName, windowName, &default_value, 1000, on_gain_update, nullptr);
    //default_value = static_cast<size_t>(std::floor((seed[0] * 1E2) / M_PI + 500));
    // first convert from -1.0 to 1.0 by div by pi
    // then + 1.0 to get 0.0 - 2.0 then by 512 and floor
    angle_pos = std::floor(((angle.value / iso::pi) + 1.0) * 512);
    cv::createTrackbar(seedName, windowName, &angle_pos, 1024, on_seed_angle_update, nullptr); // from -pi to +pi by going from 0 to 1024
    default_value = static_cast<size_t>(std::floor(scale - 1));
    cv::createTrackbar(scaleName, windowName, &default_value, 128, on_scale_update, nullptr);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> huge(-10000.0,10000.0);
    std::uniform_real_distribution<> modr(-10.0, 10.0);

    do {
        if (should_render) {
            generate_noise_image();
            copy_to_cv_image();
            should_render = false;
        }
        snprintf(buffer, sizeof(buffer), "Perlin Noise Angle:%lf (pos:%d) seed:{%lf,%lf} gain:%lf scale:%lf", angle.value, angle_pos, seed[0], seed[1], gain, scale);
        printf("%s\n", buffer);
        cv::imshow(windowName, render_image);
        int key = cv::waitKey(0) & 0x00FFFFFF;// wait for keypress
        printf("Pressed key %d\n", key);
        switch (key) {
            case 27: // [fall-through]
            case 'q': should_quit = true; break;  // ESC or q
            case 13: should_render = true; break;  // (CR) ENTER
            case '[': scale -= 1.0; break;
            case ']': scale += 1.0; break;
            case 'x': gain = M_PI * huge(gen); break;
            case 'w': gain += M_PI / 100.0; break;
            case 's': gain -= M_PI / 100.0; break;
            case 'd': on_seed_angle_update(angle_pos + 1, nullptr); break;
            case 'a': on_seed_angle_update(angle_pos - 1, nullptr); break;
            default: break;
        }
    } while (not should_quit);
    return 0;
}
