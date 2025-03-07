#pragma once
#include <cstdio>
#include <cstdint>
#include <filesystem>

/// A namespace to hold the defined structures of the OpenEXR format
/// @see https://openexr.com/en/latest/OpenEXRFileLayout.html
namespace openexr {

static constexpr std::uint32_t magic = 20000630lu;  // 0x76, 0x2f, 0x31, 0x01

struct Version {
    std::uint32_t version : 8;         ///< bits 0-7
    std::uint32_t : 1;                 ///< bit 8
    std::uint32_t is_single_tile : 1;  ///< bit 9, is a single tiled imaged
    std::uint32_t has_long_names : 1;  ///< bit 10
    std::uint32_t has_non_image : 1;   ///< bit 11
    std::uint32_t is_multipart : 1;    ///< bit 12
    std::uint32_t : 19;                ///< bits 13-31
};

class Serializable {
public:
    virtual std::size_t Size() const = 0;
    virtual void Write(FILE* fp) = 0;
    virtual void Read(FILE* fp) = 0;
};

struct Attribute : public Serializable {
    std::string name;    // written as up to 32 bytes of null-terminated string
    std::string type;    // written as up to 32 bytes of null-terminated string
    std::uint32_t size;  // size of the attribute value

    std::size_t Size() const override {
        return name.size() + 1 + type.size() + 1 + sizeof(size);
    }

    void Write(FILE* fp) override {
        fwrite(name.c_str(), sizeof(char), name.size() + 1, fp);
        fwrite(type.c_str(), sizeof(char), type.size() + 1, fp);
        fwrite(&size, sizeof(size), 1, fp);
    }

    void Read(FILE* fp) override {
        char c;
        do {
            fread(&c, sizeof(char), 1, fp);
            name.push_back(c);
        } while (c != '\0');
        do {
            fread(&c, sizeof(char), 1, fp);
            type.push_back(c);
        } while (c != '\0');
        fread(&size, sizeof(size), 1, fp);
    }
};

char const* const known_attribute_types[] = {"box2i",       "box2f",
                                             "chlist",      "chromaticities",
                                             "compression", "compressionLevel",
                                             "double",      "envmap",
                                             "float",       "int",
                                             "keycode",     "lineOrder",
                                             "m33f",        "m44f",
                                             "preview",     "rational",
                                             "string",      "stringvector",
                                             "tiledesc",    "timecode",
                                             "v2f",         "v2i",
                                             "v3f",         "v3i"};

char const* const required_attribute_names[]
    = {"channels",  "compression",      "dataWindow",         "displayWindow",
       "lineOrder", "pixelAspectRatio", "screenWindowCenter", "screenWindowWidth"};

template <typename STORAGE_TYPE>
struct Point_ {
    STORAGE_TYPE x;
    STORAGE_TYPE y;
};

using Point2I = Point_<std::int32_t>;
using Point2F = Point_<float>;

template <typename STORAGE_TYPE>
struct Vector2_ {
    STORAGE_TYPE x;
    STORAGE_TYPE y;
};

template <typename STORAGE_TYPE>
struct Vector3_ {
    STORAGE_TYPE x;
    STORAGE_TYPE y;
    STORAGE_TYPE z;
};

// Use "vec2i"
using Vector2_i = Vector2_<std::int32_t>;
// Use "vec2f"
using Vector2_f = Vector2_<float>;
// Use "vec3i"
using Vector3_i = Vector3_<std::int32_t>;
// Use "vec3f"
using Vector3_f = Vector3_<float>;

template <typename STORAGE_TYPE>
struct Window_ {
    STORAGE_TYPE width;
    STORAGE_TYPE height;
};

// Use "rational"
struct Rational {
    std::int32_t numerator;
    std::uint32_t denominator;
};

// Use "timeCode"
struct TimeCode {
    std::uint32_t frame_number : 6;  // bits 0-5
    std::uint32_t seconds : 9;       // bits 6-14
    std::uint32_t minutes : 8;       // bits 15-22
    std::uint32_t hours : 7;         // bits 23-29
    std::uint32_t drop : 1;          // bit 30
    std::uint32_t color : 1;         // bit 31
    std::uint32_t user_data;
};

// Use "string"
struct String {
    std::vector<char> data;
    // written as the following
    // std::uint32_t length;
    // char const data[]; // gcc C99 extension
};

template <typename STORAGE_TYPE>
struct Box_ {
    Point_<STORAGE_TYPE> min;
    Point_<STORAGE_TYPE> max;
};

// Use "box2i"
using Box2I = Box_<std::int32_t>;
// Use "box2f"
using Box2F = Box_<float>;

struct Pixel {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;
};

// Use "chlist"
struct ChannelList : public Serializable {
    char name[256]{};  // written as a null-terminated string not 32 bytes
    enum class PixelType : std::uint32_t {
        Uint = 0,
        Half = 1,
        Float = 2
    } pixel_type{PixelType::Float};
    std::uint8_t pLinear{1};  // 0 or 1 (used as bool)
    std::uint8_t _reserved[3]{};
    Point2I sampling{1, 1};

