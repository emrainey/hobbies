#pragma once

#include <isa/types.hpp>

namespace isa {

namespace memory {

enum class Type : uint8_t {
    Unmapped = 0,
    ROM,
    RAM,
    Peripheral,
};

struct Access {
    uint8_t read : 1;
    uint8_t write : 1;
    uint8_t execute : 1;
    uint8_t privileged : 1;
    uint8_t cachable : 1;
    uint8_t bufferable : 1;  ///< Write Bufferable
    uint8_t : 2;

    constexpr Access(uint8_t r = 0, uint8_t w = 0, uint8_t x = 0, uint8_t p = 0, uint8_t c = 0, uint8_t b = 0)
        : read{r}, write{w}, execute{x}, privileged{p}, cachable{c}, bufferable{b} {
    }

    constexpr bool operator==(const Access& other) const {
        return (read == other.read) && (write == other.write) && (execute == other.execute)
               && (privileged == other.privileged) && (cachable == other.cachable) && (bufferable == other.bufferable);
    }

    constexpr bool operator!=(const Access& other) const {
        return not(*this == other);
    }

    constexpr Access operator|(const Access& other) const {
        return Access{
            static_cast<uint8_t>(read | other.read),         static_cast<uint8_t>(write | other.write),
            static_cast<uint8_t>(execute | other.execute),   static_cast<uint8_t>(privileged | other.privileged),
            static_cast<uint8_t>(cachable | other.cachable), static_cast<uint8_t>(bufferable | other.bufferable),
        };
    }

    constexpr bool Has(Access& other) const {
        return ((*this | other) == *this);
    }
};

namespace attributes {
static constexpr Access None{0, 0, 0, 0, 0, 0};
static constexpr Access Readable{1, 0, 0, 0, 0, 0};
static constexpr Access Writeable{0, 1, 0, 0, 0, 0};
static constexpr Access Executable{0, 0, 1, 0, 0, 0};
static constexpr Access Protected{0, 0, 0, 1, 0, 0};
static constexpr Access Cacheable{0, 0, 0, 0, 1, 0};
static constexpr Access Bufferable{0, 0, 0, 0, 0, 1};
}  // namespace attributes

static constexpr Access ExecuteRead = attributes::Executable | attributes::Readable | attributes::Cacheable;
static constexpr Access ExecReadWrite = attributes::Executable | attributes::Readable | attributes::Writeable
                                        | attributes::Cacheable | attributes::Bufferable;
// static constexpr Access

struct Mapping {
    constexpr Mapping(Range r, Type t) : range{r}, type{t} {
    }
    Range range;
    Type type;
    Access access;

    constexpr Mapping(Range r, Type t, Access a) : range{r}, type{t}, access{a} {
    }
};

constexpr static Address NullRegion{
    65536};  /// The space of unmapped address to catch null pointer derefs of structures, etc.

constexpr static Mapping Map[] = {
    // Unmapped (bottom is intentionally unmapped to allow null pointer hard faults)
    Mapping{Range{0x00000000, NullRegion - 1U}, Type::Unmapped, attributes::None},
    Mapping{Range{NullRegion, 0x0FFFFFFF}, Type::ROM, ExecuteRead},            // ROM
    Mapping{Range{0x10000000, 0x1FFFFFFF}, Type::RAM, ExecReadWrite},          // RAM
    Mapping{Range{0x20000000, 0x3FFFFFFF}, Type::Unmapped, attributes::None},  // Unmapped
    Mapping{Range{0x40000000, 0x5FFFFFFF}, Type::Peripheral,
            attributes::Readable | attributes::Writeable},                     // Peripherals (512MB)
    Mapping{Range{0x60000000, 0x7FFFFFFF}, Type::Unmapped, attributes::None},  // Unmapped
    Mapping{Range{0x80000000, 0x9FFFFFFF}, Type::RAM, ExecReadWrite},          // RAM
    Mapping{Range{0xA0000000, 0xEFFFFFFF}, Type::Unmapped, attributes::None},  // Unmapped
    Mapping{Range{0xF0000000, 0xFFFFFFFF}, Type::ROM,
            attributes::Readable | attributes::Writeable | attributes::Protected},  // Protected CPU Registers
};

}  // namespace memory

}  // namespace isa