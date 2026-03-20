#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace isa {

constexpr static size_t CountOfRegisters = 16;
constexpr static size_t CountOfDataBits = 32;
constexpr static size_t CountOfUnitsPerCacheLine = 16;

/// 32 bit address representation for the ISA
/// In order to keep the notion of a 32 bit pointer separated from the
/// platform's notion of a pointer, we use a struct wrapper around a
/// uint32_t to represent addresses in the ISA.
struct Address {
    using StorageType = std::enable_if_t<CountOfDataBits == 32, uint32_t>;

    /// The storage of addresses
    StorageType value{0};

    /// Truncating Constructor!
    /// @warning This constructor will truncate the input value to 32 bits, which may cause loss of information if the
    /// input value exceeds 32 bits. Use with caution!
    constexpr Address(unsigned long long int v) : value(static_cast<StorageType>(v)) {
    }

    constexpr StorageType operator()() const {
        return value;
    }

    constexpr explicit operator StorageType() const {
        return value;
    }

    constexpr explicit operator uint64_t() const {
        // upcasting is safe and does not lose precision.
        return static_cast<uint64_t>(value);
    }

    constexpr explicit operator size_t() const {
        // this may upcast to a 64 bit value on 64 bit platforms, but
        // that's fine since it's only used for calculations and comparisons, not for actual memory access.
        return static_cast<size_t>(value);
    }

    constexpr Address& operator=(size_t v) {
        value = static_cast<StorageType>(v);
        return *this;
    }

    constexpr Address& operator=(StorageType v) {
        value = v;
        return *this;
    }

    constexpr bool operator==(Address const& other) const {
        return value == other.value;
    }

    constexpr bool operator!=(Address const& other) const {
        return not operator==(other);
    }

    constexpr bool operator<(Address const& other) const {
        return value < other.value;
    }

    constexpr bool operator<=(Address const& other) const {
        return value <= other.value;
    }

    constexpr bool operator>(Address const& other) const {
        return value > other.value;
    }

    constexpr bool operator>=(Address const& other) const {
        return value >= other.value;
    }

    constexpr Address operator+(Address const& other) const {
        return Address{static_cast<StorageType>(value + other.value)};
    }

    constexpr Address operator-(Address const& other) const {
        return Address{static_cast<StorageType>(value - other.value)};
    }

#define INTEGER_OPERATOR(op)                                                           \
    template <typename INTEGER>                                                        \
    constexpr Address operator op(INTEGER offset) const {                              \
        static_assert(std::is_integral_v<INTEGER>, "Offset must be an integral type"); \
        return Address{static_cast<StorageType>(value op offset)};                     \
    }                                                                                  \
    template <typename INTEGER>                                                        \
    constexpr Address& operator op## = (INTEGER offset) {                              \
        static_assert(std::is_integral_v<INTEGER>, "Offset must be an integral type"); \
        value op## = static_cast<StorageType>(offset);                                 \
        return *this;                                                                  \
    }

    INTEGER_OPERATOR(+)
    INTEGER_OPERATOR(-)

#undef INTEGER_OPERATOR

// Bitwise operators for Address
#define SELF_OPERATOR(op)                                               \
    constexpr Address operator op(Address const& other) const {         \
        return Address{static_cast<StorageType>(value op other.value)}; \
    }                                                                   \
    constexpr Address& operator op## = (Address const& other) {         \
        value op## = static_cast<StorageType>(other.value);             \
        return *this;                                                   \
    }

    SELF_OPERATOR(&)
    SELF_OPERATOR(|)
    SELF_OPERATOR(^)
#undef SELF_OPERATOR
    //=================================================================================

    friend std::ostream& operator<<(std::ostream& os, Address const& address) {
        os << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(sizeof(isa::Address) * 2U)
           << static_cast<isa::Address::StorageType>(address.value);
        return os;
    }
};
static_assert(sizeof(Address) == 4, "Address type must be 32 bits to support 32-bit address space");

