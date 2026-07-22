#include <fourcc/image.hpp>
#include <noise/noise.hpp>
#include <opencv2/opencv.hpp>
#include <linalg/trackbar.hpp>

constexpr int width = 1024;
constexpr int height = 1024;

int angle_pos = 0;
iso::radians angle{-11 * iso::pi / 57};
noise::vector seed = noise::convert_to_seed(angle);
noise::precision gain = 32.0_p;
noise::precision scale = 32.0_p;
fourcc::image<fourcc::PixelFormat::RGB8> noise_image(height, width);
cv::Mat render_image(height, width, CV_8UC3);

void generate_noise_image(void) {
    noise_image.for_each([&](int y, int x, fourcc::rgb8 &pixel) {
        noise::point pnt{noise::precision(x), noise::precision(y)};
        noise::precision n = noise::simplex(pnt, scale, seed, gain);
        pixel.components.r = n * 255;
        pixel.components.g = n * 255;
        pixel.components.b = n * 255;
    });
}

void copy_to_cv_image(void) {
    noise_image.for_each([&](int y, int x, fourcc::rgb8 &pixel) {
        render_image.at<cv::Vec3b>(y, x)[0] = pixel.components.r;
        render_image.at<cv::Vec3b>(y, x)[1] = pixel.components.g;
        render_image.at<cv::Vec3b>(y, x)[2] = pixel.components.b;
    });
}

void on_seed_angle_update(int value, void *cookie __attribute__((unused))) {
    if (0 <= value and value <= 1023) {
        angle_pos = value;
        noise::precision u = (noise::precision)value / 512.0_p;
        noise::precision v = u * iso::pi;
        angle = iso::radians(v - iso::pi);
        seed = noise::convert_to_seed(angle);
    }
}

void on_scale_update(int value, void *cookie __attribute__((unused))) {
    scale = noise::precision(value + 1);
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    bool should_quit = false;
    bool should_render = true;
    char buffer[1024];
    int default_value = 0;
    size_t gain_fraction = 0u;
    std::string windowName{"Simplex Image"};
    std::string gainName{"Gain"};
    std::string seedName{"Seed Angle"};
    std::string scaleName{"Feature Scale"};
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);

    gain_fraction = gain / 0.125_p;
    linalg::Trackbar<size_t> trackbar_gain(gainName, windowName, 0, gain_fraction, 1024, 1, &gain_fraction);
    angle_pos = std::floor(((angle.value / iso::pi) + 1.0_p) * 512);
    cv::createTrackbar(seedName, windowName, &angle_pos, 1024, on_seed_angle_update, nullptr);
    default_value = static_cast<size_t>(std::floor(scale - 1));
    cv::createTrackbar(scaleName, windowName, &default_value, 128, on_scale_update, nullptr);

    do {
        gain = 0.125_p * gain_fraction;
        if (should_render) {
            generate_noise_image();
            copy_to_cv_image();
            should_render = false;
        }
        snprintf(buffer, sizeof(buffer), "Simplex Noise Angle:%lf seed:{%lf,%lf} gain:%lf scale:%lf", angle.value,
                 seed[0], seed[1], gain, scale);
        printf("%s\n", buffer);

        cv::imshow(windowName, render_image);
        int key = cv::waitKey(0) & 0x00FFFFFF;
        switch (key) {
            case 27:
            case 'q':
                should_quit = true;
                break;
            case 13:
                should_render = true;
                break;
            case '[':
                scale -= 1.0_p;
                break;
            case ']':
                scale += 1.0_p;
                break;
            case 'w':
                gain += iso::pi / 100.0_p;
                break;
            case 's':
                gain -= iso::pi / 100.0_p;
                break;
            case 'd':
                on_seed_angle_update(angle_pos + 1, nullptr);
                break;
            case 'a':
                on_seed_angle_update(angle_pos - 1, nullptr);
                break;
            default:
                break;
        }
    } while (not should_quit);
    return 0;
}
