#pragma once

#include <cstdint>
#include <cstddef>
#include <iostream>

#include <basal/exception.hpp>
#include <isa/types.hpp>

namespace isa {

/// The operation code for an instruction

enum class Operator : uint32_t {
    None = 0,  ///< noop
    // === Movement within Files ===
    Copy = 0x04U,           ///< copy scratchDestination, scratchSource
    Move = 0x08U,           ///< move scratchDestination, scratchSource, Source = 0
    MoveImmediate = 0x0CU,  ///< move immediate to scratch/evaluation register
    Swap = 0x10U,           ///< swap scratchRegisterA, scratchRegisterB; reg_type (0=scratch, 1=evaluation)
    Zero = 0x14U,           ///< clear registers using mask; reg_type (0=scratch, 1=evaluation)
    // === Special Register Manipulation ===
    Leap = 0x18U,  ///< leap to address in scratch with optional immediate offset and condition
    Back = 0x1CU,  ///< back from subroutine to address in Return Address Special Register
    Grow = 0x20U,  ///< grow #imm<16> (mask of scratch)
    Undo = 0x24U,  ///< undo #imm<16> (mask of scratch)
    Call = 0x28U,  ///< call #imm<16> (System Call Number)
    Trip = 0x2CU,  ///< Trip #imm<16> (Trip an exception with the given exception type encoded in the immediate value.
                   ///< This can be used to test triggering interrupts)
    Breakpoint = 0x30U,  ///< Breakpoint #imm<16> (Trigger a breakpoint exception with the given immediate value. This can be
                      ///< used to set breakpoints in software for debugging)
    // === Memory Operations ===
    Load = 0x34U,  ///< load scratchDestination, scratchAddress
    Save = 0x38U,  ///< store scratchSource, scratchAddress
    // === Comparison ===
    Compare = 0x3CU,  ///< Compare scratchA, scratchB
    // === Bit Operators ===
    // 1 arg
    Complement = 0x40U,  ///< bcmpl scratchDestination, scratchSource
    Rsh = 0x44U,         ///< brsh scratchDestination, scratchSource, #imm<5>
    Asr = 0x48U,         ///< basr scratchDestination, scratchSource, #imm<5>
    Lsh = 0x4CU,         ///< blsh scratchDestination, scratchSource, #imm<5>
    Rotate = 0x50U,      ///< brot scratchDestination, scratchSource, #imm<5>
    Count1s = 0x54U,     ///< bcnt scratchDestination, scratchSource
    CountL0s = 0x58U,    ///< bcnz scratchDestination, scratchSource
    Not = 0x5CU,         ///< bnot scratchDestination, scratchSource
    SetBit = 0x60U,      ///< bset scratchDestination, scratchSource, #imm<5>
    ClearBit = 0x64U,    ///< bclr scratchDestination, scratchSource, #imm<5>
    ToggleBit = 0x68U,   ///< btgl scratchDestination, scratchSource, #imm<5>
    Reverse = 0x6CU,  ///< brev scratchDestination, scratchSource, #imm<5> (but limited to 0 = bytes, 1 = half-words, 2
                      ///< = words)
    // 2 arg
    And = 0x70U,  ///< band scratchDestination, scratchSource, scratchSource
    Or = 0x74U,   ///< borr scratchDestination, scratchSource, scratchSource
    Xor = 0x78U,  ///< bxor scratchDestination, scratchSource, scratchSource

    // === Arithmetic (Integer) ===
    Addition = 0x7CU,  ///< add{s/u}.{type}{size} Sd, Sa, Sb : Es
    Subtract = 0x80U,  ///< sub{s/u}.{type}{size} Sd, Sa, Sb : Es
    Multiply = 0x84U,  ///< mul{s/u}.{type}{size} Sd, Sa, Sb : Es
    Divide = 0x88U,    ///< div{s/u}.{type}{size} Sd, Sa, Sb : Es
    Modulo = 0x8CU,    ///< mod{s/u}.{type}{size} Sd, Sa, Sb : Es

    // === ALU (Precision) ===
    // 1 arg
    FloatingFloor = 0x90U,       ///< fflr scratchDestination, scratchSource
    FloatingCeil = 0x94U,        ///< fcel scratchDestination, scratchSource
    FloatingAbs = 0x98U,         ///< fabs scratchDestination, scratchSource
    FloatingNegate = 0x9CU,      ///< fneg scratchDestination, scratchSource
    FloatingFractional = 0xA0U,  ///< ffrc scratchDestination, scratchSource
    FloatingConvert = 0xA4U,     ///< fcvts{type}{size} scratchDestination, scratchSource
    // 2 arg
    FloatingAddition = 0xA8U,        ///< fadd scratchDestination, scratchSource, scratchSource -> Overflow or Underflow
    FloatingSubtraction = 0xACU,     ///< fsub scratchDestination, scratchSource, scratchSource -> Overflow or Underflow
    FloatingMultiplication = 0xB0U,  ///< fmul scratchDestination, scratchSource, scratchSource -> Overflow or Underflow
    FloatingDivision = 0xB4U,  ///< fdiv scratchDestination, scratchSource, scratchSource -> Fault if scratchSource == 0

    // === ALU (SIMD) ===

