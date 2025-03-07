///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Wraps a rendered scene with some OpenCV widgets
/// @copyright Copyright (c) 2021
///

#include <basal/module.hpp>
#include <basal/options.hpp>
#include <functional>
#include <linalg/trackbar.hpp>
#include <opencv2/opencv.hpp>
#include <raytrace/raytrace.hpp>
#include <thread>

#include "world.hpp"

using namespace std::placeholders;

struct Parameters {
    std::string dim_name;
    size_t subsamples;
    size_t reflections;
    precision fov;
    std::string module;
    size_t mask_threshold;
    double fps;
};

#define my_assert(condition, statement)                       \
    {                                                         \
        if ((condition) == false) {                           \
            printf("%s failed, %s\n", #condition, statement); \
            exit(-1);                                         \
        }                                                     \
    }

int main(int argc, char* argv[]) {
    Parameters params;
    bool verbose = false;
    bool has_changed = false;
    bool should_quit = false;
    bool should_render = true;
    precision move_unit = 5.0_p;
    std::string module_name;
    bool should_show_help = true;
    bool should_pause = false;
    bool first_render = false;

    basal::options::config opts[] = {
        {"-d", "--dims", std::string("QVGA"), "Use text video format like VGA or 2K"},
        {"-b", "--subsamples", (size_t)1, "Number of subsamples"},
        {"-r", "--reflections", (size_t)4, "Reflection Depth"},
        {"-f", "--fov", 55.0_p, "Field of View in Degrees"},
        {"-v", "--verbose", false, "Enables showing the early debugging"},
        {"-m", "--module", std::string(""), "Module to load"},
        {"-a", "--aaa", (size_t)raytrace::image::AAA_MASK_DISABLED,
         "Adaptive Anti-Aliasing Threshold value (255 disables)"},
        {"-s", "--fps", 24.0, "Frames per second"},
    };

    basal::options::process(dimof(opts), opts, argc, argv);
    my_assert(basal::options::find(opts, "--dims", params.dim_name), "Must have a text value");
    my_assert(basal::options::find(opts, "--fov", params.fov), "Must have a FOV value");
    my_assert(basal::options::find(opts, "--verbose", verbose), "Must be able to assign bool");
    my_assert(basal::options::find(opts, "--subsamples", params.subsamples), "Must have some number of subsamples");
    my_assert(basal::options::find(opts, "--reflections", params.reflections), "Must have some number of reflections");
    my_assert(basal::options::find(opts, "--module", params.module), "Must choose a module to load");
    my_assert(basal::options::find(opts, "--aaa", params.mask_threshold), "Must be get value");
    my_assert(basal::options::find(opts, "--fps", params.fps), "Must be able to get the FPS value");
    basal::options::print(dimof(opts), opts);

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

    raytrace::vector world_look = world.looking_from() - world.looking_at();
    raytrace::point coordinates_cart = geometry::R3::origin + world_look.normalized();
    raytrace::point coordinates_spherical = geometry::cartesian_to_spherical(coordinates_cart);
    // find the initial spherical coordinates
    precision radius = world_look.magnitude();
    precision theta = coordinates_spherical.y;
    precision phi = coordinates_spherical.z;

    auto [width, height] = fourcc::dimensions(params.dim_name);
    printf("%s => Width: %zu, Height: %zu\n", params.dim_name.c_str(), width, height);
    if (height == 0 or width == 0) {
        printf("Invalid dimensions\n");
        return -1;
    }
    cv::Size frame_size(width, height);
    cv::VideoWriter video_writer("video.mpg", cv::VideoWriter::fourcc('M', 'P', 'G', '2'), params.fps, frame_size,
                                 true);

    linalg::Trackbar<precision> trackbar_theta("Camera Theta", world.window_name(), -iso::pi, theta, iso::pi,
                                               iso::pi / 8, &theta);
    linalg::Trackbar<precision> trackbar_phi("Camera Phi", world.window_name(), 0, phi, iso::pi, iso::pi / 16, &phi);
    radius = (world.looking_from() - world.looking_at()).magnitude();
    linalg::Trackbar<precision> trackbar_radius("Camera Radius", world.window_name(), 1.0, radius, 100.0, 5.0, &radius);
    linalg::Trackbar<size_t> trackbar_subsamples("Subsamples", world.window_name(), 1, params.subsamples, 16, 1,
                                                 &params.subsamples);
    linalg::Trackbar<size_t> trackbar_reflect("Reflections", world.window_name(), 0, params.reflections, 10, 1,
                                              &params.reflections);
    linalg::Trackbar<precision> trackbar_fov("FOV", world.window_name(), 10, params.fov, 90, 5, &params.fov);

    // the camera anchor points
    raytrace::animation::anchors anchors = world.get_anchors();
    raytrace::animation::Animator animator{params.fps, anchors};

    // what we're rendering into, these need to exist outside the loop
    // so that the displays don't get corrupted from the memory being deallocated or freed
    cv::Mat render_image(height, width, CV_8UC3);
    cv::Mat mask_image(height, width, CV_8UC3);

    do {
        if (should_render and animator) {
            raytrace::animation::Attributes cam = animator();  // get the first set of parameters
            std::cout << "Look From: " << cam.from << "(Camera at)" << std::endl;
            std::cout << "Look At: " << cam.at << " (Towards)" << std::endl;
            std::cout << "FOV: " << cam.fov.value << " (Degrees)" << std::endl;

            // tiny image, simple camera placement
            raytrace::scene scene;
            // camera setup
            raytrace::camera view(height, width, cam.fov);
            raytrace::vector looking = (cam.at - cam.from).normalized();
            raytrace::point image_plane_principal_point = cam.from + looking;
            std::cout << "Principal: " << image_plane_principal_point << std::endl;
            view.move_to(cam.from, image_plane_principal_point);

            scene.set_background_mapper(std::bind(&raytrace::world::background, &world, std::placeholders::_1));
            world.add_to(scene);
            if (verbose) {
                scene.print(world.window_name().c_str());
            }

            // The completion data will be stored in here, a bool per line.
            std::vector<bool> completed(height);
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
                        std::for_each(completed.begin(), completed.end(), [&](bool p) -> bool {
                            count += (p ? 1 : 0);
                            return p;
                        });
                        double percentage = 100.0 * count / completed.size();
                        bool done = (count == completed.size());
                        fprintf(stdout,
                                "\r[ %0.3lf %%] rays cast: %zu dots: %zu cross: %zu intersects: %zu bounced: %zu "
                                "transmitted: %zu %s ",
                                done ? 100.0 : percentage, raytrace::statistics::get().cast_rays_from_camera,
                                geometry::statistics::get().dot_operations, geometry::statistics::get().cross_products,
                                raytrace::statistics::get().intersections_with_objects,
                                raytrace::statistics::get().bounced_rays, raytrace::statistics::get().transmitted_rays,
                                done ? "\r\n" : "");
                        // if (done) return;
                    }
                    fflush(stdout);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                fprintf(stdout, "\r\n");
            };

            auto row_notifier = [&](size_t row_index, bool is_complete) -> void { completed[row_index] = is_complete; };

            if (first_render) {
                render_image.setTo(cv::Scalar(128, 128, 128));
                // if (params.mask_threshold < raytrace::image::AAA_MASK_DISABLED) {
                mask_image.setTo(cv::Scalar(params.mask_threshold));
                first_render = false;
            }
            cv::imshow("mask", mask_image);
            //}
            cv::imshow(world.window_name(), render_image);
            (void)cv::waitKey(1);
            printf("Starting Render (depth=%zu, samples=%zu, aaa?=%s thresh=%zu)...\r\n", params.reflections,
                   params.subsamples, params.mask_threshold == raytrace::image::AAA_MASK_DISABLED ? "no" : "yes",
                   params.mask_threshold);

            std::thread bar_thread(progress_bar);  // thread starts
            try {
                scene.render(view, world.output_filename(), params.subsamples, params.reflections, row_notifier,
                             params.mask_threshold);
            } catch (basal::exception const& e) {
                std::cout << "Caught basal::exception in scene.render()! " << std::endl;
                std::cout << "What:" << e.what() << " Why:" << e.why() << " Where:" << e.where() << std::endl;
            } catch (...) {
                std::cout << "Caught unknown exception in scene.render()! " << std::endl;
            }
            std::chrono::duration<double> diff = std::chrono::steady_clock::now() - start;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            running = false;
            bar_thread.join();  // thread stop

            std::cout << "Image Rendered in " << diff.count() << " seconds" << std::endl;

            view.capture.for_each([&](size_t y, size_t x, fourcc::rgb8 const& pixel) -> void {
                render_image.at<cv::Vec3b>(y, x)[0] = pixel.b;
                render_image.at<cv::Vec3b>(y, x)[1] = pixel.g;
                render_image.at<cv::Vec3b>(y, x)[2] = pixel.r;
            });
            cv::imshow(world.window_name(), render_image);
            cv::imwrite("render_image.png", render_image);
            video_writer.write(render_image);
            // if (params.mask_threshold < raytrace::image::AAA_MASK_DISABLED) {
            view.mask.for_each([&](size_t y, size_t x, uint8_t const& pixel) -> void {
                mask_image.at<cv::Vec3b>(y, x)[0] = pixel;
                mask_image.at<cv::Vec3b>(y, x)[1] = pixel;
                mask_image.at<cv::Vec3b>(y, x)[2] = pixel;
            });
            cv::imshow("mask", mask_image);
            //)
            should_render = false;
            if (animator) {
                should_pause = false;
            } else {
                should_pause = true;
            }
        } else {
            if (should_show_help) {
                printf("Press ENTER to render, ESC or q to quit\n");
                printf(
                    "Use w/s to move camera -phi/+phi, a/d to move camera -theta/+theta, r/f to move camera -r/+r\n");
                printf("Use KP 8/2 to look camera +y/-y, 4/6 to look camera -x/x, 9/3 to look camera +z/-z\n");
                should_show_help = false;
            }
        }
        int key = cv::waitKey(should_pause ? 0 : 1) & 0x00'FF'FF'FF;  // wait for keypress or not
        printf("Pressed key %d\n", key);
        switch (key) {
            case 0x00'FF'FF'FF:  // no key pressed
                should_render = true;
                break;
            case 27:
                [[fallthrough]];
            case 'q':
                should_quit = true;
                break;  // ESC or q
            case 13:
                if (has_changed) {
                    should_render = true;
                    has_changed = false;
                } else {
                    printf("No changes, not rendering\n");
                }
                break;  // (CR) ENTER
            case 'h':
                should_show_help = true;
                break;
            case '6':
                world.looking_at().x += move_unit;
                has_changed = true;
                break;
            case '4':
                world.looking_at().x -= move_unit;
                has_changed = true;
                break;
            case '8':
                world.looking_at().y += move_unit;
                has_changed = true;
                break;
            case '2':
                world.looking_at().y -= move_unit;
                has_changed = true;
                break;
            case '9':
                world.looking_at().z += move_unit;
                has_changed = true;
                break;
            case '3':
                world.looking_at().z -= move_unit;
                has_changed = true;
                break;
        }
    } while (not should_quit);
    video_writer.release();
    return 0;
}
