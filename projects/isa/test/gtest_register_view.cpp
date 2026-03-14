#include <gtest/gtest.h>

#include <isa/cpu.hpp>
#include <isa/register_view.hpp>

#include <iomanip>
#include <sstream>

namespace {

std::string AddressRow(char const* name, isa::Address value) {
    constexpr size_t special_name_width = 4U;
    constexpr size_t special_value_width = sizeof(isa::Address) * 2U + 2U;
    std::ostringstream stream;
    std::ostringstream value_stream;
    value_stream << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(isa::Address) * 2U)
                 << value;
    stream << std::right << std::setw(special_name_width) << name << " : " << std::left << std::setw(special_value_width)
           << value_stream.str();
    return stream.str();
}

std::string ScratchRow(char const* name, uint32_t value) {
    constexpr size_t scratch_name_width = 3U;
    constexpr size_t scratch_value_width = 10U;
    std::ostringstream stream;
    std::ostringstream value_stream;
    value_stream << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(8U) << value;
    stream << std::right << std::setw(scratch_name_width) << name << " : " << std::left << std::setw(scratch_value_width)
           << value_stream.str();
    return stream.str();
}

}  // namespace

TEST(RegisterViewTest, ScratchRowsReflectProcessorScratchRegisters) {
    isa::Processor cpu;
    auto& scratch = cpu.GetScratch();

    scratch[0] = isa::word<32>{0x00000001U};
    scratch[7] = isa::word<32>{0x1234ABCDU};
    scratch[15] = isa::word<32>{0xFFFFFFFFU};

    auto const rows = isa::FormatScratchRegisterRows(cpu);

    EXPECT_EQ(ScratchRow("S0", 0x00000001U), rows[0]);
    EXPECT_EQ(ScratchRow("S7", 0x1234ABCDU), rows[7]);
    EXPECT_EQ(ScratchRow("S15", 0xFFFFFFFFU), rows[15]);
    EXPECT_EQ(ScratchRow("S3", 0x00000000U), rows[3]);
}

TEST(RegisterViewTest, SpecialRowsReflectProcessorSpecialRegisters) {
    isa::Processor cpu;
    auto& special = cpu.GetSpecial();

    special.program_address_ = static_cast<isa::Address>(0x1110UL);
    special.return_address_ = static_cast<isa::Address>(0x2220UL);
    special.stack_.limit = static_cast<isa::Address>(0x3330UL);
    special.stack_.current = static_cast<isa::Address>(0x4440UL);
    special.stack_.base = static_cast<isa::Address>(0x5550UL);
    special.exception_stack_.limit = static_cast<isa::Address>(0x6660UL);
    special.exception_stack_.current = static_cast<isa::Address>(0x7770UL);
    special.exception_stack_.base = static_cast<isa::Address>(0x8880UL);

    auto const rows = isa::FormatSpecialRegisterRows(cpu);

    EXPECT_EQ(AddressRow("PA", special.program_address_), rows[0]);
    EXPECT_EQ(AddressRow("RA", special.return_address_), rows[1]);
    EXPECT_EQ(AddressRow("SLA", special.stack_.limit), rows[2]);
    EXPECT_EQ(AddressRow("SCA", special.stack_.current), rows[3]);
    EXPECT_EQ(AddressRow("SBA", special.stack_.base), rows[4]);
    EXPECT_EQ(AddressRow("ESLA", special.exception_stack_.limit), rows[5]);
    EXPECT_EQ(AddressRow("ESCA", special.exception_stack_.current), rows[6]);
    EXPECT_EQ(AddressRow("ESBA", special.exception_stack_.base), rows[7]);
}