    Halt = 0xFCU  ///< Halts the processor.
    // === NO INSTRUCTIONS PAST THIS POINT ===
};
constexpr size_t CountOfOperatorBits{8u};
constexpr uint32_t ImmediateLeapBitMask{0x1U};
constexpr uint32_t ImmediateLeapSaveBitMask{0x2U};
constexpr uint32_t ImmediateLeapAddressMask{0xFFFF'FFFCU};

enum class Size : uint32_t {
    Byte = 0,
    HalfWord = 1,
    Word = 2,
    DoubleWord = 3,
};

struct Operand {
    /// The type of the Operand
    enum class Type : uint32_t {
        None = 0,
        Scratch = 1,
        Evaluation = 2,
        Mask = 3,
    };

    constexpr Operand(Type t, uint32_t r) : index{r}, type{static_cast<uint32_t>(t)}, imm{0} {
    }
    constexpr Operand(Type t, Immediate<16> imm) : index{0}, type{static_cast<uint32_t>(t)}, imm{imm.value} {
    }
    uint32_t index : 4;
    uint32_t type : 2;
    uint32_t imm : 16;
    uint32_t : 10;

    friend std::ostream& operator<<(std::ostream& os, Operand o) {
        Operand::Type t = static_cast<Operand::Type>(o.type);
        if (t == Operand::Type::None) {
            os << "R" << o.index;
        } else if (t == Operand::Type::Scratch) {
            os << "S" << o.index;
        } else if (t == Operand::Type::Evaluation) {
            os << "E" << o.index;
        } else if (t == Operand::Type::Mask) {
            os << "0b" << std::bitset<16>(o.imm);
            return os;
        } else {
            os << "???";
            return os;
        }
        return os;
    }
};

namespace instructions {

/// The base instruction class allows copy/move semantics
class Base {
public:
    Base() : op{Operator::None} {
    }

    Base(Base const&) = default;
    Base(Base&&) = default;
    Base& operator=(Base const&) = default;
    Base& operator=(Base&&) = default;

    Operator operator()() const {
        if ((to_underlying(op) & ImmediateLeapBitMask) != 0U) {
            return Operator::Leap;
        }
        return op;
    }

protected:
    Operator op : CountOfOperatorBits;
    uint32_t arg : CountOfDataBits - CountOfOperatorBits;
};

class NoOp {
public:
    constexpr NoOp() : op{Operator::None} {
    }
    friend std::ostream& operator<<(std::ostream& os, NoOp) {
        os << "noop";
        return os;
    }

protected:
    Operator op : CountOfOperatorBits;
    uint32_t : CountOfDataBits - CountOfOperatorBits;
};

class Halt {
public:
    constexpr Halt() : op{Operator::Halt} {
    }
    friend std::ostream& operator<<(std::ostream& os, Halt) {
        os << "halt";
        return os;
    }

protected:
    Operator op : CountOfOperatorBits;
    uint32_t : CountOfDataBits - CountOfOperatorBits;
};

struct Breakpoint {
    using ImmediateType = isa::Immediate<16>;
    constexpr Breakpoint(ImmediateType imm) : op{Operator::Breakpoint}, imm{imm.value} {
    }
    friend std::ostream& operator<<(std::ostream& os, Breakpoint b) {
        os << "bkpt " << Operand{Operand::Type::Mask, b.imm};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t : CountOfDataBits - CountOfOperatorBits - ImmediateType::Bits;
    uint32_t imm : ImmediateType::Bits;
};

struct Copy {
    Copy() = delete;
    constexpr Copy(Operand dst, Operand src) : op{Operator::Copy}, dst{dst.index}, src{src.index}, eval{0}, cond{0} {
        basal::exception::throw_unless(
            dst.type == to_underlying(Operand::Type::Scratch) and src.type == to_underlying(Operand::Type::Scratch),
            __FILE__, __LINE__,
            "Copy instruction requires both source and destination operands to be Scratch registers");
    }
    constexpr Copy(Operand dst, Operand src, Operand eval)
        : op{Operator::Copy}, dst{dst.index}, src{src.index}, eval{eval.index}, cond{1} {
        basal::exception::throw_unless(
            dst.type == to_underlying(Operand::Type::Scratch) and src.type == to_underlying(Operand::Type::Scratch),
            __FILE__, __LINE__,
            "Copy instruction requires both source and destination operands to be Scratch registers");
    }
    friend std::ostream& operator<<(std::ostream& os, Copy c) {
        os << "copy " << Operand{Operand::Type::Scratch, c.dst} << ", " << Operand{Operand::Type::Scratch, c.src};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfScratchIndexBits;
    uint32_t src : CountOfScratchIndexBits;
    uint32_t eval : CountOfEvalIndexBits;
    uint32_t cond : 1;  ///< If == 1, then the copy will set the evaluation register
    uint32_t : CountOfDataBits - CountOfOperatorBits - (2 * CountOfScratchIndexBits) - CountOfEvalIndexBits - 1;
};

struct Move {
    using ImmediateType = isa::Immediate<log2(CountOfDataShiftBits)>;
    Move() = delete;
    constexpr Move(Operand dst, Operand src)
        : op{Operator::Move}
        , eval{0}
        , mask{0}
        , dst{dst.index}
        , src{src.index}
        , cond{0}
        , type{dst.type == static_cast<uint32_t>(Operand::Type::Evaluation)
                       && src.type == static_cast<uint32_t>(Operand::Type::Evaluation)
                   ? 1U
                   : 0U}
        , dir{0}
        , shift{0} {
    }

    constexpr Move(Operand eval, Operand mask, Operand dst, Operand src)
        : op{Operator::Move}
        , eval{eval.index}
        , mask{mask.index}
        , dst{dst.index}
        , src{src.index}
        , cond{1}
        , type{dst.type == static_cast<uint32_t>(Operand::Type::Evaluation)
                       && src.type == static_cast<uint32_t>(Operand::Type::Evaluation)
                   ? 1U
                   : 0U}
        , dir{0}
        , shift{0} {
    }

    constexpr Move(Operand dst, Operand src, bool shift_right, ImmediateType shift_amount)
        : op{Operator::Move}
        , eval{0}
        , mask{0}
        , dst{dst.index}
        , src{src.index}
        , cond{0}
        , type{dst.type == static_cast<uint32_t>(Operand::Type::Evaluation)
                       && src.type == static_cast<uint32_t>(Operand::Type::Evaluation)
                   ? 1U
                   : 0U}
        , dir{shift_right ? 1U : 0U}
        , shift{shift_amount.value} {
    }

    constexpr Move(Operand eval, Operand mask, Operand dst, Operand src, bool shift_right, ImmediateType shift_amount)
        : op{Operator::Move}
        , eval{eval.index}
        , mask{mask.index}
        , dst{dst.index}
        , src{src.index}
        , cond{1}
        , type{dst.type == static_cast<uint32_t>(Operand::Type::Evaluation)
                       && src.type == static_cast<uint32_t>(Operand::Type::Evaluation)
                   ? 1U
                   : 0U}
        , dir{shift_right ? 1U : 0U}
        , shift{shift_amount.value} {
    }

    friend std::ostream& operator<<(std::ostream& os, Move m) {
        os << "move ";
        os << Operand{Operand::Type::Scratch, m.dst} << ", " << Operand{Operand::Type::Scratch, m.src};
        if (m.shift > 0) {
            os << (m.dir ? " >> " : " << ") << m.shift;
        }
        if (m.cond) {
            os << " : " << Operand{Operand::Type::Evaluation, m.eval} << ", "
               << Operand{Operand::Type::Evaluation, m.mask};
        }
        return os;
    }

    const Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;    ///< Evaluation
    uint32_t mask : CountOfEvalIndexBits;    ///< Evaluation Mask
    uint32_t dst : CountOfScratchIndexBits;  ///< Operand
    uint32_t src : CountOfScratchIndexBits;  ///< Operand
    uint32_t cond : 1;                       ///< If == 1, then the move is conditional on the evaluation and mask.
    uint32_t type : 1;                       ///< 0 = Scratch to Scratch, 1 = Evaluation to Evaluation
    uint32_t dir : 1;                        ///< Direction for Shift (0 = Left, 1 = Right)
    uint32_t shift : CountOfDataShiftBits;   ///< Shift Amount (for shift instructions)
};

struct MoveImmediate {
    using ImmediateType = isa::Immediate<16>;
    MoveImmediate() = delete;
    constexpr MoveImmediate(Operand dst, ImmediateType imm, bool upper = false)
        : op{Operator::MoveImmediate}
        , dst{dst.index}
        , eval{dst.type == to_underlying(Operand::Type::Evaluation) ? 1U : 0U}
        , upper{upper ? 1U : 0U}
        , imm{imm.value} {
        basal::exception::throw_if(
            dst.type != to_underlying(Operand::Type::Scratch) && dst.type != to_underlying(Operand::Type::Evaluation),
            __FILE__, __LINE__,
            "MoveImmediate instruction requires the destination operand to be a Scratch or Evaluation register");
        basal::exception::throw_if(dst.type == to_underlying(Operand::Type::Evaluation) and upper == true, __FILE__,
                                   __LINE__,
                                   "Upper shifts are not allowed when moving an immediate to an Evaluation register "
                                   "since Evaluation registers are only 8 bits");
    }
    constexpr MoveImmediate(Operand dst, Evaluation eval)
        : op{Operator::MoveImmediate}
        , dst{dst.index}
        , eval{dst.type == to_underlying(Operand::Type::Evaluation) ? 1U : 0U}
        , upper{0}
        , imm{eval.value} {
        basal::exception::throw_unless(dst.type == to_underlying(Operand::Type::Evaluation), __FILE__, __LINE__,
                                       "MoveImmediate instruction with ComparisonEvaluation value requires the "
                                       "destination operand to be an Evaluation register");
        basal::exception::throw_if(dst.type == to_underlying(Operand::Type::Evaluation) and eval.value > 0xFFU,
                                   __FILE__, __LINE__,
                                   "When moving a ComparisonEvaluation to an Evaluation register, the value must fit "
                                   "within 8 bits since Evaluation registers are only 8 bits");
    }

    friend std::ostream& operator<<(std::ostream& os, MoveImmediate mi) {
        os << "move "
           << (mi.eval ? Operand{Operand::Type::Evaluation, mi.dst} : Operand{Operand::Type::Scratch, mi.dst}) << ", "
           << ImmediateType{mi.imm};
        if (mi.upper) {
            os << " << 16U";
        }
        return os;
    }

    constexpr ImmediateType Immediate() const {
        return ImmediateType{imm};
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : std::max(CountOfScratchIndexBits, CountOfEvalIndexBits);
    uint32_t eval : 1;
    uint32_t upper : 1;
    uint32_t : 2;
    uint32_t imm : ImmediateType::Bits;
};

struct Swap {
    Swap() = delete;
    constexpr Swap(Operand a, Operand b) : op{Operator::Swap}, a{a.index}, b{b.index}, type{0} {
        basal::exception::throw_unless(a.type == b.type, __FILE__, __LINE__,
                                       "Swap instruction requires both operands to be of the same type");
        basal::exception::throw_if(
            a.type == static_cast<uint32_t>(Operand::Type::None)
                || a.type > static_cast<uint32_t>(Operand::Type::Evaluation),
            __FILE__, __LINE__, "Swap instruction requires both operands to be either Scratch or Evaluation registers");
        type = (a.type == to_underlying(Operand::Type::Scratch)) ? RegisterType::Scratch : RegisterType::Evaluation;
    }

    friend std::ostream& operator<<(std::ostream& os, Swap s) {
        Operand::Type register_type
            = s.type == RegisterType::Scratch ? Operand::Type::Scratch : Operand::Type::Evaluation;
        os << "swap " << Operand{register_type, s.a} << ", " << Operand{register_type, s.b};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t a : CountOfScratchIndexBits;
    uint32_t b : CountOfScratchIndexBits;
    RegisterType type : 1;
    uint32_t : 15;
};

struct Zero {
    using ImmediateType = isa::Immediate<std::max(CountOfScratchRegisters, CountOfEvaluationRegisters)>;
    Zero() = delete;
    constexpr Zero(Operand mask, RegisterType evaluation = RegisterType::Scratch)
        : op{Operator::Zero}, mask{mask.imm}, type{to_underlying(evaluation)} {
    }
    friend std::ostream& operator<<(std::ostream& os, Zero c) {
        os << "clear " << (c.type == RegisterType::Scratch ? 'S' : 'E') << ", "
           << Operand{Operand::Type::Mask, ImmediateType{c.mask}};
        return os;
    }

    Operator op : CountOfOperatorBits;
    /// Each bit corresponds to a register. If the bit is set, the corresponding register will be cleared.
    uint32_t mask : ImmediateType::Bits;
    RegisterType type : 1;
    uint32_t : 7;
};

class Load {
public:
    using ImmediateType = isa::Immediate<10>;
    Load() = delete;

    constexpr static Load Byte(Operand src, Operand base, uint32_t shift, ImmediateType imm = ImmediateType{0},
                               bool inc_or_off = false) {
        return Load(src, base, shift, Size::Byte, imm, inc_or_off);
    }

    constexpr static Load HalfWord(Operand src, Operand base, uint32_t shift, ImmediateType imm = ImmediateType{0},
                                   bool inc_or_off = false) {
        return Load(src, base, shift, Size::HalfWord, imm, inc_or_off);
    }

    constexpr static Load Word(Operand src, Operand base, ImmediateType imm = ImmediateType{0},
                               bool inc_or_off = false) {
        return Load(src, base, 0, Size::Word, imm, inc_or_off);
    }

    friend std::ostream& operator<<(std::ostream& os, Load l) {
        os << "load";
        switch (Size(l.size)) {
            case Size::Byte:
                os << ".b ";
                break;
            case Size::HalfWord:
                os << ".h ";
                break;
            case Size::Word:
                os << ".w ";
                break;
            default:
                os << ".? ";
                break;
        }
        os << Operand{Operand::Type::Scratch, l.dst} << " <- " << Operand{Operand::Type::Scratch, l.base};
        if (l.shift > 0) {
            os << " << " << l.shift;
        }
        if (l.imm > 0) {
            if (l.off) {
                os << " + " << l.imm;
            } else {
                os << " += " << l.imm;
            }
        }
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t base : CountOfScratchIndexBits;  ///< Base Scratch Register containing the address
    uint32_t : 1;                             ///< Unused
    uint32_t off : 1;  ///< If set, the base scratch register will be used as an offset from a base address in a special
                       ///< register instead of an absolute address.
    uint32_t shift : 2;
    Size size : 2;                       ///< Size of the load (0 = byte, 1 = half-word, 2 = word)
    uint32_t imm : ImmediateType::Bits;  ///< The immediate value of the offset or increment in bytes.
protected:
    constexpr Load(Operand dst, Operand base, uint32_t shift = 0, Size size = Size::Word,
                   ImmediateType imm = ImmediateType{0}, bool inc_or_off = false)
        : op{Operator::Load}
        , dst{dst.index}
        , base{base.index}
        , off{inc_or_off ? 0U : 1U}
        , shift{shift}
        , size{size}
        , imm{imm.value} {
        if (imm.value == 0U) {
            off = 0U;
        }
        basal::exception::throw_if(shift > 0 and size == Size::Word, __FILE__, __LINE__,
                                   "Shift values greater than 0 are only allowed for byte and half-word stores");
        basal::exception::throw_if(shift > 1 and size == Size::HalfWord, __FILE__, __LINE__,
                                   "Shift values greater than 1 are only allowed for byte");
    }
};

class Save {
public:
    using ImmediateType = isa::Immediate<10>;
    Save() = delete;

    constexpr static Save Byte(Operand src, Operand base, uint32_t shift, ImmediateType imm = ImmediateType{0},
                               bool inc_or_off = false) {
        return Save(src, base, shift, Size::Byte, imm, inc_or_off);
    }

    constexpr static Save HalfWord(Operand src, Operand base, uint32_t shift, ImmediateType imm = ImmediateType{0},
                                   bool inc_or_off = false) {
        return Save(src, base, shift, Size::HalfWord, imm, inc_or_off);
    }

    constexpr static Save Word(Operand src, Operand base, ImmediateType imm = ImmediateType{0},
                               bool inc_or_off = false) {
        return Save(src, base, 0, Size::Word, imm, inc_or_off);
    }

    friend std::ostream& operator<<(std::ostream& os, Save s) {
        os << "save";
        switch (Size(s.size)) {
            case Size::Byte:
                os << ".b ";
                break;
            case Size::HalfWord:
                os << ".h ";
                break;
            case Size::Word:
                os << ".w ";
                break;
            default:
                os << ".? ";
                break;
        }
        os << Operand{Operand::Type::Scratch, s.src};
        if (s.shift > 0) {
            os << " << " << s.shift;
        }
        os << " -> " << Operand{Operand::Type::Scratch, s.base};
        if (s.imm > 0) {
            if (s.off) {
                os << " + " << s.imm;
            } else {
                os << " += " << s.imm;
            }
        }
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t src : CountOfScratchIndexBits;   ///< Source Scratch Register
    uint32_t base : CountOfScratchIndexBits;  ///< Base Scratch Register containing the address
    uint32_t : 1;                             ///< Unused bit to allow for symmetry with Load instruction encoding
    uint32_t off : 1;  ///< If set, the base scratch register will be used as an offset from a base address in a special
                       ///< register instead of an absolute address.
    uint32_t shift : 2;
    Size size : 2;                       ///< Size of the load (0 = byte, 1 = half-word, 2 = word)
    uint32_t imm : ImmediateType::Bits;  ///< The immediate value of the offset or increment in bytes
protected:
    constexpr Save(Operand src, Operand base, uint32_t shift = 0, Size size = Size::Word,
                   ImmediateType imm = ImmediateType{0}, bool inc_or_off = false)
        : op{Operator::Save}
        , src{src.index}
        , base{base.index}
        , off{inc_or_off ? 0U : 1U}
        , shift{shift}
        , size{size}
        , imm{imm.value} {
        if (imm.value == 0U) {
            off = 0U;
        }
        basal::exception::throw_if(shift > 0 and size == Size::Word, __FILE__, __LINE__,
                                   "Shift values greater than 0 are only allowed for byte and half-word stores");
        basal::exception::throw_if(shift > 1 and size == Size::HalfWord, __FILE__, __LINE__,
                                   "Shift values greater than 1 are only allowed for byte");
    }
};

/// The Leap Instruction
struct Leap {
    using ImmediateType = isa::SignedImmediate<10>;
    Leap() = delete;
    constexpr Leap(Operand dst, ImmediateType imm, bool save = false)
        : op{Operator::Leap}, eval{0}, mask{0}, dst{dst.index}, cond{0}, save{save ? 1U : 0U}, imm{imm.value} {
    }

    constexpr Leap(Operand eval, Operand mask, Operand dst, ImmediateType imm, bool save = false)
        : op{Operator::Leap}
        , eval{eval.index}
        , mask{mask.index}
        , dst{dst.index}
        , cond{1}
        , save{save ? 1U : 0U}
        , imm{imm.value} {
    }
    friend std::ostream& operator<<(std::ostream& os, Leap j) {
        os << "leap ";
        os << Operand{Operand::Type::Scratch, j.dst} << " + (" << ImmediateType{j.imm} << " * 4)";
        if (j.cond) {
            os << " : " << Operand{Operand::Type::Evaluation, j.eval} << ", "
               << Operand{Operand::Type::Evaluation, j.mask};
        }
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;    ///< Evaluation Register to check
    uint32_t mask : CountOfEvalIndexBits;    ///< Mask for the evaluation register
    uint32_t dst : CountOfScratchIndexBits;  ///< Scratch Register containing the destination address
    uint32_t cond : 1;                       ///< If == 1, then the leap is conditional on the evaluation and mask.
    uint32_t save : 1;  ///< If == 1, then the leap will save the return address in the Return Address Special Register.
    /// SIGNED Immediate value for the leap (e.g. for relative leap or as an offset). The value is multiplied by 4 to
    /// get a byte offset, since instructions are 4 bytes. This means that the relative leap range is +/- 2^12
    /// instructions, which should be sufficient for most use cases.
    int32_t imm : ImmediateType::Bits;
};

/// An absolute immediate leap encoded as a 30-bit aligned address and two LSB control bits.
struct LeapImmediate {
    LeapImmediate() = delete;
    constexpr LeapImmediate(Address target, bool save = false)
        : mode{save ? (ImmediateLeapBitMask | ImmediateLeapSaveBitMask) : ImmediateLeapBitMask}
        , imm{target.value >> 2U} {
        basal::exception::throw_if((target.value & 0x3U) != 0U, __FILE__, __LINE__,
                                   "Immediate leap target address must be 4-byte aligned");
    }

    friend std::ostream& operator<<(std::ostream& os, LeapImmediate j) {
        os << "leap #" << Address{static_cast<Address::StorageType>(j.imm) << 2U};
        if ((j.mode & ImmediateLeapSaveBitMask) != 0U) {
            os << " (save)";
        }
        return os;
    }

    uint32_t mode : 2;  ///< b0=1 indicates immediate leap mode, b1=1 saves PA+4 into RA.
    uint32_t imm : 30;  ///< Absolute aligned address, stored as address >> 2.
};

struct Back {
    Back() = delete;
    constexpr Back(bool zero = false) : op{Operator::Back}, zero{zero ? 1U : 0U} {
    }
    friend std::ostream& operator<<(std::ostream& os, Back b) {
        os << "back";
        if (b.zero) {
            os << " zero";
        }
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t zero : 1;  ///< If == 1, then the back will zero the return address after returning.
    uint32_t : CountOfDataBits - CountOfOperatorBits - 1;
};

struct Grow {
    using ImmediateType = isa::Immediate<10>;
    Grow() = delete;
    constexpr Grow(ImmediateType imm) : op{Operator::Grow}, eval{0}, mask{0}, cond{0}, imm{imm.value} {
    }
    constexpr Grow(Operand e, Operand m, ImmediateType imm)
        : op{Operator::Grow}, eval{e.index}, mask{m.index}, cond{1}, imm{imm.value} {
        basal::exception::throw_if(e.type != static_cast<uint32_t>(Operand::Type::Evaluation), __FILE__, __LINE__,
                                   "Grow instruction requires the first operand to be an Evaluation register");
        basal::exception::throw_if(m.type != static_cast<uint32_t>(Operand::Type::Evaluation), __FILE__, __LINE__,
                                   "Grow instruction requires the second operand to be an Evaluation register");
    }
    friend std::ostream& operator<<(std::ostream& os, Grow g) {
        os << "grow " << ImmediateType{g.imm};
        if (g.cond) {
            os << " : " << Operand{Operand::Type::Evaluation, g.eval} << ", "
               << Operand{Operand::Type::Evaluation, g.mask};
        }
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;  ///< Evaluation Register to check for conditional trip (if cond == 1)
    uint32_t mask : CountOfEvalIndexBits;  ///< Mask for the evaluation register
    uint32_t cond : 1;                     ///< If == 1, then the trip will be conditional on the evaluation and mask.
    uint32_t : CountOfDataBits - CountOfOperatorBits - (2 * CountOfEvalIndexBits) - ImmediateType::Bits - 1;
    uint32_t imm : ImmediateType::Bits;  ///< Each bit corresponds to a scratch register. If the bit is set, the
                                         ///< corresponding scratch register will be grown.
};

struct Undo {
    using ImmediateType = isa::Immediate<10>;
    Undo() = delete;
    constexpr Undo(ImmediateType imm) : op{Operator::Undo}, eval{0}, mask{0}, cond{0}, imm{imm.value} {
    }
    constexpr Undo(Operand e, Operand m, ImmediateType imm)
        : op{Operator::Undo}, eval{e.index}, mask{m.index}, cond{1}, imm{imm.value} {
        basal::exception::throw_if(e.type != static_cast<uint32_t>(Operand::Type::Evaluation), __FILE__, __LINE__,
                                   "Undo instruction requires the first operand to be an Evaluation register");
        basal::exception::throw_if(m.type != static_cast<uint32_t>(Operand::Type::Evaluation), __FILE__, __LINE__,
                                   "Undo instruction requires the second operand to be an Evaluation register");
    }
    friend std::ostream& operator<<(std::ostream& os, Undo u) {
        os << "undo " << ImmediateType{u.imm};
        if (u.cond) {
            os << " : " << Operand{Operand::Type::Evaluation, u.eval} << ", "
               << Operand{Operand::Type::Evaluation, u.mask};
        }
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;  ///< Evaluation Register to check for conditional trip (if cond == 1)
    uint32_t mask : CountOfEvalIndexBits;  ///< Mask for the evaluation register
    uint32_t cond : 1;                     ///< If == 1, then the trip will be conditional on the evaluation and mask.
    uint32_t : CountOfDataBits - CountOfOperatorBits - (2 * CountOfEvalIndexBits) - ImmediateType::Bits - 1;
    uint32_t imm : ImmediateType::Bits;  ///< Each bit corresponds to a scratch register. If the bit is set, the
                                         ///< corresponding scratch register will be undone (shrunk).
};

struct Call {
    using ImmediateType = isa::Immediate<10>;
    Call() = delete;
    constexpr Call(ImmediateType imm) : op{Operator::Call}, eval{0}, mask{0}, cond{0}, imm{imm.value} {
    }
    constexpr Call(Operand e, Operand m, ImmediateType imm)
        : op{Operator::Call}, eval{e.index}, mask{m.index}, cond{1}, imm{imm.value} {
        basal::exception::throw_if(e.type != static_cast<uint32_t>(Operand::Type::Evaluation), __FILE__, __LINE__,
                                   "Call instruction requires the first operand to be an Evaluation register");
        basal::exception::throw_if(m.type != static_cast<uint32_t>(Operand::Type::Evaluation), __FILE__, __LINE__,
                                   "Call instruction requires the second operand to be an Evaluation register");
    }
    friend std::ostream& operator<<(std::ostream& os, Call c) {
        os << "call " << ImmediateType{c.imm};
        if (c.cond) {
            os << " : " << Operand{Operand::Type::Evaluation, c.eval} << ", "
               << Operand{Operand::Type::Evaluation, c.mask};
        }
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;  ///< Evaluation Register to check for conditional trip (if cond == 1)
    uint32_t mask : CountOfEvalIndexBits;  ///< Mask for the evaluation register
    uint32_t cond : 1;                     ///< If == 1, then the trip will be conditional on the evaluation and mask.
    uint32_t : CountOfDataBits - CountOfOperatorBits - (2 * CountOfEvalIndexBits) - ImmediateType::Bits - 1;
    uint32_t imm : ImmediateType::Bits;  ///< The immediate value of the system call number to call.
};

struct Trip {
    using ImmediateType = isa::Immediate<10>;
    Trip() = delete;
    constexpr Trip(ImmediateType imm) : op{Operator::Trip}, eval{0}, mask{0}, cond{0}, imm{imm.value} {
    }
    constexpr Trip(Operand e, Operand m, ImmediateType imm)
        : op{Operator::Trip}, eval{e.index}, mask{m.index}, cond{1}, imm{imm.value} {
        basal::exception::throw_if(e.type != static_cast<uint32_t>(Operand::Type::Evaluation), __FILE__, __LINE__,
                                   "Trip instruction requires the first operand to be an Evaluation register");
        basal::exception::throw_if(m.type != static_cast<uint32_t>(Operand::Type::Evaluation), __FILE__, __LINE__,
                                   "Trip instruction requires the second operand to be an Evaluation register");
    }
    friend std::ostream& operator<<(std::ostream& os, Trip t) {
        os << "trip " << ImmediateType{t.imm};
        if (t.cond) {
            os << " : " << Operand{Operand::Type::Evaluation, t.eval} << ", "
               << Operand{Operand::Type::Evaluation, t.mask};
        }
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;  ///< Evaluation Register to check for conditional trip (if cond == 1)
    uint32_t mask : CountOfEvalIndexBits;  ///< Mask for the evaluation register
    uint32_t cond : 1;                     ///< If == 1, then the trip will be conditional on the evaluation and mask.
    uint32_t : CountOfDataBits - CountOfOperatorBits - (2 * CountOfEvalIndexBits) - ImmediateType::Bits - 1;
    uint32_t imm : ImmediateType::Bits;  ///< The immediate value of the exception type to trip.
};

/// Compares two scratch registers and sets flags in the Evaluation Register based on the result of the comparison.
struct Compare {
    constexpr Compare() : op{Operator::Compare}, e{0}, a{0}, b{0}, s{0}, et{EvaluationType::Comparison} {
    }

    constexpr Compare(Operand e, Operand a, Operand b, bool signed_comparison = false,
                      EvaluationType evaluation_type = EvaluationType::Comparison)
        : op{Operator::Compare}
        , e{e.index}
        , a{a.index}
        , b{b.index}
        , s{signed_comparison ? 1U : 0U}
        , et{evaluation_type} {
        basal::exception::throw_if(
            evaluation_type == EvaluationType::None, __FILE__, __LINE__,
            "Compare instruction requires a valid evaluation type (Logical, Arithmetic, or Floating)");
        basal::exception::throw_if(e.type != static_cast<uint32_t>(Operand::Type::Evaluation), __FILE__, __LINE__,
                                   "Compare instruction requires the first operand to be an Evaluation register");
        basal::exception::throw_if(a.type != static_cast<uint32_t>(Operand::Type::Scratch), __FILE__, __LINE__,
                                   "Compare instruction requires the second operand to be a Scratch register");
        basal::exception::throw_if(b.type != static_cast<uint32_t>(Operand::Type::Scratch), __FILE__, __LINE__,
                                   "Compare instruction requires the third operand to be a Scratch register");
    }

    friend std::ostream& operator<<(std::ostream& os, Compare c) {
        os << "comp " << Operand{Operand::Type::Scratch, c.a} << ", " << Operand{Operand::Type::Scratch, c.b} << " : "
           << Operand{Operand::Type::Evaluation, c.e};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t e : CountOfEvalIndexBits;     ///< Evaluation Register to set flags in
    uint32_t a : CountOfScratchIndexBits;  ///< Scratch Register A
    uint32_t b : CountOfScratchIndexBits;  ///< Scratch Register B
    uint32_t s : 1;                        ///< If == 1, then the comparison is signed, otherwise it is unsigned.
    EvaluationType et : 2;                 ///< Evaluation Type (Logical, Arithmetic, Floating)
    uint32_t : CountOfDataBits - CountOfOperatorBits - (3 * CountOfScratchIndexBits) - 3;
};

/// One argument bitwise operations (Complement, Rsh, Lsh, Count, Leading Zeros, etc)
class Bitwise1 {
public:
    using ImmediateType = isa::Immediate<5>;
    Bitwise1() = delete;

    constexpr static Bitwise1 Complement(Operand dst, Operand src) {
        return Bitwise1(Operator::Complement, dst, src);
    }

    constexpr static Bitwise1 Rsh(Operand dst, Operand src, ImmediateType shift) {
        return Bitwise1(Operator::Rsh, dst, src, shift);
    }

    constexpr static Bitwise1 Asr(Operand dst, Operand src, ImmediateType shift) {
        return Bitwise1(Operator::Asr, dst, src, shift);
    }

    constexpr static Bitwise1 Rotate(Operand dst, Operand src, ImmediateType shift, bool right = true) {
        Bitwise1 b(Operator::Rotate, dst, src, shift, right);
        return b;
    }

    constexpr static Bitwise1 Lsh(Operand dst, Operand src, ImmediateType shift) {
        return Bitwise1(Operator::Lsh, dst, src, shift);
    }

    constexpr static Bitwise1 Not(Operand dst, Operand src) {
        return Bitwise1(Operator::Not, dst, src);
    }

    constexpr static Bitwise1 Count(Operand dst, Operand src) {
        return Bitwise1(Operator::Count1s, dst, src);
    }

    constexpr static Bitwise1 LeadingZeros(Operand dst, Operand src) {
        return Bitwise1(Operator::CountL0s, dst, src);
    }

    constexpr static Bitwise1 Set(Operand dst, Operand src, ImmediateType shift) {
        return Bitwise1(Operator::SetBit, dst, src, shift);
    }

    constexpr static Bitwise1 Clear(Operand dst, Operand src, ImmediateType shift) {
        return Bitwise1(Operator::ClearBit, dst, src, shift);
    }

    constexpr static Bitwise1 Toggle(Operand dst, Operand src, ImmediateType shift) {
        return Bitwise1(Operator::ToggleBit, dst, src, shift);
    }

    constexpr static Bitwise1 ReverseBytes(Operand dst, Operand src) {
        return Bitwise1(Operator::Reverse, dst, src, ImmediateType{0});
    }

    constexpr static Bitwise1 ReverseHalf(Operand dst, Operand src) {
        return Bitwise1(Operator::Reverse, dst, src, ImmediateType{1});
    }

    constexpr static Bitwise1 ReverseWord(Operand dst, Operand src) {
        return Bitwise1(Operator::Reverse, dst, src, ImmediateType{2});
    }

    friend std::ostream& operator<<(std::ostream& os, Bitwise1 b) {
        if (b.op == Operator::Complement) {
            os << "bcpl ";
        } else if (b.op == Operator::Rsh) {
            os << "brsh ";
        } else if (b.op == Operator::Lsh) {
            os << "blsh ";
        } else if (b.op == Operator::Asr) {
            os << "basr ";
        } else if (b.op == Operator::Rotate) {
            os << "brot." << (b.dir ? "r " : "l ");
        } else if (b.op == Operator::Not) {
            os << "bnot ";
        } else if (b.op == Operator::Count1s) {
            os << "bcnt ";
        } else if (b.op == Operator::CountL0s) {
            os << "bclz ";
        } else if (b.op == Operator::SetBit) {
            os << "bset ";
        } else if (b.op == Operator::ClearBit) {
            os << "bclr ";
        } else if (b.op == Operator::ToggleBit) {
            os << "btgl ";
        } else if (b.op == Operator::Reverse) {
            os << "brev ";
        } else {
            os << "??? ";
        }
        os << Operand{Operand::Type::Scratch, b.dst} << ", " << Operand{Operand::Type::Scratch, b.src};
        if (b.op == Operator::Rsh || b.op == Operator::Lsh || b.op == Operator::Asr
            || (b.op == Operator::Rotate && b.shift > 0)) {
            os << ", " << b.shift;
        }
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfScratchIndexBits;  ///< Destination Scratch Register
    uint32_t src : CountOfScratchIndexBits;  ///< Source Scratch Register
    uint32_t eval : CountOfEvalIndexBits;    ///< Evaluation Register to check for conditional operation (if cond == 1)
    uint32_t shift : ImmediateType::Bits;    ///< Shift Amount (for shift instructions)
    uint32_t cond : 1;                       ///< If == 1, then the operation will set the evaluation register
    uint32_t dir : 1;  ///< Direction for Shift/Rotate (0 = Left, 1 = Right) (only used for shift instructions)
    uint32_t : CountOfDataBits - CountOfOperatorBits - (3 * CountOfScratchIndexBits) - ImmediateType::Bits - 2;

protected:
    constexpr Bitwise1(Operator op, Operand dst, Operand src, ImmediateType shift = ImmediateType{0}, bool right = true)
        : op{op}, dst{dst.index}, src{src.index}, eval{0}, shift{shift.value}, cond{0}, dir{right ? 1U : 0U} {
        basal::exception::throw_unless(dst.type == static_cast<uint32_t>(Operand::Type::Scratch)
                                           && src.type == static_cast<uint32_t>(Operand::Type::Scratch),
                                       __FILE__, __LINE__,
                                       "Bitwise1 instruction requires both operands to be Scratch registers");
    }
};

/// Two argument bitwise operations (AND, OR, XOR)
class Bitwise2 {
public:
    Bitwise2() = delete;

    constexpr static Bitwise2 And(Operand dst, Operand src1, Operand src2) {
        return Bitwise2(Operator::And, dst, src1, src2);
    }

    constexpr static Bitwise2 Or(Operand dst, Operand src1, Operand src2) {
        return Bitwise2(Operator::Or, dst, src1, src2);
    }

    constexpr static Bitwise2 Xor(Operand dst, Operand src1, Operand src2) {
        return Bitwise2(Operator::Xor, dst, src1, src2);
    }

    friend std::ostream& operator<<(std::ostream& os, Bitwise2 a) {
        if (a.op == Operator::And) {
            os << "band ";
        } else if (a.op == Operator::Or) {
            os << "borr ";
        } else if (a.op == Operator::Xor) {
            os << "bxor ";
        } else {
            os << "??? ";
        }
        os << Operand{Operand::Type::Scratch, a.dst} << ", " << Operand{Operand::Type::Scratch, a.src1} << ", "
           << Operand{Operand::Type::Scratch, a.src2};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t src1 : CountOfScratchIndexBits;  ///< Source Scratch Register 1
    uint32_t src2 : CountOfScratchIndexBits;  ///< Source Scratch Register 2
    uint32_t : CountOfDataBits - CountOfOperatorBits - (3 * CountOfScratchIndexBits);

protected:
    constexpr Bitwise2(Operator op, Operand dst, Operand src1, Operand src2)
        : op{op}, dst{dst.index}, src1{src1.index}, src2{src2.index} {
        basal::exception::throw_unless(dst.type == static_cast<uint32_t>(Operand::Type::Scratch)
                                           && src1.type == static_cast<uint32_t>(Operand::Type::Scratch)
                                           && src2.type == static_cast<uint32_t>(Operand::Type::Scratch),
                                       __FILE__, __LINE__,
                                       "Bitwise2 instruction requires all operands to be Scratch registers");
    }
};

class Arithmetic {
public:
    enum class Size : uint32_t {
        Byte = 0,
        HalfWord = 1,
        Word = 2,
        DoubleWord = 3,
    };

    constexpr Arithmetic() = delete;

    constexpr static Arithmetic Addu(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Addition, dst, src1, src2, size, false, false);
    }

    constexpr static Arithmetic Adds(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Addition, dst, src1, src2, size, true, false);
    }

    constexpr static Arithmetic AdduSat(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Addition, dst, src1, src2, size, false, true);
    }

    constexpr static Arithmetic AddsSat(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Addition, dst, src1, src2, size, true, true);
    }

    constexpr static Arithmetic Subu(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Subtract, dst, src1, src2, size, false, false);
    }

    constexpr static Arithmetic Subs(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Subtract, dst, src1, src2, size, true, false);
    }

    constexpr static Arithmetic SubuSat(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Subtract, dst, src1, src2, size, false, true);
    }

    constexpr static Arithmetic SubsSat(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Subtract, dst, src1, src2, size, true, true);
    }

    constexpr static Arithmetic Mulu(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Multiply, dst, src1, src2, size, false, false);
    }

    constexpr static Arithmetic Muls(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Multiply, dst, src1, src2, size, true, false);
    }

    constexpr static Arithmetic MuluSat(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Multiply, dst, src1, src2, size, false, true);
    }

    constexpr static Arithmetic MulsSat(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Multiply, dst, src1, src2, size, true, true);
    }

    constexpr static Arithmetic Divu(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Divide, dst, src1, src2, size, false, false);
    }

    constexpr static Arithmetic Divs(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Divide, dst, src1, src2, size, true, false);
    }

    constexpr static Arithmetic DivuSat(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Divide, dst, src1, src2, size, false, true);
    }

    constexpr static Arithmetic DivsSat(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Divide, dst, src1, src2, size, true, true);
    }

    constexpr static Arithmetic Modu(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Modulo, dst, src1, src2, size, false, false);
    }

    constexpr static Arithmetic Mods(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Modulo, dst, src1, src2, size, true, false);
    }

