#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <isa/cpu.hpp>

#include <chrono>
#include <filesystem>

namespace {

constexpr char kScratchRegistersFile[] = "scratch_registers.bin";
constexpr char kSpecialRegistersFile[] = "special_registers.bin";
constexpr char kInstructionCacheFile[] = "instruction_cache.bin";
constexpr char kDataCacheFile[] = "data_cache.bin";
constexpr char kEvaluationRegistersFile[] = "evaluation_registers.bin";
constexpr char kTightlyCoupledMemory0File[] = "tightly_coupled_memory_0.bin";

class ScopedTempDirectory {
public:
    ScopedTempDirectory()
        : path_{std::filesystem::temp_directory_path()
                / ("isa-cpu-persistence-"
                   + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))} {
        std::filesystem::create_directories(path_);
    }

    ~ScopedTempDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path_, error);
    }

    std::filesystem::path const& Path() const {
        return path_;
    }

private:
    std::filesystem::path path_;
};

}  // namespace

TEST(CpuPersistenceTest, SaveAndLoadRoundTripReportsFilesAndRestoresState) {
    ScopedTempDirectory temp_directory;

    isa::Processor saved_cpu;
    isa::TightlyCoupledMemory saved_memory{isa::Range{0x10000000U, 0x1000000FU}};
    ASSERT_TRUE(saved_cpu.AddTightlyCoupledMemory(saved_memory));
    saved_cpu.GetScratch()[2] = isa::word<32>{0xDEADBEEFU};
    saved_cpu.GetScratch()[9] = isa::word<32>{0x12345678U};

    auto& saved_special = saved_cpu.GetSpecial();
    saved_special.program_address_ = static_cast<isa::Address>(0x11110000UL);
    saved_special.return_address_ = static_cast<isa::Address>(0x22220000UL);
    saved_special.stack_.current = static_cast<isa::Address>(0x33330000UL);
    saved_special.exception_stack_.base = static_cast<isa::Address>(0x44440000UL);

    saved_cpu.GetInstructionCache()[1] = isa::instructions::Instruction{
        isa::instructions::MoveImmediateToScratch{isa::Operand{isa::OperandType::Scratch, 4}, isa::Immediate<20>{7U}}};
    saved_cpu.GetDataCache()[3][5] = 0x5AU;
    saved_cpu.GetEvaluations()[4].comparison = 1U;
    saved_cpu.GetEvaluations()[4].greater_than = 1U;
    saved_cpu.GetEvaluations()[4].non_zero = 1U;
    saved_memory[0x10000000U] = 0x10203040U;
    saved_memory[0x10000004U] = 0xA0B0C0D0U;

    auto const save_report = saved_cpu.Save(temp_directory.Path().string());

    EXPECT_TRUE(save_report.success);
    EXPECT_THAT(save_report.files,
                testing::ElementsAre(kScratchRegistersFile, kSpecialRegistersFile, kInstructionCacheFile,
                                     kDataCacheFile, kEvaluationRegistersFile, kTightlyCoupledMemory0File));

    isa::Processor restored_cpu;
    isa::TightlyCoupledMemory restored_memory{isa::Range{0x10000000U, 0x1000000FU}};
    ASSERT_TRUE(restored_cpu.AddTightlyCoupledMemory(restored_memory));
    restored_cpu.GetScratch()[2] = isa::word<32>{0x00000000U};
    restored_cpu.GetSpecial().program_address_ = static_cast<isa::Address>(0x1UL);
    restored_cpu.GetInstructionCache()[1] = isa::instructions::Instruction{isa::instructions::NoOp{}};
    restored_cpu.GetDataCache()[3][5] = 0x00U;
    restored_cpu.GetEvaluations()[4] = isa::Evaluation{};
    restored_memory[0x10000000U] = 0x00000000U;
    restored_memory[0x10000004U] = 0x00000000U;

    auto const load_report = restored_cpu.Load(temp_directory.Path().string());

    EXPECT_TRUE(load_report.success);
    EXPECT_THAT(load_report.files,
                testing::ElementsAre(kScratchRegistersFile, kSpecialRegistersFile, kInstructionCacheFile,
                                     kDataCacheFile, kEvaluationRegistersFile, kTightlyCoupledMemory0File));
    EXPECT_EQ(0xDEADBEEFU, restored_cpu.ViewScratch()[2].as_u32[0]);
    EXPECT_EQ(0x12345678U, restored_cpu.ViewScratch()[9].as_u32[0]);
    EXPECT_EQ(saved_special.program_address_, restored_cpu.ViewSpecial().program_address_);
    EXPECT_EQ(saved_special.return_address_, restored_cpu.ViewSpecial().return_address_);
    EXPECT_EQ(saved_special.stack_.current, restored_cpu.ViewSpecial().stack_.current);
    EXPECT_EQ(saved_special.exception_stack_.base, restored_cpu.ViewSpecial().exception_stack_.base);
    EXPECT_EQ(isa::Operator::MoveImmediateToScratch, restored_cpu.ViewInstructionCache()[1].base());
    EXPECT_EQ(0x5AU, restored_cpu.ViewDataCache()[3][5]);
    EXPECT_EQ(1U, restored_cpu.ViewEvaluations()[4].comparison);
    EXPECT_EQ(1U, restored_cpu.ViewEvaluations()[4].greater_than);
    EXPECT_EQ(1U, restored_cpu.ViewEvaluations()[4].non_zero);
    EXPECT_EQ(0x10203040U, restored_memory[0x10000000U]);
    EXPECT_EQ(0xA0B0C0D0U, restored_memory[0x10000004U]);
}

TEST(CpuPersistenceTest, LoadFailureLeavesProcessorStateUnchanged) {
    ScopedTempDirectory temp_directory;

    isa::Processor saved_cpu;
    saved_cpu.GetScratch()[3] = isa::word<32>{0xABCDEF12U};
    ASSERT_TRUE(saved_cpu.Save(temp_directory.Path().string()).success);
    std::filesystem::remove(temp_directory.Path() / kDataCacheFile);

    isa::Processor loaded_cpu;
    loaded_cpu.GetScratch()[3] = isa::word<32>{0x13572468U};
    loaded_cpu.GetSpecial().program_address_ = static_cast<isa::Address>(0xCAFEUL);

    auto const load_report = loaded_cpu.Load(temp_directory.Path().string());

    EXPECT_FALSE(load_report.success);
    EXPECT_THAT(load_report.summary, testing::HasSubstr(kDataCacheFile));
    EXPECT_EQ(0x13572468U, loaded_cpu.ViewScratch()[3].as_u32[0]);
    EXPECT_EQ(static_cast<isa::Address>(0xCAFEUL), loaded_cpu.ViewSpecial().program_address_);
}