/// Used to store comparison results, arithmetic flags, and precision flags for instructions to be used by conditional
/// moves and other instructions that depend on the results of previous instructions.
struct Evaluation {
    constexpr Evaluation()
        : comparison{0}
        , arithmetic{0}
        , precision{0}
        , less_than{0}
        , equal{0}
        , greater_than{0}
        , not_equal{0}
        , positive{0}
        , negative{0}
        , zero{0}
        , non_zero{0}
        , overflow{0}
        , saturated{0}
        , undefined{0}
        , inexact{0}
        , rounded{0}
        , subnormal{0}
        , underflow{0} {
    }
    // Type Bits
    uint32_t comparison : 1;  ///< 1 when a comparison was made
    uint32_t arithmetic : 1;  ///< 1 when an arithmetic evaluation is made
    uint32_t precision : 1;   ///< 1 when a precision evaluation is made
    uint32_t : 5;
    // Comparison flags for comparison results
    // When >= both greater_than and equal will be set (in addition to others)
    // When <= both less_than and equal will be set (in addition to others)
    // When the values are == equal will be set (in addition to others)
    // When the values are != not_equal will be set (in addition to others)
    uint32_t less_than : 1;     ///< [comparison] 1 when a < b, 0 otherwise
    uint32_t equal : 1;         ///< [comparison] 1 when a == b, 0 otherwise
    uint32_t greater_than : 1;  ///< [comparison] 1 when a > b, 0 otherwise
    uint32_t not_equal : 1;     ///< [comparison] 1 when a != b, 0 otherwise
    uint32_t : 4;
    // Arithmetic Flags for ALU results
    uint32_t positive : 1;   ///< [arithmetic] 1 when result is positive, 0 when negative
    uint32_t negative : 1;   ///< [arithmetic] 1 when result is negative, 0 when positive
    uint32_t zero : 1;       ///< [arithmetic] 1 when result is zero. 0 otherwise
    uint32_t non_zero : 1;   ///< [arithmetic] 1 when result is non-zero. 0 otherwise
    uint32_t overflow : 1;   ///< [arithmetic] 1 when a signed overflow occurred (result was > than signed max)
    uint32_t saturated : 1;  ///< [arithmetic] 1 when result was saturated
    uint32_t undefined : 1;  ///< [arithmetic] 1 when result is undefined (e.g., div by zero)
    uint32_t : 1;
    // Precision Flags for Floating Point results
    uint32_t inexact : 1;    ///< [precision] 1 when result is inexact
    uint32_t rounded : 1;    ///< [precision] 1 when result was rounded
    uint32_t subnormal : 1;  ///< [precision] 1 when result is subnormal
    uint32_t underflow : 1;  ///< [precision] 1 when result underflowed
    uint32_t : 4;
};

/// Platforms specific use of a Union for expressing the notion of a register word
template <size_t BITS>
union word {
    constexpr static size_t SizeInBits{BITS};
    constexpr static size_t SizeInBytes{BITS / 8};

    /// Default Constructor
    constexpr word() : as_u32{0} {
    }

    /// Constructs a word from a char array
    constexpr word(const char* v) : as_char{*v} {
    }
    /// Constructs a word from a uint8_t
    constexpr word(uint8_t v) : as_u08{v, 0U, 0U, 0U} {
    }
    /// Constructs a word from two uint8_t
    constexpr word(uint8_t b, uint8_t a) : as_u08{a, b, 0U, 0U} {
    }
    /// Constructs a word from three uint8_t
    constexpr word(uint8_t c, uint8_t b, uint8_t a) : as_u08{a, b, c, 0U} {
    }
    /// Constructs a word from four uint8_t
    constexpr word(uint8_t d, int8_t c, uint8_t b, uint8_t a) : as_u08{a, b, c, d} {
    }
    /// Constructs a word from a uint16_t
    constexpr word(uint16_t v) : as_u16{v, 0U} {
    }
    /// Constructs a word from a 32-bit unsigned integer
    constexpr word(uint32_t v) : as_u32{v} {
    }
    /// Constructs a word from an int8_t
    constexpr word(int8_t v) : as_s08{v, 0, 0, 0} {
    }
    /// Constructs a word from an int16_t
    constexpr word(int16_t v) : as_s16{v, 0} {
    }
    /// Constructs a word from an int32_t
    constexpr word(int32_t v) : as_s32{v} {
    }
    /// Constructs a word from a float
    constexpr word(float v) : as_float{v} {
    }
    /// Constructs a word from an Address
    constexpr word(Address v) : as_address{v} {
    }