    constexpr static Arithmetic ModuSat(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Modulo, dst, src1, src2, size, false, true);
    }

    constexpr static Arithmetic ModsSat(Operand dst, Operand src1, Operand src2, Size size = Size::Word) {
        return Arithmetic(Operator::Modulo, dst, src1, src2, size, true, true);
    }

    friend std::ostream& operator<<(std::ostream& os, Arithmetic a) {
        if (a.op == Operator::Addition) {
            os << "add";
        } else if (a.op == Operator::Subtract) {
            os << "sub";
        } else if (a.op == Operator::Multiply) {
            os << "mul";
        } else if (a.op == Operator::Divide) {
            os << "div";
        } else if (a.op == Operator::Modulo) {
            os << "mod";
        } else {
            os << "???";
        }
        os << (a.sat ? "s" : "u");
        os << Operand{Operand::Type::Scratch, a.dst} << ", " << Operand{Operand::Type::Scratch, a.src1} << ", "
           << Operand{Operand::Type::Scratch, a.src2};
        if (a.cond) {
            os << " : " << Operand{Operand::Type::Evaluation, a.eval};
        }
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t src1 : CountOfScratchIndexBits;  ///< Source Scratch Register 1
    uint32_t src2 : CountOfScratchIndexBits;  ///< Source Scratch Register 2
    uint32_t eval : CountOfEvalIndexBits;     ///< Evaluation Register to check for conditional operation (if cond == 1)
    uint32_t cond : 1;                        ///< If == 1, then the operation will set the evaluation register
    uint32_t sign : 1;                        ///< If == 1, then the operation will be signed
    uint32_t sat : 1;                         ///< If == 1, then the operation will be saturating
    uint32_t : CountOfDataBits - CountOfOperatorBits - (3 * CountOfScratchIndexBits) - CountOfEvalIndexBits - 5;
    uint32_t size : 2;  ///< Size of the operation (0 = 8-bit, 1 = 16-bit, 2 = 32-bit, 3 = reserved)
protected:
    constexpr Arithmetic(Operator op, Operand dst, Operand src1, Operand src2, Size size, bool sign = false,
                         bool sat = false)
        : op{op}
        , dst{dst.index}
        , src1{src1.index}
        , src2{src2.index}
        , eval{0}
        , cond{0}
        , sign{sign ? 1U : 0U}
        , sat{sat ? 1U : 0U}
        , size{static_cast<uint32_t>(size)} {
        basal::exception::throw_unless(dst.type == static_cast<uint32_t>(Operand::Type::Scratch)
                                           && src1.type == static_cast<uint32_t>(Operand::Type::Scratch)
                                           && src2.type == static_cast<uint32_t>(Operand::Type::Scratch),
                                       __FILE__, __LINE__,
                                       "Arithmetic instruction requires all operands to be Scratch registers");
    }
    constexpr Arithmetic(Operator op, Operand dst, Operand src1, Operand src2, Operand eval, Size size,
                         bool sign = false, bool sat = false)
        : op{op}
        , dst{dst.index}
        , src1{src1.index}
        , src2{src2.index}
        , eval{eval.index}
        , cond{1}
        , sign{sign ? 1U : 0U}
        , sat{sat ? 1U : 0U}
        , size{static_cast<uint32_t>(size)} {
        basal::exception::throw_if(
            eval.type != static_cast<uint32_t>(Operand::Type::Evaluation), __FILE__, __LINE__,
            "Arithmetic instruction with evaluation requires the evaluation operand to be an Evaluation register");
        basal::exception::throw_unless(dst.type == static_cast<uint32_t>(Operand::Type::Scratch)
                                           && src1.type == static_cast<uint32_t>(Operand::Type::Scratch)
                                           && src2.type == static_cast<uint32_t>(Operand::Type::Scratch),
                                       __FILE__, __LINE__,
                                       "Arithmetic instruction requires all operands to be Scratch registers");
    }
};

