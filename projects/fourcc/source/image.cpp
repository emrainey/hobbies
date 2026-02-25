/// @file
/// Implements the image operations

#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <filesystem>

#include "fourcc/image.hpp"
#include "fourcc/convert.hpp"
#include "fourcc/targa.hpp"
#include "fourcc/openexr.hpp"

namespace fourcc {

static bool is_extension(std::string filename, std::string ext) {
    // std::filesystem::path fp = filename;
    // return fp.extension() == ext;
    char const* pext = strrchr(filename.c_str(), '.');
    if (strcmp(pext, ext.c_str()) == 0) {
        return true;
    }
    return false;
}

// +==============================================================+
// Forward Declare the Template Specializations for saving the images. These are defined after the convert functions
// since some of the convert functions use them.

template <>
bool image<PixelFormat::RGB8>::save(std::string filename) const;

template <>
bool image<PixelFormat::RGBf>::save(std::string filename) const;

template <>
bool image<PixelFormat::RGBh>::save(std::string filename) const;

// +==============================================================+

template <>
bool image<PixelFormat::RGBId>::save(std::string filename) const {
    if (is_extension(filename, ".pfm")) {
        image<PixelFormat::RGBf> output{height, width};
        fourcc::convert(*this, output);
        return output.save(filename);
    } else if (is_extension(filename, ".ppm") or is_extension(filename, ".tga")) {
        image<PixelFormat::RGB8> output{height, width};
        fourcc::convert(*this, output);
        return output.save(filename);
    } else if (is_extension(filename, ".exr")) {
        image<PixelFormat::RGBh> output{height, width};
        fourcc::convert(*this, output);
        return output.save(filename);
    }
    return false;
}

template <>
bool image<PixelFormat::RGBA>::save(std::string filename) const {
    if (is_extension(filename, ".rgb")) {
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp) {
            for_each([&](fourcc::rgba const& pixel) -> void {
                fwrite(&pixel.components.r, sizeof(pixel.components.r), 1, fp);
                fwrite(&pixel.components.g, sizeof(pixel.components.g), 1, fp);
                fwrite(&pixel.components.b, sizeof(pixel.components.b), 1, fp);
            });
            fclose(fp);
            return true;
        }
    } else if (is_extension(filename, ".bgr")) {
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp) {
            for_each([&](fourcc::rgba const& pixel) -> void {
                fwrite(&pixel.components.b, sizeof(pixel.components.b), 1, fp);
                fwrite(&pixel.components.g, sizeof(pixel.components.g), 1, fp);
                fwrite(&pixel.components.r, sizeof(pixel.components.r), 1, fp);
            });
            fclose(fp);
            return true;
        }
    } else if (is_extension(filename, ".pam")) {
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp) {
            fprintf(fp, "P7\n");
            fprintf(fp, "WIDTH %zu\nHEIGHT %zu\nDEPTH %zu\n", width, height, depth);
            fprintf(fp, "MAXVAL %u\n", 255u);
            fprintf(fp, "TUPLTYPE %s\n", channel_order(format));
            fprintf(fp, "ENDHDR\n");
            for_each([&](rgba const& pixel) -> void {
                fwrite(&pixel.components.r, sizeof(pixel.components.r), 1, fp);
                fwrite(&pixel.components.g, sizeof(pixel.components.g), 1, fp);
                fwrite(&pixel.components.b, sizeof(pixel.components.b), 1, fp);
            });
            fclose(fp);
        }
        return true;
    } else if (is_extension(filename, ".tga")) {
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp) {
            targa::header hdr;
            hdr.id_length = 0;
            hdr.color_map_type = targa::ColorMapType::None;
            hdr.image_map_type = targa::ImageMapType::UncompressedTrueColor;
            hdr.color_map.first_index = 0;
            hdr.color_map.length = 0;
            hdr.color_map.entry_size = 0;
            hdr.image_map.x_origin = 0;
            hdr.image_map.y_origin = 0;
            hdr.image_map.image_width = width & 0xFFFFU;
            hdr.image_map.image_height = height & 0xFFFFU;
            hdr.image_map.pixel_depth = (depth & 0x0FU) * 8U;
            hdr.image_map.image_descriptor = 0;  // avoid complex interleaving
            fwrite(&hdr, sizeof(hdr), 1, fp);
            // identification string
            // color map
            // image data as B. then G, then R. but y is inverted
            for (size_t y = (height - 1); /* y >= 0 */; y--) {
                for (size_t x = 0; x < width; x++) {
                    rgba const& pixel = at(y, x);
                    fwrite(&pixel.components.b, sizeof(pixel.components.b), 1, fp);
                    fwrite(&pixel.components.g, sizeof(pixel.components.g), 1, fp);
                    fwrite(&pixel.components.r, sizeof(pixel.components.r), 1, fp);
                }
                if (y == 0) {
                    break;
                }
            }
            fclose(fp);
        }
    }
    return false;
}