    char as_char[SizeInBytes / sizeof(char)];
    uint8_t as_u08[SizeInBytes / sizeof(uint8_t)];
    uint16_t as_u16[SizeInBytes / sizeof(uint16_t)];
    uint32_t as_u32[SizeInBytes / sizeof(uint32_t)];
    int8_t as_s08[SizeInBytes / sizeof(int8_t)];
    int16_t as_s16[SizeInBytes / sizeof(int16_t)];
    int32_t as_s32[SizeInBytes / sizeof(int32_t)];
    float as_float[SizeInBytes / sizeof(float)];
    Address as_address;  ///< Can only be 1 address in a word
};
static_assert(sizeof(word<32>) == sizeof(Address), "Must be the same size");

/// Recursively computes a log2 input value.
/// @param n The value to compute the log2 of
/// @return Returns the log2 value of the input
template <typename TYPE>
constexpr TYPE log2(TYPE value) {
    static_assert(std::is_integral<TYPE>::value, "Must be an integral type");
    return ((value < 2U) ? 0U : 1U + log2(value / 2U));
}
static_assert(log2(1U << 1U) == 1U, "Must be this value exactly");
static_assert(log2(1U << 3U) == 3U, "Must be this value exactly");
static_assert(log2(1U << 19U) == 19U, "Must be this value exactly");

/// The number of bits needed to index a Scratch Register or Evaluation Register
constexpr static size_t CountOfIndexBits{log2(CountOfRegisters)};
constexpr static size_t CountOfDataShiftBits{log2(CountOfDataBits)};

/// An index over a COUNT of items.
template <size_t BITS>
struct index {
    /// Constructs an index
    constexpr index(uint32_t v) : value{v} {
    }

    uint32_t value : BITS;
    uint32_t : 32 - BITS;

    operator size_t() const {
        return static_cast<size_t>(value);
    }

    operator uint32_t const&() const {
        return value;
    }

    template <size_t _BITS>
    friend std::ostream& operator<<(std::ostream& os, index<_BITS> index) {
        os << std::dec << index.value;
        return os;
    }
};

/// Used to express an immediate value of a given bit size
template <size_t BITS>
struct Immediate {
    /// Constructs an immediate value
    constexpr Immediate(uint32_t v) : value{v} {
    }

    uint32_t value : BITS;
    uint32_t : 32 - BITS;

    template <size_t _BITS>
    friend std::ostream& operator<<(std::ostream& os, Immediate<_BITS> imm) {
        os << "#" << std::hex << imm.value;
        return os;
    }
};

/// The memory range that this Bus should respond to.
struct Range {
    constexpr Range(Address s, Address l) : start{s}, limit{l} {
    }
    Address start;  ///< Inclusive Start Value
    Address limit;  ///< Inclusive End Value

    /// Checks to see if an address is contained in this range.
    constexpr bool Contains(Address const& addr) const {
        return (start <= addr) and (addr <= limit);
    }

    /// Checks to see if a Range is fully contained in this range.
    constexpr bool Contains(Range const& r) const {
        return (start <= r.start) and (r.limit <= limit);
    }

    /// Returns the minimum type-addressable unit count of this range (i.e. bytes).
    constexpr size_t Size() const {
        return static_cast<size_t>(limit - start + 1U);
    }
};

}  // namespace isa
