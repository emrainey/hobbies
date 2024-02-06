///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Renders a raytraced image of a bunch of speheres
/// @date 2021-01-02
/// @copyright Copyright (c) 2021
///

#include <linalg/trackbar.hpp>
#include <opencv2/opencv.hpp>
#include <raytrace/raytrace.hpp>

size_t width = 640;
size_t height = 480;
iso::degrees fov(55);

using namespace raytrace;
using namespace basal::literals;

size_t ambient_color_index = 0;
precision ambient_scale = 0.1_p;
size_t diffuse_color_index = 0;
precision smoothiness = mediums::smoothness::polished;
precision tightness = mediums::roughness::tight;
color color_choices[] = {colors::white, colors::red,     colors::green,  colors::blue,
                         colors::cyan,  colors::magenta, colors::yellow, colors::black};
constexpr size_t number_of_colors = dimof(color_choices);

int main(int argc, char* argv[]) {
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

    // the render image
    cv::Mat render_image(height, width, CV_8UC3);
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);

    linalg::Trackbar<size_t> trackbar_diff_color("Diffuse Color", windowName, 0u, diffuse_color_index, number_of_colors,
                                                 1u, &diffuse_color_index);
    linalg::Trackbar<size_t> trackbar_amb_color("Ambient Color", windowName, 0u, ambient_color_index, number_of_colors,
                                                1u, &ambient_color_index);
    linalg::Trackbar trackbar_amb_scale("Ambient Scale", windowName, 0.0_p, ambient_scale, 1.0_p, 0.1_p, &ambient_scale);
    linalg::Trackbar trackbar_smoothness("Smoothness", windowName, 0.0_p, smoothiness, 1.0_p, 0.1_p, &smoothiness);
    linalg::Trackbar trackbar_tightness("Tightness", windowName, 0.0_p, tightness, 100.0_p, 5.0_p, &tightness);

    precision move_unit = 5.0_p;
    raytrace::point look_from(0, 0, 60);
    vector looking{{0.8_p, 0.8_p, -1}};
    // looking.normalize();
    raytrace::point look_at = look_from + looking;

    vector up = R3::basis::Z;
    vector down = -R3::basis::Z;

    raytrace::objects::plane ground(R3::origin, up, 1.0_p);
    mediums::plain plain_green(colors::green, mediums::ambient::dim, colors::green, mediums::smoothness::barely,
                               mediums::roughness::loose);
    ground.material(&plain_green);

    raytrace::point sphere_center(40, 40, 10);
    raytrace::objects::sphere test_sphere(sphere_center, 10);

    // define the light
    lights::beam sunlight(raytrace::vector{-2, 2, -1}, colors::white, 1E11);

    do {
        // tiny image, simple camera placement
        scene scene;
        camera view(height, width, fov);
        view.move_to(look_from, look_at);
        // scene.background = colors::black;

        // define some surfaces
        mediums::plain testsurface(color_choices[ambient_color_index], ambient_scale,
                                   color_choices[diffuse_color_index], smoothiness, tightness);
        test_sphere.material(&testsurface);

        // add the objects to the scene.
        scene.add_object(&test_sphere);
        scene.add_object(&ground);
        scene.add_light(&sunlight);

        scene.print("Scene");
        if (should_render) {
            render_image.setTo(cv::Scalar(128, 128, 128));
            cv::imshow(windowName, render_image);
            (void)cv::waitKey(1);
            printf("Starting Render (depth=%zu, samples=%zu)...\r\n", reflection_depth, subsamples);
            scene.render(view, "demo_sphere_surfaces.tga", subsamples, reflection_depth, std::nullopt, show_bar);
            // copy to the cv::Mat
            view.capture.for_each ([&](size_t y, size_t x, const fourcc::rgb8& pixel) -> void {
                render_image.at<cv::Vec3b>(y, x)[0] = pixel.b;
                render_image.at<cv::Vec3b>(y, x)[1] = pixel.g;
                render_image.at<cv::Vec3b>(y, x)[2] = pixel.r;
            });
            cv::imshow(windowName, render_image);
            should_render = false;
            printf("Ambient Scale: %lf Reflectivity: %lf Roughness: %lf\r\n", ambient_scale, smoothiness, tightness);
            printf("Render Complete!\r\n");
        }
        int key = cv::waitKey(0) & 0x00FFFFFF;  // wait for keypress
        printf("Pressed key %d\n", key);
        switch (key) {
            case 27:  // [fall-through]
            case 'q':
                should_quit = true;
                break;  // ESC or q
            case 13:
                should_render = true;
                break;  // (CR) ENTER
            case 'w':
                look_at.x += move_unit;
                break;
            case 's':
                look_at.x -= move_unit;
                break;
            case 'a':
                look_at.y += move_unit;
                break;
            case 'd':
                look_at.y -= move_unit;
                break;
            case 'f':
                fov = iso::degrees(60);
                break;
        }
    } while (not should_quit);
}
