///
/// @file
/// @author Erik Rainey (erik.rainey@gmail.com)
/// @brief Displays a Fractal image using SDL2 with live updating and user input to pan and zoom
/// @copyright Copyright (c) 2026
///

#if defined(__APPLE__)
// Homebrew/CMake doesn't include the normal include root, it prepends SDL2 on the -I path
// -isystem /opt/homebrew/Cellar/sdl2/2.32.10/include/SDL2
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include "basal/basal.hpp"
#include "basal/options.hpp"
#include "fractals/fractals.hpp"

#include <atomic>
#include <thread>

constexpr static bool generation_debug{false};
constexpr static bool conversion_debug{false};
constexpr static bool live_preview{true};

using precision = basal::precision;

enum class FractalChoice : int {
    Mandelbrot = 2,
    Mandelbrot3 = 3,
};

struct Parameters {
    precision x_min;
    precision x_max;
    precision y_min;
    precision y_max;
    size_t width;
    size_t height;
    size_t max_iterations;
    std::string filename;
    FractalChoice choice;
};

fractals::image::PixelStorageType mandelbrot(std::complex<precision> const& c, uint16_t max_iterations) {
    std::complex<precision> z = c;
    fractals::image::PixelStorageType iteration_count{0U};

    while (((z.real()*z.real()) + (z.imag()*z.imag())) <= 4.0_p and iteration_count < max_iterations) {
        z = z * z + c;
        iteration_count++;
    }
    return iteration_count;
}


fractals::image::PixelStorageType mandelbrot3(std::complex<precision> const& c, uint16_t max_iterations) {
    std::complex<precision> z = c;
    fractals::image::PixelStorageType iteration_count{0U};

    while (((z.real()*z.real()) + (z.imag()*z.imag())) <= 4.0_p and iteration_count < max_iterations) {
        z = z * z * z + c;
        iteration_count++;
    }
    return iteration_count;
}

