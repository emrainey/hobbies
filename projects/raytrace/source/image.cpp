#include "raytrace/image.hpp"

#include <chrono>
#include <random>
#include <thread>
#include <vector>

#include "basal/exception.hpp"

namespace raytrace {

image::image(size_t h, size_t w) : fourcc::image<fourcc::PixelFormat::RGBId>(h, w) {
    basal::exception::throw_if(basal::is_odd(height) or basal::is_odd(width), __FILE__, __LINE__,
                               "Height %d and Width %d must be even", height, width);
}

image::PixelStorageType& image::at(size_t y, size_t x) {
    return fourcc::image<format>::at(y, x);
}

image::PixelStorageType const& image::at(size_t y, size_t x) const {
    return fourcc::image<format>::at(y, x);
}

image::PixelStorageType& image::at(image::point const& p) {
    size_t x = static_cast<size_t>(std::floor(p.x()));
    size_t y = static_cast<size_t>(std::floor(p.y()));
    return fourcc::image<format>::at(y, x);
}

/// @brief The sampling pattern for the image plane, defined as vectors from the center of the pixel. (0.5_p, 0.5_p)
static image::vector fixed_sampling_pattern[] = {
    // dead center
    image::vector{0.0_p, 0.0_p},
    // corners
    image::vector{0.25_p, 0.25_p},    // lower right
    image::vector{0.25_p, -0.25_p},   // upper right
    image::vector{-0.25_p, 0.25_p},   // lower left
    image::vector{-0.25_p, -0.25_p},  // upper left
    // cross around center
    image::vector{0.0_p, 0.25_p},   // up
    image::vector{0.0_p, -0.25_p},  // down
    image::vector{0.25_p, 0.0_p},   // right
    image::vector{-0.25_p, 0.0_p},  // left
    // now a grid across the pixels in 0.125 increments
    image::vector{-0.375_p, -0.375_p},  // top left
    image::vector{-0.125_p, -0.375_p},
    image::vector{0.125_p, -0.375_p},
    image::vector{0.375_p, -0.375_p},
    // next row
    image::vector{-0.375_p, -0.125_p},
    image::vector{-0.125_p, -0.125_p},
    image::vector{0.125_p, -0.125_p},
    image::vector{0.375_p, -0.125_p},
    // next row
    image::vector{-0.375_p, 0.125_p},
    image::vector{-0.125_p, 0.125_p},
    image::vector{0.125_p, 0.125_p},
    image::vector{0.375_p, 0.125_p},
    // last row
    image::vector{-0.375_p, 0.375_p},
    image::vector{-0.125_p, 0.375_p},
    image::vector{0.125_p, 0.375_p},
    image::vector{0.375_p, 0.375_p},
};
static constexpr size_t fixed_sampling_pattern_count = basal::dimof(fixed_sampling_pattern);

class RandomSampleFuzzer : public image::SampleFuzzer {
public:
    RandomSampleFuzzer()
        : m_generator(), m_distribution(-0.5_p, 0.5_p), m_func{std::bind(m_distribution, m_generator)} {
    }

    image::vector operator()(size_t) override {
        return image::vector{m_func(), m_func()};
    }

protected:
    std::default_random_engine m_generator;
    std::uniform_real_distribution<precision> m_distribution;
    std::function<precision()> m_func;
};

class FixedSampleFuzzer : public image::SampleFuzzer {
public:
    FixedSampleFuzzer() {
    }

    image::vector operator()(size_t index) override {
        index = index % fixed_sampling_pattern_count;
        return fixed_sampling_pattern[index];
    }
};

void image::generate_each(subsampler get_color, size_t number_of_samples, std::optional<rendered_line> row_notifier,
                          fourcc::image<fourcc::PixelFormat::Y8>* mask, uint8_t mask_threshold, bool tone_mapping) {
    SampleFuzzer* delta;
    if constexpr (use_random_sample_points) {
        delta = new RandomSampleFuzzer();
    } else {
        delta = new FixedSampleFuzzer();
    }

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
                image::point p = image::point{precision(x) + 0.5_p, precision(y) + 0.5_p} + (*delta)(s);
                samples[s] = get_color(p);
            }
            // now average all the samples together.
            color value = color::blend_samples(samples);
            if (tone_mapping) {
                ReinhardToneMapper mapper;
                value = mapper(value);
            }
            auto out = value.to_<fourcc::PixelFormat::RGBId>();
            fourcc::image<format>::at(y, x) = out;
        }
        if (row_notifier != std::nullopt) {
            rendered_line func = row_notifier.value();
            func(y, true);
        }
    }
    delete delta;
}

}  // namespace raytrace
