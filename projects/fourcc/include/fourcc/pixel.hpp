#pragma once

/// @file
/// Definitions for The Pixel Template Class and related functions.


#include <iso/iso.hpp>
#include <cstdint>
#include <functional>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ostream>
#include <type_traits>

#include <fourcc/types.hpp>
#include <fourcc/encoding.hpp>

namespace fourcc {

/// A single pixel value in a given type and format.
template <PixelFormat PIXEL_FORMAT>
class pixel {
public:
    /// The externally usable format of the pixel
    constexpr static PixelFormat const format{PIXEL_FORMAT};
    /// The storage type of the pixel
    using StorageType = decltype(GetStorageType<PIXEL_FORMAT>());
    /// The type which is used to store each channel
    using ChannelType = typename StorageType::ChannelType;
    /// The number of channels comes from the Storage Type.
    constexpr static size_t channel_count = StorageType::channel_count;
    /// The Color Space of the channels.
    constexpr static ColorSpace color_space = GetColorSpace(PIXEL_FORMAT);

protected:
    /// The pixel data
    StorageType data_;
    /// The current encoding of the pixel data
    Encoding encoding_;

public:

    /// Default Constructor
    template <typename = void>
    constexpr pixel() : data_{}, encoding_{Encoding::Linear} {
    }

    /// Constructor from the storage type directly
    constexpr explicit pixel(StorageType const& data) : data_{data}, encoding_{Encoding::Linear} {
    }

    /// Explicit constructor from the specific rgbid type
    constexpr explicit pixel(ChannelType _r, ChannelType _g, ChannelType _b, ChannelType _i = 1.0_p)
        : data_{}, encoding_{Encoding::Linear} {
        static_assert(std::is_same_v<StorageType,rgbid>, "This constructor is only for RGBId types");
        data_.components.r = _r;
        data_.components.g = _g;
        data_.components.b = _b;
        data_.components.i = _i;
    }

    constexpr Encoding GetEncoding() const {
        return encoding_;
    }

    void ToEncoding(Encoding desired) {
        if (desired == Encoding::Linear and encoding_ == Encoding::GammaCorrected ) {
            data_.channels[0] = gamma::apply_correction(data_.channels[0]);
            data_.channels[1] = gamma::apply_correction(data_.channels[1]);
            data_.channels[2] = gamma::apply_correction(data_.channels[2]);
            if constexpr (channel_count >= 4) {
                data_.channels[3] = gamma::apply_correction(data_.channels[3]);
            }
            encoding_ = Encoding::Linear;
        } else if (desired == Encoding::GammaCorrected and encoding_ == Encoding::Linear) {
            data_.channels[0] = gamma::remove_correction(data_.channels[0]);
            data_.channels[1] = gamma::remove_correction(data_.channels[1]);
            data_.channels[2] = gamma::remove_correction(data_.channels[2]);
            if constexpr (channel_count >= 4) {
                data_.channels[3] = gamma::remove_correction(data_.channels[3]);
            }
            encoding_ = Encoding::GammaCorrected ;
        } else {
        }
    }

    void per_channel(std::function<ChannelType(ChannelType c)> iter) {
        for (size_t i = 0; i < channel_count; i++) {
            data_.channels[i] = iter(data_.channels[i]);
        }
    }

    inline void scale(ChannelType const a) {
        basal::exception::throw_unless(encoding_ == Encoding::Linear, __FILE__, __LINE__,
                                       "Color should be in linear Encoding");
        for (auto& c : data_.channels) {
            c *= a;
        }
    }

    inline void clamp() {
        static_assert(std::is_floating_point_v<ChannelType>, "Must be a floating point type to clamp!");
        for (auto& c : data_.channels) {
            c = std::clamp(c, 0.0_p, 1.0_p);
        }
    }

    inline pixel& operator*=(ChannelType const a) {
        scale(a);
        return (*this);
    }

    pixel& operator+=(pixel const& other) {
        basal::exception::throw_unless(encoding_ == Encoding::Linear, __FILE__, __LINE__,
                                       "Color should be in linear Encoding");
        for (size_t i = 0; i < channel_count; i++) {
            data_.channels[i] += other.data_.channels[i];
        }
        return (*this);
    }