template <>
bool image<PixelFormat::ABGR>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fprintf(fp, "P7\n");
        fprintf(fp, "WIDTH %zu\nHEIGHT %zu\nDEPTH %zu\n", width, height, depth);
        fprintf(fp, "MAXVAL %u\n", 255u);
        fprintf(fp, "TUPLTYPE %s\n", channel_order(format));
        fprintf(fp, "ENDHDR\n");
        for_each([&](abgr const& pixel) -> void {
            fwrite(&pixel.components.b, sizeof(uint8_t), 1, fp);
            fwrite(&pixel.components.g, sizeof(uint8_t), 1, fp);
            fwrite(&pixel.components.r, sizeof(uint8_t), 1, fp);
        });
        fclose(fp);
        return true;
    }
    return false;
}

template <>
bool image<PixelFormat::RGB8>::save(std::string filename) const {
    if (is_extension(filename, ".ppm")) {
        static constexpr bool use_p6 = true;
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp) {
            if constexpr (use_p6) {
                fprintf(fp, "P6\n");
                fprintf(fp, "%zu %zu\n255\n", width, height);
                for_each([&](rgb8 const& pixel) -> void { fwrite(&pixel, sizeof(rgb8), 1, fp); });
            } else {
                fprintf(fp, "P7\n");
                fprintf(fp, "WIDTH %zu\nHEIGHT %zu\nDEPTH %zu\n", width, height, depth);
                fprintf(fp, "MAXVAL %u\n", 255u);
                fprintf(fp, "TUPLTYPE %s\n", channel_order(format));
                fprintf(fp, "ENDHDR\n");
                for_each([&](rgb8 const& pixel) -> void { fwrite(&pixel, sizeof(rgb8), 1, fp); });
            }
            fclose(fp);
            return true;
        }
    } else if (is_extension(filename, ".tga")) {
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp) {
            targa::header hdr;
            hdr.id_length = 0;
            hdr.color_map_type = targa::ColorMapType::None;
            hdr.image_map_type = targa::ImageMapType::UncompressedTrueColor;
            hdr.color_map.first_index = 0;
            hdr.color_map.length = 0;
            hdr.color_map.entry_size = 0;
            hdr.image_map.x_origin = 0;
            hdr.image_map.y_origin = 0;
            hdr.image_map.image_width = width & 0xFFFFU;
            hdr.image_map.image_height = height & 0xFFFFU;
            hdr.image_map.pixel_depth = (depth & 0x0FU) * 8;
            hdr.image_map.image_descriptor = 0;  // avoid complex interleaving
            fwrite(&hdr, sizeof(hdr), 1, fp);
            // identification string
            // color map
            // image data as B, then G, then R. but upside down where y is inverted
            for (size_t y = (height - 1); /* y >= 0 */; y--) {
                for (size_t x = 0; x < width; x++) {
                    rgb8 const& pixel = at(y, x);
                    fwrite(&pixel.components.b, sizeof(pixel.components.b), 1, fp);
                    fwrite(&pixel.components.g, sizeof(pixel.components.g), 1, fp);
                    fwrite(&pixel.components.r, sizeof(pixel.components.r), 1, fp);
                }
                if (y == 0) {
                    break;
                }
            }
            fclose(fp);
            return true;
        }
    }
    return false;
}

