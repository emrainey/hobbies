/// @file
/// Implementations for image conversions
/// @copyright Copyright 2026 (C) Erik Rainey.

#include <cmath>

#include "fourcc/pairs.hpp"

namespace fourcc {

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
    return std::make_pair(0, 0);
}

}  // namespace fourcc
