#include <cstdint>

#include <gtest/gtest.h>

#include <Vregister_file.h>

namespace {

constexpr uint8_t kNoPorts = 0b00;
constexpr uint8_t kFirstPort = 0b01;
constexpr uint8_t kSecondPort = 0b10;
constexpr uint8_t kBothPorts = 0b11;
constexpr uint32_t kRegisterCount = 16;

class RtlRegisterFileTest : public ::testing::Test {
protected:
    void SetUp() override {
        dut_.clock = 0;
        dut_.reset = 0;
        dut_.write_control = kNoPorts;
        dut_.read_control = kNoPorts;
        dut_.write_index[0] = 0;
        dut_.write_index[1] = 0;
        dut_.write_data[0] = 0;
        dut_.write_data[1] = 0;
        dut_.read_index[0] = 0;
        dut_.read_index[1] = 0;
        dut_.eval();

        dut_.reset = 1;
        dut_.eval();
        dut_.reset = 0;
        dut_.eval();
    }

    void Clock() {
        dut_.clock = 0;
        dut_.eval();
        dut_.clock = 1;
        dut_.eval();
        dut_.clock = 0;
        dut_.eval();
    }

    void Write(uint8_t control, uint8_t index0, uint32_t data0, uint8_t index1, uint32_t data1) {
        dut_.write_control = control;
        dut_.write_index[0] = index0;
        dut_.write_data[0] = data0;
        dut_.write_index[1] = index1;
        dut_.write_data[1] = data1;
        Clock();
        dut_.write_control = kNoPorts;
        dut_.eval();
    }

    void SetRead(uint8_t control, uint8_t index0, uint8_t index1) {
        dut_.read_control = control;
        dut_.read_index[0] = index0;
        dut_.read_index[1] = index1;
        dut_.eval();
    }

    Vregister_file dut_;
};

TEST_F(RtlRegisterFileTest, ResetClearsEveryRegister) {
    for (uint8_t index = 0; index < kRegisterCount; ++index) {
        SetRead(kFirstPort, index, 0);
        EXPECT_EQ(dut_.read_data[0], 0U) << "scratch[" << unsigned(index) << "]";
    }
}

TEST_F(RtlRegisterFileTest, ReadsAreCombinationalAndHonorPortSelection) {
    dut_.write_control = kBothPorts;
    dut_.write_index[0] = 3;
    dut_.write_index[1] = 9;
    dut_.write_data[0] = 0x12345678U;
    dut_.write_data[1] = 0xABCDEF01U;
    Clock();
    dut_.write_control = kNoPorts;

    SetRead(kBothPorts, 3, 9);
    EXPECT_EQ(dut_.read_data[0], 0x12345678U);
    EXPECT_EQ(dut_.read_data[1], 0xABCDEF01U);

    SetRead(kFirstPort, 9, 3);
    EXPECT_EQ(dut_.read_data[0], 0xABCDEF01U);
    EXPECT_EQ(dut_.read_data[1], 0U);

    SetRead(kSecondPort, 3, 9);
    EXPECT_EQ(dut_.read_data[0], 0U);
    EXPECT_EQ(dut_.read_data[1], 0xABCDEF01U);

    SetRead(kNoPorts, 3, 9);
    EXPECT_EQ(dut_.read_data[0], 0U);
    EXPECT_EQ(dut_.read_data[1], 0U);
}

TEST_F(RtlRegisterFileTest, ReadsOldValueBeforeWriteEdgeAndUpdatedValueAfterward) {
    dut_.write_control = kFirstPort;
    dut_.write_index[0] = 5;
    dut_.write_data[0] = 0x11223344U;
    Clock();

    dut_.write_control = kFirstPort;
    dut_.write_index[0] = 5;
    dut_.write_data[0] = 0x55667788U;
    SetRead(kFirstPort, 5, 0);
    EXPECT_EQ(dut_.read_data[0], 0x11223344U);

    Clock();
    EXPECT_EQ(dut_.read_data[0], 0x55667788U);
}

TEST_F(RtlRegisterFileTest, WritesCanUseEitherPortIndependently) {
    dut_.write_control = kFirstPort;
    dut_.write_index[0] = 2;
    dut_.write_data[0] = 0x22222222U;
    Clock();

    dut_.write_control = kSecondPort;
    dut_.write_index[1] = 7;
    dut_.write_data[1] = 0x77777777U;
    Clock();

    SetRead(kBothPorts, 2, 7);
    EXPECT_EQ(dut_.read_data[0], 0x22222222U);
    EXPECT_EQ(dut_.read_data[1], 0x77777777U);
}

TEST_F(RtlRegisterFileTest, SimultaneousWritesUpdateDistinctRegisters) {
    dut_.write_control = kBothPorts;
    dut_.write_index[0] = 1;
    dut_.write_index[1] = 14;
    dut_.write_data[0] = 0x01010101U;
    dut_.write_data[1] = 0x14141414U;
    Clock();

    SetRead(kBothPorts, 1, 14);
    EXPECT_EQ(dut_.read_data[0], 0x01010101U);
    EXPECT_EQ(dut_.read_data[1], 0x14141414U);
}

TEST_F(RtlRegisterFileTest, PortZeroWinsWhenBothWritesTargetSameRegister) {
    dut_.write_control = kBothPorts;
    dut_.write_index[0] = 6;
    dut_.write_index[1] = 6;
    dut_.write_data[0] = 0xAAAAAAAAU;
    dut_.write_data[1] = 0xBBBBBBBBU;
    Clock();

    SetRead(kFirstPort, 6, 0);
    EXPECT_EQ(dut_.read_data[0], 0xAAAAAAAAU);
}

TEST_F(RtlRegisterFileTest, NoWriteControlPreservesStoredValues) {
    dut_.write_control = kFirstPort;
    dut_.write_index[0] = 4;
    dut_.write_data[0] = 0x44444444U;
    Clock();

    dut_.write_control = kNoPorts;
    dut_.write_data[0] = 0xFFFFFFFFU;
    Clock();

    SetRead(kFirstPort, 4, 0);
    EXPECT_EQ(dut_.read_data[0], 0x44444444U);
}

TEST_F(RtlRegisterFileTest, ClearMaskZeroesOnlySelectedRegisters) {
    Write(kBothPorts, 1, 0xDEADBEEF, 2, 0xDEADBEEF);
    Write(kBothPorts, 3, 0xDEADBEEF, 4, 0xDEADBEEF);

    dut_.clear_mask = 0b00110;  // registers 1 and 2
    Clock();
    dut_.clear_mask = 0;

    SetRead(kBothPorts, 1, 2);
    EXPECT_EQ(dut_.read_data[0], 0U);
    EXPECT_EQ(dut_.read_data[1], 0U);
    SetRead(kBothPorts, 3, 4);
    EXPECT_EQ(dut_.read_data[0], 0xDEADBEEF);
    EXPECT_EQ(dut_.read_data[1], 0xDEADBEEF);
}

TEST_F(RtlRegisterFileTest, WriteWinsOverClearMaskOnTheSameRegister) {
    dut_.clear_mask = 0b00010;
    Write(kFirstPort, 1, 0xDEADBEEF, 0, 0);
    dut_.clear_mask = 0;

    SetRead(kFirstPort, 1, 0);
    EXPECT_EQ(dut_.read_data[0], 0xDEADBEEF);
}

}  // namespace
