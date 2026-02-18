#pragma once
/// @file
/// Definitions for any visualization objects.
/// @copyright Copyright 2022 (C) Erik Rainey.

#include <basal/exception.hpp>
#include <basal/ieee754.hpp>
#include <iso/iso.hpp>
#include <cstdint>
#include <functional>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ostream>
#include <type_traits>

#include <fourcc/pixel.hpp>

/// The Four Character Code Namespace for Images
namespace fourcc {

/// A single plane image format
template <PixelFormat PIXEL_FORMAT>
class image {
public:
    /// The image pixel type used in this image based on the Pixel format
    using PixelStorageType = decltype(GetStorageType<PIXEL_FORMAT>());

    /// The number of channel_count in the pixel
    size_t const depth;
    /// The number of pixels per row
    size_t const width;
    /// The number of rows per plane
    size_t const height;
    /// The number of planes per image
    size_t const planes;
    /// The format for the image
    constexpr static PixelFormat const format{PIXEL_FORMAT};

    /// Default Constructor
    image() : depth{1}, width{0}, height{0}, planes{1}, data{} {
    }

    /// Sized and typed constructor
    image(size_t h, size_t w)
        : depth{channels_in_format(format)}
        , width{w}
        , height{h}
        , planes{planes_in_format(format)}
        , data{planes} {
        for (auto& plane : data) {
            // each unit is a PixelStorageType
            plane.resize(width * height);
        }
    }

    /// Copy constructor
    image(image const& other)
        : depth{channels_in_format(format)}
        , width{other.width}
        , height{other.height}
        , planes{planes_in_format(format)}
        , data{planes} {
        for (auto& plane : data) {
            // each unit is a PixelStorageType
            plane.resize(width * height);
        }
        // copy
        for_each([&](size_t y, size_t x, PixelStorageType& pixel) { pixel = other.at(y, x); });
    }

    /// Move constructor
    image(image&& other)
        : depth(other.depth)
        , width(other.width)
        , height(other.height)
        , planes(other.planes)
        , data(std::move(other.data)) {
        // data should be moved over, so no copy needed
    }

    /// Destructor
    virtual ~image() = default;

    /// Copy Assignment
    image& operator=(image const& other) = delete;

    /// Move Assignment
    image& operator=(image&& other) = delete;

    /// Used to change each pixel
    using coord_ref_pixel = std::function<void(size_t y, size_t x, PixelStorageType& pixel)>;

    /// Used to read each pixel
    using coord_const_ref_pixel = std::function<void(size_t y, size_t x, PixelStorageType const& pixel)>;

    /// Used to change each pixel
    using ref_pixel = std::function<void(PixelStorageType& pixel)>;

    /// Used to read each pixel
    using const_ref_pixel = std::function<void(PixelStorageType const& pixel)>;

    /// Iterates over each pixel giving a mutable reference to the iterator
    image& for_each(coord_ref_pixel iter) {
        for (size_t p = 0; p < planes; p++) {
            // each plane in it's own buffer
            for (size_t y = 0; y < height; y++) {
                for (size_t x = 0; x < width; x++) {
                    size_t off = (y * width) + x;
                    iter(y, x, data[p][off]);
                }
            }
        }
        return (*this);
    }

    /// Iterates over each pixel giving a mutable reference to the iterator
    image& for_each(ref_pixel iter) {
        for (size_t p = 0; p < planes; p++) {
            // each plane in it's own buffer
            for (size_t y = 0; y < height; y++) {
                for (size_t x = 0; x < width; x++) {
                    size_t off = (y * width) + x;
                    iter(data[p][off]);
                }
            }
        }
        return (*this);
    }

    /// Iterates over each pixel giving a const reference to the iterator
    void for_each(coord_const_ref_pixel iter) const {
        for (size_t p = 0; p < planes; p++) {
            // each plane in it's own buffer
            for (size_t y = 0; y < height; y++) {
                for (size_t x = 0; x < width; x++) {
                    size_t off = (y * width) + x;
                    iter(y, x, data[p][off]);
                }
            }
        }
    }

    /// Iterates over each pixel giving a const reference to the iterator
    void for_each(const_ref_pixel iter) const {
        for (size_t p = 0; p < planes; p++) {
            // each plane in it's own buffer
            for (size_t y = 0; y < height; y++) {
                for (size_t x = 0; x < width; x++) {
                    size_t off = (y * width) + x;
                    iter(data[p][off]);
                }
            }
        }
    }

    /// Returns pixel area, not byte size
    inline int area() const {
        return width * height;
    }

    /// Returns total byte size of the image
    inline size_t bytes() const {
        return sizeof(PixelStorageType) * width * height * planes;
    }

    /// Able to load the image type from file into an image
    // static image load(std::string name);

    /// Gets the pixel data a specific location
    virtual PixelStorageType& at(size_t y, size_t x) {
        basal::exception::throw_if(y >= height or x >= width, __FILE__, __LINE__, "Out of bounds x,y=%z,%z", x, y);
        size_t offset = (y * width) + x;
        return data[0][offset];
    }

    /// Gets the read only pixel data a specific location (Const)
    virtual PixelStorageType const& at(size_t y, size_t x) const {
        basal::exception::throw_if(y >= height or x >= width, __FILE__, __LINE__, "Out of bounds x,y=%z,%z", x, y);
        size_t offset = (y * width) + x;
        return data[0][offset];
    }

    /// Saves the existing image to a file based on the type
    bool save(std::string filename) const;  // no impl so that the specializations can be used

protected:
    // we build a set of planes and 2d addressing within a vector
    std::vector<std::vector<PixelStorageType>> data;
};

/// Gamma correction namespace
namespace gamma {

/// Interpolates between two colors using the gamma correction scheme. The ratio is between 0 and 1, where 0 is all x and 1 is all y.
/// @param x The first color to interpolate between.
/// @param y The second color to interpolate between.
/// @param a The ratio between x and y to use for the interpolation. Should be between 0 and 1, where 0 is all x and 1 is all y.
color interpolate(color const& x, color const& y, precision a);

}  // namespace gamma

}  // namespace fourcc
