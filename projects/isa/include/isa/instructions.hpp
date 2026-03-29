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
    Move,  ///< move destination, Source

    MoveImmediateToScratch,     ///< move immediate to scratch
    MoveImmediateToEvaluation,  ///< move immediate to evaluation

    Swap,  ///< swap registerA, registerB; reg_type (0=scratch, 1=evaluation)
    Zero,  ///< clear registers using mask; reg_type (0=scratch, 1=evaluation)

    // === Special Register Manipulation ===
    Leap,    ///< leap to address in scratch with optional immediate offset and condition
    Back,    ///< back from subroutine to address in Return Address Special Register
    Grow,    ///< grow #imm<16> (mask of scratch)
    Undo,    ///< undo #imm<16> (mask of scratch)
    Call,    ///< call #imm<16> (System Call Number)
    Trip,    ///< Trip #imm<16> (Trip an exception with the given exception type encoded in the immediate value. This can be used to test triggering interrupts)

    // === Memory Operations ===
    Load,    ///< load scratchDestination, scratchAddress
    Save,    ///< store scratchSource, scratchAddress

    // === Comparison ===
    Compare,  ///< Compare scratchA, scratchB

    // === Bit Operators ===
    And,    ///< band scratchDestination, scratchSource, scratchSource
    Or,     ///< bor  scratchDestination, scratchSource, scratchSource
    Xor,    ///< bxor scratchDestination, scratchSource, scratchSource
    Complement,    ///< bcmpl scratchDestination, scratchSource
    Rsh,    ///< brsh scratchDestination, scratchSource, #imm<5>
    Lsh,    ///< blsh scratchDestination, scratchSource, #imm<5>
    Rotate, ///< brot scratchDestination, scratchSource, #imm<5>
    Count,  ///< bcount scratchDestination, scratchSource
    // === ALU (Integer) ===
    Add,               ///< {s/u}{s}add.{type}{size} Es, Sd, Sa, Sb

    SignedAdd,         ///< sadd scratchDestination, scratchSource, scratchSource -> OverFlow
    SignedSub,         ///< ssub scratchDestination, scratchSource, scratchSource -> Underflow
    SignedMultiply,    ///< smult scratchDestination, scratchSource, scratchSource -> Overflow or Underflow
    SignedDivide,      ///< sdiv  scratchDestination, scratchSource, scratchSource -> Fault if scratchSource == 0
    SignedModulo,      ///< smod scratchDestination, scratchSource, scratchSource -> Fault if scratchSource == 0
    UnsignedAdd,       ///< uadd scratchDestination, scratchSource, scratchSource -> OverFlow
    UnsignedSub,       ///< usub scratchDestination, scratchSource, scratchSource -> Underflow
    UnsignedMultiply,  ///< umult scratchDestination, scratchSource, scratchSource -> Overflow
    UnsignedDivide,    ///< udiv  scratchDestination, scratchSource, scratchSource ->
    UnsignedModulo,    ///< umod scratchDestination, scratchSource, scratchSource -> Fault if scratchSource == 0
    // === ALU (Precision) ===
    Fadd,       ///< fadd scratchDestination, scratchSource, scratchSource -> Overflow or Underflow
    Fsub,       ///< fsub scratchDestination, scratchSource, scratchSource -> Overflow or Underflow
    Fmultiply,  ///< fmult scratchDestination, scratchSource, scratchSource -> Overflow or Underflow
    Fdivide,    ///< fdiv  scratchDestination, scratchSource, scratchSource -> Fault if scratchSource == 0
    floor,      ///< ffloor scratchDestination, scratchSource
    ceil,       ///< fceil  scratchDestination, scratchSource

    // === ALU (SIMD) ===

    Halt = 0xFFU  ///< Halts the processor.
    // === NO INSTRUCTIONS PAST THIS POINT ===
};
constexpr size_t CountOfOperatorBits{8u};

/// @section Calling_Convention Calling Conventions for Instructions
/// A call

/// The type of the Operand
enum class OperandType : uint32_t {
    None = 0,
    Scratch = 1,
    Evaluation = 2,
    Mask = 3,
};

struct Operand {
    constexpr Operand(OperandType t, uint32_t r) : index{r}, type{static_cast<uint32_t>(t)}, imm{0} {
    }
    constexpr Operand(OperandType t, Immediate<16> imm) : index{0}, type{static_cast<uint32_t>(t)}, imm{imm.value} {
    }
    uint32_t index : 4;
    uint32_t type : 2;
    uint32_t imm : 16;
    uint32_t : 10;

