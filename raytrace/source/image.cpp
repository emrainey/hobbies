#include <random>
#include <vector>
#include <thread>
#include <chrono>
#include "basal/exception.hpp"
#include "raytrace/image.hpp"

namespace raytrace {

#define is_odd(x) (x & 1)

image::image(size_t h, size_t w)
    : fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8>(h, w)
    , mask(h, w)
    {
    // initialize it to all white
    mask.for_each([](size_t, size_t, uint8_t& pixel) -> void {
        pixel = AAA_MASK_DISABLED;
    });
    basal::exception::throw_if(is_odd(height) or is_odd(width), __FILE__, __LINE__, "Height %d and Width %d must be even", height, width);
}

fourcc::rgb8& image::at(size_t y, size_t x) {
    return fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8>::at(y, x);
}

fourcc::rgb8& image::at(const image::point& p) {
    size_t x = static_cast<size_t>(std::floor(p.x));
    size_t y = static_cast<size_t>(std::floor(p.y));
    return fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8>::at(y, x);
}

void image::generate_each(subsampler get_color,
                     size_t number_of_samples,
                     std::optional<rendered_line> row_notifier,
                     uint8_t mask_threshold) {
    std::default_random_engine generator;
    std::uniform_real_distribution<element_type> distribution(-0.5, 0.5);
    auto delta = std::bind(distribution, generator);

    #pragma omp parallel for shared(data, delta, get_color)
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            if (mask.at(y, x) < mask_threshold) {
                // skip pixel that are below the threshold
                continue;
            }
            std::vector<color> samples(number_of_samples);
            for (size_t s = 0; s < number_of_samples; s++) {
                // create a random 2d unit vector from this point.
                // except the first point, it should be dead center
                element_type dx = s > 0 ? delta() : 0.0;
                element_type dy = s > 0 ? delta() : 0.0;
                element_type _x = element_type(x) + dx;
                element_type _y = element_type(y) + dy;
                image::point p(_x + 0.5, _y + 0.5);
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

void image::compute_mask() {

    fourcc::image<fourcc::iyu2, fourcc::pixel_format::IYU2> iyu2_image(height, width);
    fourcc::convert(*this, iyu2_image);

    // create a x and Y gradient image and then sum them and down scale to the mask
    const int16_t sobel_x[3][3] = {
        {-1,  0, +1},
        {-2,  0, +2},
        {-1,  0, +1},
    };
    const int16_t sobel_y[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        {+1, +2, +1},
    };

    fourcc::image<int16_t, fourcc::pixel_format::Y16> grad_x(height, width);
    fourcc::image<int16_t, fourcc::pixel_format::Y16> grad_y(height, width);

    fourcc::convolve(grad_x, sobel_x, iyu2_image, fourcc::channel::Y);
    fourcc::convolve(grad_y, sobel_y, iyu2_image, fourcc::channel::Y);

    mask.for_each([&](size_t y, size_t x, uint8_t& pixel) {
        int16_t a = grad_x.at(y, x);
        int16_t b = grad_y.at(y, x);
        int32_t c = std::sqrt(a*a + b*b);
        pixel = std::min(c, 255);
    });
}

} // namespace raytrace
