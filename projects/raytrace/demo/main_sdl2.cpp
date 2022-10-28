/**
 * @file
 * @author Erik Rainey (erik.rainey@gmail.com)
 * @brief Wraps a rendered scene with some OpenCV widgets
 * @copyright Copyright (c) 2021
 */

#include <SDL.h>

#include <basal/module.hpp>
#include <basal/options.hpp>
#include <functional>
#include <raytrace/raytrace.hpp>
#include <thread>

#include "world.hpp"

using namespace std::placeholders;

struct Parameters {
    size_t dim_index;
    size_t subsamples;
    size_t reflections;
    double fov;
    std::string module;
    size_t mask_threshold;
};

// QQVGA, QVGA, VGA, XGA, HD, UWGA, 4K
size_t dimensions[][2] = {{160, 120}, {320, 240}, {640, 480}, {1024, 768}, {1920, 1080}, {2560, 1440}, {3840, 2160}};

#define my_assert(condition, statement)                       \
    {                                                         \
        if ((condition) == false) {                           \
            printf("%s failed, %s\n", #condition, statement); \
            exit(-1);                                         \
        }                                                     \
    }

int main(int argc, char *argv[]) {
    Parameters params;
    bool verbose = false;
    bool should_quit = false;
    bool should_render = true;
    double move_unit = 5.0;
    std::string module_name;

    basal::options::config opts[] = {
        {"-d", "--dims", (size_t)1, "WxH Pairs"},
        {"-b", "--subsamples", (size_t)1, "Nubmer of subsamples"},
        {"-r", "--reflections", (size_t)4, "Reflection Depth"},
        {"-f", "--fov", 55.0, "Field of View in Degrees"},
        {"-v", "--verbose", false, "Enables showing the early debugging"},
        {"-m", "--module", std::string(""), "Module to load"},
        {"-a", "--aaa", (size_t)raytrace::image::AAA_MASK_DISABLED,
         "Adaptive Anti-Aliasing Threshold value (255 disables)"},
    };

    basal::options::process(dimof(opts), opts, argc, argv);
    my_assert(basal::options::find(opts, "--dims", params.dim_index), "Must have a width value");
    my_assert(basal::options::find(opts, "--fov", params.fov), "Must have a FOV value");
    my_assert(basal::options::find(opts, "--verbose", verbose), "Must be able to assign bool");
    my_assert(basal::options::find(opts, "--subsamples", params.subsamples), "Must have some number of subsamples");
    my_assert(basal::options::find(opts, "--reflections", params.reflections), "Must have some number of reflections");
    my_assert(basal::options::find(opts, "--module", params.module), "Must choose a module to load");
    my_assert(basal::options::find(opts, "--aaa", params.mask_threshold), "Must be get value");
    basal::options::print(dimof(opts), opts);

    basal::module mod(params.module.c_str());
    my_assert(mod.is_loaded(), "Must have loaded module");

    // get the symbol to load wth
    auto get_world = mod.get_symbol<raytrace::world_getter>("get_world");
    my_assert(get_world != nullptr, "Must find module to load");

    // creates a local reference to the object
    raytrace::world &world = *get_world();

    raytrace::vector look = world.looking_from() - world.looking_at();
    raytrace::point cart = geometry::R3::origin + look.normalized();
    raytrace::point sphl = geometry::cartesian_to_spherical(cart);

    double radius = look.magnitude();
    double theta = sphl.y;
    double phi = sphl.z;

    std::cout << "ρ=" << radius << ", Θ=" << theta << ", Φ=" << phi << std::endl;

    SDL_Init(SDL_INIT_VIDEO);
    size_t width = dimensions[params.dim_index][0];
    size_t height = dimensions[params.dim_index][1];
    SDL_Window *window
        = SDL_CreateWindow("Raytracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    // Fill the surface white
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));
    SDL_UpdateWindowSurface(window);

    do {
        if (should_render) {
            double x = radius * std::sin(phi) * std::cos(theta);
            double y = radius * std::sin(phi) * std::sin(theta);
            double z = radius * std::cos(phi);

            raytrace::vector ring_rail{x, y, z};
            raytrace::point from = world.looking_at() + ring_rail;
            std::cout << "Look From: " << from << std::endl;
            std::cout << "Look At: " << world.looking_at() << " (Towards)" << std::endl;

            // tiny image, simple camera placement
            raytrace::scene scene;
            // camera setup
            scene.views.emplace_back(height, width, iso::degrees(params.fov));
            raytrace::vector looking = (world.looking_at() - from).normalized();
            raytrace::point image_plane_principal_point = from + looking;
            std::cout << "Principal: " << image_plane_principal_point << std::endl;
            scene.views[0].move_to(from, image_plane_principal_point);

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
                        std::for_each (completed.begin(), completed.end(), [&](bool p) -> bool {
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
            printf("Starting Render (depth=%zu, samples=%zu, aaa?=%s thresh=%zu)...\r\n", params.reflections,
                   params.subsamples, params.mask_threshold == raytrace::image::AAA_MASK_DISABLED ? "no" : "yes",
                   params.mask_threshold);

            std::thread bar_thread(progress_bar);  // thread starts
            try {
                scene.render(world.output_filename(), params.subsamples, params.reflections, row_notifier,
                             params.mask_threshold);
            } catch (const basal::exception &e) {
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

            SDL_LockSurface(surface);
            scene.views[0].capture.for_each ([&](size_t y, size_t x, const fourcc::rgb8 &pixel) -> void {
                SDL_Rect rect = {int(x), int(y), 1, 1};
                SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, pixel.r, pixel.g, pixel.b));
            });
            SDL_UpdateWindowSurface(window);
            SDL_UnlockSurface(surface);

            should_render = false;
        }
        int key;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYUP) {
                key = event.key.keysym.sym;
                printf("Pressed key %d\n", key);
                switch (key) {
                    case SDLK_ESCAPE:  // [fall-through]
                    case SDLK_q:
                        should_quit = true;
                        break;  // ESC or q
                    case SDLK_RETURN:
                        should_render = true;
                        break;  // (CR) ENTER
                    case SDLK_w:
                        world.looking_at().x += move_unit;
                        break;
                    case SDLK_s:
                        world.looking_at().x -= move_unit;
                        break;
                    case SDLK_a:
                        world.looking_at().y += move_unit;
                        break;
                    case SDLK_d:
                        world.looking_at().y -= move_unit;
                        break;
                }
                break;
            } else if (event.type == SDL_QUIT) {
                should_quit = true;
                break;
            }
            SDL_Delay(10);
        }
    } while (not should_quit);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
