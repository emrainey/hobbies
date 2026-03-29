#pragma once

#include <array>
#include <string>

#include <isa/cpu.hpp>

namespace isa {

class Processor;

using ScratchRegisterRows = std::array<std::string, CountOfScratchRegisters>;
using SpecialRegisterRows = std::array<std::string, CountOfSpecialRegisters>;
using EvaluationRegisterRows = std::array<std::string, CountOfEvaluationRegisters>;

ScratchRegisterRows FormatScratchRegisterRows(Processor const& cpu);
SpecialRegisterRows FormatSpecialRegisterRows(Processor const& cpu);
EvaluationRegisterRows FormatEvaluationRegisterRows(Processor const& cpu);

}  // namespace isa