/// Unary Precision (Floating) Operations
class Precision1 {
public:
    Precision1() = delete;

    constexpr static Precision1 FAbs(Operand dst, Operand src) {
        return Precision1(Operator::FloatingAbs, dst, src);
    }

    constexpr static Precision1 FNeg(Operand dst, Operand src) {
        return Precision1(Operator::FloatingNegate, dst, src);
    }

    constexpr static Precision1 Floor(Operand dst, Operand src) {
        return Precision1(Operator::FloatingFloor, dst, src);
    }

    constexpr static Precision1 Ceil(Operand dst, Operand src) {
        return Precision1(Operator::FloatingCeil, dst, src);
    }

    constexpr static Precision1 Convert(Operand dst, Operand src, bool float_to_int = true) {
        Precision1 p(Operator::FloatingConvert, dst, src);
        p.dir = float_to_int ? 1U : 0U;
        return p;
    }

    constexpr static Precision1 Fractional(Operand dst, Operand src) {
        return Precision1(Operator::FloatingFractional, dst, src);
    }

    friend std::ostream& operator<<(std::ostream& os, Precision1 p) {
        if (p.op == Operator::FloatingAbs) {
            os << "fabs ";
        } else if (p.op == Operator::FloatingNegate) {
            os << "fneg ";
        } else if (p.op == Operator::FloatingFloor) {
            os << "fflr ";
        } else if (p.op == Operator::FloatingCeil) {
            os << "fcel ";
        } else if (p.op == Operator::FloatingConvert) {
            os << "fcvt" << (p.dir ? ".f2i " : ".i2f ");
        } else if (p.op == Operator::FloatingFractional) {
            os << "ffrc ";
        } else {
            os << "??? ";
        }
        os << Operand{Operand::Type::Scratch, p.dst} << ", " << Operand{Operand::Type::Scratch, p.src};
        return os;
    }

