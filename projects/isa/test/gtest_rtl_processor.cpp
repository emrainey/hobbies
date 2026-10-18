#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

#include <Vprocessor.h>
#include <isa/cpu.hpp>

namespace {

constexpr uint32_t kFlashBase = 0x00100000U;
constexpr uint32_t kFlashLimit = 0x0010FFFFU;
constexpr uint32_t kCodeAddress = 0x00100100U;
constexpr uint32_t kStackInitial = 0x0010F000U;
constexpr uint32_t kStackBoundary = 0x00100000U;
constexpr uint32_t kExceptionStackInitial = 0x0010E000U;
constexpr uint32_t kExceptionStackBoundary = 0x0010D000U;
constexpr uint32_t kRetiredSteps = 32U;

}  // namespace

/// Lockstep harness for M0: the C++ model is the oracle and the RTL is stepped one
/// retired instruction at a time. Both models share `oracle`'s attached flash, which
/// services the DUT bus exactly as `Processor::Peek` services the fast model.
class RtlProcessorTest : public testing::Test {
protected:
    isa::Processor oracle;
    isa::FlashMemory flash{isa::Range{kFlashBase, kFlashLimit}};
    Vprocessor dut;

    void SetUp() override {
        ASSERT_TRUE(oracle.AttachFlashMemory(flash));

        // Vector table, matching isa::VectorTable field order.
        ASSERT_TRUE(oracle.Poke(isa::Address{kFlashBase + 0U}, kStackInitial));
        ASSERT_TRUE(oracle.Poke(isa::Address{kFlashBase + 4U}, kStackBoundary));
        ASSERT_TRUE(oracle.Poke(isa::Address{kFlashBase + 8U}, kExceptionStackInitial));
        ASSERT_TRUE(oracle.Poke(isa::Address{kFlashBase + 12U}, kExceptionStackBoundary));
        ASSERT_TRUE(oracle.Poke(isa::Address{kFlashBase + 16U}, kCodeAddress));

        // An erased flash decodes as a stream of noops.
        for (uint32_t offset = 0U; offset < kRetiredSteps * 4U; offset += 4U) {
            ASSERT_TRUE(oracle.Poke(isa::Address{kCodeAddress + offset}, 0xFFFFFFFFU));
        }

        oracle.Reset();

        dut.reset = 1;
        Clock();
        dut.reset = 0;

        ASSERT_TRUE(RunUntilBooted());
    }

    /// Services one bus access from the DUT using the oracle's memory.
    void ServiceBus() {
        uint32_t value = 0U;
        dut.mem_rdata = 0U;
        if (dut.mem_req && oracle.Peek(isa::Address{static_cast<uint32_t>(dut.mem_addr)}, value)) {
            dut.mem_rdata = value;
            dut.mem_fault = 0;
        } else {
            dut.mem_fault = dut.mem_req ? 1 : 0;
        }
    }

    /// Drives one rising clock edge, with the bus serviced before the edge.
    void Clock() {
        dut.clk = 0;
        dut.eval();
        ServiceBus();
        dut.eval();
        dut.clk = 1;
        dut.eval();
    }

    bool RunUntilBooted() {
        for (int i = 0; i < 100 && !dut.booted; ++i) {
            Clock();
        }
        return dut.booted;
    }

    bool RunUntilRetire() {
        for (int i = 0; i < 1000; ++i) {
            Clock();
            if (dut.retire) {
                return true;
            }
            if (dut.halted) {
                return false;
            }
        }
        return false;
    }

    void ExpectStateMatches() {
        for (size_t i = 0; i < isa::CountOfScratchRegisters; ++i) {
            EXPECT_EQ(oracle.ViewScratch()[i].as_u32[0], dut.scratch[i]) << "scratch[" << i << "]";
        }
        for (size_t i = 0; i < isa::CountOfEvaluationRegisters; ++i) {
            EXPECT_EQ(oracle.ViewEvaluations()[i].value, dut.evaluation[i]) << "evaluation[" << i << "]";
        }
        for (size_t i = 0; i < isa::CountOfSpecialRegisters; ++i) {
            EXPECT_EQ(oracle.ViewSpecial().as_word(i).as_u32[0], dut.special[i]) << "special[" << i << "]";
        }
        EXPECT_EQ(oracle.IsHalted(), static_cast<bool>(dut.halted));
    }
};

TEST_F(RtlProcessorTest, ResetStateMatchesTheFastModel) {
    ExpectStateMatches();
}

TEST_F(RtlProcessorTest, ErasedProgramRetiresNoOpsInLockstep) {
    for (uint32_t step = 0U; step < kRetiredSteps; ++step) {
        SCOPED_TRACE(::testing::Message() << "step " << step);
        oracle.Cycle();
        ASSERT_TRUE(RunUntilRetire());
        ExpectStateMatches();
    }
}
