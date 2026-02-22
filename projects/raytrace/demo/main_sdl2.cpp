///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Wraps a rendered scene with some OpenCV widgets
/// @copyright Copyright (c) 2021
///

#if defined(__APPLE__)
// Homebrew/CMake doesn't include the normal include root, it prepends SDL2 on the -I path
// -isystem /opt/homebrew/Cellar/sdl2/2.32.10/include/SDL2
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include <basal/module.hpp>
#include <basal/options.hpp>
#include <functional>
#include <raytrace/raytrace.hpp>
#include <thread>
#include <mutex>

#include "world.hpp"

using namespace std::placeholders;
static constexpr bool live_preview{true};

struct Parameters {
    std::string dim_name;
    size_t subsamples;
    size_t reflections;
    precision fov;
    std::string module;
    size_t mask_threshold;
    double separation;
    bool filter;
    bool tone_mapping;
};

int main(int argc, char *argv[]) {
    Parameters params;
    bool verbose = false;
    bool has_changed = true;
    bool should_quit = false;
    bool should_render = false;
    precision move_unit = 5.0_p;
    std::string module_name;
    bool should_show_help = true;

    basal::options::config opts[] = {{"-d", "--dims", std::string("QVGA"), "Use text video format like VGA or 2K"},
                                     {"-b", "--subsamples", (size_t)1, "Number of subsamples"},
                                     {"-r", "--reflections", (size_t)4, "Reflection Depth"},
                                     {"-f", "--fov", 55.0_p, "Field of View in Degrees"},
                                     {"-v", "--verbose", false, "Enables showing the early debugging"},
                                     {"-m", "--module", std::string(""), "Module to load"},
                                     {"-a", "--aaa", (size_t)raytrace::image::AAA_MASK_DISABLED,
                                      "Adaptive Anti-Aliasing Threshold value (255 disables)"},
                                     {"-s", "--separation", 0.0_p, "Stereo Camera view separation"},
                                    {"-e", "--filter", false, "Whether to do a post-process filter on the capture before saving"},
                                    {"-t", "--tone-mapping", false, "Whether to apply tone mapping to the final image"}};

    basal::options::process(dimof(opts), opts, argc, argv);
    basal::exit_unless(basal::options::find(opts, "--dims", params.dim_name), __FILE__, __LINE__,
                       "Must have a text value");
    basal::exit_unless(basal::options::find(opts, "--fov", params.fov), __FILE__, __LINE__, "Must have a FOV value");
    basal::exit_unless(basal::options::find(opts, "--verbose", verbose), __FILE__, __LINE__,
                       "Must be able to assign bool");
    basal::exit_unless(basal::options::find(opts, "--subsamples", params.subsamples), __FILE__, __LINE__,
                       "Must have some number of subsamples");
    basal::exit_unless(basal::options::find(opts, "--reflections", params.reflections), __FILE__, __LINE__,
                       "Must have some number of reflections");
    basal::exit_unless(basal::options::find(opts, "--module", params.module), __FILE__, __LINE__,
                       "Must choose a module to load");
    basal::exit_unless(basal::options::find(opts, "--aaa", params.mask_threshold), __FILE__, __LINE__,
                       "Must be get value");
    basal::exit_unless(basal::options::find(opts, "--separation", params.separation), __FILE__, __LINE__,
                       "Must be able to assign a double");
    basal::exit_unless(basal::options::find(opts, "--filter", params.filter), __FILE__, __LINE__,
                       "Must be able to assign a bool");
    basal::exit_unless(basal::options::find(opts, "--tone-mapping", params.tone_mapping), __FILE__, __LINE__,
                       "Must be able to assign a bool");
    basal::options::print(dimof(opts), opts);

    basal::module mod(params.module.c_str());
    basal::exit_unless(mod.is_loaded(), __FILE__, __LINE__, "Must have loaded module");

    // get the symbol to load wth
    auto get_world = mod.get_symbol<raytrace::world_getter>("get_world");
    basal::exit_unless(get_world != nullptr, __FILE__, __LINE__, "Must find module to load");

    // creates a local reference to the object
    raytrace::world &world = *get_world();
    auto [width, height] = fourcc::dimensions(params.dim_name);
    printf("%s => Width: %zu, Height: %zu\n", params.dim_name.c_str(), width, height);
    if (height == 0 or width == 0) {
        printf("Invalid dimensions\n");
        return -1;
    }
    size_t stride = 2 * width;  // requires Left Right layout
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window
        = SDL_CreateWindow("Raytracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, stride, height, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    // Fill the surface white
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));
    SDL_UpdateWindowSurface(window);

    do {
        if (should_render) {
            std::cout << "Look From: " << world.looking_from() << std::endl;
            std::cout << "Look At: " << world.looking_at() << " (Towards)" << std::endl;

            // tiny image, simple camera placement
            raytrace::scene scene;
            // camera setup
            raytrace::stereo_camera stereo_view(height, width, iso::degrees(params.fov), params.separation,
                                                raytrace::stereo_camera::Layout::LeftRight);
            stereo_view.move_to(world.looking_from(), world.looking_at());
            stereo_view.print(std::cout, "Stereo View");
            scene.set_background_mapper(std::bind(&raytrace::world::background, &world, std::placeholders::_1));
            world.add_to(scene);
            if (verbose) {
                scene.print(std::cout, world.window_name().c_str());
            }
            scene.set_ambient_light(world.ambient());

            size_t view_offset = 0u;
            for (auto &view : stereo_view) {
                // The completion data will be stored in here, a bool per line.
                std::vector<bool> completed(view.capture.height);
                std::fill(completed.begin(), completed.end(), false);
                bool running = true;
                std::mutex window_mutex;
                auto start = std::chrono::steady_clock::now();
                auto progress_bar = [&]() -> void {
                    while (running) {
                        static constexpr bool use_bar = false;
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
                            double percentage = 100.0_p * count / completed.size();
                            bool done = (count == completed.size());
                            fprintf(
                                stdout,
                                "\r[ %0.3lf %%] rays cast: %zu dots: %zu cross: %zu 2r: %zu 3r: %zu 4r: %zu "
                                "intersects: %zu (%zu/%zu/%zu) bounced: %zu "
                                "transmitted: %zu missed: %zu bounds: %zu %s",
                                done ? 100.0_p : percentage, raytrace::statistics::get().cast_rays_from_camera,
                                geometry::statistics::get().dot_operations, geometry::statistics::get().cross_products,
                                linalg::statistics::get().quadratic_roots, linalg::statistics::get().cubic_roots,
                                linalg::statistics::get().quartic_roots,
                                raytrace::statistics::get().intersections_with_objects,
                                raytrace::statistics::get().intersections_with_point,
                                raytrace::statistics::get().intersections_with_points,
                                raytrace::statistics::get().intersections_with_line,
                                raytrace::statistics::get().bounced_rays, raytrace::statistics::get().transmitted_rays,
                                raytrace::statistics::get().missed_rays,
                                raytrace::statistics::get().intersections_with_bounds, done ? "\r\n" : "");
                            // if (done) return;
                        }
                        fflush(stdout);
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    fprintf(stdout, "\r\n");
                };
                auto row_notifier = [&](size_t row_index, bool is_complete) -> void {
                    completed[row_index] = is_complete;
                    if (live_preview) {
                        // surface = SDL_GetWindowSurface(window);
                        bool should_lock = SDL_MUSTLOCK(surface);
                        if (should_lock) {
                            if (0 != SDL_LockSurface(surface)) {
                                printf("Failed to lock surface!\n");
                                return;
                            }
                        }
                        view.capture.for_each([&](size_t y, size_t x, raytrace::image::PixelStorageType const &pixel) -> void {
                            if (row_index != y)
                                return;  // if it's not this row, skip it
                            uint8_t *pixels = reinterpret_cast<uint8_t *>(surface->pixels);  // this is double width!
                            size_t offset = (y * surface->pitch) + (x * sizeof(fourcc::bgra))
                                            + (view_offset * sizeof(fourcc::bgra));

                            // clamp, then convert to sRGB
                            raytrace::color value(pixel.components.r, pixel.components.g, pixel.components.b, pixel.components.i);
                            value.clamp();
                            value.ToEncoding(fourcc::Encoding::GammaCorrected);
                            auto srgb = value.to_<fourcc::PixelFormat::RGB8>();

                            // B G R A order in SDL2
                            pixels[offset + 0u] = srgb.components.b;
                            pixels[offset + 1u] = srgb.components.g;
                            pixels[offset + 2u] = srgb.components.r;
                            pixels[offset + 3u] = 0u;
                        });
                        if (window_mutex.try_lock()) {
                            SDL_UpdateWindowSurface(window);  // calling this across threads is bad
                            window_mutex.unlock();
                        }
                        if (should_lock) {
                            SDL_UnlockSurface(surface);
                        }
                    }
                };
                printf("Starting Render (depth=%zu, samples=%zu, aaa?=%s thresh=%zu)...\r\n", params.reflections,
                       params.subsamples, params.mask_threshold == raytrace::image::AAA_MASK_DISABLED ? "no" : "yes",
                       params.mask_threshold);

                std::thread bar_thread(progress_bar);  // thread starts
                try {
                    scene.render(view, world.output_filename(), params.subsamples, params.reflections, row_notifier,
                                 params.mask_threshold, params.filter, params.tone_mapping);
                } catch (basal::exception const &e) {
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

                bool should_lock = SDL_MUSTLOCK(surface);
                if (should_lock) {
                    if (0 != SDL_LockSurface(surface)) {
                        printf("Failed to lock surface!\n");
                        return -1;
                    }
                }
                view.capture.for_each([&](size_t y, size_t x, raytrace::image::PixelStorageType const &pixel) -> void {
                    uint8_t *pixels = reinterpret_cast<uint8_t *>(surface->pixels);
                    size_t offset
                        = (y * surface->pitch) + (x * sizeof(fourcc::bgra)) + (view_offset * sizeof(fourcc::bgra));
                    // clamp, then convert to sRGB
                    raytrace::color value(pixel.components.r, pixel.components.g, pixel.components.b, pixel.components.i);
                    value.clamp();
                    value.ToEncoding(fourcc::Encoding::GammaCorrected );
                    auto srgb = value.to_<fourcc::PixelFormat::RGB8>();

                    // B G R A order
                    pixels[offset + 0u] = srgb.components.b;
                    pixels[offset + 1u] = srgb.components.g;
                    pixels[offset + 2u] = srgb.components.r;
                    pixels[offset + 3u] = 0u;
                });
                SDL_UpdateWindowSurface(window);
                if (should_lock) {
                    SDL_UnlockSurface(surface);
                }
                printf("Separation: %lf\n", params.separation);
                if (not basal::nearly_zero(params.separation)) {
                    view_offset += width;
                } else {
                    break;
                }
            }
            if (not basal::nearly_zero(params.separation)) {
                stereo_view.merge_images().save(world.output_filename());
            }
            should_render = false;
        } else {
            if (should_show_help) {
                printf("Press ENTER to render, ESC or q to quit\n");
                printf("Use w/s to move camera +y/-y, a/d to move camera -x/x, r/f to move camera +z/-z\n");
                printf("Use KP 8/2 to look camera +y/-y, 4/6 to look camera -x/x, 9/3 to look camera +z/-z\n");
                printf("Use t/g to change FOV +5/-5\n");
                printf("Use z/x to change separation +0.25/-0.25\n");
                printf("Use u/j to change subsamples +1/-1\n");
                should_show_help = false;
            }
        }
        int key;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYUP) {
                key = event.key.keysym.sym;
                printf("Pressed key %d\n", key);
                switch (key) {
                    case SDLK_ESCAPE:
                        [[fallthrough]];
                    case SDLK_q:
                        should_quit = true;
                        break;  // ESC or q
                    case SDLK_RETURN:
                        if (has_changed) {
                            should_render = true;
                            has_changed = false;
                        } else {
                            printf("No changes, not rendering\n");
                        }
                        break;  // (CR) ENTER
                    case SDLK_h:
                        should_show_help = true;
                        break;
                    case SDLK_w:
                        world.looking_from().y() += move_unit;
                        has_changed = true;
                        break;
                    case SDLK_s:
                        world.looking_from().y() -= move_unit;
                        has_changed = true;
                        break;
                    case SDLK_a:
                        world.looking_from().x() -= move_unit;
                        has_changed = true;
                        break;
                    case SDLK_d:
                        world.looking_from().x() += move_unit;
                        has_changed = true;
                        break;
                    case SDLK_f:
                        world.looking_from().z() -= move_unit;
                        has_changed = true;
                        break;
                    case SDLK_r:
                        world.looking_from().z() += move_unit;
                        has_changed = true;
                        break;
                    case SDLK_t:
                        params.fov += move_unit;
                        has_changed = true;
                        break;
                    case SDLK_g:
                        params.fov -= move_unit;
                        has_changed = true;
                        break;
                    case SDLK_KP_8:
                        world.looking_at().y() += move_unit;
                        has_changed = true;
                        break;
                    case SDLK_KP_2:
                        world.looking_at().y() -= move_unit;
                        has_changed = true;
                        break;
                    case SDLK_KP_4:
                        world.looking_at().x() -= move_unit;
                        has_changed = true;
                        break;
                    case SDLK_KP_6:
                        world.looking_at().x() += move_unit;
                        has_changed = true;
                        break;
                    case SDLK_KP_9:
                        world.looking_at().z() += move_unit;
                        has_changed = true;
                        break;
                    case SDLK_KP_3:
                        world.looking_at().z() -= move_unit;
                        has_changed = true;
                        break;
                    case SDLK_z:
                        params.separation += 0.25_p;
                        has_changed = true;
                        break;
                    case SDLK_x:
                        params.separation -= 0.25_p;
                        has_changed = true;
                        break;
                    case SDLK_u:
                        params.subsamples++;
                        has_changed = true;
                        break;
                    case SDLK_j:
                        params.subsamples--;
                        has_changed = true;
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