    Operator op : CountOfOperatorBits;  ///< The specific precision operation to perform (e.g. fabs, fneg, fflr, fcel,
                                        ///< fcvt, etc)
    uint32_t dst : CountOfScratchIndexBits;  ///< Destination Scratch Register
    uint32_t src : CountOfScratchIndexBits;  ///< Source Scratch Register
    uint32_t dir : 1;  ///< Direction for conversion (0 = Int to Float, 1 = Float to Int) (only used for fcvt)
    uint32_t : CountOfDataBits - CountOfOperatorBits - (2 * CountOfScratchIndexBits) - 1;

protected:
    constexpr Precision1(Operator op, Operand dst, Operand src) : op{op}, dst{dst.index}, src{src.index}, dir{0} {
        basal::exception::throw_unless(dst.type == static_cast<uint32_t>(Operand::Type::Scratch)
                                           && src.type == static_cast<uint32_t>(Operand::Type::Scratch),
                                       __FILE__, __LINE__,
                                       "Precision1 instruction requires both operands to be Scratch registers");
    }
};

class Precision2 {
public:
    Precision2() = delete;

    constexpr static Precision2 FAdd(Operand dst, Operand src1, Operand src2) {
        return Precision2(Operator::FloatingAddition, dst, src1, src2);
    }

