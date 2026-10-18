#include <cstdint>
#include <string_view>

#include <gtest/gtest.h>

#include <isa/instructions.hpp>
#include <isa/operator.hpp>

namespace {

using isa::Operand;
using isa::instructions::Instruction;
namespace layout = isa::layout;

template <size_t N>
uint32_t Extract(uint32_t raw, layout::Field const (&fields)[N], char const* name) {
    for (auto const& f : fields) {
        if (std::string_view{f.name} == name) {
            uint64_t mask = (1ULL << f.width) - 1ULL;
            return static_cast<uint32_t>((raw >> f.lsb) & mask);
        }
    }
    ADD_FAILURE() << "no field " << name;
    return 0;
}

template <size_t N>
uint32_t TotalWidth(layout::Field const (&fields)[N]) {
    uint32_t total = 0;
    for (auto const& f : fields) {
        total += f.width;
    }
    return total;
}

constexpr Operand S(uint32_t i) {
    return Operand{Operand::Type::Scratch, i};
}
constexpr Operand E(uint32_t i) {
    return Operand{Operand::Type::Evaluation, i};
}

TEST(LayoutTest, EveryFormatFillsTheWord) {
    EXPECT_EQ(32U, TotalWidth(layout::Noop));
    EXPECT_EQ(32U, TotalWidth(layout::Copy));
    EXPECT_EQ(32U, TotalWidth(layout::Move));
    EXPECT_EQ(32U, TotalWidth(layout::MoveImmediate));
    EXPECT_EQ(32U, TotalWidth(layout::Swap));
    EXPECT_EQ(32U, TotalWidth(layout::Zero));
    EXPECT_EQ(32U, TotalWidth(layout::Memory));
    EXPECT_EQ(32U, TotalWidth(layout::Leap));
    EXPECT_EQ(32U, TotalWidth(layout::LeapImmediate));
    EXPECT_EQ(32U, TotalWidth(layout::Back));
    EXPECT_EQ(32U, TotalWidth(layout::Frame));
    EXPECT_EQ(32U, TotalWidth(layout::Trip));
    EXPECT_EQ(32U, TotalWidth(layout::PushPull));
    EXPECT_EQ(32U, TotalWidth(layout::Compare));
    EXPECT_EQ(32U, TotalWidth(layout::Bitwise1));
    EXPECT_EQ(32U, TotalWidth(layout::Binary));
    EXPECT_EQ(32U, TotalWidth(layout::Negate));
    EXPECT_EQ(32U, TotalWidth(layout::Extend));
    EXPECT_EQ(32U, TotalWidth(layout::Arithmetic));
    EXPECT_EQ(32U, TotalWidth(layout::FloatUnary));
}

TEST(LayoutTest, OperatorValuesMatchTheOpcodeField) {
    EXPECT_EQ(0x04U, static_cast<uint32_t>(isa::Operator::Copy));
    EXPECT_EQ(0xFCU, static_cast<uint32_t>(isa::Operator::Halt));
    Instruction i{isa::instructions::Copy{S(1), S(2)}};
    EXPECT_EQ(static_cast<uint32_t>(isa::Operator::Copy), Extract(i.raw, layout::Copy, "op"));
}

TEST(LayoutTest, CopyFieldsMatchCppBitfields) {
    Instruction i{isa::instructions::Copy{S(5), S(9), E(3)}};
    EXPECT_EQ(5U, Extract(i.raw, layout::Copy, "dst"));
    EXPECT_EQ(9U, Extract(i.raw, layout::Copy, "src"));
    EXPECT_EQ(3U, Extract(i.raw, layout::Copy, "eval"));
    EXPECT_EQ(1U, Extract(i.raw, layout::Copy, "cond"));
}

TEST(LayoutTest, MoveFieldsMatchCppBitfields) {
    using isa::instructions::Move;
    Instruction i{Move{E(3), E(4), E(5), E(6), true, Move::ImmediateType{17}}};
    EXPECT_EQ(3U, Extract(i.raw, layout::Move, "eval"));
    EXPECT_EQ(4U, Extract(i.raw, layout::Move, "mask"));
    EXPECT_EQ(5U, Extract(i.raw, layout::Move, "dst"));
    EXPECT_EQ(6U, Extract(i.raw, layout::Move, "src"));
    EXPECT_EQ(1U, Extract(i.raw, layout::Move, "cond"));
    EXPECT_EQ(1U, Extract(i.raw, layout::Move, "type_"));
    EXPECT_EQ(1U, Extract(i.raw, layout::Move, "dir"));
    EXPECT_EQ(17U, Extract(i.raw, layout::Move, "shift"));
}

TEST(LayoutTest, MoveShiftOperandEncodesEveryShiftAmount) {
    using isa::instructions::Move;
    // P-023: the operand used to be Immediate<log2(CountOfDataShiftBits)>, which is two
    // bits wide, so every shift of 4 or more encoded as zero.
    EXPECT_EQ(isa::CountOfDataShiftBits, Move::ImmediateType::Bits)
        << "the shift operand must span the whole shift field";
    for (uint32_t shift = 0U; shift < isa::CountOfDataBits; ++shift) {
        Instruction const i{Move{S(1), S(2), false, Move::ImmediateType{shift}}};
        EXPECT_EQ(shift, Extract(i.raw, layout::Move, "shift")) << "shift " << shift;
    }
}

TEST(LayoutTest, SwapFieldsMatchCppBitfields) {
    Instruction i{isa::instructions::Swap{S(6), S(11)}};
    EXPECT_EQ(6U, Extract(i.raw, layout::Swap, "a"));
    EXPECT_EQ(11U, Extract(i.raw, layout::Swap, "b"));
}

TEST(LayoutTest, ArithmeticFieldsMatchCppBitfields) {
    using isa::instructions::Arithmetic;
    Instruction i{Arithmetic::AddsSat(S(1), S(2), S(3), Arithmetic::Size::HalfWord)};
    EXPECT_EQ(1U, Extract(i.raw, layout::Arithmetic, "dst"));
    EXPECT_EQ(2U, Extract(i.raw, layout::Arithmetic, "src1"));
    EXPECT_EQ(3U, Extract(i.raw, layout::Arithmetic, "src2"));
    EXPECT_EQ(0U, Extract(i.raw, layout::Arithmetic, "cond"));
    EXPECT_EQ(1U, Extract(i.raw, layout::Arithmetic, "sign"));
    EXPECT_EQ(1U, Extract(i.raw, layout::Arithmetic, "sat"));
    EXPECT_EQ(1U, Extract(i.raw, layout::Arithmetic, "size"));
}

TEST(LayoutTest, NegateAndExtendFieldsMatchCppBitfields) {
    using isa::instructions::Extend;
    using isa::instructions::Negate;
    Instruction n{Negate::Make(S(4), S(5), isa::Size::Byte, true).Evaluate(E(2))};
    EXPECT_EQ(4U, Extract(n.raw, layout::Negate, "dst"));
    EXPECT_EQ(5U, Extract(n.raw, layout::Negate, "src"));
    EXPECT_EQ(2U, Extract(n.raw, layout::Negate, "eval"));
    EXPECT_EQ(1U, Extract(n.raw, layout::Negate, "sat"));
    EXPECT_EQ(0U, Extract(n.raw, layout::Negate, "size"));
    Instruction x{Extend::Sign(S(1), S(2), isa::Size::HalfWord)};
    EXPECT_EQ(1U, Extract(x.raw, layout::Extend, "sign"));
    EXPECT_EQ(1U, Extract(x.raw, layout::Extend, "size"));
}

TEST(LayoutTest, LeapFieldsMatchCppBitfields) {
    using isa::instructions::Leap;
    Instruction i{Leap{E(1), E(2), S(3), Leap::ImmediateType{-3}, true}};
    EXPECT_EQ(1U, Extract(i.raw, layout::Leap, "eval"));
    EXPECT_EQ(2U, Extract(i.raw, layout::Leap, "mask"));
    EXPECT_EQ(3U, Extract(i.raw, layout::Leap, "dst"));
    EXPECT_EQ(1U, Extract(i.raw, layout::Leap, "cond"));
    EXPECT_EQ(1U, Extract(i.raw, layout::Leap, "save"));
    EXPECT_EQ((-3) & 0x3FF, static_cast<int>(Extract(i.raw, layout::Leap, "imm")));
}

TEST(LayoutTest, LeapImmediateFieldsMatchCppBitfields) {
    Instruction i{isa::instructions::LeapImmediate{isa::Address{0x1000U}, true}};
    EXPECT_EQ(1U, Extract(i.raw, layout::LeapImmediate, "immediate"));
    EXPECT_EQ(1U, Extract(i.raw, layout::LeapImmediate, "save"));
    EXPECT_EQ(0x1000U >> 2U, Extract(i.raw, layout::LeapImmediate, "imm"));
}

TEST(LayoutTest, GrowFieldsMatchCppBitfields) {
    using isa::instructions::Grow;
    Instruction i{Grow{E(1), E(2), Grow::ImmediateType{0x2A5}}};
    EXPECT_EQ(1U, Extract(i.raw, layout::Frame, "eval"));
    EXPECT_EQ(2U, Extract(i.raw, layout::Frame, "mask"));
    EXPECT_EQ(1U, Extract(i.raw, layout::Frame, "cond"));
    EXPECT_EQ(0x2A5U, Extract(i.raw, layout::Frame, "imm"));
}

}  // namespace
