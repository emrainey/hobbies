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
    Move,        ///< move destination, Source (use E or S prefix)

    MoveImmediateToScratch,     ///< move immediate to scratch
    MoveImmediateToEvaluation,  ///< move immediate to evaluation

    MoveEvaluationToScratch,  ///< move evaluation to scratch
    MoveScratchToEvaluation,  ///< move scratch to evaluation

    Swap,   ///< swap registerA, registerB; reg_type (0=scratch, 1=evaluation)
    Clear,  ///< clear registers using mask; reg_type (0=scratch, 1=evaluation)

    // === Memory Operations ===
    LoadSingle,    ///< load scratchDestination, scratchAddress
    LoadMultiple,  ///< loadm scratchAddress, scratchFlags<16>

    StoreSingle,    ///< store scratchSource, scratchAddress
    StoreMultiple,  ///< storem scratchAddress, scratchFlags<16>

    Jump,  ///< jump evaluation, mask, scratchAddress

    // === Comparison ===
    Compare,  ///< Compare scratchA, scratchB

    // === Bit Operators ===
    And,    ///< and scratchDestination, scratchSource, scratchSource
    Or,     ///< or  scratchDestination, scratchSource, scratchSource
    Xor,    ///< xor scratchDestination, scratchSource, scratchSource
    Not,    ///< not scratchDestination, scratchSource
    Rsh,    ///< rsh scratchDestination, scratchSource, #imm<5>
    Lsh,    ///< lsh scratchDestination, scratchSource, #imm<5>
    Count,  ///< count scratchDestination, scratchSource
    // === ALU (Integer) ===
    Add,       ///< add scratchDestination, scratchSource, scratchSource -> OverFlow
    Sub,       ///< sub scratchDestination, scratchSource, scratchSource -> Underflow
    Multiply,  ///< mult scratchDestination, scratchSource, scratchSource -> Overflow or Underflow
    Divide,    ///< div  scratchDestination, scratchSource, scratchSource -> Fault if scratchSource == 0
    Modulo,    ///< modu scratchDestination, scratchSource, scratchSource -> Fault if scratchSource == 0
    // === ALU (Float) ===
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
        , type{dst.type == static_cast<uint32_t>(OperandType::Evaluation) && src.type == static_cast<uint32_t>(OperandType::Evaluation) ? 1U : 0U}
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
        , type{dst.type == static_cast<uint32_t>(OperandType::Evaluation) && src.type == static_cast<uint32_t>(OperandType::Evaluation) ? 1U : 0U}
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
        , type{dst.type == static_cast<uint32_t>(OperandType::Evaluation) && src.type == static_cast<uint32_t>(OperandType::Evaluation) ? 1U : 0U}
        , dir{shift_right ? 1U : 0U}
        , shift{shift_amount.value} {
    }

    constexpr Move(Operand eval, Operand mask, Operand dst, Operand src, bool shift_right,
                                   ImmediateType shift_amount)
        : op{Operator::Move}
        , eval{eval.index}
        , mask{mask.index}
        , dst{dst.index}
        , src{src.index}
        , cond{1}
        , type{dst.type == static_cast<uint32_t>(OperandType::Evaluation) && src.type == static_cast<uint32_t>(OperandType::Evaluation) ? 1U : 0U}
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
    uint32_t eval : CountOfIndexBits;  ///< Evaluation
    uint32_t mask : CountOfIndexBits;  ///< Evaluation Mask
    uint32_t dst : CountOfIndexBits;   ///< Operand
    uint32_t src : CountOfIndexBits;   ///< Operand
    uint32_t cond : 1;                 ///< If == 1, then the move is conditional on the evaluation and mask.
    uint32_t type: 1;                  ///< 0 = Scratch to Scratch, 1 = Evaluation to Evaluation
    uint32_t dir : 1;                  ///< Direction for Shift (0 = Left, 1 = Right)
    uint32_t shift : CountOfDataShiftBits;  ///< Shift Amount (for shift instructions)
};