    inline size_t Size() const override {
        return strlen(name) + 1u + sizeof(pixel_type) + sizeof(pLinear) + sizeof(_reserved) + sizeof(sampling);
    };

    inline void Write(FILE* fp) override {
        fwrite(name, sizeof(char), strlen(name) + 1, fp);
        fwrite(&pixel_type, sizeof(pixel_type), 1, fp);
        fwrite(&pLinear, sizeof(pLinear), 1, fp);
        fwrite(_reserved, sizeof(_reserved), 1, fp);
        fwrite(&sampling, sizeof(sampling), 1, fp);
    }

    inline void Read(FILE* fp) override {
        for (size_t i = 0; i < sizeof(name); i++) {
            fread(&name[i], sizeof(char), 1, fp);
            if (name[i] == '\0') {
                break;
            }
        }
        fread(&pixel_type, sizeof(pixel_type), 1, fp);
        fread(&pLinear, sizeof(pLinear), 1, fp);
        fread(_reserved, sizeof(_reserved), 1, fp);
        fread(&sampling, sizeof(sampling), 1, fp);
    }
};

// Use "compression"
enum class Compression : uint8_t {
    None = 0,
    RLE = 1,
    ZIPS = 2,
    ZIP = 3,
    PIZ = 4,
    PXR24 = 5,
    B44 = 6,
    B44A = 7,
    DWAA = 8,
    DWAB = 9
};

// Use "chromaticities"
struct Chromaticities {
    Point2F red;
    Point2F green;
    Point2F blue;
    Point2F white;
};

// Use "envmap"
enum class EnvironmentMap : std::uint8_t {
    LatLong = 0,
    Cube = 1
};

// Use "keycode"
struct KeyCode {
    std::int32_t film_manufacturer_code;
    std::int32_t film_type;
    std::int32_t prefix;
    std::int32_t count;
    std::int32_t perf_offset;
    std::int32_t perfs_per_frame;
    std::int32_t perfs_per_count;
};

// Use "lineOrder"
enum class LineOrder : std::uint8_t {
    Increasing_Y = 0,
    Decreasing_Y = 1,
    Random_Y = 2
};

// Use "m33f"
using Matrix3x3f = float[3][3];

// Use "m44f"
using Matrix4x4f = float[4][4];

// Use "preview"
struct Preview {
    Window_<std::uint32_t> window;
    std::vector<std::uint8_t> data;  ///< 4 x width x height (which seems excessive...)
    // written as the following...
    // Pixel pixel[];
};

// Use "tiledesc"
struct TileDescriptor : public Serializable {
    enum class LevelMode : std::uint8_t {
        OneLevel = 0,
        MipmapLevels = 1,
        RipmapLevels = 2,
    };

    enum class RoundMode : std::uint8_t {
        RoundDown = 0,
        RoundUp = 1,
    };

    std::uint32_t x_size;
    std::uint32_t y_size;
    LevelMode level_mode : 4;
    RoundMode round_mode : 4;

    inline size_t Size() const override {
        return sizeof(x_size) + sizeof(y_size) + sizeof(std::uint8_t);
    }

    inline void Write(FILE* fp) override {
        fwrite(&x_size, sizeof(x_size), 1, fp);
        fwrite(&y_size, sizeof(y_size), 1, fp);
        std::uint8_t mode = static_cast<std::uint8_t>(level_mode) | (static_cast<std::uint8_t>(round_mode) << 4);
        fwrite(&mode, sizeof(mode), 1, fp);
    }

    inline void Read(FILE* fp) override {
        fread(&x_size, sizeof(x_size), 1, fp);
        fread(&y_size, sizeof(y_size), 1, fp);
        std::uint8_t mode;
        fread(&mode, sizeof(mode), 1, fp);
        level_mode = static_cast<LevelMode>(mode & 0x0F);
        round_mode = static_cast<RoundMode>((mode & 0xF0) >> 4);
    }
};  // 9 bytes

struct ScanLine : public Serializable {
    std::uint32_t number;
    std::vector<std::uint8_t> data;  ///< Pixel data is listed in the channel order specified in the header.
    // written as the following...
    // std::uint32_t pixel_data_size;  ///< sum of all pixel data across all channels on this row.
    // std::uint8_t data[]; // gcc extension

    inline size_t Size() const override {
        return sizeof(number) + sizeof(std::uint32_t) + data.size();
    }

    inline void Write(FILE* fp) override {
        fwrite(&number, sizeof(number), 1, fp);
        std::uint32_t pixel_data_size = data.size();
        fwrite(&pixel_data_size, sizeof(pixel_data_size), 1, fp);
        fwrite(data.data(), sizeof(std::uint8_t), data.size(), fp);
        // std::cout << "Wrote " << data.size() << " bytes of pixel data" << std::endl;
    }

    inline void Read(FILE* fp) override {
        fread(&number, sizeof(number), 1, fp);
        std::uint32_t pixel_data_size;
        fread(&pixel_data_size, sizeof(pixel_data_size), 1, fp);
        data.resize(pixel_data_size);
        fread(data.data(), sizeof(std::uint8_t), pixel_data_size, fp);
    }
};

}  // namespace openexr