    StorageType& data() {
        return data_;
    }

    StorageType const& data() const {
        return data_;
    }

    constexpr StorageType copy() const {
        StorageType pixel;
        for (size_t i = 0; i < channel_count; i++) {
            pixel.data_.channels[i] = data_.channels[i];
        }
        return pixel;
    }

    template <typename = std::enable_if_t<color_space == ColorSpace::RGB>>
    constexpr ChannelType const& red() const {
        return data_.channels[0];
    }

    template <typename = std::enable_if_t<color_space == ColorSpace::RGB>>
    constexpr ChannelType const& green() const {
        return data_.channels[1];
    }

    template <typename = std::enable_if_t<color_space == ColorSpace::RGB>>
    constexpr ChannelType const& blue() const {
        return data_.channels[2];
    }

    constexpr ChannelType intensity() const {
        static_assert(std::is_same_v<StorageType,rgbid>, "Must be an RGBId type");
        if constexpr (channel_count >= 4) {
            return data_.channels[3];
        } else {
            return ChannelType{1.0_p};
        }
    }

    constexpr static ChannelType equality_limit = 1.0E-6;

    static pixel blend_samples(std::vector<pixel> const& subsamples) {
        pixel output;
        precision div = static_cast<precision>(subsamples.size());
        for (auto& sub : subsamples) {
            basal::exception::throw_unless(sub.GetEncoding() == fourcc::Encoding::Linear, __FILE__, __LINE__,
                                           "Color should be in linear Encoding");
            auto tmp = sub;
            tmp *= (1.0_p / div);
            output += tmp;
        }
        return output;
    }

protected:
    rgba to_rgba() const {
        rgba pixel;
        basal::exception::throw_unless(encoding_ == Encoding::GammaCorrected , __FILE__, __LINE__,
                                       "Color should be in GammaCorrected Encoding");
        pixel.components.r = static_cast<uint8_t>(std::round(red() * 255));
        pixel.components.g = static_cast<uint8_t>(std::round(green() * 255));
        pixel.components.b = static_cast<uint8_t>(std::round(blue() * 255));
        pixel.components.a = 255;
        return pixel;
    }

    abgr to_abgr() const {
        abgr pixel;
        basal::exception::throw_unless(encoding_ == Encoding::GammaCorrected , __FILE__, __LINE__,
                                       "Color should be in GammaCorrected Encoding");
        pixel.components.r = static_cast<uint8_t>(std::round(red() * 255));
        pixel.components.g = static_cast<uint8_t>(std::round(green() * 255));
        pixel.components.b = static_cast<uint8_t>(std::round(blue() * 255));
        pixel.components.a = 255;
        return pixel;
    }

    rgb8 to_rgb8() const {
        rgb8 pixel;
        basal::exception::throw_unless(encoding_ == Encoding::GammaCorrected , __FILE__, __LINE__,
                                       "Color should be in GammaCorrected Encoding");
        pixel.components.r = static_cast<uint8_t>(std::round(red() * 255));
        pixel.components.g = static_cast<uint8_t>(std::round(green() * 255));
        pixel.components.b = static_cast<uint8_t>(std::round(blue() * 255));
        return pixel;
    }

    bgr8 to_bgr8() const {
        bgr8 pixel;
        basal::exception::throw_unless(encoding_ == Encoding::GammaCorrected , __FILE__, __LINE__,
                                       "Color should be in GammaCorrected Encoding");
        pixel.components.r = static_cast<uint8_t>(std::round(red() * 255));
        pixel.components.g = static_cast<uint8_t>(std::round(green() * 255));
        pixel.components.b = static_cast<uint8_t>(std::round(blue() * 255));
        return pixel;
    }

    rgbh to_rgbh() const {
        rgbh pixel;
        basal::exception::throw_unless(encoding_ == Encoding::Linear, __FILE__, __LINE__,
                                       "Color should be in Linear Encoding");
        pixel.components.r = static_cast<basal::half>(red());
        pixel.components.g = static_cast<basal::half>(green());
        pixel.components.b = static_cast<basal::half>(blue());
        return pixel;
    }

