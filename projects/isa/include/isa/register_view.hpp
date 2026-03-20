#pragma once

#include <array>
#include <string>

namespace isa {

class Processor;

using ScratchRegisterRows = std::array<std::string, 16>;
using SpecialRegisterRows = std::array<std::string, 11>;
using EvaluationRegisterRows = std::array<std::string, 16>;

ScratchRegisterRows FormatScratchRegisterRows(Processor const& cpu);
SpecialRegisterRows FormatSpecialRegisterRows(Processor const& cpu);
EvaluationRegisterRows FormatEvaluationRegisterRows(Processor const& cpu);

}  // namespace isa