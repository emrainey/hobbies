/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Allows you to view a function mapper
/// @version 0.1
/// @date 2025-03-18
/// @copyright Copyright (c) 2025

#include <linalg/trackbar.hpp>
#include <opencv2/opencv.hpp>
#include <raytrace/raytrace.hpp>

constexpr int width = 1024;
constexpr int height = 1024;

raytrace::image func_image(height, width);
cv::Mat render_image(height, width, CV_8UC3);

enum class Functions : int {
    Simple = 0,
    Diagonal = 1,
    Dots = 2,
    Grid = 3,
    Checkerboard = 4,
    HappyFace = 5,
    Random = 6,
    Contours = 7,
    _max = Contours,
};

Functions mapping_function_name = Functions::Simple;

void on_function_update(int value, void *) {
    mapping_function_name = static_cast<Functions>(value);
    std::cout << "Mapping Set to " << value << std::endl;
}

int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused))) {
    bool should_quit = false;
    bool should_render = true;
    bool should_animate = false;

    int default_value = 0;

    std::string windowName("Function Mapping Image");
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);

    // these are UoM and not easy to apply to the template unless I specialize it?
    default_value = to_underlying(mapping_function_name);
    cv::createTrackbar(std::string("Function"), windowName, &default_value, to_underlying(Functions::_max),
                       on_function_update, nullptr);

    raytrace::tuning::pseudo_random_noise_params.update();
    do {
        if (should_animate) {
            should_render = true;
        }

        if (should_render) {
            raytrace::palette palette
                = {raytrace::colors::black, raytrace::colors::white, raytrace::colors::blue,   raytrace::colors::yellow,
                   raytrace::colors::red,   raytrace::colors::green, raytrace::colors::violet, raytrace::colors::pink};
            func_image.generate_each([&](raytrace::image::point const &pnt) {
                // create a UV vector which is centered in the middle of the image. This means
                // the u, v, values are between -1 and 1
                raytrace::image::point uv(2.0_p * ((pnt.x() / func_image.width) - 0.5_p),
                                          2.0_p * ((pnt.y() / func_image.height) - 0.5_p));
                // std::cout << "UV: " << uv << " with mapping " << to_underlying(mapping_function_name) << std::endl;
                switch (mapping_function_name) {
                    case Functions::Simple:
                        return raytrace::functions::simple(uv, palette);
                    case Functions::Diagonal:
                        return raytrace::functions::diagonal(uv, palette);
                    case Functions::Dots:
                        return raytrace::functions::dots(uv, palette);
                    case Functions::Grid:
                        return raytrace::functions::grid(uv, palette);
                    case Functions::Checkerboard:
                        return raytrace::functions::checkerboard(uv, palette);
                    case Functions::HappyFace:
                        return raytrace::functions::happy_face(uv, palette);
                    case Functions::Random:
                        return raytrace::functions::pseudo_random_noise(uv, palette);
                    case Functions::Contours:
                        return raytrace::functions::contours(uv, palette);
                    default:
                        return raytrace::colors::pink;
                }
            });
            func_image.for_each([&](int y, int x, raytrace::image::PixelStorageType &pixel) {
                raytrace::color value(pixel.components.r, pixel.components.g, pixel.components.b, pixel.components.i);
                value.clamp();
                value.ToEncoding(fourcc::Encoding::GammaCorrected);
                auto srgb = value.to_<fourcc::PixelFormat::RGB8>();
                render_image.at<cv::Vec3b>(y, x)[0] = srgb.components.b;
                render_image.at<cv::Vec3b>(y, x)[1] = srgb.components.g;
                render_image.at<cv::Vec3b>(y, x)[2] = srgb.components.r;
            });
            should_render = false;
        }
        cv::imshow(windowName, render_image);
        int key = 0;
        int delay = should_animate ? 1 : 0;
        key = cv::waitKey(delay) & 0x00FFFFFF;  // wait for key-press
        printf("Pressed key %d\n", key);
        switch (key) {
            case 27:  // ESC
                [[fallthrough]];
            case 'q':
                should_quit = true;
                break;
            case 13:  // (CR) ENTER
                should_render = true;
                break;
            default:
                break;
        }
    } while (not should_quit);
    return 0;
}
