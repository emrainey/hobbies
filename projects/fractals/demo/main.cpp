#include "basal/basal.hpp"
#include "basal/options.hpp"
#include "fractals/fractals.hpp"

constexpr static bool generation_debug{false};
constexpr static bool conversion_debug{false};

using precision = basal::precision;

struct Parameters {
    precision x_min;
    precision x_max;
    precision y_min;
    precision y_max;
    size_t width;
    size_t height;
    size_t max_iterations;
    std::string filename;
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

int main(int argc, char *argv[]) {
    Parameters params;
    basal::options::config opts[] = {
        {"-xmin", "--x-min", -2.5_p, "Minimum X value"},
        {"-xmax", "--x-max", 1.0_p, "Maximum X value"},
        {"-ymin", "--y-min", -1.0_p, "Minimum Y value"},
        {"-ymax", "--y-max", 1.0_p, "Maximum Y value"},
        {"-w", "--width", (size_t)6000, "Image Width"},
        {"-h", "--height", (size_t)4000, "Image Height"},
        {"-m", "--max-iterations", (size_t)72, "Maximum Iterations"},
        {"-f", "--file", std::string("fractal.tga"), "Output File Name"},
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
    basal::options::print(basal::dimof(opts), opts);

    fractals::image computation_image{params.height, params.width};
    fourcc::image<fourcc::PixelFormat::RGBId> color_image{params.height, params.width};

    size_t total_pixels = params.width * params.height;
    uint16_t max_iterations = static_cast<uint16_t>(params.max_iterations);
    printf("Generating fractal with %zu x %zu = %zu pixels, %u iterations\n", params.width, params.height, total_pixels, max_iterations);

    auto line_notifier = [&params](size_t row_index, bool completed_flag) {
        // this is a data race to update, but it's just for progress reporting
        if (row_index % 100 == 0 || completed_flag) {
            printf("Completed row %zu of %zu\n", row_index, params.height);
        }
    };

    auto mandelbrot_sampler = [params](fractals::image::point const& p, uint16_t max_iterations) -> fractals::image::PixelStorageType {
        // Map point to complex plane
        precision h = static_cast<precision>(params.height);
        precision w = static_cast<precision>(params.width);
        precision x0 = ((p.x() / w) * (params.x_max - params.x_min)) + params.x_min;
        precision i0 = ((p.y() / h) * (params.y_max - params.y_min)) + params.y_min;
        std::complex<precision> const c{x0, i0};
        fractals::image::PixelStorageType pixel = mandelbrot(c, max_iterations);
        if (generation_debug) {
            printf("Point (%f, %f) => Complex (%f, %f) => Iterations %u\n",
                   p.x(), p.y(), c.real(), c.imag(), static_cast<uint16_t>(pixel));
        }
        return pixel;
    };

    // only needs to be done once with a possibly high iteration count
    computation_image.generate_sample(mandelbrot_sampler, max_iterations, line_notifier);
    computation_image.save("mandelbrot_raw.pgm");

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
    return 0;
}