/**
 * @file
 * @author Erik Rainey (erik.rainey@gmail.com)
 * @brief Renders a raytraced image of a bunch of speheres
 * @date 2021-01-02
 * @copyright Copyright (c) 2021
 */

#include <opencv2/opencv.hpp>
#include <raytrace/raytrace.hpp>

size_t width = 640;
size_t height = 480;
iso::degrees fov(55);

using namespace raytrace;

size_t scale = 100;
size_t tightness_scale = 1;

size_t ambient_color_index = 0;
double ambient_scale = 0.1;
size_t specular_color_index = 0;
size_t diffuse_color_index = 0;
double smoothiness = smoothness::polished;
double tightness = roughness::tight;
color color_choices[] = {colors::white, colors::red, colors::green, colors::blue, colors::cyan, colors::magenta, colors::yellow, colors::black};
constexpr size_t number_of_colors = dimof(color_choices);

void on_ambient_color_change(int value, void *cookie) {
    (void)cookie;
    ambient_color_index = value;
}

void on_ambient_change(int value, void *cookie) {
    (void)cookie;
    ambient_scale = (double)value / scale;
}

void on_diffuse_color_change(int value, void *cookie) {
    (void)cookie;
    diffuse_color_index = value;
}

void on_smoothiness_change(int value, void *cookie) {
    (void)cookie;
    smoothiness = (double)value / scale;
}

void on_tightness_change(int value, void *cookie) {
    (void)cookie;
    tightness = (double)value * tightness_scale;
}

int main(int argc, char *argv[]) {
    using namespace raytrace;
    bool show_bar = true;
    if (argc > 1) {
        if (strncmp(argv[1], "-s", 3) == 0) {
            show_bar = false;
        }
    }

    constexpr bool per_pixel = false;
    size_t subsamples = 1;
    size_t reflection_depth = 4;
    bool should_quit = false;
    bool should_render = true;
    std::string windowName("RayTracing Surface Tester");

    std::string ambientColorName("Ambient Color");
    std::string ambientName("Ambient Scale");
    std::string diffuseColorName("Diffuse Color");
    std::string smoothinessName("Smoothness");
    std::string tightnessName("Tightness");

    // the render image
    cv::Mat render_image(height, width, CV_8UC3);
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);

    int default_value = 0;

    default_value = static_cast<int>(ambient_color_index);
    cv::createTrackbar(ambientColorName, windowName, &default_value, number_of_colors-1, on_ambient_color_change, nullptr);
    default_value = static_cast<int>(ambient_scale * scale);
    cv::createTrackbar(ambientName, windowName, &default_value, scale+1, on_ambient_change, nullptr);
    default_value = static_cast<int>(diffuse_color_index);
    cv::createTrackbar(diffuseColorName, windowName, &default_value, number_of_colors-1, on_diffuse_color_change, nullptr);
    default_value = static_cast<int>(smoothiness * scale);
    cv::createTrackbar(smoothinessName, windowName, &default_value, scale+1, on_smoothiness_change, nullptr);
    default_value = static_cast<int>(tightness / tightness_scale);
    cv::createTrackbar(tightnessName, windowName, &default_value, scale+1, on_tightness_change, nullptr);

    double move_unit = 5.0;
    raytrace::point look_from(0, 0, 60);
    vector looking{{0.8, 0.8, -1}};
    //looking.normalize();
    raytrace::point look_at = look_from + looking;

    vector up = R3::basis::Z;
    vector down = -R3::basis::Z;


    raytrace::plane ground(R3::origin, up, 1.0);
    plain plain_green(colors::green, 0.1, colors::green, 0.2, 100.0);
    ground.material(&plain_green);

    raytrace::point sphere_center(40, 40, 10);
    raytrace::sphere test_sphere(sphere_center, 10);

    // define the light
    beam sunlight(raytrace::vector{-2, 2, -1}, colors::white, 1E11);

    do {
        // tiny image, simple camera placement
        scene scene(height, width, fov);
        scene.view.move_to(look_from, look_at);
        //scene.background = colors::black;

        // define some surfaces
        plain testsurface(color_choices[ambient_color_index], ambient_scale, color_choices[diffuse_color_index], smoothiness, tightness);
        test_sphere.material(&testsurface);

        // add the objects to the scene.
        scene.add_object(&test_sphere);
        scene.add_object(&ground);
        scene.add_light(&sunlight);

        scene.print("Scene");
        if (should_render) {
            render_image.setTo(cv::Scalar(128,128,128));
            cv::imshow(windowName, render_image);
            (void)cv::waitKey(1);
            printf("Starting Render (depth=%zu, samples=%zu)...\r\n", reflection_depth, subsamples);
            scene.render("demo_sphere_surfaces.ppm", subsamples, reflection_depth, std::nullopt, show_bar);
            // copy to the cv::Mat
            scene.view.capture.for_each([&](size_t y, size_t x, const fourcc::rgb8& pixel) -> void {
                render_image.at<cv::Vec3b>(y, x)[0] = pixel.b;
                render_image.at<cv::Vec3b>(y, x)[1] = pixel.g;
                render_image.at<cv::Vec3b>(y, x)[2] = pixel.r;
            });
            cv::imshow(windowName, render_image);
            should_render = false;
            printf("Ambient Scale: %lf Reflectivity: %lf Roughness: %lf\r\n", ambient_scale, smoothiness, tightness);
            printf("Render Complete!\r\n");
        }
        int key = cv::waitKey(0) & 0x00FFFFFF;// wait for keypress
        printf("Pressed key %d\n", key);
        switch (key) {
            case 27: // [fall-through]
            case 'q': should_quit = true; break;  // ESC or q
            case 13: should_render = true; break;  // (CR) ENTER
            case 'w': look_at.x += move_unit; break;
            case 's': look_at.x -= move_unit; break;
            case 'a': look_at.y += move_unit; break;
            case 'd': look_at.y -= move_unit; break;
            case 'f': fov = iso::degrees(60); break;
        }
    } while (not should_quit);
}
