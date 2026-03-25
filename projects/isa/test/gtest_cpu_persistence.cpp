#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <isa/cpu.hpp>

#include <chrono>
#include <filesystem>

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

class CpuPersistenceTest : public testing::Test {
protected:
    ScopedTempDirectory temp_directory;
    isa::Processor saved_cpu;
    isa::TightlyCoupledMemory saved_memory{isa::Range{0x10000000U, 0x1000000FU}};
    isa::Processor restored_cpu;
    isa::TightlyCoupledMemory restored_memory{isa::Range{0x10000000U, 0x1000000FU}};

    CpuPersistenceTest()
        : saved_memory{isa::Range{0x10000000U, 0x1000000FU}}, restored_memory{isa::Range{0x10000000U, 0x1000000FU}} {
        EXPECT_TRUE(saved_cpu.AddTightlyCoupledMemory(saved_memory));
        EXPECT_TRUE(restored_cpu.AddTightlyCoupledMemory(restored_memory));
    }

    void PopulateSavedState() {
        saved_cpu.GetScratch()[2] = isa::word<32>{0xDEADBEEFU};
        saved_cpu.GetScratch()[9] = isa::word<32>{0x12345678U};

        auto& saved_special = saved_cpu.GetSpecial();
        saved_special.program_address_ = static_cast<isa::Address>(0x11110000UL);
        saved_special.return_address_ = static_cast<isa::Address>(0x22220000UL);
        saved_special.stack_.current = static_cast<isa::Address>(0x33330000UL);
        saved_special.exception_stack_.base = static_cast<isa::Address>(0x44440000UL);

        saved_cpu.GetEvaluations()[4].bits.comparison = 1U;
        saved_cpu.GetEvaluations()[4].bits.greater_than = 1U;
        saved_cpu.GetEvaluations()[4].bits.non_zero = 1U;
        saved_memory[0x10000000U] = 0x10203040U;
        saved_memory[0x10000004U] = 0xA0B0C0D0U;
    }

    void InitializeRestoredState() {
        restored_cpu.GetScratch()[2] = isa::word<32>{0x00000000U};
        restored_cpu.GetSpecial().program_address_ = static_cast<isa::Address>(0x1UL);
        restored_cpu.GetEvaluations()[4] = isa::Evaluation{};
        restored_memory[0x10000000U] = 0x00000000U;
        restored_memory[0x10000004U] = 0x00000000U;
    }

    void ExpectSaveReportSuccess() {
        auto save_report = saved_cpu.Save(temp_directory.Path().string());
        ASSERT_TRUE(save_report.success);
        // Only non-empty components are saved; empty caches are not persisted
        EXPECT_THAT(save_report.files, testing::ElementsAre(kScratchRegistersFile, kSpecialRegistersFile,
                                                            kEvaluationRegistersFile, kTightlyCoupledMemory0File));
    }

    void ExpectLoadReportSuccess() {
        auto load_report = restored_cpu.Load(temp_directory.Path().string());
        ASSERT_TRUE(load_report.success);
        EXPECT_THAT(load_report.files, testing::ElementsAre(kScratchRegistersFile, kSpecialRegistersFile,
                                                            kEvaluationRegistersFile, kTightlyCoupledMemory0File));
    }

    void VerifyRestoredState() {
        EXPECT_EQ(0xDEADBEEFU, restored_cpu.ViewScratch()[2].as_u32[0]);
        EXPECT_EQ(0x12345678U, restored_cpu.ViewScratch()[9].as_u32[0]);
        EXPECT_EQ(static_cast<isa::Address>(0x11110000UL), restored_cpu.ViewSpecial().program_address_);
        EXPECT_EQ(static_cast<isa::Address>(0x22220000UL), restored_cpu.ViewSpecial().return_address_);
        EXPECT_EQ(static_cast<isa::Address>(0x33330000UL), restored_cpu.ViewSpecial().stack_.current);
        EXPECT_EQ(static_cast<isa::Address>(0x44440000UL), restored_cpu.ViewSpecial().exception_stack_.base);
        EXPECT_EQ(1U, restored_cpu.ViewEvaluations()[4].bits.comparison);
        EXPECT_EQ(1U, restored_cpu.ViewEvaluations()[4].bits.greater_than);
        EXPECT_EQ(1U, restored_cpu.ViewEvaluations()[4].bits.non_zero);
        EXPECT_EQ(0x10203040U, restored_memory[0x10000000U]);
        EXPECT_EQ(0xA0B0C0D0U, restored_memory[0x10000004U]);
    }
};

TEST_F(CpuPersistenceTest, SaveAndLoadRoundTripReportsFilesAndRestoresState) {
    PopulateSavedState();
    auto const save_report = saved_cpu.Save(temp_directory.Path().string());
    ASSERT_TRUE(save_report.success);
    ExpectSaveReportSuccess();

    InitializeRestoredState();
    auto const load_report = restored_cpu.Load(temp_directory.Path().string());
    ASSERT_TRUE(load_report.success);
    ExpectLoadReportSuccess();

    VerifyRestoredState();
}

TEST_F(CpuPersistenceTest, LoadFailureLeavesProcessorStateUnchanged) {
    saved_cpu.GetScratch()[3] = isa::word<32>{0xABCDEF12U};
    auto save_report = saved_cpu.Save(temp_directory.Path().string());
    ASSERT_TRUE(save_report.success);

    // Delete a file that was actually saved to corrupt the save set
    std::filesystem::remove(temp_directory.Path() / kScratchRegistersFile);

    restored_cpu.GetScratch()[3] = isa::word<32>{0x13572468U};
    restored_cpu.GetSpecial().program_address_ = static_cast<isa::Address>(0xCAFEUL);

    auto const load_report = restored_cpu.Load(temp_directory.Path().string());

    EXPECT_FALSE(load_report.success);
    EXPECT_THAT(load_report.summary, testing::HasSubstr(kScratchRegistersFile));
    // Verify state was not changed by the failed load
    EXPECT_EQ(0x13572468U, restored_cpu.ViewScratch()[3].as_u32[0]);
    EXPECT_EQ(static_cast<isa::Address>(0xCAFEUL), restored_cpu.ViewSpecial().program_address_);
}