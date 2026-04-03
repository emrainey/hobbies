#pragma once

#include <string>

#include <isa/cpu.hpp>

namespace isa::demo {

enum class MemoryViewMode {
    Byte = 1,
    HalfWord = 2,
    Word = 4,
};

inline constexpr size_t MemoryViewBytesPerRow = 16U;

char const* MemoryViewModeLabel(MemoryViewMode mode);
std::string FormatMemoryRow(Processor const& cpu, Address row_base_address, MemoryViewMode mode);
std::string FormatUnavailableMemoryRow(Address row_base_address, MemoryViewMode mode);

}  // namespace isa::demo