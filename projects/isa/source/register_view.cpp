#include <isa/register_view.hpp>

#include <isa/cpu.hpp>

#include <cstring>
#include <iomanip>
#include <sstream>

namespace isa {

namespace {

std::string FormatHexValue(Address value) {
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

std::string FormatRegisterRow(std::string const& name, int name_width, std::string const& value, int value_width) {
    std::ostringstream stream;
    stream << std::right << std::setw(name_width) << name << " : " << std::left << std::setw(value_width) << value;
    return stream.str();
}

}  // namespace

ScratchRegisterRows FormatScratchRegisterRows(Processor const& cpu) {
    ScratchRegisterRows rows{};
    auto const& scratch = cpu.ViewScratch();
    constexpr int scratch_name_width = 3;    // S15
    constexpr int scratch_value_width = 10;  // 0x + 8 hex digits

    for (size_t index = 0; index < rows.size(); ++index) {
        std::ostringstream name_stream;
        name_stream << 'S' << std::dec << index;
        rows[index] = FormatRegisterRow(name_stream.str(), scratch_name_width, FormatHexValue(scratch[index].as_u32[0]),
                                        scratch_value_width);
    }

    return rows;
}

SpecialRegisterRows FormatSpecialRegisterRows(Processor const& cpu) {
    SpecialRegisterRows rows{};
    auto const& special = cpu.ViewSpecial();
    constexpr int special_name_width = 4;                                       // ESBA
    constexpr int special_hex_digits = static_cast<int>(sizeof(Address) * 2U);  // 2 hex digits per byte
    constexpr int special_value_width = special_hex_digits + 2;                 // 0x + digits
    std::ostringstream register_stream;

    rows[0]
        = FormatRegisterRow("PA", special_name_width, FormatHexValue(special.program_address_), special_value_width);
    rows[1] = FormatRegisterRow("RA", special_name_width, FormatHexValue(special.return_address_), special_value_width);
    rows[2] = FormatRegisterRow("SLA", special_name_width, FormatHexValue(special.stack_.limit), special_value_width);
    rows[3] = FormatRegisterRow("SCA", special_name_width, FormatHexValue(special.stack_.current), special_value_width);
    rows[4] = FormatRegisterRow("SBA", special_name_width, FormatHexValue(special.stack_.base), special_value_width);
    rows[5] = FormatRegisterRow("ESLA", special_name_width, FormatHexValue(special.exception_stack_.limit),
                                special_value_width);
    rows[6] = FormatRegisterRow("ESCA", special_name_width, FormatHexValue(special.exception_stack_.current),
                                special_value_width);
    rows[7] = FormatRegisterRow("ESBA", special_name_width, FormatHexValue(special.exception_stack_.base),
                                special_value_width);

    register_stream << special.exception_;
    rows[8] = FormatRegisterRow("EXC", special_name_width, register_stream.str(), special_value_width);
    // clear the stream for reuse
    register_stream.str("");
    register_stream.clear();
    register_stream << special.mode_;
    rows[9] = FormatRegisterRow("MODE", special_name_width, register_stream.str(), special_value_width);
    // clear the stream for reuse
    register_stream.str("");
    register_stream.clear();
    rows[10] = FormatRegisterRow("PERF", special_name_width, FormatHexValue(special.performance_counter_),
                                 special_value_width);
    return rows;
}

EvaluationRegisterRows FormatEvaluationRegisterRows(Processor const& cpu) {
    EvaluationRegisterRows rows{};
    auto const& evals = cpu.ViewEvaluations();
    constexpr int eval_name_width = 3;    // E15
    constexpr int eval_value_width = 10;  // 0x + 8 hex digits

    for (size_t index = 0; index < rows.size(); ++index) {
        uint32_t raw = 0U;
        static_assert(sizeof(Evaluation) == sizeof(uint32_t), "Evaluation must be 32 bits");
        std::memcpy(&raw, &evals[index], sizeof(uint32_t));
        std::ostringstream name_stream;
        name_stream << 'E' << std::dec << index;
        std::ostringstream value_stream;
        value_stream << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << raw;
        rows[index] = FormatRegisterRow(name_stream.str(), eval_name_width, value_stream.str(), eval_value_width);
    }

    return rows;
}

}  // namespace isa