    constexpr static Precision2 FAdd(Operand dst, Operand src1, Operand src2, Operand eval) {
        return Precision2(Operator::FloatingAddition, dst, src1, src2, eval);
    }

    constexpr static Precision2 FSub(Operand dst, Operand src1, Operand src2) {
        return Precision2(Operator::FloatingSubtraction, dst, src1, src2);
    }

    constexpr static Precision2 FSub(Operand dst, Operand src1, Operand src2, Operand eval) {
        return Precision2(Operator::FloatingSubtraction, dst, src1, src2, eval);
    }

    constexpr static Precision2 FMul(Operand dst, Operand src1, Operand src2) {
        return Precision2(Operator::FloatingMultiplication, dst, src1, src2);
    }

    constexpr static Precision2 FMul(Operand dst, Operand src1, Operand src2, Operand eval) {
        return Precision2(Operator::FloatingMultiplication, dst, src1, src2, eval);
    }

    constexpr static Precision2 FDiv(Operand dst, Operand src1, Operand src2) {
        return Precision2(Operator::FloatingDivision, dst, src1, src2);
    }

    constexpr static Precision2 FDiv(Operand dst, Operand src1, Operand src2, Operand eval) {
        return Precision2(Operator::FloatingDivision, dst, src1, src2, eval);
    }

