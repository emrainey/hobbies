#include <gtest/gtest.h>

#include <isa/cpu.hpp>

namespace {

constexpr size_t kExpectedBreakPointSlots = 16U;

}  // namespace

TEST(BreakPointTest, DefaultsToNoConfiguredBreakPoints) {
    isa::Processor cpu;

    EXPECT_EQ(isa::invalid, cpu.GetBreakPoint());
    for (size_t i = 0; i < kExpectedBreakPointSlots; ++i) {
        EXPECT_EQ(isa::invalid, cpu.GetBreakPoint(i));
    }

    // Out-of-range index should return invalid.
    EXPECT_EQ(isa::invalid, cpu.GetBreakPoint(kExpectedBreakPointSlots));
}

TEST(BreakPointTest, SetAndGetReturnConfiguredAddresses) {
    isa::Processor cpu;
    isa::Address const first{0x00100010U};
    isa::Address const second{0x00100020U};

    ASSERT_TRUE(cpu.SetBreakPoint(first));
    ASSERT_TRUE(cpu.SetBreakPoint(second));

    EXPECT_EQ(first, cpu.GetBreakPoint());
    EXPECT_EQ(first, cpu.GetBreakPoint(0));
    EXPECT_EQ(second, cpu.GetBreakPoint(1));
}

TEST(BreakPointTest, SettingDuplicateBreakPointDoesNotConsumeAnotherSlot) {
    isa::Processor cpu;
    isa::Address const address{0x00100100U};

    ASSERT_TRUE(cpu.SetBreakPoint(address));
    ASSERT_TRUE(cpu.SetBreakPoint(address));

    EXPECT_EQ(address, cpu.GetBreakPoint(0));
    EXPECT_EQ(isa::invalid, cpu.GetBreakPoint(1));
}

TEST(BreakPointTest, ClearRemovesConfiguredBreakPoint) {
    isa::Processor cpu;
    isa::Address const first{0x00100010U};
    isa::Address const second{0x00100020U};

    ASSERT_TRUE(cpu.SetBreakPoint(first));
    ASSERT_TRUE(cpu.SetBreakPoint(second));
    ASSERT_TRUE(cpu.ClearBreakPoint(first));

    EXPECT_EQ(second, cpu.GetBreakPoint());
    EXPECT_EQ(isa::invalid, cpu.GetBreakPoint(0));
    EXPECT_EQ(second, cpu.GetBreakPoint(1));
    EXPECT_FALSE(cpu.ClearBreakPoint(first));
}

TEST(BreakPointTest, SetFailsWhenBreakPointTableIsFull) {
    isa::Processor cpu;

    for (size_t i = 0; i < kExpectedBreakPointSlots; ++i) {
        ASSERT_TRUE(cpu.SetBreakPoint(isa::Address{0x00100000U + static_cast<uint32_t>(i * 4U)}));
    }

    EXPECT_FALSE(cpu.SetBreakPoint(isa::Address{0x00101000U}));
}