
#include <cstdio>
#include <filesystem>

#error This is not supported yet. Do not include.

namespace targa {

enum class ColorMapType : uint8_t {
    None = 0,
    Present = 1,
};

enum class ImageMapType : uint8_t {
    None = 0,
    UncompressedColorMapped = 1,
    UncompressedTrueColor = 2,
    UncompressedBlackAndWhite = 3,
    RLEColorMapImage = 9,
    RLETrueColorImage = 10,
    RLEBlackAndWhite = 11,
};

struct __attribute__((packed)) header {
    constexpr header() 
        : length(sizeof(header))
        , color_map_type(ColorMapType::None)
        , image_map_type(ImageMapType::UncompressedTrueColor)
        , color_map()
        , image_map()
        {}
    const uint8_t length;
    ColorMapType color_map_type;
    ImageMapType image_map_type;
    struct ColorMapSpecification {
        uint16_t first_index;
        uint16_t length;
        uint8_t  entry_size;
    } color_map;
    struct ImageSpecification {
        uint16_t x_origin;
        uint16_t y_origin;
        uint16_t image_width;
        uint16_t image_height;
        uint8_t  pixel_depth;
        uint8_t  image_descriptor;
    } image_map;
};

static_assert(sizeof(header) == 18, "Must be 18 bytes");

}
