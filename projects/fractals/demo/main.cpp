#include "basal/basal.hpp"
#include "basal/options.hpp"
#include "fractals/fractals.hpp"

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

int main(int argc, char *argv[]) {
    Parameters params;
    basal::options::config opts[] = {
        {"-xmin", "--x-min", -2.5_p, "Minimum X value"},
        {"-xmax", "--x-max", 1.0_p, "Maximum X value"},
        {"-ymin", "--y-min", -1.0_p, "Minimum Y value"},
        {"-ymax", "--y-max", 1.0_p, "Maximum Y value"},
        {"-w", "--width", (size_t)6000, "Image Width"},
        {"-h", "--height", (size_t)4000, "Image Height"},
        {"-m", "--max-iterations", (size_t)1000, "Maximum Iterations"},
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

    // fourcc::image<fourc::PixelFormat::Y32> iterimage{params.height, params.width};
    fractals::image img{params.height, params.width};

    size_t total_pixels = params.width * params.height;
    printf("Generating fractal with %zu x %zu = %zu pixels\n", params.width, params.height, total_pixels);

    auto line_notifier = [&params](size_t row_index, bool completed_flag) {
        // this is a data race to update, but it's just for progress reporting
        if (row_index % 100 == 0 || completed_flag) {
            printf("Completed row %zu of %zu\n", row_index, params.height);
        }
    };

    auto mandelbrot = [params](fractals::image::point const& p, size_t iterations) -> fourcc::rgbid {
        // Map point to complex plane
        precision h = static_cast<precision>(params.height);
        precision w = static_cast<precision>(params.width);
        precision x0 = ((p.x() / w) * (params.x_max - params.x_min)) + params.x_min;
        precision y0 = ((p.y() / h) * (params.y_max - params.y_min)) + params.y_min;
        precision x = 0.0_p;
        precision y = 0.0_p;
        size_t iteration_count{0U};

        while (((x * x) + (y * y)) <= 4.0_p && iteration_count < iterations) {
            precision xtemp = (x * x) - (y * y) + x0;
            y = 2.0_p * x * y + y0;
            x = xtemp;
            iteration_count++;
        }

        fourcc::rgbid pixel;
        if (iteration_count == iterations) {
            // bounded
            pixel.components.b = 0.0_p;
            pixel.components.g = 0.0_p;
            pixel.components.r = 0.0_p;
            pixel.components.i = 1.0_p;
        } else {
            precision color_value = precision(iteration_count) / precision(iterations);
            precision jet_color = color_value * 2.0_p - 1.0_p;
            // printf("Point (%f, %f) escaped after %zu iterations %lf -> %lf \n", x0, y0, iteration_count, color_value, jet_color);
            pixel = fourcc::jet(jet_color).data();
        }
        return pixel;
    };

    img.generate_sample(mandelbrot, params.max_iterations, line_notifier);
    img.save(params.filename);
    return 0;
}