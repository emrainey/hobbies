#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace isa {

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

/// A function to compute if a number is a power of two which is useful for restricting sizes to binary powers.
constexpr bool is_power_of_two(size_t value) {
    return (value != 0) and ((value & (value - 1)) == 0);
}

constexpr static size_t CountOfRegisters = 16;
constexpr static size_t CountOfDataBits = 32;
constexpr static size_t CountOfUnitsPerCacheLine = 16;

constexpr auto to_underlying = [](auto e) { return static_cast<std::underlying_type_t<decltype(e)>>(e); };

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

    /// @warning This may truncate the input value to 32 bits on 64 bit systems, which may cause loss of information if
    /// the input value exceeds 32 bits. Use with caution!
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

/// The Evaluation Storage Unit is a byte.
using EvaluationUnit = uint8_t;

enum class EvaluationType : EvaluationUnit {
    None = 0U,
    Comparison = 1U,
    Arithmetic = 2U,
    Precision = 3U,
};
constexpr static size_t EvaluationTypeBits{2U};
constexpr static size_t EvaluationFlagBits{6U};

/// The base evaluation struct contains the type of the evaluation and is used for instructions that do not produce meaningful evaluation results but need to be able to be stored in an evaluation register for consistency.
struct BaseEvaluation {
    BaseEvaluation() : type{to_underlying(EvaluationType::None)}, flags{0} {
    }
    EvaluationUnit type : EvaluationTypeBits;  ///< The EvaluationType (None).
    EvaluationUnit flags : EvaluationFlagBits; ///< The Flags for Evaluation whose meaning depends on the type of the evaluation. For BaseEvaluation, these bits are unused.

    friend std::ostream& operator<<(std::ostream& os, BaseEvaluation const& base) {
        os << "E(Type: " << static_cast<uint32_t>(base.type) << ", Value: 0b" << std::bitset<EvaluationFlagBits>(base.flags) << ")";
        return os;
    }
};

/// A struct to represent the results of comparisons, which can be used for conditional moves and jumps. The struct is
/// designed to fit within a single 6-bit field of an Evaluation register, allowing for efficient storage and retrieval
/// of comparison results.
struct ComparisonEvaluation {
    ComparisonEvaluation() : type{to_underlying(EvaluationType::Comparison)}, less_than{0}, equal{0}, greater_than{0}, not_equal{0} {
    }
    ComparisonEvaluation(bool lt, bool eq, bool gt, bool ne)
        : type{to_underlying(EvaluationType::Comparison)}
        , less_than{EvaluationUnit(lt ? 1U : 0U)}
        , equal{EvaluationUnit(eq ? 1U : 0U)}
        , greater_than{EvaluationUnit(gt ? 1U : 0U)}
        , not_equal{EvaluationUnit(ne ? 1U : 0U)} {
    }

    EvaluationUnit type : EvaluationTypeBits;  ///< The EvaluationType (comparison).
    EvaluationUnit less_than : 1;              ///< 1 when a < b, 0 otherwise
    EvaluationUnit equal : 1;                  ///< 1 when a == b, 0 otherwise
    EvaluationUnit greater_than : 1;           ///< 1 when a > b, 0 otherwise
    EvaluationUnit not_equal : 1;              ///< 1 when a != b, 0 otherwise
    EvaluationUnit : 2;                        ///< Unused bits for future expansion

    friend std::ostream& operator<<(std::ostream& os, ComparisonEvaluation c) {
        os << "C(LT: " << static_cast<uint32_t>(c.less_than) << ", EQ: " << static_cast<uint32_t>(c.equal)
           << ", GT: " << static_cast<uint32_t>(c.greater_than) << ", NE: " << static_cast<uint32_t>(c.not_equal) << ")";
        return os;
    }
};

/// A struct to represent the results of arithmetic operations, which can be used for conditional moves and jumps.
struct ArithmeticEvaluation {
    ArithmeticEvaluation() : type{to_underlying(EvaluationType::Arithmetic)}, positive{0}, zero{0}, overflow{0}, saturated{0}, undefined{0} {
    }

    EvaluationUnit type : EvaluationTypeBits;  ///< The EvaluationType (arithmetic).
    EvaluationUnit positive : 1;               ///< 1 when result is positive, 0 when negative (zero is considered positive)
    EvaluationUnit zero : 1;                   ///< 1 when result is zero. 0 otherwise
    EvaluationUnit overflow : 1;               ///< 1 when a signed overflow occurred (result was > than signed max)
    EvaluationUnit saturated : 1;              ///< 1 when result was saturated
    EvaluationUnit undefined : 1;              ///< 1 when result is undefined (e.g., div by zero)
    EvaluationUnit : 1;                        ///< Unused bits for future expansion

    friend std::ostream& operator<<(std::ostream& os, ArithmeticEvaluation a) {
        os << "A(Pos: " << static_cast<uint32_t>(a.positive) << ", Zero: " << static_cast<uint32_t>(a.zero)
           << ", OVF: " << static_cast<uint32_t>(a.overflow) << ", Sat: " << static_cast<uint32_t>(a.saturated)
           << ", Undef: " << static_cast<uint32_t>(a.undefined) << ")";
        return os;
    }
};

