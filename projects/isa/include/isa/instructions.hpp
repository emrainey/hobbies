#pragma once

#include <cstdint>
#include <cstddef>
#include <iostream>

#include <isa/types.hpp>

namespace isa {

/// The operation code for an instruction

enum class Operator : uint32_t {
    None = 0,  ///< noop
    // === Movement within Files ===
    MoveScratchToScratch,        ///< move scratchDestination, scratchSource
    MoveEvaluationToEvaluation,  ///< move evaluationDestination, evaluationSource

    MoveImmediateToScratch,     ///< move immediate to scratch
    MoveImmediateToEvaluation,  ///< move immediate to evaluation

    MoveEvaluationToScratch,  ///< move evaluation to scratch
    MoveScratchToEvaluation,  ///< move scratch to evaluation

    SwapScratch,     ///< swap scratchA, scratchB
    SwapEvaluation,  ///< swap evaluationA, evaluationB

    ClearScratch,     ///< clears #imm<16> (each register is a bit)
    ClearEvaluation,  ///< cleare #imm<16> (each register is a bit)

    // === Memory Operations ===
    LoadSingle,     ///< load scratchDestination, scratchAddress
    LoadSingleInc,  ///< loadi scratchDestination, scratchAddress
    LoadMultiple,   ///< loadm scratchAddress, scratchFlags<16>

    StoreSingle,     ///< store scratchSource, scratchAddress
    StoreSingleInc,  ///< storei scratchSource, scratchAddress
    StoreMultiple,   ///< storem scratchAddress, scratchFlags<16>

    UnConditionalJump,  ///< jump evaluation, mask, scratchAddress
    ConditionalJump,    ///< cjump evaluation, mask, scratchAddress

    UnsignedCompare,  ///< ucomp evaluationDestination, scratchA, scratchB
    SignedCompare,    ///< scomp evaluationDestination, scratchA, scratchB

    // === Bit Operators ===
    And,             ///< and scratchDestination, scratchSource, scratchSource
    Or,              ///< or  scratchDestination, scratchSource, scratchSource
    Xor,             ///< xor scratchDestination, scratchSource, scratchSource
    Not,             ///< not scratchDestination, scratchSource
    scratchSourceh,  ///< rsh scratchDestination, scratchSource, #imm<5>
    Lsh,             ///< lsh scratchDestination, scratchSource, #imm<5>
    Count,           ///< count scratchDestination, scratchSource
    // === ALU (Integer) ===
    Add,       ///< add scratchDestination, scratchSource, scratchSource -> OverFlow
    Sub,       ///< sub scratchDestination, scratchSource, scratchSource -> Underflow
    Multiply,  ///< mult scratchDestination, scratchSource, scratchSource -> Overflow or Underflow
    Divide,    ///< div  scratchDestination, scratchSource, scratchSource -> Fault if scratchSource == 0
    Modulo,    ///< modu scratchDestination, scratchSource, scratchSource -> Fault if scratchSource == 0
    // === ALU (Float) ===
    // === ALU (SIMD) ===
};

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
    constexpr Operand(OperandType t, uint32_t r) : index{r}, type{static_cast<uint32_t>(t)} {
    }
    uint32_t index : 4;
    uint32_t type : 2;
    uint32_t : 26;
    friend std::ostream& operator<<(std::ostream& os, Operand o) {
        OperandType t = static_cast<OperandType>(o.type);
        if (t == OperandType::None) {
            os << "R";
        } else if (t == OperandType::Scratch) {
            os << "S";
        } else if (t == OperandType::Evaluation) {
            os << "E";
        } else if (t == OperandType::Mask) {
            os << "0b" << std::bitset<16>(o.index);
            return os;
        } else {
            os << "???";
            return os;
        }
        os << o.index;
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
    Operator op : 8;
    uint32_t arg : 24;
};

class NoOp {
public:
    constexpr NoOp() : _reserved{0} {
    }
    friend std::ostream& operator<<(std::ostream& os, NoOp) {
        os << "noop";
        return os;
    }

protected:
    uint32_t _reserved : 32;
};

struct MoveScratchToScratch {
    MoveScratchToScratch() = delete;
    constexpr MoveScratchToScratch(Operand dst, Operand src)
        : op{Operator::MoveScratchToScratch}
        , eval{0}
        , mask{0}
        , dst{dst.index}
        , src{src.index}
        , cond{0}
        , dir{0}
        , shift{0} {
    }

    friend std::ostream& operator<<(std::ostream& os, MoveScratchToScratch m) {
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

    const Operator op : 8;
    uint32_t eval : CountOfIndexBits;       ///< Evaluation
    uint32_t mask : CountOfIndexBits;       ///< Evaluation Mask
    uint32_t dst : CountOfIndexBits;        ///< Operand
    uint32_t src : CountOfIndexBits;        ///< Operand
    uint32_t cond : 1;                      ///< If == 1, then the move is conditional on the evaluation and mask.
    uint32_t : 1;                           ///< Unused
    uint32_t dir : 1;                       ///< Direction for Shift (0 = Left, 1 = Right)
    uint32_t shift : CountOfDataShiftBits;  ///< Shift Amount (for shift instructions)
};

struct MoveImmediateToScratch {
    using ImmediateType = isa::Immediate<20>;
    MoveImmediateToScratch() = delete;
    constexpr MoveImmediateToScratch(Operand dst, ImmediateType imm)
        : op{Operator::MoveImmediateToScratch}, dst{dst.index}, imm{imm.value} {
    }
    friend std::ostream& operator<<(std::ostream& os, MoveImmediateToScratch mi) {
        os << "move " << Operand{OperandType::Scratch, mi.dst} << ", " << ImmediateType{mi.imm};
        return os;
    }

