#pragma once
/// @file
/// Definitions for any visualization objects.
/// @copyright Copyright 2022 (C) Erik Rainey.

#include <basal/exception.hpp>
#include <basal/ieee754.hpp>
#include <cstdint>
#include <functional>
#include <vector>

/// The Visualization Namespace
namespace fourcc {

/// Takes a four character code and returns a specific FOURCC type
constexpr uint32_t four_character_code(char fcc0, char fcc1, char fcc2, char fcc3) {
    return (uint32_t(fcc0) << 0) | (uint32_t(fcc1) << 8) | (uint32_t(fcc2) << 16) | (uint32_t(fcc3) << 24);
}

/// Check that the function produces the right output
static_assert(0x30303859 == four_character_code('Y', '8', '0', '0'), "Must be little endian");

/// Defines the RGB8 (24 bit) type of red, then green, then blue
struct alignas(uint8_t) rgb8 {
    /// Constructor
    constexpr rgb8() : r{0}, g{0}, b{0} {
    }
    uint8_t r;  //!< Red Component
    uint8_t g;  //!< Green Component
    uint8_t b;  //!< Blue Component
} __attribute__((packed));

/// Defines the BGR8 (24 bit) type of blue, then green, then red
struct alignas(uint8_t) bgr8 {
    /// Constructor
    constexpr bgr8() : b{0}, g{0}, r{0} {
    }
    uint8_t b;  //!< Blue Component
    uint8_t g;  //!< Green Component
    uint8_t r;  //!< Red Component
} __attribute__((packed));

/// Defines the RGBA (32 bit) type of red, then green, then blue, then alpha
struct alignas(uint32_t) rgba {
    /// Constructor
    constexpr rgba() : r{0}, g{0}, b{0}, a{0} {
    }
    uint8_t r;  //!< Red Component
    uint8_t g;  //!< Green Component
    uint8_t b;  //!< Blue Component
    uint8_t a;  //!< Alpha Component
} __attribute__((packed));

/// Defines the ABGR (32 bit) type of alpha, then blue, then green, then red
struct alignas(uint32_t) abgr {
    /// Constructor
    constexpr abgr() : a{0}, b{0}, g{0}, r{0} {
    }
    uint8_t a;  //!< Alpha Component
    uint8_t b;  //!< Blue Component
    uint8_t g;  //!< Green Component
    uint8_t r;  //!< Red Component
} __attribute__((packed));

/// Defines the ABGR (32 bit) type of alpha, then blue, then green, then red
struct alignas(uint32_t) bgra {
    /// Constructor
    constexpr bgra() : b{0}, g{0}, r{0}, a{0} {
    }
    uint8_t b;  //!< Blue Component
    uint8_t g;  //!< Green Component
    uint8_t r;  //!< Red Component
    uint8_t a;  //!< Alpha Component
} __attribute__((packed));

/// Defines the 4:4:4 (non subsampled) single plane interleaved YUV format
struct alignas(uint8_t) iyu2 {
    /// Constructor
    constexpr iyu2() : u{0}, y{0}, v{0} {
    }
    uint8_t u;
    uint8_t y;
    uint8_t v;
} __attribute__((packed));

/// Defines a 1 channel 32 bit float color format (luminance)
struct alignas(float) yf {
    /// Constructor
    constexpr yf() : y{0.0f} {
    }
    float y;
};

/// Defines a 3 channel 32 bit float color format
struct alignas(float) rgbf {
    /// Constructor
    constexpr rgbf() : r{0.0f}, g{0.0f}, b{0.0f} {
    }
    float r;
    float g;
    float b;
};

/// Defines a 3 Channel 16 Bit Half Precision Color Format
struct alignas(uint16_t) rgbh {
    /// Constructor
    constexpr rgbh() : r{0.0f}, g{0.0f}, b{0.0f} {
    }
    basal::half r;
    basal::half g;
    basal::half b;
};
static_assert(sizeof(rgbh) == 6, "rgbh must be 6 bytes");

/// Defines a 4 Channel 32 Bit Float Precision Color Format
struct alignas(float) rgbaf {
    /// Constructor
    constexpr rgbaf() : r{0.0f}, g{0.0f}, b{0.0f}, a{0.0f} {
    }
    float r;
    float g;
    float b;
    float a;
};

/// Defines a 16 bit packed pixel color in 5:6:5 format. R being the highest bits, and B the lowest
struct alignas(uint16_t) rgb565 {
    uint16_t b : 5;
    uint16_t g : 6;
    uint16_t r : 5;
} __attribute__((packed));

/// Enumerates the various pixel formats in FOURCC codes and some additional types.
/// \see fourcc.org
enum class pixel_format : std::uint32_t {
    GREY8 = four_character_code('G', 'R', 'E', 'Y'),  ///< Uses uint8_t
    Y800 = four_character_code('Y', '8', '0', '0'),   ///< Uses uint8_t
    Y8 = four_character_code('Y', '8', ' ', ' '),     ///< Uses uint8_t
    Y16 = four_character_code('Y', '1', '6', ' '),    ///< Uses uint16_t
    Y32 = four_character_code('Y', '3', '2', ' '),    ///< Uses uint32_t
    RGB8 = four_character_code('R', 'G', 'B', '8'),   ///< Uses @ref rgb8
    BGR8 = four_character_code('B', 'G', 'R', '8'),   ///< Uses @ref bgr8
    RGBA = four_character_code('R', 'G', 'B', 'A'),   ///< Uses @ref rgba
    ABGR = four_character_code('A', 'B', 'G', 'R'),   ///< Uses @ref abgr
    BGRA = four_character_code('B', 'G', 'R', 'A'),   ///< Uses @ref bgra
    IYU2 = four_character_code('I', 'Y', 'U', '2'),   ///< Uses @ref @iyu2
    YF = four_character_code('Y', 'F', ' ', ' '),     ///< User @ref yf
    sRGB = four_character_code('s', 'R', 'G', 'B'),   ///< Uses @reg rgbf but limited to 0.0 to 1.0
    RGBf = four_character_code('R', 'G', 'B', 'f'),   ///< Uses @reg rgbf
    RGBh = four_character_code('R', 'G', 'B', 'h'),   ///< Uses @reg rgbh (half precision) with any value
    RGBAf = four_character_code('R', 'G', 'B', 'F'),  ///< Uses @reg rgbaf
    RGBP = four_character_code('R', 'G', 'B', 'P'),   ///< Uses @reg rgb565
};

/// Returns the string name of the format
constexpr char const* channel_order(pixel_format fmt) {
    switch (fmt) {
        case pixel_format::RGBA:
            [[fallthrough]];
        case pixel_format::RGBAf:
            return "RGBA";
        case pixel_format::RGBf:
            [[fallthrough]];
        case pixel_format::RGBh:
            [[fallthrough]];
        case pixel_format::RGB8:
            return "RGB";
        case pixel_format::RGBP:
            [[fallthrough]];
        case pixel_format::BGR8:
            [[fallthrough]];
        case pixel_format::ABGR:
            return "BGR";
        case pixel_format::BGRA:
            return "BGRA";
        case pixel_format::IYU2:
            return "UYV";
        case pixel_format::YF:
            return "YF";
        default:
            return "???";
    }
}

/// Returns the number of planes in the format
constexpr int planes_in_format(pixel_format fmt __attribute__((unused))) {
    switch (fmt) {
        default:
            return 1;
    }
}

/// Returns the number of channels for the format
constexpr int channels_in_format(pixel_format fmt) {
    switch (fmt) {
        case pixel_format::RGBA:
        case pixel_format::ABGR:
        case pixel_format::BGRA:
        case pixel_format::RGBAf:
            return 4;
        case pixel_format::RGBP:
        case pixel_format::RGBf:
        case pixel_format::RGBh:
        case pixel_format::RGB8:
        case pixel_format::BGR8:
        case pixel_format::IYU2:
            return 3;
        case pixel_format::Y16:
        case pixel_format::Y32:
        case pixel_format::YF:
        case pixel_format::GREY8:
        case pixel_format::Y800:
        case pixel_format::Y8:
        default:
            return 1;
    }
}

/// Returns true if the format uses uint8_t as a channel
constexpr bool uses_uint8(pixel_format fmt) {
    switch (fmt) {
        case pixel_format::Y800:
        case pixel_format::Y8:
        case pixel_format::GREY8:
            return true;
        default:
            break;
    }
    return false;
}

/// Returns true if the format uses uint16_t as a channel
constexpr bool uses_uint16(pixel_format fmt) {
    return (fmt == pixel_format::Y16);
}

/// Returns true if the format uses rgb8_t as a channel
constexpr bool uses_rgb8(pixel_format fmt) {
    return (fmt == pixel_format::RGB8);
}

/// Returns true if the format uses bgr8_t as a channel
constexpr bool uses_bgr8(pixel_format fmt) {
    return (fmt == pixel_format::BGR8);
}

/// Returns true if the format uses uint32_t as a channel
constexpr bool uses_uint32(pixel_format fmt) {
    switch (fmt) {
        case pixel_format::Y32:
        case pixel_format::ABGR:
        case pixel_format::RGBA:
        case pixel_format::BGRA:
            return true;
        default:
            break;
    }
    return false;
}

/// Returns true if the format uses iyu2 as a format
constexpr bool uses_iyu2(pixel_format fmt) {
    return (fmt == pixel_format::IYU2);
}

constexpr bool uses_rgbf(pixel_format fmt) {
    return (fmt == pixel_format::RGBf);
}

constexpr bool uses_rgbaf(pixel_format fmt) {
    return (fmt == pixel_format::RGBAf);
}

constexpr bool uses_rgbh(pixel_format fmt) {
    return (fmt == pixel_format::RGBh);
}

constexpr bool uses_yf(pixel_format fmt) {
    return (fmt == pixel_format::YF);
}

/// Returns true if the format uses rgb565 (either endianess) as a format
constexpr bool uses_rgb565(pixel_format fmt) {
    return (fmt == pixel_format::RGBP);
}

/// A single plane image format
template <typename PIXEL_TYPE, pixel_format PIXEL_FORMAT>
class image {
public:
    /// The number of channels in the pixel
    size_t const depth;
    /// The number of pixels per row
    size_t const width;
    /// The number of rows per plane
    size_t const height;
    /// The number of planes per image
    size_t const planes;
    /// The format for the image
    pixel_format const format;

