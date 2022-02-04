/**
 * @file
 * @author Erik Rainey (erik.rainey@gmail.com)
 * @brief Wraps a rendered scene with some OpenCV widgets
 * @copyright Copyright (c) 2021
 */

#include <opencv2/opencv.hpp>
#include <raytrace/raytrace.hpp>
#include <basal/options.hpp>
#include <basal/module.hpp>
#include "world.hpp"
#include <functional>
#include <thread>

using namespace std::placeholders;

struct Parameters {
    size_t width;
    size_t height;
    size_t subsamples;
    size_t reflections;
    double fov;
    std::string module;
    size_t mask_threshold;
};

// QQVGA, QVGA, VGA, XGA, HD, UWGA, 4K
size_t dimensions[][2] = {{160,120}, {320,240}, {640,480}, {1024,768}, {1920,1080}, {2560,1440}, {3840, 2160}};

void on_dimension_change(int index, void *cookie) {
    Parameters *param = reinterpret_cast<Parameters *>(cookie);
    if (param) {
        param->width = dimensions[index][0];
        param->height = dimensions[index][1];
    }
};

void on_subsampling_change(int value, void *cookie) {
    Parameters *param = reinterpret_cast<Parameters *>(cookie);
    if (param) {
        param->subsamples = (double)value + 1;
    }
};

void on_reflections_change(int value, void *cookie) {
    Parameters *param = reinterpret_cast<Parameters *>(cookie);
    if (param) {
        param->reflections = (double)value + 1;
    }
};

void on_fov_change(int value, void *cookie) {
    Parameters *param = reinterpret_cast<Parameters *>(cookie);
    if (param) {
        param->fov = (double)value;
    }
};

#define my_assert(condition, statement) { \
    if ((condition) == false) { \
        printf("%s failed, %s\n", #condition, statement); \
        exit(-1); \
    } \
}

