#pragma once
/// @file
/// Definitions for any visualization objects.
/// @copyright Copyright 2022 (C) Erik Rainey.

#include <iso/iso.hpp>
#include <cstdint>
#include <functional>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ostream>
#include <type_traits>

#include <basal/exception.hpp>
#include <basal/ieee754.hpp>

/// The Visualization Namespace
namespace fourcc {

/// Use the platform precision for the gamma functions
using precision = basal::precision;

// Expose the base literal operators
using namespace basal::literals;

/// Takes a four character code and returns a specific FOURCC type
constexpr uint32_t four_character_code(char fcc0, char fcc1, char fcc2, char fcc3) {
    return (uint32_t(fcc0) << 0) | (uint32_t(fcc1) << 8) | (uint32_t(fcc2) << 16) | (uint32_t(fcc3) << 24);
}

/// Enumerates the ChannelType names
enum class ChannelName : int8_t {
    R,  ///< Red
    G,  ///< Green
    B,  ///< Blue
    Y,  ///< Luminance
    U,  ///< Chrominance U
    V,  ///< Chrominance V
    A,  ///< Alpha
};

/// The types of encoding to use to convert the values to the other
/// types.
enum class Encoding : int {
    Linear,          ///< No range compression, full range of storage type.
    GammaCorrected,  ///< Gamma corrected encoding (uses the 2.2 scheme)
};

/// The high level color space that that pixels use to represent color.
/// This does not reflect _format_ or order of channels or
enum class ColorSpace : int {
    RGB = 1,
    YUV = 2,
};

/// Check that the function produces the right output
static_assert(0x30303859 == four_character_code('Y', '8', '0', '0'), "Must be little endian");

/// Defines the RGB8 (24 bit) type of red, then green, then blue
union rgb8 {
    using ChannelType = uint8_t;
    constexpr static size_t channel_count = 3;
    constexpr rgb8() : components{} {
    }
    struct {
        ChannelType r{0u};  //!< Red Component
        ChannelType g{0u};  //!< Green Component
        ChannelType b{0u};  //!< Blue Component
    } components;
    ChannelType channels[channel_count];
} __attribute__((packed));

/// Defines the BGR8 (24 bit) type of blue, then green, then red
union bgr8 {
    using ChannelType = uint8_t;
    constexpr static size_t channel_count = 3;
    constexpr bgr8() : components{} {
    }
    struct {
        ChannelType b{0u};  //!< Blue Component
        ChannelType g{0u};  //!< Green Component
        ChannelType r{0u};  //!< Red Component
    } components;
    ChannelType channels[channel_count];
} __attribute__((packed));

/// Defines the RGBA (32 bit) type of red, then green, then blue, then alpha
union rgba {
    using ChannelType = uint8_t;
    constexpr static size_t channel_count = 4;
    constexpr rgba() : components{} {
    }
    struct {
        ChannelType r{0u};  //!< Red Component
        ChannelType g{0u};  //!< Green Component
        ChannelType b{0u};  //!< Blue Component
        ChannelType a{0u};  //!< Alpha Component
    } components;
    ChannelType channels[channel_count];
} __attribute__((packed));

/// Defines the ABGR (32 bit) type of alpha, then blue, then green, then red
union abgr {
    using ChannelType = uint8_t;
    constexpr static size_t channel_count = 4;
    constexpr abgr() : components{} {
    }
    struct {
        ChannelType a{0u};  //!< Alpha Component
        ChannelType b{0u};  //!< Blue Component
        ChannelType g{0u};  //!< Green Component
        ChannelType r{0u};  //!< Red Component
    } components;
    ChannelType channels[channel_count];
} __attribute__((packed));

/// Defines the ABGR (32 bit) type of alpha, then blue, then green, then red
union bgra {
    using ChannelType = uint8_t;
    constexpr static size_t channel_count = 4;
    constexpr bgra() : components{} {
    }
    struct {
        ChannelType b{0u};  //!< Blue Component
        ChannelType g{0u};  //!< Green Component
        ChannelType r{0u};  //!< Red Component
        ChannelType a{0u};  //!< Alpha Component
    } components;
    ChannelType channels[channel_count];
} __attribute__((packed));

/// Defines the 4:4:4 (non subsampled) single plane interleaved YUV format
union iyu2 {
    using ChannelType = uint8_t;
    constexpr static size_t channel_count = 3;
    constexpr iyu2() : components{} {
    }
    struct {
        ChannelType u{0u};  //!< U Component
        ChannelType y{0u};  //!< Y Component
        ChannelType v{0u};  //!< V Component
    } components;
    ChannelType channels[channel_count];
} __attribute__((packed));

/// Defines a 1 ChannelType 32 bit float color format (luminance)
union yf {
    using ChannelType = float;
    constexpr static size_t channel_count = 1;
    constexpr yf() : components{} {
    }
    struct {
        ChannelType y{0.0f};  //!< Y Component
    } components;
    ChannelType channels[channel_count];
};

/// Defines a 3 ChannelType 32 bit float color format
union rgbf {
    using ChannelType = float;
    constexpr static size_t channel_count = 3;
    constexpr rgbf() : components{} {
    }
    struct {
        ChannelType r{0.0f};  //!< Red Component
        ChannelType g{0.0f};  //!< Green Component
        ChannelType b{0.0f};  //!< Blue Component
    } components;
    ChannelType channels[channel_count];
};

/// Defines a 3 Channel 16 Bit Half Precision Color Format
union rgbh {
    using ChannelType = basal::half;
    constexpr static size_t channel_count = 3;
    constexpr rgbh() : components{} {
    }
    struct {
        ChannelType r{0.0f};  //!< Red Component
        ChannelType g{0.0f};  //!< Green Component
        ChannelType b{0.0f};  //!< Blue Component
    } components;
    ChannelType channels[channel_count];
};
static_assert(sizeof(rgbh) == 6, "rgbh must be 6 bytes");

/// Defines a 4 Channel 32 Bit Float Precision Color Format
union rgbaf {
    using ChannelType = float;
    constexpr static size_t channel_count = 4;
    constexpr rgbaf() : components{} {
    }
    struct {
        ChannelType r{0.0f};  //!< Red Component
        ChannelType g{0.0f};  //!< Green Component
        ChannelType b{0.0f};  //!< Blue Component
        ChannelType a{0.0f};  //!< Alpha Component
    } components;
    ChannelType channels[channel_count];
};

/// RGB + Intensity format for intermediate calculations where we want to preserve the full range of values without
/// clamping, and also have an intensity ChannelType for things like alpha or other uses. This is in the Linear space
/// and not clamped.
union rgbid {
    using ChannelType = double;
    constexpr static size_t channel_count = 4;
    constexpr rgbid() : components{} {
    }
    struct {
        ChannelType r{0.0};  //!< Red Component
        ChannelType g{0.0};  //!< Green Component
        ChannelType b{0.0};  //!< Blue Component
        ChannelType i{0.0};  //!< Intensity Component
    } components;
    ChannelType channels[channel_count];
};

/// Defines a 16 bit packed pixel color in 5:6:5 format. R being the highest bits, and B the lowest
union rgb565 {
    /// Each ChannelType is either 5 or 6 bits, but we use uint16_t to pack them together, but each fits better in a
    /// uint8_t.
    using ChannelType = uint8_t;
    constexpr static size_t channel_count = 3;
    constexpr rgb565() : channels{} {
    }
    struct {
        uint16_t b : 5;
        uint16_t g : 6;
        uint16_t r : 5;
    } components;
    ChannelType channels[1];  //!< Packed channels in RGB565 format
} __attribute__((packed));

/// Enumerates the various pixel formats in FOURCC codes and some additional types.
/// \see fourcc.org
enum class PixelFormat : std::uint32_t {
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
    RGBId = four_character_code(
        'R', 'G', 'B',
        'I'),  ///< Uses @reg rgbid (linear space with no clamping, typically for intermediate calculations)
    RGBP = four_character_code('R', 'G', 'B', 'P'),  ///< Uses @reg rgb565
};

/// Returns the string name of the format
constexpr char const* channel_order(PixelFormat fmt) {
    switch (fmt) {
        case PixelFormat::RGBA:
            [[fallthrough]];
        case PixelFormat::RGBAf:
            return "RGBA";
        case PixelFormat::RGBId:
            [[fallthrough]];
        case PixelFormat::RGBf:
            [[fallthrough]];
        case PixelFormat::RGBh:
            [[fallthrough]];
        case PixelFormat::RGB8:
            return "RGB";
        case PixelFormat::RGBP:
            [[fallthrough]];
        case PixelFormat::BGR8:
            [[fallthrough]];
        case PixelFormat::ABGR:
            return "BGR";
        case PixelFormat::BGRA:
            return "BGRA";
        case PixelFormat::IYU2:
            return "UYV";
        case PixelFormat::YF:
            return "YF";
        default:
            return "???";
    }
}

/// Returns the number of planes in the format
constexpr int planes_in_format(PixelFormat fmt __attribute__((unused))) {
    switch (fmt) {
        default:
            return 1;
    }
}

/// Returns the number of channel_count for the format
constexpr int channels_in_format(PixelFormat fmt) {
    switch (fmt) {
        case PixelFormat::RGBA:
        case PixelFormat::ABGR:
        case PixelFormat::BGRA:
        case PixelFormat::RGBAf:
        case PixelFormat::RGBId:
            return 4;
        case PixelFormat::RGBP:
        case PixelFormat::RGBf:
        case PixelFormat::RGBh:
        case PixelFormat::RGB8:
        case PixelFormat::BGR8:
        case PixelFormat::IYU2:
            return 3;
        case PixelFormat::Y16:
        case PixelFormat::Y32:
        case PixelFormat::YF:
        case PixelFormat::GREY8:
        case PixelFormat::Y800:
        case PixelFormat::Y8:
        default:
            return 1;
    }
}

/// Returns true if the format uses uint8_t as a channel
constexpr bool uses_uint8(PixelFormat fmt) {
    switch (fmt) {
        case PixelFormat::Y800:
        case PixelFormat::Y8:
        case PixelFormat::GREY8:
            return true;
        default:
            break;
    }
    return false;
}

/// Returns true if the format uses uint16_t as a channel
constexpr bool uses_uint16(PixelFormat fmt) {
    return (fmt == PixelFormat::Y16);
}

/// Returns true if the format uses rgb8_t as a channel
constexpr bool uses_rgb8(PixelFormat fmt) {
    return (fmt == PixelFormat::RGB8);
}

/// Returns true if the format uses bgr8_t as a channel
constexpr bool uses_bgr8(PixelFormat fmt) {
    return (fmt == PixelFormat::BGR8);
}

/// Returns true if the format uses uint32_t as a channel
constexpr bool uses_uint32(PixelFormat fmt) {
    switch (fmt) {
        case PixelFormat::Y32:
            return true;
        default:
            break;
    }
    return false;
}

/// Returns true if the format uses iyu2 as a format
constexpr bool uses_iyu2(PixelFormat fmt) {
    return (fmt == PixelFormat::IYU2);
}

constexpr bool uses_rgbf(PixelFormat fmt) {
    return (fmt == PixelFormat::RGBf);
}

constexpr bool uses_rgbaf(PixelFormat fmt) {
    return (fmt == PixelFormat::RGBAf);
}

constexpr bool uses_rgbid(PixelFormat fmt) {
    return (fmt == PixelFormat::RGBId);
}

constexpr bool uses_rgbh(PixelFormat fmt) {
    return (fmt == PixelFormat::RGBh);
}

constexpr bool uses_rgba(PixelFormat fmt) {
    return (fmt == PixelFormat::RGBA);
}

constexpr bool uses_abgr(PixelFormat fmt) {
    return (fmt == PixelFormat::ABGR);
}

constexpr bool uses_bgra(PixelFormat fmt) {
    return (fmt == PixelFormat::BGRA);
}

constexpr bool uses_yf(PixelFormat fmt) {
    return (fmt == PixelFormat::YF);
}

/// Returns true if the format uses rgb565 (either endianess) as a format
constexpr bool uses_rgb565(PixelFormat fmt) {
    return (fmt == PixelFormat::RGBP);
}

template <PixelFormat PIXEL_FORMAT>
auto GetStorageType() {
    if constexpr (uses_uint8(PIXEL_FORMAT)) {
        return uint8_t{};
    } else if constexpr (uses_uint16(PIXEL_FORMAT)) {
        return uint16_t{};
    } else if constexpr (uses_rgb8(PIXEL_FORMAT)) {
        return rgb8{};
    } else if constexpr (uses_bgr8(PIXEL_FORMAT)) {
        return bgr8{};
    } else if constexpr (uses_uint32(PIXEL_FORMAT)) {
        return uint32_t{};
    } else if constexpr (uses_iyu2(PIXEL_FORMAT)) {
        return iyu2{};
    } else if constexpr (uses_rgbf(PIXEL_FORMAT)) {
        return rgbf{};
    } else if constexpr (uses_rgbaf(PIXEL_FORMAT)) {
        return rgbaf{};
    } else if constexpr (uses_rgbid(PIXEL_FORMAT)) {
        return rgbid{};
    } else if constexpr (uses_rgbh(PIXEL_FORMAT)) {
        return rgbh{};
    } else if constexpr (uses_yf(PIXEL_FORMAT)) {
        return yf{};
    } else if constexpr (uses_rgba(PIXEL_FORMAT)) {
        return rgba{};
    } else if constexpr (uses_abgr(PIXEL_FORMAT)) {
        return abgr{};
    } else if constexpr (uses_bgra(PIXEL_FORMAT)) {
        return bgra{};
    } else if constexpr (uses_rgb565(PIXEL_FORMAT)) {
        return rgb565{};
    }
}

/// Returns the high level color space that the fmt uses.
constexpr ColorSpace GetColorSpace(PixelFormat fmt) {
    switch (fmt) {
        case PixelFormat::IYU2:
        case PixelFormat::YF:
        case PixelFormat::Y16:
        case PixelFormat::Y32:
        case PixelFormat::Y800:
        case PixelFormat::Y8:
            return ColorSpace::YUV;
        default:
            return ColorSpace::RGB;
    }
}

/// @returns True when teh format is an RGB type
constexpr bool is_rgb_type(PixelFormat fmt) {
    return GetColorSpace(fmt) == ColorSpace::RGB;
}

/// @returns True when the format is a YUV type
constexpr bool is_yuv_type(PixelFormat fmt) {
    return GetColorSpace(fmt) == ColorSpace::YUV;
}

constexpr PixelFormat GetFormatForExtension(std::string_view extension) {
    if (extension == ".ppm") {
        return PixelFormat::RGB8;
    } else if (extension == ".pgm") {
        return PixelFormat::Y16;
    } else if (extension == ".pfm") {
        return PixelFormat::RGBf;
    } else if (extension == ".tga") {
        return PixelFormat::RGB8;
    } else if (extension == ".exr") {
        return PixelFormat::RGBh;
    }
    return PixelFormat::RGB8;  // default to RGB8
}

namespace {  // anonymous namespace for testing types
using TestType = decltype(GetStorageType<PixelFormat::RGB8>());
static_assert(std::is_same_v<TestType, rgb8>, "Must be this type!");
static_assert(ColorSpace::RGB == GetColorSpace(PixelFormat::RGB8), "RGB8 should be in the RGB colorspace");
}  // namespace

}  // namespace fourcc