template <>
bool image<PixelFormat::RGBf>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fprintf(fp, "PF\n");
        fprintf(fp, "%zu %zu\n", width, height);
        fprintf(fp, "-1.000000\n");  // little endian float
        for (size_t y = (height - 1); /* y >= 0 */; y--) {
            for (size_t x = 0; x < width; x++) {
                rgbf const& pixel = at(y, x);
                fwrite(&pixel, sizeof(rgbf), 1, fp);
            }
            if (y == 0) {
                break;
            }
        }
        fclose(fp);
        return true;
    }
    return false;
}

template <>
bool image<PixelFormat::BGR8>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fprintf(fp, "P7\n");
        fprintf(fp, "WIDTH %zu\nHEIGHT %zu\nDEPTH %zu\n", width, height, depth);
        fprintf(fp, "MAXVAL %u\n", 255u);
        fprintf(fp, "TUPLTYPE %s\n", channel_order(format));
        fprintf(fp, "ENDHDR\n");
        for_each([&](bgr8 const& pixel) -> void { fwrite(&pixel, sizeof(bgr8), 1, fp); });
        fclose(fp);
        return true;
    }
    return false;
}

template <>
bool image<PixelFormat::RGBh>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        uint8_t const zero = 0;
        size_t pixel_size = sizeof(rgbh);

        // write the file as OpenEXR format
        fwrite(&openexr::magic, sizeof(openexr::magic), 1, fp);
        // write the version
        openexr::Version version;
        version.version = 2;
        version.is_single_tile = 0;
        version.has_long_names = 0;
        version.has_non_image = 0;
        version.is_multipart = 0;
        fwrite(&version, sizeof(version), 1, fp);
        // write the header (a set of attributes)
        // write the attributes
        openexr::Attribute attribute;
        attribute.name = "channels";
        attribute.type = "chlist";
        // write the channels
        openexr::ChannelList channel;
        strncpy(channel.name, "R", sizeof(channel.name));
        channel.pixel_type = openexr::ChannelList::PixelType::Half;
        channel.pLinear = 1;
        channel.sampling.x = 1;
        channel.sampling.y = 1;
        attribute.size = static_cast<uint32_t>(channel.Size() * 3u) + 1u;
        attribute.Write(fp);
        channel.Write(fp);
        strncpy(channel.name, "G", sizeof(channel.name));
        channel.Write(fp);
        strncpy(channel.name, "B", sizeof(channel.name));
        channel.Write(fp);
        fwrite(&zero, sizeof(zero), 1, fp);  // end of the channel lists

        openexr::Compression compression = openexr::Compression::None;
        attribute.name = "compression";
        attribute.type = "compression";
        attribute.size = sizeof(compression);
        attribute.Write(fp);
        fwrite(&compression, sizeof(compression), 1, fp);

        openexr::Box2I dataWindow;
        dataWindow.min.x = 0;
        dataWindow.min.y = 0;
        dataWindow.max.x = static_cast<int>(width - 1);
        dataWindow.max.y = static_cast<int>(height - 1);
        attribute.name = "dataWindow";
        attribute.type = "box2i";
        attribute.size = sizeof(dataWindow);
        attribute.Write(fp);
        fwrite(&dataWindow, sizeof(dataWindow), 1, fp);

        openexr::Box2I displayWindow;
        displayWindow.min.x = 0;
        displayWindow.min.y = 0;
        displayWindow.max.x = static_cast<int>(width - 1);
        displayWindow.max.y = static_cast<int>(height - 1);
        attribute.name = "displayWindow";
        attribute.type = "box2i";
        attribute.size = sizeof(displayWindow);
        attribute.Write(fp);
        fwrite(&displayWindow, sizeof(displayWindow), 1, fp);

        openexr::LineOrder lineOrder = openexr::LineOrder::Increasing_Y;
        attribute.name = "lineOrder";
        attribute.type = "lineOrder";
        attribute.size = sizeof(lineOrder);
        attribute.Write(fp);
        fwrite(&lineOrder, sizeof(lineOrder), 1, fp);

        float pixelAspectRatio = 1.0f;
        attribute.name = "pixelAspectRatio";
        attribute.type = "float";
        attribute.size = sizeof(pixelAspectRatio);
        attribute.Write(fp);
        fwrite(&pixelAspectRatio, sizeof(pixelAspectRatio), 1, fp);

        openexr::Vector2_f screenWindowCenter;
        screenWindowCenter.x = 0.5f;
        screenWindowCenter.y = 0.5f;
        attribute.name = "screenWindowCenter";
        attribute.type = "v2f";
        attribute.size = sizeof(screenWindowCenter);
        attribute.Write(fp);
        fwrite(&screenWindowCenter, sizeof(screenWindowCenter), 1, fp);

        float screenWindowWidth = 1.0f;
        attribute.name = "screenWindowWidth";
        attribute.type = "float";
        attribute.size = sizeof(screenWindowWidth);
        attribute.Write(fp);
        fwrite(&screenWindowWidth, sizeof(screenWindowWidth), 1, fp);

        if (version.is_single_tile) {
            openexr::TileDescriptor tiledesc;
            tiledesc.x_size = 1;
            tiledesc.y_size = 1;
            tiledesc.level_mode = openexr::TileDescriptor::LevelMode::OneLevel;
            tiledesc.round_mode = openexr::TileDescriptor::RoundMode::RoundDown;
            attribute.name = "tiles";
            attribute.type = "tiledesc";
            attribute.size = static_cast<uint32_t>(tiledesc.Size());
            attribute.Write(fp);
            tiledesc.Write(fp);
        }

        fwrite(&zero, sizeof(zero), 1, fp);  // end of the header
        // get the current position of the file
        size_t header_end = static_cast<size_t>(ftell(fp));
        size_t scan_line_pixel_data_size = (width * pixel_size);
        size_t offset_table_size = (height * sizeof(std::uint64_t));
        // allocate the scan line offset table
        std::uint64_t* scan_line_offset_table = new std::uint64_t[height];  // it's an older code but it checks out
        // the start of the image data is after the scan line offset table
        size_t image_data_start = header_end + offset_table_size;
        // the size of each scan line is know already since we already know our channel list
        size_t scan_line_size = scan_line_pixel_data_size + sizeof(uint32_t) /* count */ + sizeof(uint32_t) /* size */;
        // fill in the scan line offset table
        for (size_t y = 0; y < height; y++) {
            scan_line_offset_table[y] = image_data_start + (y * scan_line_size);
        }
        // write the scan line offset table (32 bit offsets)
        fwrite(scan_line_offset_table, sizeof(std::uint64_t), height, fp);

        size_t here = static_cast<size_t>(ftell(fp));
        basal::exception::throw_unless(here == image_data_start, __func__, __LINE__,
                                       "File offset %zu but should be %zu\r\n", here, image_data_start);

        openexr::ScanLine scan_line;
        scan_line.number = 0;
        scan_line.data.reserve(scan_line_pixel_data_size);
        size_t column = 0;
        size_t row = 0;

        basal::exception::throw_unless(scan_line.data.capacity() == scan_line_pixel_data_size, __func__, __LINE__,
                                       "Scan line capacity %zu but should be %zu\r\n", scan_line.data.capacity(),
                                       scan_line_size);

        for_each([&](rgbh const& pixel) -> void {
            if (column == 0) {
                size_t offset = static_cast<size_t>(ftell(fp));
                basal::exception::throw_if(offset != scan_line_offset_table[row], __func__, __LINE__,
                                           "File offset %zu but should be %zu, width=%zu\r\n", offset,
                                           scan_line_offset_table[row], width);
                scan_line.data.resize(scan_line_pixel_data_size);
            }
            if (column < width) {
                // accumulate into the scan line
                uint8_t const* r = reinterpret_cast<uint8_t const*>(&pixel.components.r);
                uint8_t const* g = reinterpret_cast<uint8_t const*>(&pixel.components.g);
                uint8_t const* b = reinterpret_cast<uint8_t const*>(&pixel.components.b);
                size_t r_index = column * sizeof(basal::half);
                size_t g_index = r_index + (width * sizeof(basal::half));
                size_t b_index = g_index + (width * sizeof(basal::half));
                scan_line.data[r_index + 0] = r[0];
                scan_line.data[r_index + 1] = r[1];
                scan_line.data[g_index + 0] = g[0];
                scan_line.data[g_index + 1] = g[1];
                scan_line.data[b_index + 0] = b[0];
                scan_line.data[b_index + 1] = b[1];
                column++;
            }
            if (column == width) {
                scan_line.Write(fp);
                scan_line.number++;
                column = 0;
                row++;
            }
        });

        // free the memory
        delete[] scan_line_offset_table;
        scan_line_offset_table = nullptr;

        fclose(fp);
        return true;
    }
    return false;
}

