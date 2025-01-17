#include "fourcc/image.hpp"

#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <filesystem>

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

template <>
bool image<rgba, pixel_format::RGBA>::save(std::string filename) const {
    if (is_extension(filename, ".rgb")) {
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp) {
            for_each ([&](fourcc::rgba const& pixel) -> void {
                fwrite(&pixel.r, sizeof(pixel.r), 1, fp);
                fwrite(&pixel.g, sizeof(pixel.g), 1, fp);
                fwrite(&pixel.b, sizeof(pixel.b), 1, fp);
            });
            fclose(fp);
            return true;
        }
    } else if (is_extension(filename, ".bgr")) {
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp) {
            for_each ([&](fourcc::rgba const& pixel) -> void {
                fwrite(&pixel.b, sizeof(pixel.b), 1, fp);
                fwrite(&pixel.g, sizeof(pixel.g), 1, fp);
                fwrite(&pixel.r, sizeof(pixel.r), 1, fp);
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
            for_each ([&](rgba const& pixel) -> void {
                fwrite(&pixel.r, sizeof(pixel.r), 1, fp);
                fwrite(&pixel.g, sizeof(pixel.g), 1, fp);
                fwrite(&pixel.b, sizeof(pixel.b), 1, fp);
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
            hdr.image_map.image_width = width;
            hdr.image_map.image_height = height;
            hdr.image_map.pixel_depth = depth * 8;
            hdr.image_map.image_descriptor = 0;  // avoid complex interleaving
            fwrite(&hdr, sizeof(hdr), 1, fp);
            // identification string
            // color map
            // image data as B. then G, then R. but y is inverted
            for (size_t y = (height - 1); /* y >= 0 */; y--) {
                for (size_t x = 0; x < width; x++) {
                    rgba const& pixel = at(y, x);
                    fwrite(&pixel.b, sizeof(pixel.b), 1, fp);
                    fwrite(&pixel.g, sizeof(pixel.g), 1, fp);
                    fwrite(&pixel.r, sizeof(pixel.r), 1, fp);
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
bool image<abgr, pixel_format::ABGR>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fprintf(fp, "P7\n");
        fprintf(fp, "WIDTH %zu\nHEIGHT %zu\nDEPTH %zu\n", width, height, depth);
        fprintf(fp, "MAXVAL %u\n", 255u);
        fprintf(fp, "TUPLTYPE %s\n", channel_order(format));
        fprintf(fp, "ENDHDR\n");
        for_each ([&](abgr const& pixel) -> void {
            fwrite(&pixel.b, sizeof(uint8_t), 1, fp);
            fwrite(&pixel.g, sizeof(uint8_t), 1, fp);
            fwrite(&pixel.r, sizeof(uint8_t), 1, fp);
        });
        fclose(fp);
        return true;
    }
    return false;
}

template <>
bool image<rgb8, pixel_format::RGB8>::save(std::string filename) const {
    if (is_extension(filename, ".ppm")) {
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp) {
            fprintf(fp, "P7\n");
            fprintf(fp, "WIDTH %zu\nHEIGHT %zu\nDEPTH %zu\n", width, height, depth);
            fprintf(fp, "MAXVAL %u\n", 255u);
            fprintf(fp, "TUPLTYPE %s\n", channel_order(format));
            fprintf(fp, "ENDHDR\n");
            for_each ([&](rgb8 const& pixel) -> void {
                fwrite(&pixel, sizeof(rgb8), 1, fp);
            });
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
            hdr.image_map.image_width = width;
            hdr.image_map.image_height = height;
            hdr.image_map.pixel_depth = depth * 8;
            hdr.image_map.image_descriptor = 0;  // avoid complex interleaving
            fwrite(&hdr, sizeof(hdr), 1, fp);
            // identification string
            // color map
            // image data as B, then G, then R. but upside down where y is inverted
            for (size_t y = (height - 1); /* y >= 0 */; y--) {
                for (size_t x = 0; x < width; x++) {
                    rgb8 const& pixel = at(y, x);
                    fwrite(&pixel.b, sizeof(pixel.b), 1, fp);
                    fwrite(&pixel.g, sizeof(pixel.g), 1, fp);
                    fwrite(&pixel.r, sizeof(pixel.r), 1, fp);
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
bool image<bgr8, pixel_format::BGR8>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fprintf(fp, "P7\n");
        fprintf(fp, "WIDTH %zu\nHEIGHT %zu\nDEPTH %zu\n", width, height, depth);
        fprintf(fp, "MAXVAL %u\n", 255u);
        fprintf(fp, "TUPLTYPE %s\n", channel_order(format));
        fprintf(fp, "ENDHDR\n");
        for_each([&](bgr8 const& pixel) -> void {
            fwrite(&pixel, sizeof(bgr8), 1, fp);
        });
        fclose(fp);
        return true;
    }
    return false;
}

template <>
bool image<rgbh, pixel_format::RGBh>::save(std::string filename) const {
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
        attribute.size = (channel.Size() * 3u) + 1u;
        attribute.Write(fp);
        channel.Write(fp);
        strncpy(channel.name, "G", sizeof(channel.name));
        channel.Write(fp);
        strncpy(channel.name, "B", sizeof(channel.name));
        channel.Write(fp);
        fwrite(&zero, sizeof(zero), 1, fp); // end of the channel lists

        openexr::Compression compression = openexr::Compression::None;
        attribute.name = "compression";
        attribute.type = "compression";
        attribute.size = sizeof(compression);
        attribute.Write(fp);
        fwrite(&compression, sizeof(compression), 1, fp);

        openexr::Box2I dataWindow;
        dataWindow.min.x = 0;
        dataWindow.min.y = 0;
        dataWindow.max.x = width - 1;
        dataWindow.max.y = height - 1;
        attribute.name = "dataWindow";
        attribute.type = "box2i";
        attribute.size = sizeof(dataWindow);
        attribute.Write(fp);
        fwrite(&dataWindow, sizeof(dataWindow), 1, fp);

        openexr::Box2I displayWindow;
        displayWindow.min.x = 0;
        displayWindow.min.y = 0;
        displayWindow.max.x = width - 1;
        displayWindow.max.y = height - 1;
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
            attribute.size = tiledesc.Size();
            attribute.Write(fp);
            tiledesc.Write(fp);
        }

        fwrite(&zero, sizeof(zero), 1, fp); // end of the header
        // get the current position of the file
        size_t header_end = ftell(fp);
        size_t scan_line_pixel_data_size = (width * pixel_size);
        size_t offset_table_size = (height * sizeof(std::uint64_t));
        // allocate the scan line offset table
        std::uint64_t* scan_line_offset_table = new std::uint64_t[height]; // it's an older code but it checks out
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

        size_t here = ftell(fp);
        basal::exception::throw_unless(here == image_data_start, __func__, __LINE__, "File offset %zu but should be %zu\r\n", here, image_data_start);

        openexr::ScanLine scan_line;
        scan_line.number = 0;
        scan_line.data.reserve(scan_line_pixel_data_size);
        size_t column = 0;
        size_t row = 0;

        basal::exception::throw_unless(scan_line.data.capacity() == scan_line_pixel_data_size, __func__, __LINE__, "Scan line capacity %zu but should be %zu\r\n", scan_line.data.capacity(), scan_line_size);

        for_each([&](rgbh const& pixel) -> void {
            if (column == 0) {
                size_t offset = ftell(fp);
                basal::exception::throw_if(offset != scan_line_offset_table[row], __func__, __LINE__, "File offset %zu but should be %zu, width=%zu\r\n", offset, scan_line_offset_table[row], width);
                scan_line.data.resize(scan_line_pixel_data_size);
            }
            if (column < width) {
                // accumulate into the scan line
                uint8_t const* r = reinterpret_cast<uint8_t const*>(&pixel.r);
                uint8_t const* g = reinterpret_cast<uint8_t const*>(&pixel.g);
                uint8_t const* b = reinterpret_cast<uint8_t const*>(&pixel.b);
                size_t r_index = column * sizeof(basal::half);
                size_t g_index = r_index + (width * sizeof(basal::half));
                size_t b_index = g_index + (width * sizeof(basal::half));
                scan_line.data[r_index+0] = r[0];
                scan_line.data[r_index+1] = r[1];
                scan_line.data[g_index+0] = g[0];
                scan_line.data[g_index+1] = g[1];
                scan_line.data[b_index+0] = b[0];
                scan_line.data[b_index+1] = b[1];
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
bool image<uint8_t, pixel_format::GREY8>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fprintf(fp, "P5\n");
        fprintf(fp, "%zu %zu\n", width, height);
        fprintf(fp, "%" PRIu32 "\n", uint32_t(1 << (8 * depth)) - 1);
        for_each([&](uint8_t const& pixel) -> void {
            fwrite(&pixel, sizeof(pixel), 1, fp);
        });
        fclose(fp);
        return true;
    }
    return false;
}

template <>
bool image<uint8_t, pixel_format::Y8>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fprintf(fp, "P5\n");
        fprintf(fp, "%zu %zu\n", width, height);
        fprintf(fp, "%" PRIu32 "\n", uint32_t(1 << (8 * depth)) - 1);
        for_each([&](uint8_t const& pixel) -> void {
            fwrite(&pixel, sizeof(pixel), 1, fp);
        });
        fclose(fp);
        return true;
    }
    return false;
}

template <>
bool image<uint16_t, pixel_format::Y16>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fprintf(fp, "P5\n");
        fprintf(fp, "%zu %zu\n", width, height);
        fprintf(fp, "%" PRIu32 "\n", uint32_t(1 << (8 * depth)) - 1);
        for_each([&](uint16_t const& pixel) -> void {
            fwrite(&pixel, sizeof(pixel), 1, fp);
        });
        fclose(fp);
        return true;
    }
    return false;
}

template <>
bool image<rgb565, pixel_format::RGBP>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fprintf(fp, "P565\n");
        fprintf(fp, "%zu %zu\n", width, height);
        fprintf(fp, "%" PRIu32 "\n", uint32_t(1 << (8 * depth)) - 1);
        for_each([&](rgb565 const& pixel) -> void {
            fwrite(&pixel, sizeof(pixel), 1, fp);
        });
        fclose(fp);
        return true;
    }
    return false;
}

template <>
bool image<uint32_t, pixel_format::Y32>::save(std::string filename) const {
    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp) {
        fprintf(fp, "P5\n");
        fprintf(fp, "%zu %zu\n", width, height);
        fprintf(fp, "%" PRIu32 "\n", uint32_t(1 << (8 * depth)) - 1);
        for_each([&](uint32_t const& pixel) -> void {
            fwrite(&pixel, sizeof(pixel), 1, fp);
        });
        fclose(fp);
        return true;
    }
    return false;
}

std::pair<size_t, size_t> dimensions(std::string type) {
    if (type == std::string("SQCIF")) {
        return std::make_pair(128, 96);
    } else if (type == std::string("QCIF")) {
        return std::make_pair(176, 144);
    } else if (type == std::string("CIF")) {
        return std::make_pair(352, 288);
    } else if (type == std::string("CGA")) {
        return std::make_pair(320, 200);
    } else if (type == std::string("QVGA")) {
        return std::make_pair(320, 240);
    } else if (type == std::string("VGA")) {
        return std::make_pair(640, 480);
    } else if (type == std::string("WVGA")) {
        return std::make_pair(800, 480);
    } else if (type == std::string("XGA")) {
        return std::make_pair(1024, 768);
    } else if (type == std::string("SXGA")) {
        return std::make_pair(1280, 1024);
    } else if (type == std::string("UXGA")) {
        return std::make_pair(1600, 1200);
    } else if (type == std::string("QXGA")) {
        return std::make_pair(2048, 1536);
    } else if (type == std::string("WXGA")) {
        return std::make_pair(1280, 768);
    } else if (type == std::string("WSXGA+")) {
        return std::make_pair(1680, 1050);
    } else if (type == std::string("HD720")) {
        return std::make_pair(1280, 720);
    } else if (type == std::string("HD1080")) {
        return std::make_pair(1920, 1080);
    } else if (type == std::string("2K")) {
        return std::make_pair(2048, 1080);
    } else if (type == std::string("UWFHD")) {
        return std::make_pair(2560, 1080);
    } else if (type == std::string("WQHD")) {
        return std::make_pair(2560, 1440);
    } else if (type == std::string("UWQHD")) {
        return std::make_pair(3440, 1440);
    } else if (type == std::string("4KUHD")) {
        return std::make_pair(3840, 2160);
    } else if (type == std::string("5KUHD")) {
        return std::make_pair(5120, 2880);
    }
    return std::make_pair(0,0);
}

}  // namespace fourcc
