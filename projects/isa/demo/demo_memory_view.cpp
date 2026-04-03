#include "demo_memory_view.hpp"

#include <cctype>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

namespace isa::demo {

namespace {

std::string FormatAddressHex(Address address) {
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(8)
           << static_cast<unsigned int>(address.value);
    return stream.str();
}

size_t MemoryCellWidthBytes(MemoryViewMode mode) {
    switch (mode) {
        case MemoryViewMode::Byte:
            return 1U;
        case MemoryViewMode::HalfWord:
            return 2U;
        case MemoryViewMode::Word:
            return 4U;
    }

    return 1U;
}

std::string UnavailableMemoryCell(MemoryViewMode mode) {
    return std::string(MemoryCellWidthBytes(mode) * 2U, '-');
}

char AsciiPlaceholderForByte(bool available, uint8_t value) {
    if (not available) {
        return '-';
    }

    if (std::isprint(static_cast<unsigned char>(value)) != 0) {
        return static_cast<char>(value);
    }

    return '.';
}

bool TryAddAddress(Address address, size_t offset, Address& out) {
    using StorageType = Address::StorageType;
    constexpr StorageType kAddressMax = std::numeric_limits<StorageType>::max();
    const auto safe_offset = static_cast<StorageType>(offset);
    if (safe_offset > kAddressMax - address.value) {
        return false;
    }

    out = Address{address.value + safe_offset};
    return true;
}

bool TryReadMemoryByte(Processor const& cpu, Address address, uint8_t& value) {
    uint32_t word = 0U;
    if (not cpu.Peek(address, word)) {
        return false;
    }

    constexpr uint32_t kBytesPerWord = sizeof(uint32_t);
    const uint32_t byte_lane = static_cast<uint32_t>(address.value % kBytesPerWord);
    value = static_cast<uint8_t>((word >> (byte_lane * 8U)) & 0xFFU);
    return true;
}

std::string FormatAsciiRow(Processor const* cpu, Address row_base_address) {
    std::string ascii;
    ascii.reserve(MemoryViewBytesPerRow);

    for (size_t offset = 0; offset < MemoryViewBytesPerRow; ++offset) {
        Address address{0};
        if (not TryAddAddress(row_base_address, offset, address)) {
            ascii.push_back(AsciiPlaceholderForByte(false, 0U));
            continue;
        }

        if (cpu == nullptr) {
            ascii.push_back(AsciiPlaceholderForByte(false, 0U));
            continue;
        }

        uint8_t byte = 0U;
        if (not TryReadMemoryByte(*cpu, address, byte)) {
            ascii.push_back(AsciiPlaceholderForByte(false, 0U));
            continue;
        }

        ascii.push_back(AsciiPlaceholderForByte(true, byte));
    }

    return ascii;
}

std::string FormatMemoryCell(Processor const& cpu, Address base_address, MemoryViewMode mode) {
    const size_t cell_width = MemoryCellWidthBytes(mode);
    uint32_t value = 0U;

    for (size_t offset = 0; offset < cell_width; ++offset) {
        Address address{0};
        if (not TryAddAddress(base_address, offset, address)) {
            return UnavailableMemoryCell(mode);
        }

        uint8_t byte = 0U;
        if (not TryReadMemoryByte(cpu, address, byte)) {
            return UnavailableMemoryCell(mode);
        }

        value |= static_cast<uint32_t>(byte) << (offset * 8U);
    }

    std::ostringstream stream;
    stream << std::uppercase << std::hex << std::setfill('0') << std::setw(static_cast<int>(cell_width * 2U)) << value;
    return stream.str();
}

std::string FormatMemoryRowImpl(Processor const* cpu, Address row_base_address, MemoryViewMode mode) {
    const size_t cell_width = MemoryCellWidthBytes(mode);
    const size_t cell_count = MemoryViewBytesPerRow / cell_width;

    std::ostringstream stream;
    stream << FormatAddressHex(row_base_address) << ": ";

    for (size_t cell = 0; cell < cell_count; ++cell) {
        Address cell_address{0};
        if (not TryAddAddress(row_base_address, cell * cell_width, cell_address)) {
            stream << UnavailableMemoryCell(mode);
        } else if (cpu != nullptr) {
            stream << FormatMemoryCell(*cpu, cell_address, mode);
        } else {
            stream << UnavailableMemoryCell(mode);
        }

        if (cell + 1U < cell_count) {
            const size_t bytes_rendered = (cell + 1U) * cell_width;
            stream << ((bytes_rendered % 4U == 0U) ? "  " : " ");
        }
    }

    stream << "  |" << FormatAsciiRow(cpu, row_base_address) << '|';

    return stream.str();
}

}  // namespace

char const* MemoryViewModeLabel(MemoryViewMode mode) {
    switch (mode) {
        case MemoryViewMode::Byte:
            return "byte";
        case MemoryViewMode::HalfWord:
            return "short";
        case MemoryViewMode::Word:
            return "word";
    }

    return "byte";
}

std::string FormatMemoryRow(Processor const& cpu, Address row_base_address, MemoryViewMode mode) {
    return FormatMemoryRowImpl(&cpu, row_base_address, mode);
}

std::string FormatUnavailableMemoryRow(Address row_base_address, MemoryViewMode mode) {
    return FormatMemoryRowImpl(nullptr, row_base_address, mode);
}

}  // namespace isa::demo