/// A struct to represent the results of floating-point operations, which can be used for conditional moves and jumps.
struct PrecisionEvaluation {
    PrecisionEvaluation() : type{to_underlying(EvaluationType::Precision)}, inexact{0}, rounded{0}, subnormal{0}, underflow{0} {
    }
    EvaluationUnit type : EvaluationTypeBits;  ///< The EvaluationType (precision).
    EvaluationUnit inexact : 1;                ///< 1 when result is inexact
    EvaluationUnit rounded : 1;                ///< 1 when result was rounded
    EvaluationUnit subnormal : 1;              ///< 1 when result is subnormal
    EvaluationUnit underflow : 1;              ///< 1 when result underflowed
    EvaluationUnit : 2;                        ///< Unused bits for future expansion

    friend std::ostream& operator<<(std::ostream& os, PrecisionEvaluation p) {
        os << "P(Inexact: " << static_cast<uint32_t>(p.inexact) << ", Rounded: " << static_cast<uint32_t>(p.rounded)
           << ", Subnormal: " << static_cast<uint32_t>(p.subnormal) << ", Underflow: " << static_cast<uint32_t>(p.underflow) << ")";
        return os;
    }
};

/// The Evaluation union allows for different types of evaluation results to be stored in the same register, with the type field indicating how to interpret the value.
union Evaluation {
    Evaluation() : value{0} {
    }
    Evaluation(EvaluationUnit raw) : value{raw} {
    }
    Evaluation(ComparisonEvaluation comparison) : comparison{comparison} {
    }
    Evaluation(ArithmeticEvaluation arithmetic) : arithmetic{arithmetic} {
    }
    Evaluation(PrecisionEvaluation precision) : precision{precision} {
    }
    // === Assignment Operators for ease of use ===
    Evaluation& operator=(EvaluationUnit raw) {
        value = raw;
        return *this;
    }
    Evaluation& operator=(ComparisonEvaluation comparison) {
        this->comparison = comparison;
        return *this;
    }
    Evaluation& operator=(ArithmeticEvaluation arithmetic) {
        this->arithmetic = arithmetic;
        return *this;
    }
    Evaluation& operator=(PrecisionEvaluation precision) {
        this->precision = precision;
        return *this;
    }

    // === Storage ===
    /// An empty evaluation with no flags set, used for instructions that do not produce meaningful evaluation results
    BaseEvaluation base;
    /// The results of a comparison operation
    ComparisonEvaluation comparison;
    /// Flags for arithmetic operations (e.g., positive, negative, zero, overflow)
    ArithmeticEvaluation arithmetic;
    /// Flags for floating-point precision (e.g., inexact, rounded)
    PrecisionEvaluation precision;
    /// The raw value of the evaluation for easy access and storage
    EvaluationUnit value;
    // === Storage ===

    friend std::ostream& operator<<(std::ostream& os, Evaluation const& eval) {
        if (eval.base.type == to_underlying(EvaluationType::None)) {
            os << eval.base;
        } else if (eval.base.type == to_underlying(EvaluationType::Comparison)) {
            os << eval.comparison;
        } else if (eval.base.type == to_underlying(EvaluationType::Arithmetic)) {
            os << eval.arithmetic;
        } else if (eval.base.type == to_underlying(EvaluationType::Precision)) {
            os << eval.precision;
        }
        return os;
    }

    EvaluationType Type() const {
        return static_cast<EvaluationType>(base.type);
    }
};
static_assert(sizeof(Evaluation) == sizeof(EvaluationUnit), "Evaluation must be 8 bits");

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

    /// Returns a copy of the value
    operator uint32_t() const {
        return value;
    }

    constexpr bool operator==(index<BITS> const& other) const {
        return value == other.value;
    }

    constexpr bool operator!=(index<BITS> const& other) const {
        return not operator==(other);
    }

    friend std::ostream& operator<<(std::ostream& os, index<BITS> index) {
        os << std::dec << index.value;
        return os;
    }
};

/// Used to express an unsigned immediate value of a given bit size
template <size_t BITS>
struct Immediate {
    constexpr static size_t Bits{BITS};

    /// Constructs an immediate value
    constexpr Immediate(uint32_t v) : value{v} {
    }

    uint32_t value : BITS;
    uint32_t : 32 - BITS;

    friend std::ostream& operator<<(std::ostream& os, Immediate<BITS> imm) {
        os << "#" << std::hex << imm.value;
        return os;
    }
};

/// Used to express a signed immediate value of a given bit size
template <size_t BITS>
struct SignedImmediate {
    constexpr static size_t Bits{BITS};

    /// Constructs a signed immediate value
    constexpr SignedImmediate(int32_t v) : value{v} {
    }

    int32_t value : BITS;
    int32_t : 32 - BITS;

    friend std::ostream& operator<<(std::ostream& os, SignedImmediate<BITS> imm) {
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
