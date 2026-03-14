#pragma once

#include <cstdint>
#include <cstddef>
#include <iostream>

namespace isa {

/// The operation code for an instruction

enum class Operator : uint32_t {
    None = 0,  ///< noop
    // === Movement within Files ===
    MoveScratchToScratch,        ///< moves2s scratchDestination, scratchSource
    MoveParameterToParameter,    ///< movep2p parameterDestination, parameterSource
    MoveEvaluationToEvaluation,  ///< movee2e evaluationDestination, evaluationSource

    MoveImmediateToScratch,     ///< movi2s scratchDestination, #imm<16>
    MoveImmediateToParameter,   ///< movi2p parameterIndex, #imm<16>
    MoveImmediateToEvaluation,  ///< movi2e evaluationIndex, #imm<16>

    MoveParameterToScratch,   ///< movp2s scratchDestination, parameterIndex
    MoveScratchToParameter,   ///< movs2p parameterIndex, scratchSource
    MoveEvaluationToScratch,  ///< movee2s scratchDestination, evaluationIndex
    MoveScratchToEvaluation,  ///< movs2e evaluationIndex, scratchSource

    SwapScratch,     ///< swaps scratchA, scratchB
    SwapParameter,   ///< swapp parameterA, parameterB
    SwapEvaluation,  ///< swape evaluationA, evaluationB

    ClearScratch,     ///< clears #imm<16> (each register is a bit)
    ClearEvaluation,  ///< cleare #imm<16> (each register is a bit)
    ClearParameter,   ///< clearp #imm<16> (each register is a bit)

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
    Parameter = 2,
    Evaluation = 3
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
        } else if (t == OperandType::Parameter) {
            os << "P";
        } else if (t == OperandType::Evaluation) {
            os << "E";
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

class MoveScratchToScratch {
public:
    MoveScratchToScratch() = delete;
    constexpr MoveScratchToScratch(Operand dst, Operand src)
        : op{Operator::MoveScratchToScratch}, eval{0}, mask{0}, dst{dst.index}, src{src.index} {
    }

    // constexpr Move(Operand dst, Immediate imm) : Instruction{Operator::Move, dst, immediate{imm}} {}
    friend std::ostream& operator<<(std::ostream& os, MoveScratchToScratch m) {
        os << "moves2s" << Operand{OperandType::Scratch, m.dst} << ", " << Operand{OperandType::Scratch, m.src};
        return os;
    }

protected:
    const Operator op : 8;
    uint32_t eval : 4;  ///< Evaluation
    uint32_t mask : 4;  ///< Evaluation Mask
    uint32_t dst : 4;   ///< Operand
    uint32_t src : 4;   ///< Operand
    uint32_t : 8;       // Reserved
};

class MoveImmediateToScratch {
public:
    MoveImmediateToScratch() = delete;
    constexpr MoveImmediateToScratch(Operand dst, Immediate<20> imm)
        : op{Operator::MoveImmediateToScratch}, dst{dst.index}, imm{imm.value} {
    }
    friend std::ostream& operator<<(std::ostream& os, MoveImmediateToScratch mi) {
        os << "movi " << Operand{OperandType::Scratch, mi.dst} << ", " << Immediate<20>{mi.imm};
        return os;
    }

protected:
    Operator op : 8;
    uint32_t dst : 4;
    uint32_t imm : 20;
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
    //=================================
    Base base;
    NoOp noop;
    MoveScratchToScratch moves2s;
    MoveImmediateToScratch movis;
    //=================================
    friend std::ostream& operator<<(std::ostream& os, Instruction instr) {
        if (instr.base() == Operator::None) {
            os << instr.noop;
        } else if (instr.base() == Operator::MoveScratchToScratch) {
            os << instr.moves2s;
        } else if (instr.base() == Operator::MoveImmediateToScratch) {
            os << instr.movis;
        }
        return os;
    }
};
static_assert(sizeof(Instruction) == sizeof(uint32_t), "Must be this size");

}  // namespace instructions

/// A program is an unbounded array of instructions as far as we know.
using program = instructions::Instruction[];

}  // namespace isa