template <>
bool image<PixelFormat::GREY8>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fprintf(fp, "P5\n");
        fprintf(fp, "%zu %zu\n", width, height);
        fprintf(fp, "%" PRIu32 "\n", uint32_t(1 << (8 * depth)) - 1);
        for_each([&](uint8_t const& pixel) -> void { fwrite(&pixel, sizeof(pixel), 1, fp); });
        fclose(fp);
        return true;
    }
    return false;
}

template <>
bool image<PixelFormat::Y8>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fprintf(fp, "P5\n");
        fprintf(fp, "%zu %zu\n", width, height);
        fprintf(fp, "%" PRIu32 "\n", uint32_t(1 << (8 * depth)) - 1);
        for_each([&](uint8_t const& pixel) -> void { fwrite(&pixel, sizeof(pixel), 1, fp); });
        fclose(fp);
        return true;
    }
    return false;
}

template <>
bool image<PixelFormat::Y16>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fprintf(fp, "P5\n");
        fprintf(fp, "%zu %zu\n", width, height);
        fprintf(fp, "%" PRIu32 "\n", uint32_t(1 << (8 * depth)) - 1);
        for_each([&](uint16_t const& pixel) -> void { fwrite(&pixel, sizeof(pixel), 1, fp); });
        fclose(fp);
        return true;
    }
    return false;
}