    constexpr ImmediateType Immediate() const {
        return ImmediateType{imm};
    }

    Operator op : 8;
    uint32_t dst : 4;
    uint32_t imm : 20;
};

struct SwapScratch {
    SwapScratch() = delete;
    constexpr SwapScratch(Operand a, Operand b) : op{Operator::SwapScratch}, a{a.index}, b{b.index} {
    }
    friend std::ostream& operator<<(std::ostream& os, SwapScratch s) {
        os << "swap " << Operand{OperandType::Scratch, s.a} << ", " << Operand{OperandType::Scratch, s.b};
        return os;
    }

    Operator op : 8;
    uint32_t a : 4;
    uint32_t b : 4;
    uint32_t : 16;
};

struct SwapEvaluation {
    SwapEvaluation() = delete;
    constexpr SwapEvaluation(Operand a, Operand b) : op{Operator::SwapEvaluation}, a{a.index}, b{b.index} {
    }
    friend std::ostream& operator<<(std::ostream& os, SwapEvaluation s) {
        os << "swap " << Operand{OperandType::Evaluation, s.a} << ", " << Operand{OperandType::Evaluation, s.b};
        return os;
    }
    Operator op : 8;
    uint32_t a : 4;     ///< Evaluation Index A (0-15)
    uint32_t b : 4;     ///< Evaluation Index B (0-15)
    uint32_t : 16;      ///< Unused
};

struct ClearScratch {
    ClearScratch() = delete;
    constexpr ClearScratch(Operand mask) : op{Operator::ClearScratch}, mask{mask.index} {
    }
    friend std::ostream& operator<<(std::ostream& os, ClearScratch c) {
        os << "clears " << Operand{OperandType::Mask, c.mask};
        return os;
    }

    Operator op : 8;
    uint32_t mask : 16;  ///< Each bit corresponds to a register. If the bit is set, the corresponding register will be cleared.
    uint32_t : 8;        ///< Unused
};

struct ClearEvaluation {
    ClearEvaluation() = delete;
    constexpr ClearEvaluation(Operand mask) : op{Operator::ClearEvaluation}, mask{mask.index} {
    }
    friend std::ostream& operator<<(std::ostream& os, ClearEvaluation c) {
        os << "cleare " << Operand{OperandType::Mask, c.mask};
        return os;
    }

    Operator op : 8;
    uint32_t mask : 16;  ///< Each bit corresponds to a register. If the bit is set, the corresponding register will be cleared.
    uint32_t : 8;        ///< Unused
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
    constexpr Instruction(MoveScratchToScratch m) : moves2s{m} {
    }
    /// Typed Constructor
    constexpr Instruction(MoveImmediateToScratch mi) : movis{mi} {
    }
    /// Typed Constructor
    constexpr Instruction(SwapScratch s) : swapS{s} {
    }
    /// Typed Constructor
    constexpr Instruction(SwapEvaluation s) : swapE{s} {
    }
    /// Typed Constructor for ClearScratch
    constexpr Instruction(ClearScratch c) : clearS{c} {
    }
    /// Typed Constructor for ClearEvaluation
    constexpr Instruction(ClearEvaluation c) : clearE{c} {
    }
    //=================================
    uint32_t raw;                  ///< The raw bits of the instruction as it would be stored in memory
    Base base;                     ///< The base instruction for decoding the operator
    NoOp noop;                     ///< No Operation
    MoveScratchToScratch moves2s;  ///< Move from scratch to scratch
    MoveImmediateToScratch movis;  ///< Move immediate to scratch
    SwapScratch swapS;              ///< Swap two scratch registers
    SwapEvaluation swapE;           ///< Swap two evaluation registers
    ClearScratch clearS;           ///< Clear scratch registers based on a mask
    ClearEvaluation clearE;        ///< Clear evaluation registers based on a mask
    //=================================
    friend std::ostream& operator<<(std::ostream& os, Instruction instr) {
        if (instr.base() == Operator::None) {
            os << instr.noop;
        } else if (instr.base() == Operator::MoveScratchToScratch) {
            os << instr.moves2s;
        } else if (instr.base() == Operator::MoveImmediateToScratch) {
            os << instr.movis;
        } else if (instr.base() == Operator::SwapScratch) {
            os << instr.swapS;
        } else if (instr.base() == Operator::SwapEvaluation) {
            os << instr.swapE;
        } else if (instr.base() == Operator::ClearScratch) {
            os << instr.clearS;
        } else if (instr.base() == Operator::ClearEvaluation) {
            os << instr.clearE;
        } else {
            os << "??? -------";
        }
        return os;
    }
};
static_assert(sizeof(Instruction) == sizeof(uint32_t), "Must be this size");

}  // namespace instructions

/// A program is an unbounded array of instructions as far as we know.
using program = instructions::Instruction[];

}  // namespace isa