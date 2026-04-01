#include <isa/isa.hpp>
#include <basal/basal.hpp>

namespace isa {

using namespace instructions;

// === Some helper aliases to make writing programs easier in C++

using Op = Operand;

constexpr static auto E = Operand::Type::Evaluation;
constexpr static auto S = Operand::Type::Scratch;
constexpr static auto M = Operand::Type::Mask;

template <size_t BITS>
using imm = Immediate<BITS>;

// === Example Program ===

program program1 = {
    Zero{Op{M, imm<16>{0xFFFFU}}, RegisterType::Scratch},
    Zero{Op{M, imm<16>{0xFFFFU}}, RegisterType::Evaluation},
    NoOp{},
    MoveImmediate{Op{S, 0}, imm<16>{0xCCDD}},
    MoveImmediate{Op{S, 1}, imm<16>{0xAABB}, true},
    Bitwise2::Or(Op{S, 0}, Op{S, 0}, Op{S, 1}),
    Move{Op{S, 1}, Op{S, 0}, true, Move::ImmediateType{4}},
    MoveImmediate{Op{S, 0}, imm<16>{0x3333}},
    MoveImmediate{Op{S, 2}, imm<16>{0x0010}, true},
    Save{Op{S, 1}, Op{S, 2}, Save::ImmediateType{0x00}},
    MoveImmediate{Op{S, 3}, imm<16>{0x5555}},
    MoveImmediate{Op{S, 5}, imm<16>{0xCCCC}},
    Bitwise1::Count(Op{S, 7}, Op{S, 0}),
    Bitwise2::And(Op{S, 7}, Op{S, 1}, Op{S, 3}),
    Bitwise2::Or(Op{S, 8}, Op{S, 1}, Op{S, 3}),
    Bitwise2::Xor(Op{S, 9}, Op{S, 1}, Op{S, 3}),
    Swap{Op{S, 0}, Op{S, 4}},
    Compare{Op{E, 0}, Op{S, 0}, Op{S, 3}},
    Compare{Op{E, 1}, Op{S, 1}, Op{S, 3}},
    Compare{Op{E, 2}, Op{S, 3}, Op{S, 5}},
    Compare{Op{E, 3}, Op{S, 1}, Op{S, 4}},
    Swap{Op{E, 0}, Op{E, 4}},
    MoveImmediate{Op{E, 5}, imm<16>{0xFF}},
    MoveImmediate{Op{E, 6}, ComparisonEvaluation{true, false, false, true}},
    MoveImmediate{Op{S, 9}, imm<16>{0x1000}, true},
    MoveImmediate{Op{S, 9}, imm<16>{0x0048 + (25 * 4)}, false},
    Leap{Op{S, 9}, Leap::ImmediateType{0x2}, true},
    Load{Op{S, 2}, Op{S, 0}, Load::ImmediateType{0x10}, true, false},
    Save{Op{S, 2}, Op{S, 0}, Save::ImmediateType{0x20}, true},
    Halt{},
};

}  // namespace isa