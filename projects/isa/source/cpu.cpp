/// @file
/// @brief Implementation of the CPU class

#include <isa/cpu.hpp>

#include <cstdio>
#include <filesystem>

namespace {

constexpr char kScratchRegistersFile[] = "scratch_registers.bin";
constexpr char kSpecialRegistersFile[] = "special_registers.bin";
constexpr char kInstructionCacheFile[] = "instruction_cache.bin";
constexpr char kDataCacheFile[] = "data_cache.bin";
constexpr char kEvaluationRegistersFile[] = "evaluation_registers.bin";
constexpr char kTightlyCoupledMemoryPrefix[] = "tightly_coupled_memory_";

template <typename T>
bool WriteBinaryFile(std::filesystem::path const& file_path, T const* data, size_t count, std::string& error) {
    const std::string path_string = file_path.string();
    FILE* file = std::fopen(path_string.c_str(), "wb");
    if (file == nullptr) {
        error = "could not open " + path_string + " for writing";
        return false;
    }

    const size_t written = std::fwrite(data, sizeof(T), count, file);
    std::fclose(file);
    if (written != count) {
        error = "short write to " + path_string;
        return false;
    }

    return true;
}

template <typename T>
bool ReadBinaryFile(std::filesystem::path const& file_path, T* data, size_t count, std::string& error) {
    const std::string path_string = file_path.string();
    FILE* file = std::fopen(path_string.c_str(), "rb");
    if (file == nullptr) {
        error = "could not open " + path_string + " for reading";
        return false;
    }

    const size_t read = std::fread(data, sizeof(T), count, file);
    std::fclose(file);
    if (read != count) {
        error = "short read from " + path_string;
        return false;
    }

    return true;
}

}  // namespace

namespace isa {
Processor::Processor() : sram_bus_{0, Range{isa::memory::Map[2].range.start, isa::memory::Map[2].range.limit}} {
    Reset();
}

void Processor::Reset() {
    instruction_cache_.fill(instructions::Instruction{instructions::NoOp{}});
    data_cache_.fill(CacheLine{});
    scratch_.fill(isa::word<32>{0});
    evaluation_.fill(isa::Evaluation{});
    special_ = Special{};
}

bool Processor::AddTightlyCoupledMemory(TightlyCoupledMemory& memory) {
    if (not sram_bus_.Attach(memory, memory.ViewRange())) {
        return false;
    }

    tightly_coupled_memories_.push_back(&memory);
    return true;
}

bool Processor::Peek(Address address, uint32_t& value) const {
    return sram_bus_.Peek(address, value);
}

bool Processor::Poke(Address address, uint32_t value) {
    return sram_bus_.Poke(address, value);
}

PersistenceReport Processor::Save(std::string const& folder) const {
    PersistenceReport report;
    const std::filesystem::path directory{folder};

    const auto save_component = [&](char const* file_name, auto const* data, size_t count) {
        std::string error;
        if (not WriteBinaryFile(directory / file_name, data, count, error)) {
            report.summary = "Save failed: " + error;
            return false;
        }

        report.files.emplace_back(file_name);
        return true;
    };

    if (not save_component(kScratchRegistersFile, scratch_.data(), scratch_.size())) {
        return report;
    }
    if (not save_component(kSpecialRegistersFile, &special_, 1U)) {
        return report;
    }
    if (not save_component(kInstructionCacheFile, instruction_cache_.data(), instruction_cache_.size())) {
        return report;
    }
    if (not save_component(kDataCacheFile, data_cache_.data(), data_cache_.size())) {
        return report;
    }
    if (not save_component(kEvaluationRegistersFile, evaluation_.data(), evaluation_.size())) {
        return report;
    }

    for (size_t memory_index = 0; memory_index < tightly_coupled_memories_.size(); ++memory_index) {
        auto const* memory = tightly_coupled_memories_[memory_index];
        auto const& range = memory->ViewRange();
        using Unit = TightlyCoupledMemory::Attributes::AddressableUnitType;
        constexpr size_t unit_bytes = sizeof(Unit);
        const size_t unit_count = (range.Size() + unit_bytes - 1U) / unit_bytes;

        std::vector<Unit> raw_memory;
        raw_memory.reserve(unit_count);
        for (Address address = range.start; raw_memory.size() < unit_count; address += unit_bytes) {
            raw_memory.push_back((*memory)[address]);
        }

        std::string file_name = std::string{kTightlyCoupledMemoryPrefix} + std::to_string(memory_index) + ".bin";
        if (not save_component(file_name.c_str(), raw_memory.data(), raw_memory.size())) {
            return report;
        }
    }

    report.success = true;
    report.summary = "Saved " + std::to_string(report.files.size()) + " CPU state files to " + folder;
    return report;
}

PersistenceReport Processor::Load(std::string const& folder) {
    PersistenceReport report;
    const std::filesystem::path directory{folder};

    Scratch scratch = scratch_;
    Special special = special_;
    ICache instruction_cache = instruction_cache_;
    DCache data_cache = data_cache_;
    Evaluations evaluation = evaluation_;
    using Unit = TightlyCoupledMemory::Attributes::AddressableUnitType;
    struct MemoryImage {
        TightlyCoupledMemory* memory;
        std::vector<Unit> units;
    };
    std::vector<MemoryImage> memory_images;

    const auto load_component = [&](char const* file_name, auto* data, size_t count) {
        std::string error;
        if (not ReadBinaryFile(directory / file_name, data, count, error)) {
            report.summary = "Load failed: " + error;
            return false;
        }

        report.files.emplace_back(file_name);
        return true;
    };

    if (not load_component(kScratchRegistersFile, scratch.data(), scratch.size())) {
        return report;
    }
    if (not load_component(kSpecialRegistersFile, &special, 1U)) {
        return report;
    }
    if (not load_component(kInstructionCacheFile, instruction_cache.data(), instruction_cache.size())) {
        return report;
    }
    if (not load_component(kDataCacheFile, data_cache.data(), data_cache.size())) {
        return report;
    }
    if (not load_component(kEvaluationRegistersFile, evaluation.data(), evaluation.size())) {
        return report;
    }

    memory_images.reserve(tightly_coupled_memories_.size());
    for (size_t memory_index = 0; memory_index < tightly_coupled_memories_.size(); ++memory_index) {
        auto* memory = tightly_coupled_memories_[memory_index];
        auto const& range = memory->ViewRange();
        constexpr size_t unit_bytes = sizeof(Unit);
        const size_t unit_count = (range.Size() + unit_bytes - 1U) / unit_bytes;

        std::vector<Unit> raw_memory(unit_count);
        std::string file_name = std::string{kTightlyCoupledMemoryPrefix} + std::to_string(memory_index) + ".bin";
        if (not load_component(file_name.c_str(), raw_memory.data(), raw_memory.size())) {
            return report;
        }

        memory_images.push_back(MemoryImage{memory, std::move(raw_memory)});
    }

    scratch_ = scratch;
    special_ = special;
    instruction_cache_ = instruction_cache;
    data_cache_ = data_cache;
    evaluation_ = evaluation;

    for (auto const& image : memory_images) {
        auto const& range = image.memory->ViewRange();
        constexpr size_t unit_bytes = sizeof(Unit);
        for (size_t index = 0; index < image.units.size(); ++index) {
            const Address address = range.start + static_cast<Address>(index * unit_bytes);
            (*image.memory)[address] = image.units[index];
        }
    }

    report.success = true;
    report.summary = "Loaded " + std::to_string(report.files.size()) + " CPU state files from " + folder;
    return report;
}
}  // namespace isa