int main(int argc, char *argv[]) {
    Parameters params;
    bool verbose = false;
    bool should_quit = false;
    bool should_render = true;
    double move_unit = 5.0;
    std::string module_name;

    basal::options::config opts[] = {
        {"-w", "--width", (size_t)320, "Width of the image in pixels"},
        {"-h", "--height", (size_t)240, "Height of the image in pixels"},
        {"-b", "--subsamples", (size_t)1, "Nubmer of subsamples"},
        {"-r", "--reflections", (size_t)4, "Reflection Depth"},
        {"-f", "--fov", 55.0, "Field of View in Degrees"},
        {"-v", "--verbose", false, "Enables showing the early debugging"},
        {"-m", "--module", "", "Module to load"},
        {"-a", "--aaa", (size_t)raytrace::image::AAA_MASK_DISABLED, "Adaptive Anti-Aliasing Threshold value (255 disables)"},
    };

    basal::options::process(opts, argc, argv);
    my_assert(basal::options::find(opts, "--width", params.width), "Must have a width value");
    my_assert(basal::options::find(opts, "--height", params.height), "Must have a width value");
    my_assert(basal::options::find(opts, "--fov", params.fov), "Must have a width value");
    my_assert(basal::options::find(opts, "--verbose", verbose), "Must be able to assign bool");
    my_assert(basal::options::find(opts, "--subsamples", params.subsamples), "Must have some number of subsamples");
    my_assert(basal::options::find(opts, "--reflections", params.reflections), "Must have some number of reflections");
    my_assert(basal::options::find(opts, "--module", params.module), "Must choose a module to load");
    my_assert(basal::options::find(opts, "--aaa", params.mask_threshold), "Must be get value");
    basal::options::print(opts);

    std::string DimensionsName("Dimensions");
    std::string SubsamplingName("Subsamples");
    std::string ReflectionsName("Reflections");
    std::string FovName("FOV");

    basal::module mod(params.module.c_str());
    my_assert(mod.is_loaded(), "Must have loaded module");

    // get the symbol to load wth
    auto get_world = mod.get_symbol<raytrace::world_getter>("get_world");
    my_assert(get_world != nullptr, "Must find module to load");

    // creates a local reference to the object
    raytrace::world& world = *get_world();

    // the render image
    cv::namedWindow(world.window_name(), cv::WINDOW_AUTOSIZE);
    cv::namedWindow("mask", cv::WINDOW_AUTOSIZE);

    int default_value = 0;

    default_value = 1; // QVGA
    cv::createTrackbar(DimensionsName, world.window_name(), &default_value, dimof(dimensions), on_dimension_change, &params);
    default_value = static_cast<int>(params.subsamples - 1);
    cv::createTrackbar(SubsamplingName, world.window_name(), &default_value, 16, on_subsampling_change, &params);
    default_value = static_cast<int>(params.reflections);
    cv::createTrackbar(ReflectionsName, world.window_name(), &default_value, 16, on_reflections_change, &params);
    default_value = static_cast<int>(params.fov);
    cv::createTrackbar(FovName, world.window_name(), &default_value, 90, on_fov_change, &params);

    do {
        // what we're rendering into
        cv::Mat render_image(params.height, params.width, CV_8UC3);
        cv::Mat mask_image(params.height, params.width, CV_8UC3);

        // tiny image, simple camera placement
        raytrace::scene scene(params.height, params.width, iso::degrees(params.fov));
        raytrace::vector looking = (world.looking_at() - world.looking_from()).normalized();
        raytrace::point image_plane_principal_point = world.looking_from() + looking;
        scene.view.move_to(world.looking_from(), image_plane_principal_point);
        scene.set_background_mapper(std::bind(&raytrace::world::background, &world, std::placeholders::_1));
        world.add_to(scene);
        if (verbose) {
            scene.print(world.window_name().c_str());
        }
        if (should_render) {

            // The completion data will be stored in here, a bool per line.
            std::vector<bool> completed(params.height);
            std::fill(completed.begin(), completed.end(), false);
            bool running = true;
            auto start = std::chrono::steady_clock::now();
            auto progress_bar = [&]() -> void {
                while (running) {
                    constexpr static bool use_bar = false;
                    if constexpr (use_bar) {
                        fprintf(stdout, "\r [");
                        for (size_t i = 0; i < completed.size(); i++) {
                            fprintf(stdout, "%s", completed[i] ? "#" : "_");
                        }
                        fprintf(stdout, "]");
                    } else {
                        size_t count = 0;
                        std::for_each(completed.begin(), completed.end(),
                            [&](bool p) -> bool {
                                count += (p?1:0);
                                return p;
                            }
                        );
                        double percentage = 100.0 * count / completed.size();
                        bool done = (count == completed.size());
                        fprintf(stdout, "\r[ %0.3lf %%] rays cast: %zu dots: %zu cross: %zu intersects: %zu bounced: %zu transmitted: %zu %s ",
                            done ? 100.0 : percentage,
                            raytrace::statistics::get().cast_rays_from_camera,
                            geometry::statistics::get().dot_operations,
                            geometry::statistics::get().cross_products,
                            raytrace::statistics::get().intersections_with_objects,
                            raytrace::statistics::get().bounced_rays,
                            raytrace::statistics::get().transmitted_rays,
                            done ? "\r\n" : "");
                        // if (done) return;
                    }
                    fflush(stdout);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                fprintf(stdout, "\r\n");
            };

            auto row_notifier = [&](size_t row_index, bool is_complete) -> void {
                completed[row_index] = is_complete;
            };

            render_image.setTo(cv::Scalar(128,128,128));
            //if (params.mask_threshold < raytrace::image::AAA_MASK_DISABLED) {
                mask_image.setTo(cv::Scalar(params.mask_threshold));
                cv::imshow("mask", mask_image);
            //}
            cv::imshow(world.window_name(), render_image);
            (void)cv::waitKey(1);
            printf("Starting Render (depth=%zu, samples=%zu, aaa?=%s thresh=%zu)...\r\n",
                params.reflections,
                params.subsamples,
                params.mask_threshold == raytrace::image::AAA_MASK_DISABLED ? "no":"yes",
                params.mask_threshold);

            std::thread bar_thread(progress_bar); // thread starts
            try {
                scene.render(world.output_filename(), params.subsamples, params.reflections, row_notifier, params.mask_threshold);
            } catch (const basal::exception& e) {
                std::cout << "Caught basal::exception in scene.render()! " << std::endl;
                std::cout << "What:" << e.what() << " Why:" << e.why() << " Where:" << e.where() << std::endl;
            } catch (...) {
                std::cout << "Caught unknown exception in scene.render()! " << std::endl;
            }
            std::chrono::duration<double> diff = std::chrono::steady_clock::now() - start;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            running = false;
            bar_thread.join(); // thread stop

            std::cout << "Image Rendered in " << diff.count() << " seconds" << std::endl;

            scene.view.capture.for_each([&](size_t y, size_t x, const fourcc::rgb8& pixel) -> void {
                render_image.at<cv::Vec3b>(y, x)[0] = pixel.b;
                render_image.at<cv::Vec3b>(y, x)[1] = pixel.g;
                render_image.at<cv::Vec3b>(y, x)[2] = pixel.r;
            });
            cv::imshow(world.window_name(), render_image);
            cv::imwrite("render_image.png", render_image);

            //if (params.mask_threshold < raytrace::image::AAA_MASK_DISABLED) {
                scene.view.mask.for_each([&](size_t y, size_t x, const uint8_t& pixel) -> void {
                    mask_image.at<cv::Vec3b>(y, x)[0] = pixel;
                    mask_image.at<cv::Vec3b>(y, x)[1] = pixel;
                    mask_image.at<cv::Vec3b>(y, x)[2] = pixel;
                });
                cv::imshow("mask", mask_image);
            //}
            should_render = false;

        }
        int key = cv::waitKey(0) & 0x00FFFFFF;// wait for keypress
        printf("Pressed key %d\n", key);
        switch (key) {
            case 27: // [fall-through]
            case 'q': should_quit = true; break;  // ESC or q
            case 13: should_render = true; break;  // (CR) ENTER
            case 'w': world.looking_at().x += move_unit; break;
            case 's': world.looking_at().x -= move_unit; break;
            case 'a': world.looking_at().y += move_unit; break;
            case 'd': world.looking_at().y -= move_unit; break;
        }
    } while (not should_quit);
    return 0;
}
