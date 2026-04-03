#include <gtest/gtest.h>

#include <isa/cpu.hpp>

#include "../demo/demo_memory_view.hpp"

namespace {

constexpr isa::Address kSramStart{0x10000000U};
constexpr isa::Address kSramEnd{0x1000000FU};

}  // namespace

TEST(MemoryViewTest, FormatsRowsForByteHalfWordAndWordModes) {
    isa::Processor cpu;
    isa::TightlyCoupledMemory sram{isa::Range{kSramStart, kSramEnd}};

    ASSERT_TRUE(cpu.AddTightlyCoupledMemory(sram));
    ASSERT_TRUE(cpu.Poke(kSramStart + 0U, 0x44332211U));
    ASSERT_TRUE(cpu.Poke(kSramStart + 4U, 0x88776655U));

    EXPECT_EQ("0x10000000: 11 22 33 44  55 66 77 88  00 00 00 00  00 00 00 00  |.\"3DUfw.........|",
              isa::demo::FormatMemoryRow(cpu, kSramStart, isa::demo::MemoryViewMode::Byte));
    EXPECT_EQ("0x10000000: 2211 4433  6655 8877  0000 0000  0000 0000  |.\"3DUfw.........|",
              isa::demo::FormatMemoryRow(cpu, kSramStart, isa::demo::MemoryViewMode::HalfWord));
    EXPECT_EQ("0x10000000: 44332211  88776655  00000000  00000000  |.\"3DUfw.........|",
              isa::demo::FormatMemoryRow(cpu, kSramStart, isa::demo::MemoryViewMode::Word));
}

TEST(MemoryViewTest, WordModeHandlesUnalignedBaseAcrossWordBoundary) {
    isa::Processor cpu;
    isa::TightlyCoupledMemory sram{isa::Range{kSramStart, kSramEnd}};

    ASSERT_TRUE(cpu.AddTightlyCoupledMemory(sram));
    ASSERT_TRUE(cpu.Poke(kSramStart + 0U, 0x44332211U));
    ASSERT_TRUE(cpu.Poke(kSramStart + 4U, 0x88776655U));

    EXPECT_EQ("0x10000001: 55443322  00887766  00000000  --------  |\"3DUfw.........-|",
              isa::demo::FormatMemoryRow(cpu, kSramStart + 1U, isa::demo::MemoryViewMode::Word));
}

TEST(MemoryViewTest, FormatsUnavailableRowsForEachMode) {
    EXPECT_EQ("0x00000010: -- -- -- --  -- -- -- --  -- -- -- --  -- -- -- --  |----------------|",
              isa::demo::FormatUnavailableMemoryRow(isa::Address{0x10U}, isa::demo::MemoryViewMode::Byte));
    EXPECT_EQ("0x00000010: ---- ----  ---- ----  ---- ----  ---- ----  |----------------|",
              isa::demo::FormatUnavailableMemoryRow(isa::Address{0x10U}, isa::demo::MemoryViewMode::HalfWord));
    EXPECT_EQ("0x00000010: --------  --------  --------  --------  |----------------|",
              isa::demo::FormatUnavailableMemoryRow(isa::Address{0x10U}, isa::demo::MemoryViewMode::Word));
}