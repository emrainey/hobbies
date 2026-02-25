///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Renders a ray-traced image of single spheres with different surface properties.
/// @date 2021-01-02
/// @copyright Copyright (c) 2021
///

#include <linalg/trackbar.hpp>
#include <opencv2/opencv.hpp>
#include <raytrace/raytrace.hpp>

size_t width = 640;
size_t height = 480;
iso::degrees fov(25);

using namespace raytrace;
using namespace basal::literals;

size_t ambient_color_index = 0;
precision ambient_scale = 0.1_p;
size_t diffuse_color_index = 1;
precision smoothiness = mediums::smoothness::polished;
precision tightness = mediums::roughness::tight;

color const color_choices[] = {colors::white, colors::red,     colors::green,  colors::blue,
                               colors::cyan,  colors::magenta, colors::yellow, colors::black};
constexpr size_t number_of_colors = dimof(color_choices);

raytrace::mediums::metal const* metal_choices[]
    = {&raytrace::mediums::metals::aluminum, &raytrace::mediums::metals::brass,     &raytrace::mediums::metals::bronze,
       &raytrace::mediums::metals::chrome,   &raytrace::mediums::metals::copper,    &raytrace::mediums::metals::gold,
       &raytrace::mediums::metals::silver,   &raytrace::mediums::metals::stainless, &raytrace::mediums::metals::steel,
       &raytrace::mediums::metals::tin};
size_t metal_index = 3u;
constexpr size_t number_of_metals = dimof(metal_choices);

precision repeat = 7.0_p;
precision x_scale = 1.0_p;
precision y_scale = 1.0_p;
precision scale = 256.0_p;
precision power = 1.0_p;
precision size = 32.0_p;