    rgbid to_rgbid() const {
        rgbid pixel;
        basal::exception::throw_unless(encoding_ == Encoding::Linear, __FILE__, __LINE__,
                                       "Color should be in Linear Encoding");
        pixel.components.r = red();
        pixel.components.g = green();
        pixel.components.b = blue();
        pixel.components.i = intensity();
        return pixel;
    }

public:
    template <PixelFormat OTHER_FORMAT>
    auto to_() const -> decltype(GetStorageType<OTHER_FORMAT>()) {
        using OutputType = decltype(GetStorageType<OTHER_FORMAT>());
        if constexpr (std::is_same_v<OutputType, StorageType>) {
            return data_; // copy
        } else if constexpr (std::is_same_v<OutputType, rgba>) {
            return to_rgba();
        } else if constexpr (std::is_same_v<OutputType, abgr>) {
            return to_abgr();
        } else if constexpr (std::is_same_v<OutputType, rgb8>) {
            return to_rgb8();
        } else if constexpr (std::is_same_v<OutputType, bgr8>) {
            return to_bgr8();
        } else if constexpr (std::is_same_v<OutputType, rgbh>) {
            return to_rgbh();
        } else {
            return OutputType{};
        }
    }
};

/// The highest precision pixel format for color data
using color = pixel<PixelFormat::RGBId>;

/// Simply adds values together linearly
/// @warning This is not a BLENDED pixel, it is simply accumulated!
color accumulate_samples(std::vector<color> const& samples);

/// Color-Codes a precision value from -1.0 to 1.0 using the jet color map.
color jet(precision value);

/// Color-Codes a precision value from min to max using a greyscale color map. Values outside the range are colored magenta.
color greyscale(precision d, precision min, precision max) ;

/// Generates a pixel of a random color
color random();

template <PixelFormat PIXEL_FORMAT, typename = std::enable_if_t<is_rgb_type(PIXEL_FORMAT)>>
pixel<PIXEL_FORMAT> interpolate(pixel<PIXEL_FORMAT> const& x,
                                pixel<PIXEL_FORMAT> const& y,
                                precision ratio) {
    return pixel<PIXEL_FORMAT>(gamma::interpolate(x.red(), y.red(), ratio),
                               gamma::interpolate(x.green(), y.green(), ratio),
                               gamma::interpolate(x.blue(), y.blue(), ratio));
}

template <PixelFormat PIXEL_FORMAT, typename = std::enable_if_t<is_rgb_type(PIXEL_FORMAT)>>
pixel<PIXEL_FORMAT> blend(pixel<PIXEL_FORMAT> const& x,
                          pixel<PIXEL_FORMAT> const& y) {
    return interpolate(x, y, 0.5_p);
}

color wavelength_to_color(iso::meters lambda) noexcept(false);

template <PixelFormat PIXEL_FORMAT, typename = std::enable_if_t<is_rgb_type(PIXEL_FORMAT)>>
bool operator==(pixel<PIXEL_FORMAT> const& a, pixel<PIXEL_FORMAT> const& b) {
    return basal::nearly_equals(a.red(), b.red(), pixel<PIXEL_FORMAT>::equality_limit)
           and basal::nearly_equals(a.green(), b.green(), pixel<PIXEL_FORMAT>::equality_limit)
           and basal::nearly_equals(a.blue(), b.blue(), pixel<PIXEL_FORMAT>::equality_limit);
}

template <PixelFormat PIXEL_FORMAT, typename = std::enable_if_t<is_rgb_type(PIXEL_FORMAT)>>
std::ostream& operator<<(std::ostream& os, pixel<PIXEL_FORMAT> const& c) {
    os << "color(" << c.red() << ", " << c.green() << ", " << c.blue() << ")";
    return os;
}

namespace operators {
template <PixelFormat PIXEL_FORMAT, typename = std::enable_if_t<is_rgb_type(PIXEL_FORMAT)>>
pixel<PIXEL_FORMAT> operator*(pixel<PIXEL_FORMAT> const& a,
                              pixel<PIXEL_FORMAT> const& b) {
    return pixel<PIXEL_FORMAT>(a.red() * b.red(), a.green() * b.green(), a.blue() * b.blue());
}
}  // namespace operators
}  // namespace fourcc