template <>
bool image<PixelFormat::RGBP>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fprintf(fp, "P565\n");
        fprintf(fp, "%zu %zu\n", width, height);
        fprintf(fp, "%" PRIu32 "\n", uint32_t(1 << (8 * depth)) - 1);
        for_each([&](rgb565 const& pixel) -> void { fwrite(&pixel, sizeof(pixel), 1, fp); });
        fclose(fp);
        return true;
    }
    return false;
}

template <>
bool image<PixelFormat::Y32>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fprintf(fp, "P5\n");
        fprintf(fp, "%zu %zu\n", width, height);
        fprintf(fp, "%" PRIu32 "\n", uint32_t(1 << (8 * depth)) - 1);
        for_each([&](uint32_t const& pixel) -> void { fwrite(&pixel, sizeof(pixel), 1, fp); });
        fclose(fp);
        return true;
    }
    return false;
}

fourcc::image<fourcc::PixelFormat::RGB8> convert_rgbid_to_rgb8(fourcc::image<fourcc::PixelFormat::RGBId> const& in) {
    fourcc::image<fourcc::PixelFormat::RGB8> out(in.height, in.width);
    for (size_t y = 0; y < in.height; y++) {
        for (size_t x = 0; x < in.width; x++) {
            auto const& rgb = in.at(y, x);
            // create a pixel template type
            fourcc::pixel<PixelFormat::RGBId> rgbp{rgb};
            // clamp the value
            rgbp.clamp();
            // NOW FINALLY convert to sRGB space
            rgbp.ToEncoding(fourcc::Encoding::GammaCorrected);
            // save the pixel
            out.at(y, x) = rgbp.to_<PixelFormat::RGB8>();
        }
    }
    return out;
}

namespace gamma {
color interpolate(color const& x, color const& y, precision a) {
    return color{gamma::interpolate(x.red(), y.red(), a), gamma::interpolate(x.green(), y.green(), a),
                 gamma::interpolate(x.blue(), y.blue(), a)};
}
}  // namespace gamma

namespace linear {
color interpolate(color const& x, color const& y, precision a) {
    return color{linear::interpolate(x.red(), y.red(), a), linear::interpolate(x.green(), y.green(), a),
                 linear::interpolate(x.blue(), y.blue(), a)};
}
}  // namespace linear
}  // namespace fourcc