enum class Surfaces : int {
    _First = 0,
    Plain = _First,
    Metals = 1,
    Checkers = 2,
    Dots = 3,
    Stripes = 4,
    TurbSin = 5,
    //---------------
    _Last = TurbSin,
} surfaces = Surfaces::Metals;
int surf;

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
    color ambient;
    std::string windowName("RayTracing Surface Tester");

    // the render image
    cv::Mat render_image(height, width, CV_8UC3);
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);

    linalg::Trackbar<size_t> trackbar_metal_choice("Metal Choice", windowName, 0u, metal_index, number_of_metals, 1U,
                                                   &metal_index);
    linalg::Trackbar<size_t> trackbar_diff_color("Diffuse Color", windowName, 0u, diffuse_color_index, number_of_colors,
                                                 1u, &diffuse_color_index);
    linalg::Trackbar<size_t> trackbar_amb_color("Ambient Color", windowName, 0u, ambient_color_index, number_of_colors,
                                                1u, &ambient_color_index);
    linalg::Trackbar trackbar_amb_scale("Ambient Scale", windowName, 0.0_p, ambient_scale, 1.0_p, 0.1_p,
                                        &ambient_scale);
    linalg::Trackbar trackbar_smoothness("Smoothness", windowName, 0.0_p, smoothiness, 1.0_p, 0.1_p, &smoothiness);
    linalg::Trackbar trackbar_tightness("Tightness", windowName, 0.0_p, tightness, 100.0_p, 5.0_p, &tightness);
    linalg::Trackbar trackbar_repeat("Repeat", windowName, 0.125_p, repeat, 10.0_p, 0.125_p, &repeat);
    linalg::Trackbar trackbar_x_scale("X Scale", windowName, 0.125_p, x_scale, 1024.0_p, 0.125_p, &x_scale);
    linalg::Trackbar trackbar_y_scale("Y Scale", windowName, 0.125_p, y_scale, 1024.0_p, 0.125_p, &y_scale);
    linalg::Trackbar trackbar_power("Power", windowName, 1.0_p, power, 10.0_p, 1.0_p, &power);
    linalg::Trackbar trackbar_size("Size", windowName, 0.125_p, size, 256.0_p, 0.125_p, &size);
    // determines the depth of the value for each pixel this is for 8 bits
    // linalg::Trackbar trackbar_scale("Scale", windowName, 0.125_p, scale, 10.0_p, 0.125_p, &scale);
    linalg::Trackbar trackbar_surface("Surface", windowName, 0, surf, 5, 1, &surf);

    raytrace::point look_from(0, 0, 60);
    vector looking{{0.8_p, 0.8_p, -1}};
    // looking.normalize();
    raytrace::point look_at = look_from + looking;

    vector up = R3::basis::Z;
    vector down = -R3::basis::Z;

    raytrace::objects::plane ground;
    mediums::plain plain_green(colors::green, mediums::ambient::dim, colors::green, mediums::smoothness::barely,
                               mediums::roughness::loose);

    ground.material(&plain_green);

    raytrace::point sphere_center(40, 40, 10);
    raytrace::objects::sphere test_sphere(sphere_center, 10);

    // define the light
    lights::beam sunlight(raytrace::vector{-2, 2, -1}, colors::white, lights::intensities::full);

    do {
        // tiny image, simple camera placement
        scene scene;
        camera view(height, width, fov);
        view.move_to(look_from, look_at);
        // scene.background = colors::black;

        printf("Amb=%lf Smooth=%lf Tight=%lf\n", ambient_scale, smoothiness, tightness);
        printf("Repeat=%lf Scale=%lf X Scale=%lf Y Scale=%lf Power=%lf Size=%lf\n", repeat, scale, x_scale, y_scale,
               power, size);

        // define some surfaces
        color dark = color_choices[ambient_color_index];
        color light = color_choices[diffuse_color_index];
        ambient = color_choices[ambient_color_index];
        ambient.intensity(ambient_scale);
        scene.set_ambient_light(ambient);
        mediums::plain test_surface(color_choices[ambient_color_index], ambient_scale,
                                    color_choices[diffuse_color_index], smoothiness, tightness);
        mediums::checkerboard checkers{repeat, dark, light};
        mediums::dots dots{repeat, dark, light};
        mediums::stripes stripes{repeat, dark, light};
        mediums::turbsin turbsin{1024.0_p, x_scale, y_scale, power, scale, size, dark, light};

        mediums::medium const* medium = nullptr;
        surfaces = static_cast<Surfaces>(surf);
        switch (surfaces) {
            case Surfaces::Plain:
                printf("Plain Surface\n");
                medium = &test_surface;
                break;
            case Surfaces::Metals:
                printf("Metals Surface\n");
                medium = metal_choices[metal_index];
                break;
            case Surfaces::Checkers:
                printf("Checkers Surface\n");
                checkers.mapper(std::bind(&objects::sphere::map, &test_sphere, std::placeholders::_1));
                medium = &checkers;
                break;
            case Surfaces::Dots:
                printf("Dots Surface\n");
                dots.mapper(std::bind(&objects::sphere::map, &test_sphere, std::placeholders::_1));
                medium = &dots;
                break;
            case Surfaces::Stripes:
                printf("Stripes Surface\n");
                stripes.mapper(std::bind(&objects::sphere::map, &test_sphere, std::placeholders::_1));
                medium = &stripes;
                break;
            case Surfaces::TurbSin:
                printf("TurbSin Surface\n");
                turbsin.mapper(std::bind(&objects::sphere::map, &test_sphere, std::placeholders::_1));
                medium = &turbsin;
                break;
        }
        test_sphere.material(medium);
        // medium->smoothness = smoothiness;
        // medium->tightness = tightness;

        // add the objects to the scene.
        scene.add_object(&test_sphere);
        scene.add_object(&ground);
        scene.add_light(&sunlight);

        std::cout << "Scene: " << scene << std::endl;
        if (should_render) {
            render_image.setTo(cv::Scalar(128, 128, 128));
            cv::imshow(windowName, render_image);
            (void)cv::waitKey(1);
            printf("Starting Render (depth=%zu, samples=%zu)...\r\n", reflection_depth, subsamples);
            scene.render(view, "demo_sphere_surfaces.tga", subsamples, reflection_depth, std::nullopt, show_bar);
            // copy to the cv::Mat
            view.capture.for_each([&](size_t y, size_t x, raytrace::image::PixelStorageType const& pixel) -> void {
                raytrace::color value(pixel.components.r, pixel.components.g, pixel.components.b, pixel.components.i);
                value.clamp();
                value.ToEncoding(fourcc::Encoding::GammaCorrected);
                auto srgb = value.to_<fourcc::PixelFormat::RGB8>();
                render_image.at<cv::Vec3b>(y, x)[0] = srgb.components.b;
                render_image.at<cv::Vec3b>(y, x)[1] = srgb.components.g;
                render_image.at<cv::Vec3b>(y, x)[2] = srgb.components.r;
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
            case 'r':
                fov -= iso::degrees{5};
                break;
            case 'f':
                fov += iso::degrees{5};
                break;
            case 'w':
                if (surfaces < Surfaces::_Last) {
                    surfaces = static_cast<Surfaces>(to_underlying(surfaces) + 1);
                }
                break;
            case 's':
                if (surfaces > Surfaces::_First) {
                    surfaces = static_cast<Surfaces>(to_underlying(surfaces) - 1);
                }
                break;
        }
    } while (not should_quit);
}
