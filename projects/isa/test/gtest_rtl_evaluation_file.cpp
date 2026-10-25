#include <cstdint>

#include <gtest/gtest.h>

#include <Vevaluation_file.h>

namespace {

constexpr uint8_t kNoPorts = 0b00;
constexpr uint8_t kFirstPort = 0b01;
constexpr uint8_t kSecondPort = 0b10;
constexpr uint8_t kBothPorts = 0b11;
constexpr uint32_t kRegisterCount = 16;

class RtlEvaluationFileTest : public ::testing::Test {
protected:
    void SetUp() override {
        dut_.clock = 0;
        dut_.reset = 0;
        dut_.write_control = kNoPorts;
        dut_.read_control = kNoPorts;
        for (int i = 0; i < 2; ++i) {
            dut_.write_index[i] = 0;
            dut_.write_data[i] = 0;
            dut_.read_index[i] = 0;
        }
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

    void SetRead(uint8_t control, uint8_t index0, uint8_t index1) {
        dut_.read_control = control;
        dut_.read_index[0] = index0;
        dut_.read_index[1] = index1;
        dut_.eval();
    }

    void Write(uint8_t control, uint8_t index0, uint8_t data0, uint8_t index1, uint8_t data1) {
        dut_.write_control = control;
        dut_.write_index[0] = index0;
        dut_.write_data[0] = data0;
        dut_.write_index[1] = index1;
        dut_.write_data[1] = data1;
        Clock();
        dut_.write_control = kNoPorts;
        dut_.eval();
    }

    Vevaluation_file dut_;
};

TEST_F(RtlEvaluationFileTest, ResetClearsEveryRegister) {
    for (uint8_t index = 0; index < kRegisterCount; ++index) {
        SetRead(kFirstPort, index, 0);
        EXPECT_EQ(dut_.read_data[0], 0U) << "evaluation[" << unsigned(index) << "]";
    }
}

TEST_F(RtlEvaluationFileTest, ResetClearsPreviouslyWrittenRegisters) {
    Write(kBothPorts, 2, 0xAA, 3, 0x55);
    dut_.reset = 1;
    dut_.eval();
    dut_.reset = 0;
    SetRead(kBothPorts, 2, 3);
    EXPECT_EQ(dut_.read_data[0], 0U);
    EXPECT_EQ(dut_.read_data[1], 0U);
}

TEST_F(RtlEvaluationFileTest, ReadsAreCombinationalAndHonorPortSelection) {
    Write(kBothPorts, 3, 0x12, 9, 0xAB);

    SetRead(kBothPorts, 3, 9);
    EXPECT_EQ(dut_.read_data[0], 0x12U);
    EXPECT_EQ(dut_.read_data[1], 0xABU);

    SetRead(kFirstPort, 9, 3);
    EXPECT_EQ(dut_.read_data[0], 0xABU);
    EXPECT_EQ(dut_.read_data[1], 0U);

    SetRead(kSecondPort, 3, 9);
    EXPECT_EQ(dut_.read_data[0], 0U);
    EXPECT_EQ(dut_.read_data[1], 0xABU);

    SetRead(kNoPorts, 3, 9);
    EXPECT_EQ(dut_.read_data[0], 0U);
    EXPECT_EQ(dut_.read_data[1], 0U);
}

TEST_F(RtlEvaluationFileTest, ReadsFollowIndexChangesWithoutAClockEdge) {
    Write(kBothPorts, 1, 0x11, 2, 0x22);
    SetRead(kBothPorts, 1, 2);
    EXPECT_EQ(dut_.read_data[0], 0x11U);
    EXPECT_EQ(dut_.read_data[1], 0x22U);

    SetRead(kBothPorts, 2, 1);
    EXPECT_EQ(dut_.read_data[0], 0x22U);
    EXPECT_EQ(dut_.read_data[1], 0x11U);
}

TEST_F(RtlEvaluationFileTest, ReadsOldValueBeforeWriteEdgeAndUpdatedValueAfterward) {
    Write(kFirstPort, 5, 0x11, 0, 0);

    dut_.write_control = kFirstPort;
    dut_.write_index[0] = 5;
    dut_.write_data[0] = 0x77;
    SetRead(kFirstPort, 5, 0);
    EXPECT_EQ(dut_.read_data[0], 0x11U);

    Clock();
    EXPECT_EQ(dut_.read_data[0], 0x77U);
}

TEST_F(RtlEvaluationFileTest, WritesCanUseEitherPortIndependently) {
    Write(kFirstPort, 2, 0x22, 0, 0);
    Write(kSecondPort, 0, 0, 7, 0x77);

    SetRead(kBothPorts, 2, 7);
    EXPECT_EQ(dut_.read_data[0], 0x22U);
    EXPECT_EQ(dut_.read_data[1], 0x77U);
}

TEST_F(RtlEvaluationFileTest, FirstPortWriteIgnoresSecondPortInputs) {
    Write(kFirstPort, 4, 0x44, 6, 0x66);

    SetRead(kBothPorts, 4, 6);
    EXPECT_EQ(dut_.read_data[0], 0x44U);
    EXPECT_EQ(dut_.read_data[1], 0U);
}

TEST_F(RtlEvaluationFileTest, SecondPortWriteIgnoresFirstPortInputs) {
    Write(kSecondPort, 4, 0x44, 6, 0x66);

    SetRead(kBothPorts, 4, 6);
    EXPECT_EQ(dut_.read_data[0], 0U);
    EXPECT_EQ(dut_.read_data[1], 0x66U);
}

TEST_F(RtlEvaluationFileTest, SimultaneousWritesUpdateDistinctRegisters) {
    Write(kBothPorts, 1, 0x01, 14, 0x14);

    SetRead(kBothPorts, 1, 14);
    EXPECT_EQ(dut_.read_data[0], 0x01U);
    EXPECT_EQ(dut_.read_data[1], 0x14U);
}

TEST_F(RtlEvaluationFileTest, PortZeroWinsWhenBothWritesTargetSameRegister) {
    Write(kBothPorts, 6, 0xAA, 6, 0xBB);

    SetRead(kFirstPort, 6, 0);
    EXPECT_EQ(dut_.read_data[0], 0xAAU);
}

TEST_F(RtlEvaluationFileTest, NoWriteControlPreservesStoredValues) {
    Write(kFirstPort, 4, 0x44, 0, 0);

    dut_.write_control = kNoPorts;
    dut_.write_data[0] = 0xFF;
    dut_.write_index[0] = 4;
    Clock();

    SetRead(kFirstPort, 4, 0);
    EXPECT_EQ(dut_.read_data[0], 0x44U);
}

TEST_F(RtlEvaluationFileTest, WritesToOneRegisterDoNotDisturbOthers) {
    for (uint8_t index = 0; index < kRegisterCount; ++index) {
        Write(kFirstPort, index, static_cast<uint8_t>(0x10 + index), 0, 0);
    }
    for (uint8_t index = 0; index < kRegisterCount; ++index) {
        SetRead(kFirstPort, index, 0);
        EXPECT_EQ(dut_.read_data[0], 0x10U + index) << "evaluation[" << unsigned(index) << "]";
    }
}

TEST_F(RtlEvaluationFileTest, StoresFullByteWidth) {
    Write(kBothPorts, 0, 0xFF, 15, 0x80);

    SetRead(kBothPorts, 0, 15);
    EXPECT_EQ(dut_.read_data[0], 0xFFU);
    EXPECT_EQ(dut_.read_data[1], 0x80U);
}

TEST_F(RtlEvaluationFileTest, ClearMaskZeroesOnlySelectedRegisters) {
    Write(kBothPorts, 1, 0xA5, 2, 0xA5);
    Write(kBothPorts, 3, 0xA5, 4, 0xA5);

    dut_.clear_mask = 0b00110;  // registers 1 and 2
    Clock();
    dut_.clear_mask = 0;

    SetRead(kBothPorts, 1, 2);
    EXPECT_EQ(dut_.read_data[0], 0U);
    EXPECT_EQ(dut_.read_data[1], 0U);
    SetRead(kBothPorts, 3, 4);
    EXPECT_EQ(dut_.read_data[0], 0xA5);
    EXPECT_EQ(dut_.read_data[1], 0xA5);
}

TEST_F(RtlEvaluationFileTest, WriteWinsOverClearMaskOnTheSameRegister) {
    dut_.clear_mask = 0b00010;
    Write(kFirstPort, 1, 0xA5, 0, 0);
    dut_.clear_mask = 0;

    SetRead(kFirstPort, 1, 0);
    EXPECT_EQ(dut_.read_data[0], 0xA5);
}

}  // namespace