struct MoveImmediateToScratch {
    using ImmediateType = isa::Immediate<20>;
    MoveImmediateToScratch() = delete;
    constexpr MoveImmediateToScratch(Operand dst, ImmediateType imm)
        : op{Operator::MoveImmediateToScratch}, dst{dst.index}, imm{imm.value} {
        basal::exception::throw_if(dst.type != to_underlying(OperandType::Scratch), __FILE__, __LINE__, "MoveImmediateToScratch instruction requires the destination operand to be a Scratch register");
    }
    friend std::ostream& operator<<(std::ostream& os, MoveImmediateToScratch mi) {
        os << "move " << Operand{OperandType::Scratch, mi.dst} << ", " << ImmediateType{mi.imm};
        return os;
    }

    constexpr ImmediateType Immediate() const {
        return ImmediateType{imm};
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfIndexBits;
    uint32_t imm : 20;
};

struct MoveImmediateToEvaluation {
    using ImmediateType = isa::Immediate<EvaluationTypeBits + EvaluationFlagBits>;
    MoveImmediateToEvaluation() = delete;

    constexpr MoveImmediateToEvaluation(Operand dst, ImmediateType imm)
        : op{Operator::MoveImmediateToEvaluation}, dst{dst.index}, imm{imm.value} {
        basal::exception::throw_if(dst.type != to_underlying(OperandType::Evaluation), __FILE__, __LINE__, "MoveImmediateToEvaluation instruction requires the destination operand to be an Evaluation register");
    }

    constexpr MoveImmediateToEvaluation(Operand dst, Evaluation eval)
        : op{Operator::MoveImmediateToEvaluation}, dst{dst.index}, imm{eval.value} {
        basal::exception::throw_if(dst.type != to_underlying(OperandType::Evaluation), __FILE__, __LINE__, "MoveImmediateToEvaluation instruction requires the destination operand to be an Evaluation register");
    }

    friend std::ostream& operator<<(std::ostream& os, MoveImmediateToEvaluation mi) {
        os << "move " << Operand{OperandType::Evaluation, mi.dst} << ", " << ImmediateType{mi.imm};
        return os;
    }

    constexpr ImmediateType Immediate() const {
        return ImmediateType{imm};
    }

    Operator op : CountOfOperatorBits;
    uint32_t dst : CountOfIndexBits;
    uint32_t : 20 - ImmediateType::Bits;
    uint32_t imm : ImmediateType::Bits;
};

struct Swap {
    Swap() = delete;
    constexpr Swap(Operand a, Operand b)
        : op{Operator::Swap}, a{a.index}, b{b.index}, type{0} {
        basal::exception::throw_unless(a.type == b.type, __FILE__, __LINE__, "Swap instruction requires both operands to be of the same type");
        basal::exception::throw_if(a.type == static_cast<uint32_t>(OperandType::None) || a.type > static_cast<uint32_t>(OperandType::Evaluation), __FILE__, __LINE__, "Swap instruction requires both operands to be either Scratch or Evaluation registers");
        type = (a.type == to_underlying(OperandType::Scratch)) ? RegisterType::Scratch : RegisterType::Evaluation;
    }

    friend std::ostream& operator<<(std::ostream& os, Swap s) {
        OperandType register_type = s.type == RegisterType::Scratch ? OperandType::Scratch : OperandType::Evaluation;
        os << "swap " << Operand{register_type, s.a} << ", " << Operand{register_type, s.b};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t a : CountOfIndexBits;
    uint32_t b : CountOfIndexBits;
    RegisterType type : 1;
    uint32_t : 15;
};

struct Clear {
    using ImmediateType = isa::Immediate<16>;
    Clear() = delete;
    constexpr Clear(Operand mask, RegisterType evaluation = RegisterType::Scratch)
        : op{Operator::Clear}, mask{mask.imm}, type{to_underlying(evaluation)} {
    }
    friend std::ostream& operator<<(std::ostream& os, Clear c) {
        os << "clear " << (c.type == RegisterType::Scratch ? 'S' : 'E') << ", " << Operand{OperandType::Mask, ImmediateType{c.mask}};
        return os;
    }

