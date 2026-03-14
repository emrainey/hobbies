#pragma once

#include <array>
#include <string>

namespace isa {

class Processor;

using ScratchRegisterRows = std::array<std::string, 16>;

ScratchRegisterRows FormatScratchRegisterRows(Processor const& cpu);

}  // namespace isa