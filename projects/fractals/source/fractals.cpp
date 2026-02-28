#include "fractals/fractals.hpp"

namespace fractals {

image::image(size_t width, size_t height)
    : fourcc::image<fourcc::PixelFormat::RGBId>{width, height} {
}

void image::generate_sample(fractals::image::sampler sampler_func, size_t max_iterations, std::optional<rendered_line> row_notifier) {
#pragma omp parallel for shared(data, sampler_func)
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            point pnt{precision(x), precision(y)};
            at(y, x) = sampler_func(pnt, max_iterations);
        }
        if (row_notifier.has_value()) {
            row_notifier.value()(y, y == height - 1);
        }
    }
}

}  // namespace fractals