    Operator op : CountOfOperatorBits;
    /// Each bit corresponds to a register. If the bit is set, the corresponding register will be cleared.
    uint32_t mask : ImmediateType::Bits;
    RegisterType type : 1;
    uint32_t : 7;
};

struct LoadSingle {
    using ImmediateType = isa::Immediate<14>;
    LoadSingle() = delete;
    constexpr LoadSingle(Operand dst, Operand base, ImmediateType imm, bool inc = false, bool off = false)
        : op{Operator::LoadSingle}
        , dst{dst.index}
        , base{base.index}
        , inc{inc ? 1U : 0U}
        , off{off ? 1U : 0U}
        , imm{imm.value} {
    }
    friend std::ostream& operator<<(std::ostream& os, LoadSingle l) {
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
    uint32_t dst : CountOfIndexBits;   ///< Destination Scratch Register
    uint32_t base : CountOfIndexBits;  ///< Base Scratch Register containing the address
    uint32_t inc : 1;  ///< If set, the base scratch register will be incremented by the size of the load after the load
                       ///< is performed.
    uint32_t off : 1;  ///< If set, the base scratch register will be used as an offset from a base address in a special
                       ///< register instead of an absolute address.
    uint32_t imm : ImmediateType::Bits;  ///< The immediate value of the offset or increment in bytes.
};

struct StoreSingle {
    using ImmediateType = isa::Immediate<14>;
    StoreSingle() = delete;
    constexpr StoreSingle(Operand src, Operand base, ImmediateType imm, bool inc = false, bool off = false)
        : op{Operator::StoreSingle}
        , src{src.index}
        , base{base.index}
        , inc{inc ? 1U : 0U}
        , off{off ? 1U : 0U}
        , imm{imm.value} {
    }

    friend std::ostream& operator<<(std::ostream& os, StoreSingle s) {
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
    uint32_t src : CountOfIndexBits;   ///< Source Scratch Register
    uint32_t base : CountOfIndexBits;  ///< Base Scratch Register containing the address
    uint32_t inc : 1;                  ///< If set, the base scratch register will
                                       ///< be incremented by the size of the store after the store is performed.
    uint32_t off : 1;  ///< If set, the base scratch register will be used as an offset from a base address in a special
                       ///< register instead of an absolute address.
    uint32_t imm : ImmediateType::Bits;  ///< The immediate value of the offset or increment in bytes
};

/// The Jump Instruction
struct Jump {
    using ImmediateType = isa::SignedImmediate<10>;
    Jump() = delete;
    constexpr Jump(Operand dst, ImmediateType imm, bool save = false)
        : op{Operator::Jump}, eval{0}, mask{0}, dst{dst.index}, cond{0}, save{save ? 1U : 0U}, imm{imm.value} {
    }

    constexpr Jump(Operand eval, Operand mask, Operand dst, ImmediateType imm, bool save = false)
        : op{Operator::Jump}
        , eval{eval.index}
        , mask{mask.index}
        , dst{dst.index}
        , cond{1}
        , save{save ? 1U : 0U}
        , imm{imm.value} {
    }
    friend std::ostream& operator<<(std::ostream& os, Jump j) {
        os << "jump ";
        if (j.cond) {
            os << Operand{OperandType::Evaluation, j.eval} << ", " << Operand{OperandType::Evaluation, j.mask} << ", ";
        }
        os << Operand{OperandType::Scratch, j.dst} << " + (" << ImmediateType{j.imm} << " * 4)";
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t eval : CountOfIndexBits;  ///< Evaluation Register to check
    uint32_t mask : CountOfIndexBits;  ///< Mask for the evaluation register
    uint32_t dst : CountOfIndexBits;   ///< Scratch Register containing the destination address
    uint32_t cond : 1;                 ///< If == 1, then the jump is conditional on the evaluation and mask.
    uint32_t save : 1;  ///< If == 1, then the jump will save the return address in the Return Address Special Register.
    /// SIGNED Immediate value for the jump (e.g. for relative jumps or as an offset). The value is multiplied by 4 to
    /// get a byte offset, since instructions are 4 bytes. This means that the relative jump range is +/- 2^12
    /// instructions, which should be sufficient for most use cases.
    int32_t imm : ImmediateType::Bits;
};

/// Compares two scratch registers and sets flags in the Evaluation Register based on the result of the comparison.
struct Compare {

