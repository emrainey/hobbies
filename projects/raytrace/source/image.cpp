#include "raytrace/image.hpp"

#include <chrono>
#include <random>
#include <thread>
#include <vector>

#include "basal/exception.hpp"

namespace raytrace {

#define is_odd(x) (x & 1)

image::image(size_t h, size_t w) : fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8>(h, w) {
    basal::exception::throw_if(is_odd(height) or is_odd(width), __FILE__, __LINE__,
                               "Height %d and Width %d must be even", height, width);
}

fourcc::rgb8& image::at(size_t y, size_t x) {
    return fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8>::at(y, x);
}

fourcc::rgb8 const& image::at(size_t y, size_t x) const {
    return fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8>::at(y, x);
}

fourcc::rgb8& image::at(const image::point& p) {
    size_t x = static_cast<size_t>(std::floor(p.x));
    size_t y = static_cast<size_t>(std::floor(p.y));
    return fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8>::at(y, x);
}

void image::generate_each(subsampler get_color, size_t number_of_samples, std::optional<rendered_line> row_notifier,
                          fourcc::image<uint8_t, fourcc::pixel_format::Y8>* mask, uint8_t mask_threshold) {
    std::default_random_engine generator;
    std::uniform_real_distribution<precision> distribution(-0.5, 0.5);
    auto delta = std::bind(distribution, generator);

#pragma omp parallel for shared(data, delta, get_color)
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            if (mask and (mask->at(y, x) < mask_threshold)) {
                // skip pixel that are below the threshold
                continue;
            }
            std::vector<color> samples(number_of_samples);
            for (size_t s = 0; s < number_of_samples; s++) {
                // create a random 2d unit vector from this point.
                // except the first point, it should be dead center
                precision dx = s > 0 ? delta() : 0.0;
                precision dy = s > 0 ? delta() : 0.0;
                precision _x = precision(x) + dx;
                precision _y = precision(y) + dy;
                image::point p{precision(_x + 0.5), precision(_y + 0.5)};
                samples[s] = get_color(p);
            }
            // now average all the samples together.
            color value = color::blend_samples(samples);
            // save the pixel
            fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8>::at(y, x) = value.to_rgb8();
        }
        if (row_notifier != std::nullopt) {
            rendered_line func = row_notifier.value();
            func(y, true);
        }
    }
}

}  // namespace raytrace