    friend std::ostream& operator<<(std::ostream& os, Precision2 p) {
        if (p.op == Operator::FloatingAddition) {
            os << "fadd ";
        } else if (p.op == Operator::FloatingSubtraction) {
            os << "fsub ";
        } else if (p.op == Operator::FloatingMultiplication) {
            os << "fmul ";
        } else if (p.op == Operator::FloatingDivision) {
            os << "fdiv ";
        } else {
            os << "??? ";
        }
        os << Operand{Operand::Type::Scratch, p.dst} << ", " << Operand{Operand::Type::Scratch, p.src1} << ", "
           << Operand{Operand::Type::Scratch, p.src2};
        if (p.cond) {
            os << " : " << Operand{Operand::Type::Evaluation, p.eval};
        }
        return os;
    }

    Operator op
        : CountOfOperatorBits;  ///< The specific precision operation to perform (e.g. fadd, fsub, fmul, fdiv, etc)
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t src1 : CountOfScratchIndexBits;  ///< Source Scratch Register
    uint32_t src2 : CountOfScratchIndexBits;  ///< Source Scratch Register 2
    uint32_t eval : CountOfEvalIndexBits;     ///< Evaluation Register to check for conditional operation (if cond == 1)
    uint32_t cond : 1;                        ///< If == 1, then the operation
    uint32_t : CountOfDataBits - CountOfOperatorBits - (3 * CountOfScratchIndexBits) - CountOfEvalIndexBits - 1;

protected:
    constexpr Precision2(Operator op, Operand dst, Operand src1, Operand src2)
        : op{op}, dst{dst.index}, src1{src1.index}, src2{src2.index}, eval{0}, cond{0} {
        basal::exception::throw_unless(dst.type == static_cast<uint32_t>(Operand::Type::Scratch)
                                           && src1.type == static_cast<uint32_t>(Operand::Type::Scratch)
                                           && src2.type == static_cast<uint32_t>(Operand::Type::Scratch),
                                       __FILE__, __LINE__,
                                       "Precision2 instruction requires all operands to be Scratch registers");
    }

    constexpr Precision2(Operator op, Operand dst, Operand src1, Operand src2, Operand eval)
        : op{op}, dst{dst.index}, src1{src1.index}, src2{src2.index}, eval{eval.index}, cond{1} {
        basal::exception::throw_unless(dst.type == static_cast<uint32_t>(Operand::Type::Scratch)
                                           && src1.type == static_cast<uint32_t>(Operand::Type::Scratch)
                                           && src2.type == static_cast<uint32_t>(Operand::Type::Scratch),
                                       __FILE__, __LINE__,
                                       "Precision2 instruction requires all operands to be Scratch registers");
        basal::exception::throw_if(
            eval.type != static_cast<uint32_t>(Operand::Type::None)
                && eval.type != static_cast<uint32_t>(Operand::Type::Evaluation),
            __FILE__, __LINE__,
            "Precision2 instruction requires the evaluation operand to be either None or an Evaluation register");
    }
};

