#pragma once

/// @file
/// The fractals main header

#include "geometry/geometry.hpp"
#include "fourcc/fourcc.hpp"

namespace fractals {

using precision = basal::precision;

class image : public fourcc::image<fourcc::PixelFormat::RGBId> {
public:
    image() = delete;

    image(size_t width, size_t height);

    virtual ~image() = default;

    using point = geometry::R2::point;

    struct Rect {
        point top_left;
        point bottom_right;
    };

    /// The function which generates the color pixel for each point in the image.
    using sampler = std::function<fourcc::rgbid(point const&, size_t iterations)>;

    /// A function callback which is called when a line is complete
    using rendered_line = std::function<void(size_t row_index, bool completed)>;

    /// The function which iterates over each pixel to generate the fractal.
    void generate_sample(sampler sampler, size_t max_iterations, std::optional<rendered_line> row_notifier = std::nullopt);

protected:

};

}  // namespace fractals