    /// Default Constructor
    image() : depth{1}, width{0}, height{0}, planes{1}, format{PIXEL_FORMAT}, data{} {
    }

    /// Sized and typed constructor
    image(size_t h, size_t w)
        : depth{channels_in_format(PIXEL_FORMAT)},
          width{w},
          height{h},
          planes{planes_in_format(PIXEL_FORMAT)},
          format{PIXEL_FORMAT},
          data{planes} {
        for (auto& plane : data) {
            // each unit is a PIXEL_TYPE
            plane.resize(width * height);
        }
    }

    /// Copy constructor
    image(image const& other)
        : depth{channels_in_format(PIXEL_FORMAT)},
          width{other.width},
          height{other.height},
          planes{planes_in_format(PIXEL_FORMAT)},
          format{PIXEL_FORMAT},
          data{planes} {
        for (auto& plane : data) {
            // each unit is a PIXEL_TYPE
            plane.resize(width * height);
        }
        // copy
        for_each([&](size_t y, size_t x, PIXEL_TYPE& pixel) { pixel = other.at(y, x); });
    }

    /// Move constructor
    image(image&& other) = delete;
    /// Destructor
    virtual ~image() = default;

    /// Copy Assignment
    image& operator=(image const& other) = delete;
    /// Move Assignment
    image& operator=(image&& other) = delete;