int main(int argc, char *argv[]) {
    Parameters params;
    bool should_render = true;
    bool should_quit = false;
    double scalar = 0.1;
    basal::options::config opts[] = {
        {"-xmin", "--x-min", -2.5_p, "Minimum X value"},
        {"-xmax", "--x-max", 1.0_p, "Maximum X value"},
        {"-ymin", "--y-min", -1.0_p, "Minimum Y value"},
        {"-ymax", "--y-max", 1.0_p, "Maximum Y value"},
        {"-w", "--width", (size_t)1920, "Image Width"},
        {"-h", "--height", (size_t)1080, "Image Height"},
        {"-m", "--max-iterations", (size_t)72, "Maximum Iterations"},
        {"-f", "--file", std::string("fractal.tga"), "Output File Name"},
        {"-c", "--choice", (int)FractalChoice::Mandelbrot, "Fractal Choice (Mandelbrot or Mandelbrot3)"},
    };

    basal::options::process(basal::dimof(opts), opts, argc, argv);
    basal::exit_unless(basal::options::find(opts, "--x-min", params.x_min), __FILE__, __LINE__,
                       "Must have a text value");
    basal::exit_unless(basal::options::find(opts, "--x-max", params.x_max), __FILE__, __LINE__,
                       "Must have a text value");
    basal::exit_unless(basal::options::find(opts, "--y-min", params.y_min), __FILE__, __LINE__,
                       "Must have a text value");
    basal::exit_unless(basal::options::find(opts, "--y-max", params.y_max), __FILE__, __LINE__,
                       "Must have a text value");
    basal::exit_unless(basal::options::find(opts, "--width", params.width), __FILE__, __LINE__,
                       "Must have a text value");
    basal::exit_unless(basal::options::find(opts, "--height", params.height), __FILE__, __LINE__,
                       "Must have a text value");
    basal::exit_unless(basal::options::find(opts, "--max-iterations", params.max_iterations), __FILE__, __LINE__,
                       "Must have a text value");
    basal::exit_unless(basal::options::find(opts, "--file", params.filename), __FILE__, __LINE__,
                       "Must have a text value");
    basal::exit_unless(basal::options::find(opts, "--choice", (int&)params.choice), __FILE__, __LINE__,
                       "Must have a text value");
    basal::options::print(basal::dimof(opts), opts);

    fractals::image computation_image{params.height, params.width};
    fourcc::image<fourcc::PixelFormat::RGBId> color_image{params.height, params.width};
    fourcc::image<fourcc::PixelFormat::RGBA> display_image{params.height, params.width};

    size_t total_pixels = params.width * params.height;
    uint16_t max_iterations = static_cast<uint16_t>(params.max_iterations);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window
        = SDL_CreateWindow("Fractal Images", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, params.width, params.height, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    // Fill the surface white
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));
    SDL_UpdateWindowSurface(window);

    auto mandelbrot_sampler = [&](fractals::image::point const& p, uint16_t max_iterations) -> fractals::image::PixelStorageType {
        // Map point to complex plane
        precision h = static_cast<precision>(params.height);
        precision w = static_cast<precision>(params.width);
        precision x0 = ((p.x() / w) * (params.x_max - params.x_min)) + params.x_min;
        precision i0 = ((p.y() / h) * (params.y_max - params.y_min)) + params.y_min;
        std::complex<precision> const c{x0, i0};
        fractals::image::PixelStorageType pixel;
        if (params.choice == FractalChoice::Mandelbrot) {
            pixel = mandelbrot(c, max_iterations);
        } else if (params.choice == FractalChoice::Mandelbrot3) {
            pixel = mandelbrot3(c, max_iterations);
        } else {
            pixel = mandelbrot(c, max_iterations);
        }
        if (generation_debug) {
            printf("Point (%f, %f) => Complex (%f, %f) => Iterations %u\n",
                   p.x(), p.y(), c.real(), c.imag(), static_cast<uint16_t>(pixel));
        }
        return pixel;
    };

    auto preview = [&](SDL_Surface* surface, SDL_Window* window,
                       std::vector<std::atomic<uint8_t>> const& completed_rows,
                       std::vector<uint8_t>& copied_rows, fractals::image& computation_image,
                       uint16_t max_iterations) -> void {
        // now copy to SDL surface but first must lock
        bool should_lock = SDL_MUSTLOCK(surface);
        if (should_lock) {
            if (0 != SDL_LockSurface(surface)) {
                printf("Failed to lock surface!\n");
                return;
            }
        }
        // copy the row over to the display image and change it to RGBA
        computation_image.for_each([surface, &completed_rows, &copied_rows, max_iterations](size_t y, size_t x, fractals::image::PixelStorageType const& iteration) {
            if (completed_rows[y].load(std::memory_order_acquire) == 0u) {
                return;
            }
            if (copied_rows[y] != 0u) {
                return;
            }
            fourcc::bgra *pixel = reinterpret_cast<fourcc::bgra *>(surface->pixels);
            size_t index = (y * (surface->pitch/sizeof(fourcc::bgra))) + x;
            if (iteration == max_iterations) {
                // bounded
                pixel[index].components.b = 0u;
                pixel[index].components.g = 0u;
                pixel[index].components.r = 0u;
                pixel[index].components.a = 255u;
            } else {
                precision normalized_iterations = precision(iteration) / precision(max_iterations);
                precision jet_scalar = normalized_iterations * 2.0_p - 1.0_p;
                auto rgb_pixel = fourcc::jet(jet_scalar).data();
                pixel[index].components.b = static_cast<uint8_t>(basal::clamp(rgb_pixel.components.b * 255.0_p, 0.0_p, 255.0_p));
                pixel[index].components.g = static_cast<uint8_t>(basal::clamp(rgb_pixel.components.g * 255.0_p, 0.0_p, 255.0_p));
                pixel[index].components.r = static_cast<uint8_t>(basal::clamp(rgb_pixel.components.r * 255.0_p, 0.0_p, 255.0_p));
                pixel[index].components.a = 255u;
            }
            if (x == surface->w - 1) {
                copied_rows[y] = 1u;
            }
        });
        SDL_UpdateWindowSurface(window);  // calling this across threads is bad
        if (should_lock) {
            SDL_UnlockSurface(surface);
        }
    };

    do {
        if (should_render) {
            printf("Generating fractal with %zu x %zu = %zu pixels, %u iterations, x={%lf, %lf}, y={%lf, %lf}\n", params.width, params.height, total_pixels, max_iterations, params.x_min, params.x_max, params.y_min, params.y_max);
            std::vector<std::atomic<uint8_t>> completed(params.height);
            for (auto& value : completed) {
                value.store(0u, std::memory_order_relaxed);
            }
            bool preview_active = live_preview;

            auto render_preview_thread = [&]() {
                std::vector<uint8_t> copied_rows(params.height, 0u);
                while (preview_active) {
                    preview(surface, window, completed, copied_rows, computation_image, params.max_iterations);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    if (std::all_of(completed.begin(), completed.end(), [](std::atomic<uint8_t> const& value) {
                            return value.load(std::memory_order_acquire) != 0u;
                        }) and
                        std::all_of(copied_rows.begin(), copied_rows.end(), [](uint8_t value) { return value != 0u; })) {
                        preview_active = false;
                    }
                }
            };

            std::thread render_preview_thread_handle(render_preview_thread);

            auto row_notifier = [&](size_t row_index, bool completed_flag) {
                completed[row_index].store(1u, std::memory_order_release);
            };

            // only needs to be done once with a possibly high iteration count
            computation_image.generate_sample(mandelbrot_sampler, max_iterations, row_notifier);
            render_preview_thread_handle.join();
            should_render = false;
        }
        int key;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYUP) {
                key = event.key.keysym.sym;
                printf("Pressed key %d\n", key);
                switch (key) {
                    case SDLK_ESCAPE:
                        should_quit = true;
                        break;
                    case SDLK_a:
                        params.x_min -= scalar;
                        params.x_max -= scalar;
                        should_render = true;
                        break;
                    case SDLK_s:
                        params.y_min += scalar;
                        params.y_max += scalar;
                        should_render = true;
                        break;
                    case SDLK_d:
                        params.x_min += scalar;
                        params.x_max += scalar;
                        should_render = true;
                        break;
                    case SDLK_w:
                        params.y_min -= scalar;
                        params.y_max -= scalar;
                        should_render = true;
                        break;
                    case SDLK_q: {
                        // zoom in
                        scalar *= 0.875;
                        precision x_center = (params.x_min + params.x_max) / 2.0_p;
                        precision y_center = (params.y_min + params.y_max) / 2.0_p;
                        precision x_range = (params.x_max - params.x_min) * 0.5_p;
                        precision y_range = (params.y_max - params.y_min) * 0.5_p;
                        params.x_min = x_center - x_range * 0.9;
                        params.x_max = x_center + x_range * 0.9;
                        params.y_min = y_center - y_range * 0.9;
                        params.y_max = y_center + y_range * 0.9;
                        should_render = true;
                        break;
                    }
                    case SDLK_e: {
                        // zoom out
                        scalar *= 1.125;
                        precision x_center = (params.x_min + params.x_max) / 2.0_p;
                        precision y_center = (params.y_min + params.y_max) / 2.0_p;
                        precision x_range = (params.x_max - params.x_min) * 0.5_p;
                        precision y_range = (params.y_max - params.y_min) * 0.5_p;
                        params.x_min = x_center - x_range * 1.1;
                        params.x_max = x_center + x_range * 1.1;
                        params.y_min = y_center - y_range * 1.1;
                        params.y_max = y_center + y_range * 1.1;
                        should_render = true;
                        break;
                    }
                    case SDLK_f: {
                        printf("Saved image to %s\n", params.filename.c_str());
                        computation_image.save("fractal.pgm");
                        // map the computation image to a color image
                        computation_image.for_each([&color_image, params](size_t y, size_t x, fractals::image::PixelStorageType& pixel) {
                            uint16_t iteration = static_cast<uint16_t>(pixel);
                            fourcc::image<fourcc::PixelFormat::RGBId>::PixelStorageType color_pixel;
                            if (iteration == params.max_iterations) {
                                // bounded
                                color_pixel.components.b = 0.0_p;
                                color_pixel.components.g = 0.0_p;
                                color_pixel.components.r = 0.0_p;
                                color_pixel.components.i = 1.0_p;
                            } else {
                                precision normalized_iterations = precision(iteration) / precision(params.max_iterations);
                                precision jet_scalar = normalized_iterations * 2.0_p - 1.0_p;
                                color_pixel = fourcc::jet(jet_scalar).data();
                            }
                            if (conversion_debug) {
                                printf("Pixel (%zu, %zu) => Iterations %u => Color (%f, %f, %f)\n",
                                    x, y, iteration,
                                    color_pixel.components.r,
                                    color_pixel.components.g,
                                    color_pixel.components.b);
                            }
                            color_image.at(y, x) = color_pixel;
                        });
                        color_image.save(params.filename);
                        break;
                    }
                    case SDLK_t:
                        // increase iterations
                        params.max_iterations = static_cast<size_t>(static_cast<precision>(params.max_iterations) * 1.25_p);
                        max_iterations = static_cast<uint16_t>(params.max_iterations);
                        should_render = true;
                        break;
                    case SDLK_g:
                        // decrease iterations
                        params.max_iterations = static_cast<size_t>(static_cast<precision>(params.max_iterations) * 0.75_p);
                        if (params.max_iterations < 10u) {
                            params.max_iterations = 10u;
                        }
                        max_iterations = static_cast<uint16_t>(params.max_iterations);
                        should_render = true;
                        break;
                }
                break; // pop out of while loop
            } else if (event.type == SDL_QUIT) {
                std::cout << "Quitting ... " << std::endl;
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