union Instruction {
    /// Default Constructor
    constexpr Instruction() : noop{} {
    }
    /// Copy Constructor
    constexpr Instruction(Instruction const& other) : base{other.base} {
    }
    /// Move Constructor
    constexpr Instruction(Instruction&& other) : base{std::move(other.base)} {
    }
    /// Copy Assignment
    Instruction& operator=(Instruction const& other) {
        base = other.base;
        return *this;
    }
    /// Move Assignment
    Instruction& operator=(Instruction&& other) {
        base = std::move(other.base);
        return *this;
    }
    /// Raw Value Constructor (DATA in instruction streams)
    constexpr Instruction(uint32_t raw) : raw{raw} {
    }
    /// Typed Constructor
    constexpr Instruction(NoOp) : noop{} {
    }
    /// Typed Constructor
    constexpr Instruction(Halt) : halt{} {
    }
    /// Typed Constructor
    constexpr Instruction(Breakpoint b) : breakpoint{b} {
    }
    /// Typed Constructor for Copy
    constexpr Instruction(Copy c) : copy{c} {
    }
    /// Typed Constructor
    constexpr Instruction(Move m) : move{m} {
    }
    /// Typed Constructor
    constexpr Instruction(MoveImmediate mi) : movi{mi} {
    }
    /// Typed Constructor
    constexpr Instruction(Swap s) : swap{s} {
    }
    /// Typed Constructor
    constexpr Instruction(Zero c) : clear{c} {
    }
    /// Typed Constructor for Load
    constexpr Instruction(Load l) : loads{l} {
    }
    /// Typed Constructor for Store
    constexpr Instruction(Save s) : save{s} {
    }
    /// Typed Constructor for Leap
    constexpr Instruction(Leap j) : leap{j} {
    }
    /// Typed Constructor for Immediate Leap
    constexpr Instruction(LeapImmediate j) : leapi{j} {
    }
    /// Typed Constructor for Back
    constexpr Instruction(Back b) : back{b} {
    }
    /// Typed Constructor for Grow
    constexpr Instruction(Grow g) : grow{g} {
    }
    /// Typed Constructor for Undo
    constexpr Instruction(Undo u) : undo{u} {
    }
    /// Typed Constructor for Call
    constexpr Instruction(Call c) : call{c} {
    }
    /// Typed Constructor for Trip
    constexpr Instruction(Trip t) : trip{t} {
    }
    /// Typed Constructor for Compare
    constexpr Instruction(Compare c) : compare{c} {
    }
    /// Typed Constructor for Bitwise1
    constexpr Instruction(Bitwise1 n) : bitwise1{n} {
    }
    /// Typed Constructor for Bitwise2
    constexpr Instruction(Bitwise2 x) : bitwise2{x} {
    }
    /// Typed Constructor for Arithmetic
    constexpr Instruction(Arithmetic a) : arithmetic{a} {
    }
    /// Typed Constructor for Precision1
    constexpr Instruction(Precision1 p) : precision1{p} {
    }
    /// Typed Constructor for Precision2
    constexpr Instruction(Precision2 p) : precision2{p} {
    }
    //=================================
    uint32_t raw;           ///< The raw bits of the instruction as it would be stored in memory
    Base base;              ///< The base instruction for decoding the operator
    NoOp noop;              ///< No Operation
    Halt halt;              ///< Halt the processor
    Breakpoint breakpoint;    ///< Breakpoint for debugging (not a real instruction, used for tooling)
    Copy copy;              ///< Copy from one evaluation register to another
    Move move;              ///< Move from scratch to scratch
    MoveImmediate movi;     ///< Move immediate to scratch/evaluation register
    Swap swap;              ///< Swap registers based on reg_type
    Zero clear;             ///< Zero registers based on reg_type and mask
    Load loads;             ///< Load from memory to scratch register
    Save save;              ///< Saves a value from scratch register to memory
    Leap leap;              ///< Leap to an address based on an evaluation and mask
    LeapImmediate leapi;    ///< Absolute immediate leap with optional return address save
    Back back;              ///< Returns from a Leap
    Grow grow;              ///< Grow the memory by a number of pages specified in an evaluation register
    Undo undo;              ///< Undo the last memory growth
    Call call;              ///< Call a subroutine at an address specified in an evaluation register
    Trip trip;              ///< Trip to an address specified in an evaluation register and save the return
    Compare compare;        ///< Compare two scratch registers and set flags in an evaluation register
    Bitwise2 bitwise2;      ///< Holds the 2 argument bitwise operations (AND, OR, XOR)
    Bitwise1 bitwise1;      ///< Holds the 1 argument bitwise operations
    Arithmetic arithmetic;  ///< Holds the arithmetic operations (ADD, SUB, MUL, DIV, MOD)
    Precision1 precision1;  ///< Holds the unary precision operations (FAbs, FNeg, Floor, Ceil, Convert)
    Precision2 precision2;  ///< Holds the binary precision operations (FAdd, FSub, FMul, FDiv)
    //=================================
    friend std::ostream& operator<<(std::ostream& os, Instruction instr) {
        if (instr.base() == Operator::None) {
            os << instr.noop;
        } else if (instr.base() == Operator::Halt) {
            os << instr.halt;
        } else if (instr.base() == Operator::Breakpoint) {
            os << instr.breakpoint;
        } else if (instr.base() == Operator::Copy) {
            os << instr.copy;
        } else if (instr.base() == Operator::Move) {
            os << instr.move;
        } else if (instr.base() == Operator::MoveImmediate) {
            os << instr.movi;
        } else if (instr.base() == Operator::Swap) {
            os << instr.swap;
        } else if (instr.base() == Operator::Zero) {
            os << instr.clear;
        } else if (instr.base() == Operator::Load) {
            os << instr.loads;
        } else if (instr.base() == Operator::Save) {
            os << instr.save;
        } else if (instr.base() == Operator::Leap) {
            if ((instr.raw & ImmediateLeapBitMask) != 0U) {
                os << instr.leapi;
            } else {
                os << instr.leap;
            }
        } else if (instr.base() == Operator::Back) {
            os << instr.back;
        } else if (instr.base() == Operator::Grow) {
            os << instr.grow;
        } else if (instr.base() == Operator::Undo) {
            os << instr.undo;
        } else if (instr.base() == Operator::Call) {
            os << instr.call;
        } else if (instr.base() == Operator::Trip) {
            os << instr.trip;
        } else if (instr.base() == Operator::Compare) {
            os << instr.compare;
        } else if (instr.base() == Operator::Complement) {
            os << instr.bitwise1;
        } else if (instr.base() == Operator::Rsh) {
            os << instr.bitwise1;
        } else if (instr.base() == Operator::Asr) {
            os << instr.bitwise1;
        } else if (instr.base() == Operator::Rotate) {
            os << instr.bitwise1;
        } else if (instr.base() == Operator::Lsh) {
            os << instr.bitwise1;
        } else if (instr.base() == Operator::Count1s) {
            os << instr.bitwise1;
        } else if (instr.base() == Operator::CountL0s) {
            os << instr.bitwise1;
        } else if (instr.base() == Operator::SetBit) {
            os << instr.bitwise1;
        } else if (instr.base() == Operator::ClearBit) {
            os << instr.bitwise1;
        } else if (instr.base() == Operator::ToggleBit) {
            os << instr.bitwise1;
        } else if (instr.base() == Operator::Reverse) {
            os << instr.bitwise1;
        } else if (instr.base() == Operator::And) {
            os << instr.bitwise2;
        } else if (instr.base() == Operator::Or) {
            os << instr.bitwise2;
        } else if (instr.base() == Operator::Xor) {
            os << instr.bitwise2;
        } else if (instr.base() == Operator::Addition) {
            os << instr.arithmetic;
        } else if (instr.base() == Operator::Subtract) {
            os << instr.arithmetic;
        } else if (instr.base() == Operator::Multiply) {
            os << instr.arithmetic;
        } else if (instr.base() == Operator::Divide) {
            os << instr.arithmetic;
        } else if (instr.base() == Operator::Modulo) {
            os << instr.arithmetic;
        } else if (instr.base() == Operator::FloatingAbs) {
            os << instr.precision1;
        } else if (instr.base() == Operator::FloatingNegate) {
            os << instr.precision1;
        } else if (instr.base() == Operator::FloatingFloor) {
            os << instr.precision1;
        } else if (instr.base() == Operator::FloatingCeil) {
            os << instr.precision1;
        } else if (instr.base() == Operator::FloatingConvert) {
            os << instr.precision1;
        } else if (instr.base() == Operator::FloatingFractional) {
            os << instr.precision1;
        } else if (instr.base() == Operator::FloatingAddition) {
            os << instr.precision2;
        } else if (instr.base() == Operator::FloatingSubtraction) {
            os << instr.precision2;
        } else if (instr.base() == Operator::FloatingMultiplication) {
            os << instr.precision2;
        } else if (instr.base() == Operator::FloatingDivision) {
            os << instr.precision2;
        } else {
            os << "???";
        }
        return os;
    }
};
static_assert(sizeof(Instruction) == sizeof(uint32_t), "Must be this size");

}  // namespace instructions

/// A program is an ordered sequence of instructions
using program = std::vector<instructions::Instruction>;

}  // namespace isa