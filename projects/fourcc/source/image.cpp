#include "fourcc/image.hpp"

#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <filesystem>

#include "fourcc/targa.hpp"

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
                fwrite(&pixel.r, 1, sizeof(pixel.r), fp);
                fwrite(&pixel.g, 1, sizeof(pixel.g), fp);
                fwrite(&pixel.b, 1, sizeof(pixel.b), fp);
            });
            fclose(fp);
            return true;
        }
    } else if (is_extension(filename, ".bgr")) {
        FILE* fp = fopen(filename.c_str(), "wb");
        if (fp) {
            for_each ([&](fourcc::rgba const& pixel) -> void {
                fwrite(&pixel.b, 1, sizeof(pixel.b), fp);
                fwrite(&pixel.g, 1, sizeof(pixel.g), fp);
                fwrite(&pixel.r, 1, sizeof(pixel.r), fp);
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
                fwrite(&pixel.r, 1, sizeof(pixel.r), fp);
                fwrite(&pixel.g, 1, sizeof(pixel.g), fp);
                fwrite(&pixel.b, 1, sizeof(pixel.b), fp);
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
                    fwrite(&pixel.b, 1, sizeof(pixel.b), fp);
                    fwrite(&pixel.g, 1, sizeof(pixel.g), fp);
                    fwrite(&pixel.r, 1, sizeof(pixel.r), fp);
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
            fwrite(&pixel.b, 1, sizeof(uint8_t), fp);
            fwrite(&pixel.g, 1, sizeof(uint8_t), fp);
            fwrite(&pixel.r, 1, sizeof(uint8_t), fp);
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
                fwrite(&pixel, 1, sizeof(rgb8), fp);
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
                    fwrite(&pixel.b, 1, sizeof(pixel.b), fp);
                    fwrite(&pixel.g, 1, sizeof(pixel.g), fp);
                    fwrite(&pixel.r, 1, sizeof(pixel.r), fp);
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
            fwrite(&pixel, 1, sizeof(bgr8), fp);
        });
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
            fwrite(&pixel, 1, sizeof(pixel), fp);
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
            fwrite(&pixel, 1, sizeof(pixel), fp);
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
            fwrite(&pixel, 1, sizeof(pixel), fp);
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
            fwrite(&pixel, 1, sizeof(pixel), fp);
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
            fwrite(&pixel, 1, sizeof(pixel), fp);
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