    /// Used to change each pixel
    using coord_ref_pixel = std::function<void(size_t y, size_t x, PIXEL_TYPE& pixel)>;

    /// Used to read each pixel
    using coord_const_ref_pixel = std::function<void(size_t y, size_t x, PIXEL_TYPE const& pixel)>;

    /// Used to change each pixel
    using ref_pixel = std::function<void(PIXEL_TYPE& pixel)>;

    /// Used to read each pixel
    using const_ref_pixel = std::function<void(PIXEL_TYPE const& pixel)>;

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
        return sizeof(PIXEL_TYPE) * width * height * planes;
    }

    /// Able to load the image type from file into an image
    // static image load(std::string name);

    /// Gets the pixel data a specific location
    virtual PIXEL_TYPE& at(size_t y, size_t x) {
        basal::exception::throw_if(y >= height or x >= width, __FILE__, __LINE__, "Out of bounds x,y=%z,%z", x, y);
        size_t offset = (y * width) + x;
        return data[0][offset];
    }

    /// Gets the read only pixel data a specific location (Const)
    virtual PIXEL_TYPE const& at(size_t y, size_t x) const {
        basal::exception::throw_if(y >= height or x >= width, __FILE__, __LINE__, "Out of bounds x,y=%z,%z", x, y);
        size_t offset = (y * width) + x;
        return data[0][offset];
    }

