#include <gtest/gtest.h>

#include <isa/cpu.hpp>
#include <isa/register_view.hpp>

TEST(RegisterViewTest, ScratchRowsReflectProcessorScratchRegisters) {
    isa::Processor cpu;
    auto& scratch = cpu.GetScratch();

    scratch[0] = isa::word<32>{0x00000001U};
    scratch[7] = isa::word<32>{0x1234ABCDU};
    scratch[15] = isa::word<32>{0xFFFFFFFFU};

    auto const rows = isa::FormatScratchRegisterRows(cpu);

    EXPECT_EQ("S0 : 0x00000001", rows[0]);
    EXPECT_EQ("S7 : 0x1234ABCD", rows[7]);
    EXPECT_EQ("S15 : 0xFFFFFFFF", rows[15]);
    EXPECT_EQ("S3 : 0x00000000", rows[3]);
}