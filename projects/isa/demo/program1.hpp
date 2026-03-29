#include <isa/isa.hpp>
#include <basal/basal.hpp>

namespace isa {

using namespace instructions;

program program1 = {
    Instruction{Zero{Operand{OperandType::Mask, Immediate<16>{0xFFFFU}}, RegisterType::Scratch}},
    Instruction{Zero{Operand{OperandType::Mask, Immediate<16>{0xFFFFU}}, RegisterType::Evaluation}},
    Instruction{NoOp{}},
    Instruction{MoveImmediateToScratch{Operand{OperandType::Scratch, 0}, Immediate<20>{0xAAAAA}}},
    Instruction{
        Move{Operand{OperandType::Scratch, 1}, Operand{OperandType::Scratch, 0}, true, Move::ImmediateType{4}}},
    Instruction{MoveImmediateToScratch{Operand{OperandType::Scratch, 0}, Immediate<20>{0x33333}}},
    Instruction{MoveImmediateToScratch{Operand{OperandType::Scratch, 3}, Immediate<20>{0x55555}}},
    Instruction{MoveImmediateToScratch{Operand{OperandType::Scratch, 5}, Immediate<20>{0xCCCCC}}},
    Instruction{BitCount{Operand{OperandType::Scratch, 7}, Operand{OperandType::Scratch, 0}}},
    Instruction{Bitwise::And(Operand{OperandType::Scratch, 7}, Operand{OperandType::Scratch, 1},
                            Operand{OperandType::Scratch, 3})},
    Instruction{Bitwise::Or(Operand{OperandType::Scratch, 8}, Operand{OperandType::Scratch, 1},
                            Operand{OperandType::Scratch, 3})},
    Instruction{Bitwise::Xor(Operand{OperandType::Scratch, 9}, Operand{OperandType::Scratch, 1},
                            Operand{OperandType::Scratch, 3})},
    Instruction{Swap{Operand{OperandType::Scratch, 0}, Operand{OperandType::Scratch, 4}}},
    Instruction{Compare{Operand{OperandType::Evaluation, 0}, Operand{OperandType::Scratch, 0},
                        Operand{OperandType::Scratch, 3}}},
    Instruction{Compare{Operand{OperandType::Evaluation, 1}, Operand{OperandType::Scratch, 1},
                        Operand{OperandType::Scratch, 3}}},
    Instruction{Compare{Operand{OperandType::Evaluation, 2}, Operand{OperandType::Scratch, 3},
                        Operand{OperandType::Scratch, 5}}},
    Instruction{Compare{Operand{OperandType::Evaluation, 3}, Operand{OperandType::Scratch, 1},
                        Operand{OperandType::Scratch, 4}}},
    Instruction{Swap{Operand{OperandType::Evaluation, 0}, Operand{OperandType::Evaluation, 4}}},
    Instruction{MoveImmediateToEvaluation{Operand{OperandType::Evaluation, 5},
                                            MoveImmediateToEvaluation::ImmediateType{0xFF}}},
    Instruction{MoveImmediateToEvaluation{Operand{OperandType::Evaluation, 6},
                                            ComparisonEvaluation{true, false, false, true}}},
    Instruction{Leap{Operand{OperandType::Scratch, 0}, Leap::ImmediateType{0x2}, true}},
    Instruction{Load{Operand{OperandType::Scratch, 2}, Operand{OperandType::Scratch, 0},
                            Load::ImmediateType{0x10}, true, false}},
    Instruction{Save{Operand{OperandType::Scratch, 2}, Operand{OperandType::Scratch, 0},
                            Save::ImmediateType{0x20}, true, false}},
    Instruction{Halt{}},
};

}  // namespace isa