    /// Saves the existing image to a file based on the type
    bool save(std::string filename) const;  // no impl so that the specializations can be used

protected:
    // we build a set of planes and 2d addressing within a vector
    std::vector<std::vector<PIXEL_TYPE>> data;
};

/// Enumerates the channels
enum class channel : int8_t {
    R,  ///< Red
    G,  ///< Green
    B,  ///< Blue
    Y,  ///< Luminance
    U,  ///< Chrominance U
    V,  ///< Chrominance V
    A,  ///< Alpha
};

/// Convolves a specific channel of the input image and the kernel into the output gradient image
void convolve(fourcc::image<int16_t, fourcc::pixel_format::Y16>& out, int16_t const (&kernel)[3][3],
              fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> const& input, channel channel);

/// Convolves a specific channel of the input image and the kernel into the output gradient image
void convolve(fourcc::image<int16_t, fourcc::pixel_format::Y16>& out, int16_t const (&kernel)[3][3],
              fourcc::image<fourcc::iyu2, fourcc::pixel_format::IYU2> const& input, channel channel);

/// Converts RGB8 to IYU2
void convert(fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> const& in,
             fourcc::image<fourcc::iyu2, fourcc::pixel_format::IYU2>& out);

/// Produces a Sobel Mask image
void sobel_mask(fourcc::image<fourcc::iyu2, fourcc::pixel_format::IYU2> const& iyu2_image,
                fourcc::image<uint8_t, fourcc::pixel_format::Y8>& mask);

/// Produces a Sobel Mask image
void sobel_mask(fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> const& rgb_image,
                fourcc::image<uint8_t, fourcc::pixel_format::Y8>& mask);

/// Runs a 1D filter across an image
/// @warning no gamma correction!
void filter(fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8>& output,
            fourcc::image<fourcc::rgb8, fourcc::pixel_format::RGB8> const& input, int16_t const kernel[3]);

/// Runs a 2D 3x3 Filter across an image
/// @warning no gamma correction!
void filter(image<rgb8, pixel_format::RGB8>& output, image<rgb8, pixel_format::RGB8> const& input,
            int16_t (&kernel)[3][3]);

/// @brief Box 2D filter
/// @param output The output image
/// @param input The input image
void box(image<rgb8, pixel_format::RGB8>& output, image<rgb8, pixel_format::RGB8> const& input);

/// @brief Gaussian 2D filter
/// @param output The output image
/// @param input The input image
void gaussian(image<rgb8, pixel_format::RGB8>& output, image<rgb8, pixel_format::RGB8> const& input);

/// Returns the dimensions given a constant string.
/// @code
/// auto [width, height] = fourcc::dimensions("VGA");
/// @endcode
std::pair<size_t, size_t> dimensions(std::string type);

}  // namespace fourcc
