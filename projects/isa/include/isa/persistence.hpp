#pragma once

#include <iostream>
#include <iomanip>
#include <filesystem>

namespace isa {

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

}  // namespace isa
