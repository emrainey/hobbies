#pragma once

#include <basal/exception.hpp>
#include <fourcc/image.hpp>
#include <functional>
#include <linalg/linalg.hpp>
#include <optional>

#include "raytrace/color.hpp"

namespace raytrace {

/// The subclass of a sRGB interface to a fourcc::image in 24bit RGB
class image : public fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> {
public:
    image() = delete;  // No default construction

    /// The general constructor.
    /// @throw Can throw an exception if the height or width is odd.
    ///
    image(size_t height, size_t width);

    // Normal destructor
    virtual ~image() = default;

    /// Points in the image are 2D
    using point = geometry::point_<2>;

    /// Vectors in the image plane are 2D
    using vector = geometry::vector_<2>;

    /// A function which gives an image point and expects a color returned.
    using subsampler = std::function<color(image::point const&)>;

    /// A function callback which is called when a line is complete
    using rendered_line = std::function<void(size_t row_index, bool completed)>;

    ///
    /// The iterator for the image plane. It produces a subsampled point in the image plane and
    /// expects a color returned for each subsample. Each subsample will be averaged together.
    /// After the pixel is computed, the optional renderer is called.
    /// @param sub_func The subsampler functor
    /// @param number_of_samples The number of samples per pixel.
    /// @param opt_func The optional renderer callback
    /// @param mask The mask image to use
    /// @param mask_threshold Used to do adaptive anti-aliasing. If the mask pixel is above the threshold value, then it
    /// will attempt to recompute. If 255, anti-aliasing is disabled.
    void generate_each(subsampler sub_func, size_t number_of_samples = 1,
                       std::optional<rendered_line> opt_func = std::nullopt,
                       fourcc::image<uint8_t, fourcc::pixel_format::Y8>* mask = nullptr,
                       uint8_t mask_threshold = AAA_MASK_DISABLED);

    /// Returns the image pixel at the point (rounded raster coordinates)
    fourcc::rgb8& at(point const& p);

    // declared const override
    fourcc::rgb8 const& at(size_t y, size_t x) const override;

    // declared override
    fourcc::rgb8& at(size_t y, size_t x) override;

    /// The value of the threshold in which the Adaptive Antialiasing is always disabled
    constexpr static uint8_t AAA_MASK_DISABLED = 255u;

    /// The value of the threshold in which the Adaptive Antialiasing is always enabled
    constexpr static uint8_t AAA_MASK_ENABLED = 1u;
};

}  // namespace raytrace
