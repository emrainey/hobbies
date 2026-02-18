/// @file
/// Conversions implementations

#include <fourcc/convert.hpp>

namespace fourcc {

void convert(image<PixelFormat::RGB8> const& in, image<PixelFormat::IYU2>& out) {
    basal::exception::throw_unless(in.height == out.height, __FILE__, __LINE__, "Must be the same height %zu != %zu",
                                   in.height, out.height);
    basal::exception::throw_unless(in.width == out.width, __FILE__, __LINE__, "Must be the same width %zu != %zu",
                                   in.width, out.width);
    for (size_t y = 0; y < in.height; y++) {
        for (size_t x = 0; x < in.width; x++) {
            precision R = in.at(y, x).components.r;
            precision G = in.at(y, x).components.g;
            precision B = in.at(y, x).components.b;
            precision Y = 0 + 0.2215_p * R + 0.7154_p * G + 0.0721_p * B;
            precision Cb = 0 - 0.1145_p * R - 0.3855_p * G + 0.5000_p * B;
            precision Cr = 0 + 0.5016_p * R - 0.4556_p * G - 0.0459_p * B;
            int32_t y_ = (int32_t)Y;
            int32_t u_ = (int32_t)Cb + 128;
            int32_t v_ = (int32_t)Cr + 128;
            uint8_t _u = (u_ > 255 ? 255 : (u_ < 0 ? 0U : static_cast<uint8_t>(u_)));
            uint8_t _y = (y_ > 255 ? 255 : (y_ < 0 ? 0U : static_cast<uint8_t>(y_)));
            uint8_t _v = (v_ > 255 ? 255 : (v_ < 0 ? 0U : static_cast<uint8_t>(v_)));
            iyu2 pixel;
            pixel.components.u = _u;
            pixel.components.y = _y;
            pixel.components.v = _v;
            out.at(y, x) = pixel;
        }
    }
}


void convert(image<PixelFormat::RGBId> const& in, image<PixelFormat::RGB8>& out) {
    basal::exception::throw_unless(in.height == out.height, __FILE__, __LINE__, "Must be the same height %zu != %zu",
                                   in.height, out.height);
    basal::exception::throw_unless(in.width == out.width, __FILE__, __LINE__, "Must be the same width %zu != %zu",
                                   in.width, out.width);
    for (size_t y = 0; y < in.height; y++) {
        for (size_t x = 0; x < in.width; x++) {
            color in_pixel{in.at(y, x)};
            in_pixel.clamp();
            in_pixel.ToEncoding(Encoding::GammaCorrected);
            out.at(y, x) = in_pixel.to_<PixelFormat::RGB8>();
        }
    }
}

}  // namespace fourcc