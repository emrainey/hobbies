/**
 * @file
 * @author Erik Rainey (erik.rainey@gmail.com)
 * @brief Allows you to tune a random noise generator.
 * @version 0.1
 * @date 2020-05-24
 * @copyright Copyright (c) 2020
 * @note Creates a noise image in a fourcc::image, then transfoms to a cv::Mat to render. Installs a callback for
 * keystrokes to allow tuning of the perlin noise seeds.
 */

#include <linalg/trackbar.hpp>
#include <opencv2/opencv.hpp>
#include <raytrace/raytrace.hpp>

constexpr int width = 1024;
constexpr int height = 1024;

//
raytrace::image random_noise_image(height, width);
// the rendered image
cv::Mat render_image(height, width, CV_8UC3);

void on_vec_r_update(int value, void *cookie __attribute__((unused))) {
    // value is 0 to 628 (2*PI)
    // divide by 100 to get radians.
    raytrace::tuning::pseudo_random_noise_params.theta_r = iso::radians(double(value) / 100.0);
}

void on_vec_g_update(int value, void *cookie __attribute__((unused))) {
    // value is 0 to 628 (2*PI)
    // divide by 100 to get radians.
    raytrace::tuning::pseudo_random_noise_params.theta_g = iso::radians(double(value) / 100.0);
}

void on_vec_b_update(int value, void *cookie __attribute__((unused))) {
    // value is 0 to 628 (2*PI)
    // divide by 100 to get radians.
    raytrace::tuning::pseudo_random_noise_params.theta_b = iso::radians(double(value) / 100.0);
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    bool should_quit = false;
    bool should_render = true;
    bool should_animate = false;
    int default_value = 0;

    raytrace::tuning::pseudo_random_noise_params.initialize();
    std::string windowName("Random Noise Image");
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);

    // these are UoM and not easy to apply to the template unless I specialize it?
    default_value = int(raytrace::tuning::pseudo_random_noise_params.theta_r.value * 100);
    cv::createTrackbar(std::string("Vec Red"), windowName, &default_value, 628, on_vec_r_update, nullptr);
    default_value = int(raytrace::tuning::pseudo_random_noise_params.theta_g.value * 100);
    cv::createTrackbar(std::string("Vec Grn"), windowName, &default_value, 628, on_vec_g_update, nullptr);
    default_value = int(raytrace::tuning::pseudo_random_noise_params.theta_b.value * 100);
    cv::createTrackbar(std::string("Vec Blu"), windowName, &default_value, 628, on_vec_b_update, nullptr);
    // these are atomic types and easier to use the template
    linalg::Trackbar trackbar_gain("Gain", windowName, 0.125, 1.0, 100.0, 0.125,
                                   &raytrace::tuning::pseudo_random_noise_params.gain);
    linalg::Trackbar trackbar_radius("Radius", windowName, 0.125, 1.0, 100.0, 0.125,
                                     &raytrace::tuning::pseudo_random_noise_params.radius);

    do {
        if (should_animate) {
            raytrace::tuning::pseudo_random_noise_params.radius += 0.125;
            trackbar_radius.set(raytrace::tuning::pseudo_random_noise_params.radius);
            should_render = true;
        }

        if (should_render) {
            raytrace::tuning::pseudo_random_noise_params.update();
            raytrace::palette colors = {raytrace::colors::black};  // unused
            random_noise_image.generate_each([&](const raytrace::image::point &pnt) {
                raytrace::image::point uv((pnt.x / random_noise_image.width) - 0.5,
                                          (pnt.y / random_noise_image.height) - 0.5);
                return raytrace::functions::pseudo_random_noise(uv, colors);
            });
            random_noise_image.for_each([&](int y, int x, fourcc::rgb8 &pixel) {
                render_image.at<cv::Vec3b>(y, x)[0] = pixel.r;
                render_image.at<cv::Vec3b>(y, x)[1] = pixel.g;
                render_image.at<cv::Vec3b>(y, x)[2] = pixel.b;
            });
            should_render = false;
        }
        cv::imshow(windowName, render_image);
        int key = 0;
        int delay = should_animate ? 1 : 0;
        key = cv::waitKey(delay) & 0x00FFFFFF;  // wait for key-press
        printf("Pressed key %d\n", key);
        switch (key) {
            case 27:  // [fall-through]
            case 'q':
                should_quit = true;
                break;  // ESC or q
            case 'a':
                should_animate = should_animate ? false : true;
                break;
            case 13:
                should_render = true;
                break;  // (CR) ENTER
            case 'r':
                raytrace::tuning::pseudo_random_noise_params.initialize(true);
                break;
            case '[':
                raytrace::tuning::pseudo_random_noise_params.radius
                    = (raytrace::tuning::pseudo_random_noise_params.radius > 0.125)
                          ? raytrace::tuning::pseudo_random_noise_params.radius - 0.125
                          : 0.125;
                trackbar_radius.set(raytrace::tuning::pseudo_random_noise_params.radius);
                break;
            case ']':
                raytrace::tuning::pseudo_random_noise_params.radius += 0.125;
                trackbar_radius.set(raytrace::tuning::pseudo_random_noise_params.radius);
                break;
            default:
                break;
        }
    } while (not should_quit);
    return 0;
}
