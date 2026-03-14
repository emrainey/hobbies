#include <isa/register_view.hpp>

#include <isa/cpu.hpp>

#include <iomanip>
#include <sstream>

namespace isa {

ScratchRegisterRows FormatScratchRegisterRows(Processor const& cpu) {
    ScratchRegisterRows rows{};
    auto const& scratch = cpu.GetScratch();

    for (size_t index = 0; index < rows.size(); ++index) {
        std::ostringstream stream;
        stream << 'S' << std::dec << index << " : 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(8)
               << scratch[index].as_u32[0];
        rows[index] = stream.str();
    }

    return rows;
}

}  // namespace isa