    friend std::ostream& operator<<(std::ostream& os, Operand o) {
        OperandType t = static_cast<OperandType>(o.type);
        if (t == OperandType::None) {
            os << "R" << o.index;
        } else if (t == OperandType::Scratch) {
            os << "S" << o.index;
        } else if (t == OperandType::Evaluation) {
            os << "E" << o.index;
        } else if (t == OperandType::Mask) {
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

struct Copy {
    Copy() = delete;
    constexpr Copy(Operand dst, Operand src)
        : op{Operator::None}, dst{dst.index}, src{src.index}, eval{0}, cond{0} {
        basal::exception::throw_unless(
            dst.type == to_underlying(OperandType::Scratch) and src.type == to_underlying(OperandType::Scratch),
            __FILE__, __LINE__, "Copy instruction requires both source and destination operands to be Scratch registers");
    }
    constexpr Copy(Operand dst, Operand src, Operand eval)
        : op{Operator::None}, dst{dst.index}, src{src.index}, eval{eval.index}, cond{1} {
        basal::exception::throw_unless(
            dst.type == to_underlying(OperandType::Scratch) and src.type == to_underlying(OperandType::Scratch),
            __FILE__, __LINE__, "Copy instruction requires both source and destination operands to be Scratch registers");
    }
    friend std::ostream& operator<<(std::ostream& os, Copy c) {
        os << "copy " << Operand{OperandType::Scratch, c.dst} << ", " << Operand{OperandType::Scratch, c.src};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfScratchIndexBits;
    uint32_t src : CountOfScratchIndexBits;
    uint32_t eval : CountOfEvalIndexBits;
    uint32_t cond : 1; ///< If == 1, then the copy will set the evaluation register
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
        , type{dst.type == static_cast<uint32_t>(OperandType::Evaluation)
                       && src.type == static_cast<uint32_t>(OperandType::Evaluation)
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
        , type{dst.type == static_cast<uint32_t>(OperandType::Evaluation)
                       && src.type == static_cast<uint32_t>(OperandType::Evaluation)
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
        , type{dst.type == static_cast<uint32_t>(OperandType::Evaluation)
                       && src.type == static_cast<uint32_t>(OperandType::Evaluation)
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
        , type{dst.type == static_cast<uint32_t>(OperandType::Evaluation)
                       && src.type == static_cast<uint32_t>(OperandType::Evaluation)
                   ? 1U
                   : 0U}
        , dir{shift_right ? 1U : 0U}
        , shift{shift_amount.value} {
    }

    friend std::ostream& operator<<(std::ostream& os, Move m) {
        if (m.cond) {
            os << "move " << Operand{OperandType::Evaluation, m.eval} << ", "
               << Operand{OperandType::Evaluation, m.mask} << ", ";
        } else {
            os << "move ";
        }
        os << Operand{OperandType::Scratch, m.dst} << ", " << Operand{OperandType::Scratch, m.src};
        if (m.shift > 0) {
            os << (m.dir ? " >> " : " << ") << m.shift;
        }
        return os;
    }

    const Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;       ///< Evaluation
    uint32_t mask : CountOfEvalIndexBits;       ///< Evaluation Mask
    uint32_t dst : CountOfScratchIndexBits;        ///< Operand
    uint32_t src : CountOfScratchIndexBits;        ///< Operand
    uint32_t cond : 1;                      ///< If == 1, then the move is conditional on the evaluation and mask.
    uint32_t type : 1;                      ///< 0 = Scratch to Scratch, 1 = Evaluation to Evaluation
    uint32_t dir : 1;                       ///< Direction for Shift (0 = Left, 1 = Right)
    uint32_t shift : CountOfDataShiftBits;  ///< Shift Amount (for shift instructions)
};

struct MoveImmediateToScratch {
    using ImmediateType = isa::Immediate<20>;
    MoveImmediateToScratch() = delete;
    constexpr MoveImmediateToScratch(Operand dst, ImmediateType imm)
        : op{Operator::MoveImmediateToScratch}, dst{dst.index}, imm{imm.value} {
        basal::exception::throw_if(
            dst.type != to_underlying(OperandType::Scratch), __FILE__, __LINE__,
            "MoveImmediateToScratch instruction requires the destination operand to be a Scratch register");
    }
    friend std::ostream& operator<<(std::ostream& os, MoveImmediateToScratch mi) {
        os << "move " << Operand{OperandType::Scratch, mi.dst} << ", " << ImmediateType{mi.imm};
        return os;
    }

    constexpr ImmediateType Immediate() const {
        return ImmediateType{imm};
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfScratchIndexBits;
    uint32_t imm : 20;
};

struct MoveImmediateToEvaluation {
    using ImmediateType = isa::Immediate<EvaluationTypeBits + EvaluationFlagBits>;
    MoveImmediateToEvaluation() = delete;

    constexpr MoveImmediateToEvaluation(Operand dst, ImmediateType imm)
        : op{Operator::MoveImmediateToEvaluation}, dst{dst.index}, imm{imm.value} {
        basal::exception::throw_if(
            dst.type != to_underlying(OperandType::Evaluation), __FILE__, __LINE__,
            "MoveImmediateToEvaluation instruction requires the destination operand to be an Evaluation register");
    }

    constexpr MoveImmediateToEvaluation(Operand dst, Evaluation eval)
        : op{Operator::MoveImmediateToEvaluation}, dst{dst.index}, imm{eval.value} {
        basal::exception::throw_if(
            dst.type != to_underlying(OperandType::Evaluation), __FILE__, __LINE__,
            "MoveImmediateToEvaluation instruction requires the destination operand to be an Evaluation register");
    }

    friend std::ostream& operator<<(std::ostream& os, MoveImmediateToEvaluation mi) {
        os << "move " << Operand{OperandType::Evaluation, mi.dst} << ", " << ImmediateType{mi.imm};
        return os;
    }

    constexpr ImmediateType Immediate() const {
        return ImmediateType{imm};
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfScratchIndexBits;
    uint32_t : CountOfDataBits - CountOfOperatorBits - CountOfScratchIndexBits - ImmediateType::Bits;
    uint32_t imm : ImmediateType::Bits;
};

struct Swap {
    Swap() = delete;
    constexpr Swap(Operand a, Operand b) : op{Operator::Swap}, a{a.index}, b{b.index}, type{0} {
        basal::exception::throw_unless(a.type == b.type, __FILE__, __LINE__,
                                       "Swap instruction requires both operands to be of the same type");
        basal::exception::throw_if(
            a.type == static_cast<uint32_t>(OperandType::None)
                || a.type > static_cast<uint32_t>(OperandType::Evaluation),
            __FILE__, __LINE__, "Swap instruction requires both operands to be either Scratch or Evaluation registers");
        type = (a.type == to_underlying(OperandType::Scratch)) ? RegisterType::Scratch : RegisterType::Evaluation;
    }

    friend std::ostream& operator<<(std::ostream& os, Swap s) {
        OperandType register_type = s.type == RegisterType::Scratch ? OperandType::Scratch : OperandType::Evaluation;
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
           << Operand{OperandType::Mask, ImmediateType{c.mask}};
        return os;
    }

    Operator op : CountOfOperatorBits;
    /// Each bit corresponds to a register. If the bit is set, the corresponding register will be cleared.
    uint32_t mask : ImmediateType::Bits;
    RegisterType type : 1;
    uint32_t : 7;
};

struct Load {
    using ImmediateType = isa::Immediate<14>;
    Load() = delete;
    constexpr Load(Operand dst, Operand base, ImmediateType imm, bool inc = false, bool off = false)
        : op{Operator::Load}
        , dst{dst.index}
        , base{base.index}
        , inc{inc ? 1U : 0U}
        , off{off ? 1U : 0U}
        , imm{imm.value} {
    }
    friend std::ostream& operator<<(std::ostream& os, Load l) {
        os << "load " << Operand{OperandType::Scratch, l.dst} << ", " << Operand{OperandType::Scratch, l.base};
        if (l.inc) {
            os << " += " << l.imm;
        }
        if (l.off) {
            os << " + " << l.imm;
        }
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t base : CountOfScratchIndexBits;  ///< Base Scratch Register containing the address
    uint32_t inc : 1;  ///< If set, the base scratch register will be incremented by the size of the load after the load
                       ///< is performed.
    uint32_t off : 1;  ///< If set, the base scratch register will be used as an offset from a base address in a special
                       ///< register instead of an absolute address.
    uint32_t imm : ImmediateType::Bits;  ///< The immediate value of the offset or increment in bytes.
};

struct Save {
    using ImmediateType = isa::Immediate<14>;
    Save() = delete;
    constexpr Save(Operand src, Operand base, ImmediateType imm, bool inc = false, bool off = false)
        : op{Operator::Save}
        , src{src.index}
        , base{base.index}
        , inc{inc ? 1U : 0U}
        , off{off ? 1U : 0U}
        , imm{imm.value} {
    }

    friend std::ostream& operator<<(std::ostream& os, Save s) {
        os << "store " << Operand{OperandType::Scratch, s.src} << ", " << Operand{OperandType::Scratch, s.base};
        if (s.inc) {
            os << " += " << s.imm;
        }
        if (s.off) {
            os << " + " << s.imm;
        }
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t src : CountOfScratchIndexBits;   ///< Source Scratch Register
    uint32_t base : CountOfScratchIndexBits;  ///< Base Scratch Register containing the address
    uint32_t inc : 1;                  ///< If set, the base scratch register will
                                       ///< be incremented by the size of the store after the store is performed.
    uint32_t off : 1;  ///< If set, the base scratch register will be used as an offset from a base address in a special
                       ///< register instead of an absolute address.
    uint32_t imm : ImmediateType::Bits;  ///< The immediate value of the offset or increment in bytes
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
        if (j.cond) {
            os << Operand{OperandType::Evaluation, j.eval} << ", " << Operand{OperandType::Evaluation, j.mask} << ", ";
        }
        os << Operand{OperandType::Scratch, j.dst} << " + (" << ImmediateType{j.imm} << " * 4)";
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;  ///< Evaluation Register to check
    uint32_t mask : CountOfEvalIndexBits;  ///< Mask for the evaluation register
    uint32_t dst : CountOfScratchIndexBits;   ///< Scratch Register containing the destination address
    uint32_t cond : 1;                 ///< If == 1, then the leap is conditional on the evaluation and mask.
    uint32_t save : 1;  ///< If == 1, then the leap will save the return address in the Return Address Special Register.
    /// SIGNED Immediate value for the leap (e.g. for relative leap or as an offset). The value is multiplied by 4 to
    /// get a byte offset, since instructions are 4 bytes. This means that the relative leap range is +/- 2^12
    /// instructions, which should be sufficient for most use cases.
    int32_t imm : ImmediateType::Bits;
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
    constexpr Grow(Operand e, Operand m, ImmediateType imm) : op{Operator::Grow}, eval{e.index}, mask{m.index}, cond{1}, imm{imm.value} {
        basal::exception::throw_if(e.type != static_cast<uint32_t>(OperandType::Evaluation), __FILE__, __LINE__,
                                   "Grow instruction requires the first operand to be an Evaluation register");
        basal::exception::throw_if(m.type != static_cast<uint32_t>(OperandType::Evaluation), __FILE__, __LINE__,
                                   "Grow instruction requires the second operand to be an Evaluation register");
    }
    friend std::ostream& operator<<(std::ostream& os, Grow g) {
        os << "grow " << Operand{OperandType::Evaluation, g.eval} << ", " << Operand{OperandType::Evaluation, g.mask} << ", "<< ImmediateType{g.imm};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;   ///< Evaluation Register to check for conditional trip (if cond == 1)
    uint32_t mask : CountOfEvalIndexBits;   ///< Mask for the evaluation register
    uint32_t cond : 1;  ///< If == 1, then the trip will be conditional on the evaluation and mask.
    uint32_t : CountOfDataBits - CountOfOperatorBits - (2 * CountOfEvalIndexBits) - ImmediateType::Bits - 1;
    uint32_t imm : ImmediateType::Bits;     ///< Each bit corresponds to a scratch register. If the bit is set, the corresponding scratch register will be grown.
};

struct Undo {
    using ImmediateType = isa::Immediate<10>;
    Undo() = delete;
    constexpr Undo(ImmediateType imm) : op{Operator::Undo}, eval{0}, mask{0}, cond{0}, imm{imm.value} {
    }
    constexpr Undo(Operand e, Operand m, ImmediateType imm) : op{Operator::Undo}, eval{e.index}, mask{m.index}, cond{1}, imm{imm.value} {
        basal::exception::throw_if(e.type != static_cast<uint32_t>(OperandType::Evaluation), __FILE__, __LINE__,
                                   "Undo instruction requires the first operand to be an Evaluation register");
        basal::exception::throw_if(m.type != static_cast<uint32_t>(OperandType::Evaluation), __FILE__, __LINE__,
                                   "Undo instruction requires the second operand to be an Evaluation register");
    }
    friend std::ostream& operator<<(std::ostream& os, Undo u) {
        os << "undo " << Operand{OperandType::Evaluation, u.eval} << ", " << Operand{OperandType::Evaluation, u.mask} << ", "<< ImmediateType{u.imm};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;   ///< Evaluation Register to check for conditional trip (if cond == 1)
    uint32_t mask : CountOfEvalIndexBits;   ///< Mask for the evaluation register
    uint32_t cond : 1;  ///< If == 1, then the trip will be conditional on the evaluation and mask.
    uint32_t : CountOfDataBits - CountOfOperatorBits - (2 * CountOfEvalIndexBits) - ImmediateType::Bits - 1;
    uint32_t imm : ImmediateType::Bits;     ///< Each bit corresponds to a scratch register. If the bit is set, the corresponding scratch register will be undone (shrunk).
};

struct Call {
    using ImmediateType = isa::Immediate<10>;
    Call() = delete;
    constexpr Call(ImmediateType imm) : op{Operator::Call}, eval{0}, mask{0}, cond{0}, imm{imm.value} {
    }
    constexpr Call(Operand e, Operand m, ImmediateType imm) : op{Operator::Call}, eval{e.index}, mask{m.index}, cond{1}, imm{imm.value} {
        basal::exception::throw_if(e.type != static_cast<uint32_t>(OperandType::Evaluation), __FILE__, __LINE__,
                                   "Call instruction requires the first operand to be an Evaluation register");
        basal::exception::throw_if(m.type != static_cast<uint32_t>(OperandType::Evaluation), __FILE__, __LINE__,
                                   "Call instruction requires the second operand to be an Evaluation register");
    }
    friend std::ostream& operator<<(std::ostream& os, Call c) {
        os << "call " << Operand{OperandType::Evaluation, c.eval} << ", " << Operand{OperandType::Evaluation, c.mask} << ", "<< ImmediateType{c.imm};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;   ///< Evaluation Register to check for conditional trip (if cond == 1)
    uint32_t mask : CountOfEvalIndexBits;   ///< Mask for the evaluation register
    uint32_t cond : 1;  ///< If == 1, then the trip will be conditional on the evaluation and mask.
    uint32_t : CountOfDataBits - CountOfOperatorBits - (2 * CountOfEvalIndexBits) - ImmediateType::Bits - 1;
    uint32_t imm : ImmediateType::Bits;     ///< The immediate value of the system call number to call.
};

struct Trip {
    using ImmediateType = isa::Immediate<10>;
    Trip() = delete;
    constexpr Trip(ImmediateType imm) : op{Operator::Trip}, eval{0}, mask{0}, cond{0}, imm{imm.value} {
    }
    constexpr Trip(Operand e, Operand m, ImmediateType imm) : op{Operator::Trip}, eval{e.index}, mask{m.index}, cond{1}, imm{imm.value} {
        basal::exception::throw_if(e.type != static_cast<uint32_t>(OperandType::Evaluation), __FILE__, __LINE__,
                                   "Trip instruction requires the first operand to be an Evaluation register");
        basal::exception::throw_if(m.type != static_cast<uint32_t>(OperandType::Evaluation), __FILE__, __LINE__,
                                   "Trip instruction requires the second operand to be an Evaluation register");
    }
    friend std::ostream& operator<<(std::ostream& os, Trip t) {
        os << "trip " << Operand{OperandType::Evaluation, t.eval} << ", " << Operand{OperandType::Evaluation, t.mask} << ", "<< ImmediateType{t.imm};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;   ///< Evaluation Register to check for conditional trip (if cond == 1)
    uint32_t mask : CountOfEvalIndexBits;   ///< Mask for the evaluation register
    uint32_t cond : 1;  ///< If == 1, then the trip will be conditional on the evaluation and mask.
    uint32_t : CountOfDataBits - CountOfOperatorBits - (2 * CountOfEvalIndexBits) - ImmediateType::Bits - 1;
    uint32_t imm : ImmediateType::Bits;     ///< The immediate value of the exception type to trip.
};

/// Compares two scratch registers and sets flags in the Evaluation Register based on the result of the comparison.
struct Compare {
    constexpr Compare() : op{Operator::Compare}, e{0}, a{0}, b{0}, s{0} {
    }

    constexpr Compare(Operand e, Operand a, Operand b, bool signed_comparison = false)
        : op{Operator::Compare}, e{e.index}, a{a.index}, b{b.index}, s{signed_comparison ? 1U : 0U} {
        basal::exception::throw_if(e.type != static_cast<uint32_t>(OperandType::Evaluation), __FILE__, __LINE__,
                                   "Compare instruction requires the first operand to be an Evaluation register");
        basal::exception::throw_if(a.type != static_cast<uint32_t>(OperandType::Scratch), __FILE__, __LINE__,
                                   "Compare instruction requires the second operand to be a Scratch register");
        basal::exception::throw_if(b.type != static_cast<uint32_t>(OperandType::Scratch), __FILE__, __LINE__,
                                   "Compare instruction requires the third operand to be a Scratch register");
    }

    friend std::ostream& operator<<(std::ostream& os, Compare c) {
        os << "compare " << Operand{OperandType::Evaluation, c.e} << ", " << Operand{OperandType::Scratch, c.a}
           << "<!=>" << Operand{OperandType::Scratch, c.b};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t e : CountOfScratchIndexBits;  ///< Evaluation Register to set flags in
    uint32_t a : CountOfScratchIndexBits;  ///< Scratch Register A
    uint32_t b : CountOfScratchIndexBits;  ///< Scratch Register B
    uint32_t s : 1;  ///< If set, the comparison will be signed. If not set, the comparison will be unsigned.
    uint32_t : CountOfDataBits - CountOfOperatorBits - (3 * CountOfScratchIndexBits) - 1;
};

class Bitwise {
public:
    Bitwise() = delete;

    constexpr static Bitwise And(Operand dst, Operand src1, Operand src2) {
        return Bitwise(Operator::And, dst, src1, src2);
    }

    constexpr static Bitwise Or(Operand dst, Operand src1, Operand src2) {
        return Bitwise(Operator::Or, dst, src1, src2);
    }

    constexpr static Bitwise Xor(Operand dst, Operand src1, Operand src2) {
        return Bitwise(Operator::Xor, dst, src1, src2);
    }

    friend std::ostream& operator<<(std::ostream& os, Bitwise a) {
        if (a.op == Operator::And) {
            os << "band ";
        } else if (a.op == Operator::Or) {
            os << "borr ";
        } else if (a.op == Operator::Xor) {
            os << "bxor ";
        } else {
            os << "??? ";
        }
        os << Operand{OperandType::Scratch, a.dst} << ", " << Operand{OperandType::Scratch, a.src1} << ", "
           << Operand{OperandType::Scratch, a.src2};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t src1 : CountOfScratchIndexBits;  ///< Source Scratch Register 1
    uint32_t src2 : CountOfScratchIndexBits;  ///< Source Scratch Register 2
    uint32_t : CountOfDataBits - CountOfOperatorBits - (3 * CountOfScratchIndexBits);

protected:
    constexpr Bitwise(Operator op, Operand dst, Operand src1, Operand src2)
        : op{op}, dst{dst.index}, src1{src1.index}, src2{src2.index} {
        basal::exception::throw_unless(dst.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src1.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src2.type == static_cast<uint32_t>(OperandType::Scratch),
                                       __FILE__, __LINE__,
                                       "Bitwise instruction requires all operands to be Scratch registers");
    }
};

struct BitwiseComplement {
    BitwiseComplement() = delete;
    constexpr BitwiseComplement(Operand dst, Operand src) : op{Operator::Complement}, dst{dst.index}, src{src.index} {
        basal::exception::throw_unless(dst.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src.type == static_cast<uint32_t>(OperandType::Scratch),
                                       __FILE__, __LINE__,
                                       "BitwiseComplement instruction requires both operands to be Scratch registers");
    }

    friend std::ostream& operator<<(std::ostream& os, BitwiseComplement n) {
        os << "cmpl " << Operand{OperandType::Scratch, n.dst} << ", " << Operand{OperandType::Scratch, n.src};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfScratchIndexBits;  ///< Destination Scratch Register
    uint32_t src : CountOfScratchIndexBits;  ///< Source Scratch Register
    uint32_t : CountOfDataBits - CountOfOperatorBits - (2 * CountOfScratchIndexBits);
};

struct BitwiseRsh {
    using ImmediateType = isa::Immediate<5>;
    BitwiseRsh() = delete;
    constexpr BitwiseRsh(Operand dst, Operand src, ImmediateType shift)
        : op{Operator::Rsh}, dst{dst.index}, src{src.index}, shift{shift.value} {
        basal::exception::throw_unless(dst.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src.type == static_cast<uint32_t>(OperandType::Scratch),
                                       __FILE__, __LINE__,
                                       "BitwiseRsh instruction requires both operands to be Scratch registers");
    }

    friend std::ostream& operator<<(std::ostream& os, BitwiseRsh r) {
        os << "rsh " << Operand{OperandType::Scratch, r.dst} << ", " << Operand{OperandType::Scratch, r.src} << ", "
           << r.shift;
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfScratchIndexBits;       ///< Destination Scratch Register
    uint32_t src : CountOfScratchIndexBits;       ///< Source Scratch Register
    uint32_t shift : ImmediateType::Bits;  ///< Shift Amount (0-31)
    uint32_t : CountOfDataBits - CountOfOperatorBits - (2 * CountOfScratchIndexBits) - ImmediateType::Bits;
};

struct BitwiseLsh {
    using ImmediateType = isa::Immediate<5>;
    BitwiseLsh() = delete;
    constexpr BitwiseLsh(Operand dst, Operand src, ImmediateType shift)
        : op{Operator::Lsh}, dst{dst.index}, src{src.index}, shift{shift.value} {
        basal::exception::throw_unless(dst.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src.type == static_cast<uint32_t>(OperandType::Scratch),
                                       __FILE__, __LINE__,
                                       "BitwiseLsh instruction requires both operands to be Scratch registers");
    }

    friend std::ostream& operator<<(std::ostream& os, BitwiseLsh l) {
        os << "lsh " << Operand{OperandType::Scratch, l.dst} << ", " << Operand{OperandType::Scratch, l.src} << ", "
           << l.shift;
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfScratchIndexBits;       ///< Destination Scratch Register
    uint32_t src : CountOfScratchIndexBits;       ///< Source Scratch Register
    uint32_t shift : ImmediateType::Bits;  ///< Shift Amount (0-31)
    uint32_t : CountOfDataBits - CountOfOperatorBits - (2 * CountOfScratchIndexBits) - ImmediateType::Bits;
};

struct BitCount {
    BitCount() = delete;
    constexpr BitCount(Operand dst, Operand src) : op{Operator::Count}, dst{dst.index}, src{src.index} {
        basal::exception::throw_unless(dst.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src.type == static_cast<uint32_t>(OperandType::Scratch),
                                       __FILE__, __LINE__,
                                       "BitCount instruction requires both operands to be Scratch registers");
    }

    friend std::ostream& operator<<(std::ostream& os, BitCount c) {
        os << "count " << Operand{OperandType::Scratch, c.dst} << ", " << Operand{OperandType::Scratch, c.src};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfScratchIndexBits;  ///< Destination Scratch Register
    uint32_t src : CountOfScratchIndexBits;  ///< Source Scratch Register
    uint32_t : CountOfDataBits - CountOfOperatorBits - (2 * CountOfScratchIndexBits);
};

struct SignedAdd {
    SignedAdd() = delete;
    constexpr SignedAdd(Operand eval, Operand dst, Operand src1, Operand src2)
        : op{Operator::SignedAdd}, eval{eval.index}, dst{dst.index}, src1{src1.index}, src2{src2.index} {
        basal::exception::throw_unless(eval.type == static_cast<uint32_t>(OperandType::Evaluation)
                                           && dst.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src1.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src2.type == static_cast<uint32_t>(OperandType::Scratch),
                                       __FILE__, __LINE__,
                                       "SignedAdd instruction requires evaluation to be Evaluation and others to be Scratch registers");
    }

    friend std::ostream& operator<<(std::ostream& os, SignedAdd a) {
        os << "sadd " << Operand{OperandType::Evaluation, a.eval} << ", " << Operand{OperandType::Scratch, a.dst}
           << ", " << Operand{OperandType::Scratch, a.src1} << ", "
           << Operand{OperandType::Scratch, a.src2};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;  ///< Evaluation Register
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t src1 : CountOfScratchIndexBits;  ///< Source Scratch Register 1
    uint32_t src2 : CountOfScratchIndexBits;  ///< Source Scratch Register 2
    uint32_t : CountOfDataBits - CountOfOperatorBits - (4 * CountOfScratchIndexBits);
};

struct SignedSub {
    SignedSub() = delete;
    constexpr SignedSub(Operand eval, Operand dst, Operand src1, Operand src2)
        : op{Operator::SignedSub}, eval{eval.index}, dst{dst.index}, src1{src1.index}, src2{src2.index} {
        basal::exception::throw_unless(eval.type == static_cast<uint32_t>(OperandType::Evaluation)
                                           && dst.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src1.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src2.type == static_cast<uint32_t>(OperandType::Scratch),
                                       __FILE__, __LINE__,
                                       "SignedSub instruction requires evaluation to be Evaluation and others to be Scratch registers");
    }

    friend std::ostream& operator<<(std::ostream& os, SignedSub s) {
        os << "ssub " << Operand{OperandType::Evaluation, s.eval} << ", " << Operand{OperandType::Scratch, s.dst}
           << ", " << Operand{OperandType::Scratch, s.src1} << ", "
           << Operand{OperandType::Scratch, s.src2};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;  ///< Evaluation Register
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t src1 : CountOfScratchIndexBits;  ///< Source Scratch Register 1
    uint32_t src2 : CountOfScratchIndexBits;  ///< Source Scratch Register 2
    uint32_t : CountOfDataBits - CountOfOperatorBits - (4 * CountOfScratchIndexBits);
};

struct SignedMultiply {
    SignedMultiply() = delete;
    constexpr SignedMultiply(Operand eval, Operand dst, Operand src1, Operand src2)
        : op{Operator::SignedMultiply}, eval{eval.index}, dst{dst.index}, src1{src1.index}, src2{src2.index} {
        basal::exception::throw_unless(eval.type == static_cast<uint32_t>(OperandType::Evaluation)
                                           && dst.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src1.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src2.type == static_cast<uint32_t>(OperandType::Scratch),
                                       __FILE__, __LINE__,
                                       "SignedMultiply instruction requires evaluation to be Evaluation and others to be Scratch registers");
    }

    friend std::ostream& operator<<(std::ostream& os, SignedMultiply m) {
        os << "smul " << Operand{OperandType::Evaluation, m.eval} << ", " << Operand{OperandType::Scratch, m.dst}
           << ", " << Operand{OperandType::Scratch, m.src1} << ", "
           << Operand{OperandType::Scratch, m.src2};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;  ///< Evaluation Register
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t src1 : CountOfScratchIndexBits;  ///< Source Scratch Register 1
    uint32_t src2 : CountOfScratchIndexBits;  ///< Source Scratch Register 2
    uint32_t : CountOfDataBits - CountOfOperatorBits - (4 * CountOfScratchIndexBits);
};

struct SignedDivide {
    SignedDivide() = delete;
    constexpr SignedDivide(Operand eval, Operand dst, Operand src1, Operand src2)
        : op{Operator::SignedDivide}, eval{eval.index}, dst{dst.index}, src1{src1.index}, src2{src2.index} {
        basal::exception::throw_unless(eval.type == static_cast<uint32_t>(OperandType::Evaluation)
                                           && dst.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src1.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src2.type == static_cast<uint32_t>(OperandType::Scratch),
                                       __FILE__, __LINE__,
                                       "SignedDivide instruction requires evaluation to be Evaluation and others to be Scratch registers");
    }

    friend std::ostream& operator<<(std::ostream& os, SignedDivide d) {
        os << "sdiv " << Operand{OperandType::Evaluation, d.eval} << ", " << Operand{OperandType::Scratch, d.dst}
           << ", " << Operand{OperandType::Scratch, d.src1} << ", "
           << Operand{OperandType::Scratch, d.src2};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;  ///< Evaluation Register
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t src1 : CountOfScratchIndexBits;  ///< Source Scratch Register 1
    uint32_t src2 : CountOfScratchIndexBits;  ///< Source Scratch Register 2
    uint32_t : CountOfDataBits - CountOfOperatorBits - (4 * CountOfScratchIndexBits);
};

struct SignedModulo {
    SignedModulo() = delete;
    constexpr SignedModulo(Operand eval, Operand dst, Operand src1, Operand src2)
        : op{Operator::SignedModulo}, eval{eval.index}, dst{dst.index}, src1{src1.index}, src2{src2.index} {
        basal::exception::throw_unless(eval.type == static_cast<uint32_t>(OperandType::Evaluation)
                                           && dst.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src1.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src2.type == static_cast<uint32_t>(OperandType::Scratch),
                                       __FILE__, __LINE__,
                                       "SignedModulo instruction requires evaluation to be Evaluation and others to be Scratch registers");
    }

    friend std::ostream& operator<<(std::ostream& os, SignedModulo m) {
        os << "smod " << Operand{OperandType::Evaluation, m.eval} << ", " << Operand{OperandType::Scratch, m.dst}
           << ", " << Operand{OperandType::Scratch, m.src1} << ", "
           << Operand{OperandType::Scratch, m.src2};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;  ///< Evaluation Register
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t src1 : CountOfScratchIndexBits;  ///< Source Scratch Register 1
    uint32_t src2 : CountOfScratchIndexBits;  ///< Source Scratch Register 2
    uint32_t : CountOfDataBits - CountOfOperatorBits - (4 * CountOfScratchIndexBits);
};

struct UnsignedAdd {
    UnsignedAdd() = delete;
    constexpr UnsignedAdd(Operand eval, Operand dst, Operand src1, Operand src2)
        : op{Operator::UnsignedAdd}, eval{eval.index}, dst{dst.index}, src1{src1.index}, src2{src2.index} {
        basal::exception::throw_unless(eval.type == static_cast<uint32_t>(OperandType::Evaluation)
                                           && dst.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src1.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src2.type == static_cast<uint32_t>(OperandType::Scratch),
                                       __FILE__, __LINE__,
                                       "UnsignedAdd instruction requires evaluation to be Evaluation and others to be Scratch registers");
    }

    friend std::ostream& operator<<(std::ostream& os, UnsignedAdd a) {
        os << "uadd " << Operand{OperandType::Evaluation, a.eval} << ", " << Operand{OperandType::Scratch, a.dst}
           << ", " << Operand{OperandType::Scratch, a.src1} << ", "
           << Operand{OperandType::Scratch, a.src2};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;  ///< Evaluation Register
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t src1 : CountOfScratchIndexBits;  ///< Source Scratch Register 1
    uint32_t src2 : CountOfScratchIndexBits;  ///< Source Scratch Register 2
    uint32_t : CountOfDataBits - CountOfOperatorBits - (4 * CountOfScratchIndexBits);
};

struct UnsignedSub {
    UnsignedSub() = delete;
    constexpr UnsignedSub(Operand eval, Operand dst, Operand src1, Operand src2)
        : op{Operator::UnsignedSub}, eval{eval.index}, dst{dst.index}, src1{src1.index}, src2{src2.index} {
        basal::exception::throw_unless(eval.type == static_cast<uint32_t>(OperandType::Evaluation)
                                           && dst.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src1.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src2.type == static_cast<uint32_t>(OperandType::Scratch),
                                       __FILE__, __LINE__,
                                       "UnsignedSub instruction requires evaluation to be Evaluation and others to be Scratch registers");
    }

    friend std::ostream& operator<<(std::ostream& os, UnsignedSub s) {
        os << "usub " << Operand{OperandType::Evaluation, s.eval} << ", " << Operand{OperandType::Scratch, s.dst}
           << ", " << Operand{OperandType::Scratch, s.src1} << ", "
           << Operand{OperandType::Scratch, s.src2};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;  ///< Evaluation Register
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t src1 : CountOfScratchIndexBits;  ///< Source Scratch Register 1
    uint32_t src2 : CountOfScratchIndexBits;  ///< Source Scratch Register 2
    uint32_t : CountOfDataBits - CountOfOperatorBits - (4 * CountOfScratchIndexBits);
};

struct UnsignedMultiply {
    UnsignedMultiply() = delete;
    constexpr UnsignedMultiply(Operand eval, Operand dst, Operand src1, Operand src2)
        : op{Operator::UnsignedMultiply}, eval{eval.index}, dst{dst.index}, src1{src1.index}, src2{src2.index} {
        basal::exception::throw_unless(eval.type == static_cast<uint32_t>(OperandType::Evaluation)
                                           && dst.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src1.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src2.type == static_cast<uint32_t>(OperandType::Scratch),
                                       __FILE__, __LINE__,
                                       "UnsignedMultiply instruction requires evaluation to be Evaluation and others to be Scratch registers");
    }

    friend std::ostream& operator<<(std::ostream& os, UnsignedMultiply m) {
        os << "umul " << Operand{OperandType::Evaluation, m.eval} << ", " << Operand{OperandType::Scratch, m.dst}
           << ", " << Operand{OperandType::Scratch, m.src1} << ", "
           << Operand{OperandType::Scratch, m.src2};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;  ///< Evaluation Register
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t src1 : CountOfScratchIndexBits;  ///< Source Scratch Register 1
    uint32_t src2 : CountOfScratchIndexBits;  ///< Source Scratch Register 2
    uint32_t : CountOfDataBits - CountOfOperatorBits - (4 * CountOfScratchIndexBits);
};

struct UnsignedDivide {
    UnsignedDivide() = delete;
    constexpr UnsignedDivide(Operand eval, Operand dst, Operand src1, Operand src2)
        : op{Operator::UnsignedDivide}, eval{eval.index}, dst{dst.index}, src1{src1.index}, src2{src2.index} {
        basal::exception::throw_unless(eval.type == static_cast<uint32_t>(OperandType::Evaluation)
                                           && dst.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src1.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src2.type == static_cast<uint32_t>(OperandType::Scratch),
                                       __FILE__, __LINE__,
                                       "UnsignedDivide instruction requires evaluation to be Evaluation and others to be Scratch registers");
    }

    friend std::ostream& operator<<(std::ostream& os, UnsignedDivide d) {
        os << "udiv " << Operand{OperandType::Evaluation, d.eval} << ", " << Operand{OperandType::Scratch, d.dst}
           << ", " << Operand{OperandType::Scratch, d.src1} << ", "
           << Operand{OperandType::Scratch, d.src2};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;  ///< Evaluation Register
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t src1 : CountOfScratchIndexBits;  ///< Source Scratch Register 1
    uint32_t src2 : CountOfScratchIndexBits;  ///< Source Scratch Register 2
    uint32_t : CountOfDataBits - CountOfOperatorBits - (4 * CountOfScratchIndexBits);
};

struct UnsignedModulo {
    UnsignedModulo() = delete;
    constexpr UnsignedModulo(Operand eval, Operand dst, Operand src1, Operand src2)
        : op{Operator::UnsignedModulo}, eval{eval.index}, dst{dst.index}, src1{src1.index}, src2{src2.index} {
        basal::exception::throw_unless(eval.type == static_cast<uint32_t>(OperandType::Evaluation)
                                           && dst.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src1.type == static_cast<uint32_t>(OperandType::Scratch)
                                           && src2.type == static_cast<uint32_t>(OperandType::Scratch),
                                       __FILE__, __LINE__,
                                       "UnsignedModulo instruction requires evaluation to be Evaluation and others to be Scratch registers");
    }

    friend std::ostream& operator<<(std::ostream& os, UnsignedModulo m) {
        os << "umod " << Operand{OperandType::Evaluation, m.eval} << ", " << Operand{OperandType::Scratch, m.dst}
           << ", " << Operand{OperandType::Scratch, m.src1} << ", "
           << Operand{OperandType::Scratch, m.src2};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfEvalIndexBits;  ///< Evaluation Register
    uint32_t dst : CountOfScratchIndexBits;   ///< Destination Scratch Register
    uint32_t src1 : CountOfScratchIndexBits;  ///< Source Scratch Register 1
    uint32_t src2 : CountOfScratchIndexBits;  ///< Source Scratch Register 2
    uint32_t : CountOfDataBits - CountOfOperatorBits - (4 * CountOfScratchIndexBits);
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
    /// Typed Constructor
    constexpr Instruction(NoOp) : noop{} {
    }
    /// Typed Constructor
    constexpr Instruction(Halt) : halt{} {
    }
    /// Typed Constructor for Copy
    constexpr Instruction(Copy c) : copy{c} {
    }
    /// Typed Constructor
    constexpr Instruction(Move m) : move{m} {
    }
    /// Typed Constructor
    constexpr Instruction(MoveImmediateToScratch mi) : movis{mi} {
    }
    /// Typed Constructor
    constexpr Instruction(MoveImmediateToEvaluation mi) : movie{mi} {
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
    /// Typed Constructor for BitwiseAnd
    constexpr Instruction(Bitwise x) : bitwise{x} {
    }
    /// Typed Constructor for BitwiseComplement
    constexpr Instruction(BitwiseComplement n) : bitwise_complement{n} {
    }
    /// Typed Constructor for BitwiseRsh
    constexpr Instruction(BitwiseRsh r) : bitwise_rsh{r} {
    }
    /// Typed Constructor for BitwiseLsh
    constexpr Instruction(BitwiseLsh l) : bitwise_lsh{l} {
    }
    /// Typed Constructor for BitCount
    constexpr Instruction(BitCount c) : bit_count{c} {
    }
    /// Typed Constructor for SignedAdd
    constexpr Instruction(SignedAdd a) : signed_add{a} {
    }
    /// Typed Constructor for SignedSub
    constexpr Instruction(SignedSub s) : signed_sub{s} {
    }
    /// Typed Constructor for SignedMultiply
    constexpr Instruction(SignedMultiply m) : signed_multiply{m} {
    }
    /// Typed Constructor for SignedDivide
    constexpr Instruction(SignedDivide d) : signed_divide{d} {
    }
    /// Typed Constructor for SignedModulo
    constexpr Instruction(SignedModulo m) : signed_modulo{m} {
    }
    /// Typed Constructor for UnsignedAdd
    constexpr Instruction(UnsignedAdd a) : unsigned_add{a} {
    }
    /// Typed Constructor for UnsignedSub
    constexpr Instruction(UnsignedSub s) : unsigned_sub{s} {
    }
    /// Typed Constructor for UnsignedMultiply
    constexpr Instruction(UnsignedMultiply m) : unsigned_multiply{m} {
    }
    /// Typed Constructor for UnsignedDivide
    constexpr Instruction(UnsignedDivide d) : unsigned_divide{d} {
    }
    /// Typed Constructor for UnsignedModulo
    constexpr Instruction(UnsignedModulo m) : unsigned_modulo{m} {
    }

    //=================================
    uint32_t raw;                     ///< The raw bits of the instruction as it would be stored in memory
    Base base;                        ///< The base instruction for decoding the operator
    NoOp noop;                        ///< No Operation
    Halt halt;                        ///< Halt the processor
    Copy copy;                        ///< Copy from one evaluation register to another
    Move move;                        ///< Move from scratch to scratch
    MoveImmediateToScratch movis;     ///< Move immediate to scratch
    MoveImmediateToEvaluation movie;  ///< Move immediate to evaluation
    Swap swap;                        ///< Swap registers based on reg_type
    Zero clear;                      ///< Zero registers based on reg_type and mask
    Load loads;                ///< Load from memory to scratch register
    Save save;                       ///< Saves a value from scratch register to memory
    Leap leap;                       ///< Leap to an address based on an evaluation and mask
    Back back;                       ///< Returns from a Leap
    Grow grow;                       ///< Grow the memory by a number of pages specified in an evaluation register
    Undo undo;                       ///< Undo the last memory growth
    Call call;                       ///< Call a subroutine at an address specified in an evaluation register
    Trip trip;                       ///< Trip to an address specified in an evaluation register and save the return
    Compare compare;                 ///< Compare two scratch registers and set flags in an evaluation register
    Bitwise bitwise;                 ///< Perform bitwise operations on two scratch registers and store the result in a destination
                                    ///< scratch register
    BitwiseComplement bitwise_complement;  ///< Perform bitwise NOT on a scratch register and store the result in a destination
                             ///< scratch register
    BitwiseRsh bitwise_rsh;  ///< Perform bitwise right shift on a scratch register by an immediate value and store the
                             ///< result in a destination scratch register
    BitwiseLsh bitwise_lsh;  ///< Perform bitwise left shift on a scratch register by an immediate value and store the
                             ///< result in a destination scratch register
    BitCount bit_count;    ///< Count the number of set bits in a scratch register and store the result in a destination
                           ///< scratch register
    SignedAdd signed_add;  ///< Perform signed addition on two scratch registers and store the result in a destination
                           ///< scratch register
    SignedSub signed_sub;  ///< Perform signed subtraction on two scratch registers and store the result
    SignedMultiply signed_multiply;  ///< Perform signed multiplication on two scratch registers and store the result in a
                                ///< destination scratch register
    SignedDivide signed_divide;    ///< Perform signed division on two scratch registers and store the result
    SignedModulo signed_modulo;   ///< Perform signed modulo on two scratch registers and store the result in a destination
                               ///< scratch register
    UnsignedAdd unsigned_add;  ///< Perform unsigned addition on two scratch registers and store the
    UnsignedSub unsigned_sub;  ///< Perform unsigned subtraction on two scratch registers and store the result in a
                               ///< destination scratch register
    UnsignedMultiply unsigned_multiply;  ///< Perform unsigned multiplication on two scratch registers and store the
    UnsignedDivide unsigned_divide;    ///< Perform unsigned division on two scratch registers and store the result in a
                                    ///< destination scratch register
    UnsignedModulo unsigned_modulo;    ///< Perform unsigned modulo on two scratch registers and store the
    //=================================
    friend std::ostream& operator<<(std::ostream& os, Instruction instr) {
        if (instr.base() == Operator::None) {
            os << instr.noop;
        } else if (instr.base() == Operator::Halt) {
            os << instr.halt;
        } else if (instr.base() == Operator::Move) {
            os << instr.move;
        } else if (instr.base() == Operator::MoveImmediateToScratch) {
            os << instr.movis;
        } else if (instr.base() == Operator::MoveImmediateToEvaluation) {
            os << instr.movie;
        } else if (instr.base() == Operator::Swap) {
            os << instr.swap;
        } else if (instr.base() == Operator::Zero) {
            os << instr.clear;
        } else if (instr.base() == Operator::Load) {
            os << instr.loads;
        } else if (instr.base() == Operator::Save) {
            os << instr.save;
        } else if (instr.base() == Operator::Leap) {
            os << instr.leap;
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
        } else if (instr.base() == Operator::And) {
            os << instr.bitwise;
        } else if (instr.base() == Operator::Or) {
            os << instr.bitwise;
        } else if (instr.base() == Operator::Xor) {
            os << instr.bitwise;
        } else if (instr.base() == Operator::Complement) {
            os << instr.bitwise_complement;
        } else if (instr.base() == Operator::Rsh) {
            os << instr.bitwise_rsh;
        } else if (instr.base() == Operator::Lsh) {
            os << instr.bitwise_lsh;
        } else if (instr.base() == Operator::Count) {
            os << instr.bit_count;
        } else if (instr.base() == Operator::SignedAdd) {
            os << instr.signed_add;
        } else if (instr.base() == Operator::SignedSub) {
            os << instr.signed_sub;
        } else if (instr.base() == Operator::SignedMultiply) {
            os << instr.signed_multiply;
        } else if (instr.base() == Operator::SignedDivide) {
            os << instr.signed_divide;
        } else if (instr.base() == Operator::SignedModulo) {
            os << instr.signed_modulo;
        } else if (instr.base() == Operator::UnsignedAdd) {
            os << instr.unsigned_add;
        } else if (instr.base() == Operator::UnsignedSub) {
            os << instr.unsigned_sub;
        } else if (instr.base() == Operator::UnsignedMultiply) {
            os << instr.unsigned_multiply;
        } else if (instr.base() == Operator::UnsignedDivide) {
            os << instr.unsigned_divide;
        } else if (instr.base() == Operator::UnsignedModulo) {
            os << instr.unsigned_modulo;
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