    constexpr Compare() : op{Operator::Compare}, e{0}, a{0}, b{0}, s{0} {
    }

    constexpr Compare(Operand e, Operand a, Operand b, bool signed_comparison = false) : op{Operator::Compare}, e{e.index}, a{a.index}, b{b.index}, s{signed_comparison ? 1U : 0U} {
        basal::exception::throw_if(e.type != static_cast<uint32_t>(OperandType::Evaluation), __FILE__, __LINE__, "Compare instruction requires the first operand to be an Evaluation register");
        basal::exception::throw_if(a.type != static_cast<uint32_t>(OperandType::Scratch), __FILE__, __LINE__, "Compare instruction requires the second operand to be a Scratch register");
        basal::exception::throw_if(b.type != static_cast<uint32_t>(OperandType::Scratch), __FILE__, __LINE__, "Compare instruction requires the third operand to be a Scratch register");
    }

    friend std::ostream& operator<<(std::ostream& os, Compare c) {
        os << "compare " << Operand{OperandType::Evaluation, c.e} << ", " << Operand{OperandType::Scratch, c.a} << "<!=>" << Operand{OperandType::Scratch, c.b};
        return os;
    }

    Operator op : CountOfOperatorBits;
    uint32_t e : CountOfIndexBits;  ///< Evaluation Register to set flags in
    uint32_t a : CountOfIndexBits;  ///< Scratch Register A
    uint32_t b : CountOfIndexBits;  ///< Scratch Register B
    uint32_t s : 1;  ///< If set, the comparison will be signed. If not set, the comparison will be unsigned.
    uint32_t : CountOfDataBits - CountOfOperatorBits - (3 * CountOfIndexBits) - 1;
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
    constexpr Instruction(Clear c) : clear{c} {
    }
    /// Typed Constructor for Load
    constexpr Instruction(LoadSingle l) : loads{l} {
    }
    /// Typed Constructor for Store
    constexpr Instruction(StoreSingle s) : stores{s} {
    }
    /// Typed Constructor for Jump
    constexpr Instruction(Jump j) : jumps{j} {
    }
    /// Typed Constructor for Compare
    constexpr Instruction(Compare c) : compare{c} {
    }
    //=================================
    uint32_t raw;                     ///< The raw bits of the instruction as it would be stored in memory
    Base base;                        ///< The base instruction for decoding the operator
    NoOp noop;                        ///< No Operation
    Halt halt;                        ///< Halt the processor
    Move move;     ///< Move from scratch to scratch
    MoveImmediateToScratch movis;     ///< Move immediate to scratch
    MoveImmediateToEvaluation movie;  ///< Move immediate to evaluation
    Swap swap;                        ///< Swap registers based on reg_type
    Clear clear;                      ///< Clear registers based on reg_type and mask
    LoadSingle loads;                 ///< Load from memory to scratch register
    StoreSingle stores;               ///< Store from scratch register to memory
    Jump jumps;                       ///< Jump to an address based on an evaluation and mask
    Compare compare;                   ///< Compare two scratch registers and set flags in an evaluation register
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
        } else if (instr.base() == Operator::Clear) {
            os << instr.clear;
        } else if (instr.base() == Operator::LoadSingle) {
            os << instr.loads;
        } else if (instr.base() == Operator::StoreSingle) {
            os << instr.stores;
        } else if (instr.base() == Operator::Jump) {
            os << instr.jumps;
        } else if (instr.base() == Operator::Compare)   {
            os << instr.compare;
        } else {
            os << "???";
        }
        return os;
    }
};
static_assert(sizeof(Instruction) == sizeof(uint32_t), "Must be this size");

}  // namespace instructions

/// A program is an unbounded array of instructions as far as we know.
using program = instructions::Instruction[];

}  // namespace isa