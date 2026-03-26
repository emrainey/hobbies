/// @file
/// @brief Implementation of the CPU class

#include <isa/cpu.hpp>

#include <cstdio>
#include <filesystem>

namespace isa {
Processor::Processor()
    : flash_bus_{0, isa::memory::Map[1].range}, sram_bus_{0, isa::memory::Map[3].range}, halted_{false} {
}

void Processor::Reset() {
    instruction_cache_.Clean();
    data_cache_.Clean();
    scratch_.fill(isa::word<32>{0xFFFF'FFFFU});
    evaluation_.fill(isa::Evaluation{0xFFU});
    special_ = Special{};
    halted_ = false;

    // Load the Entry point from the Vector Table Into the Program Address Register
    uint32_t offset = offsetof(VectorTable, reset_handler);
    uint32_t location = 0;
    if (flash_bus_.Peek(special_.vector_table_address_ + offset, location)) {
        special_.program_address_ = Address{location};
        special_.return_address_ = Address{location};
    }
    if (flash_bus_.Peek(special_.vector_table_address_ + offsetof(VectorTable, stack_initial), location)) {
        special_.stack_.limit = Address{location};
        special_.stack_.current = Address{location};
    }
    if (flash_bus_.Peek(special_.vector_table_address_ + offsetof(VectorTable, stack_boundary), location)) {
        special_.stack_.base = Address{location};
    }
    if (flash_bus_.Peek(special_.vector_table_address_ + offsetof(VectorTable, exception_stack_initial), location)) {
        special_.exception_stack_.limit = Address{location};
        special_.exception_stack_.current = Address{location};
    }
    if (flash_bus_.Peek(special_.vector_table_address_ + offsetof(VectorTable, exception_stack_boundary), location)) {
        special_.exception_stack_.base = Address{location};
    }
}

bool Processor::AttachFlashMemory(FlashMemory& memory) {
    if (not flash_bus_.Attach(memory, memory.ViewRange())) {
        return false;
    }

    flash_memories_.push_back(&memory);
    return true;
}

bool Processor::AddTightlyCoupledMemory(TightlyCoupledMemory& memory) {
    if (not sram_bus_.Attach(memory, memory.ViewRange())) {
        return false;
    }

    tightly_coupled_memories_.push_back(&memory);
    return true;
}

bool Processor::Peek(Address address, uint32_t& value) const {
    if (flash_bus_.GetRange().Contains(address)) {
        return flash_bus_.Peek(address, value);
    } else if (sram_bus_.GetRange().Contains(address)) {
        return sram_bus_.Peek(address, value);
    }
    return false;
}

bool Processor::Poke(Address address, uint32_t value) {
    if (flash_bus_.GetRange().Contains(address)) {
        return flash_bus_.Poke(address, value);
    } else if (sram_bus_.GetRange().Contains(address)) {
        return sram_bus_.Poke(address, value);
    }
    return false;
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
    if (not save_component(kEvaluationRegistersFile, evaluation_.data(), evaluation_.size())) {
        return report;
    }
    instruction_cache_.Save(folder);
    data_cache_.Save(folder);

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
    if (not instruction_cache_.Load(folder)) {
        return report;
    }
    if (not data_cache_.Load(folder)) {
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

void Processor::Cycle() {
    if (halted_) {
        return;
    }
    // For now, we'll implement a simple execution environment.
    instructions::Instruction instruction;
    if (not Peek(special_.program_address_, instruction.raw)) {
        // If we fail to fetch an instruction, we'll treat it as a NoOp and just advance the program counter.
        instruction = instructions::Instruction{instructions::NoOp{}};
    }
    switch (instruction.base()) {
        case isa::Operator::None:
            // Do nothing
            break;
        case isa::Operator::Halt:
            // For a halt instruction, we'll just stop executing further instructions until the next reset.
            halted_ = true;
            break;
        case isa::Operator::Move: {
            bool allowed = true;  // default to allowed
            if (instruction.move.cond) {
                // if the eval & mask is not zero, then perform the move
                uint32_t eval = evaluation_[instruction.move.eval].value;
                uint32_t mask = evaluation_[instruction.move.mask].value;
                allowed = (eval & mask) != 0U;
            }
            if (allowed and instruction.move.type == 0U) {
                uint32_t value = scratch_[instruction.move.src].as_u32[0];
                if (instruction.move.dir) {
                    value = static_cast<uint32_t>(static_cast<int32_t>(value) >> instruction.move.shift);
                } else {
                    value <<= instruction.move.shift;
                }
                scratch_[instruction.move.dst].as_u32[0] = value;
            }
            if (allowed and instruction.move.type == 1U) {
                EvaluationUnit value = evaluation_[instruction.move.src].value;
                if (instruction.move.dir) {
                    value = static_cast<EvaluationUnit>(static_cast<uint8_t>(value) >> instruction.move.shift);
                } else {
                    value <<= instruction.move.shift;
                }
                evaluation_[instruction.move.dst].value = value;
            }
            break;
        }
        case isa::Operator::MoveImmediateToScratch:
            scratch_[instruction.movis.dst] = instruction.movis.imm;
            break;
        case isa::Operator::MoveImmediateToEvaluation:
            evaluation_[instruction.movie.dst].value = instruction.movie.imm & 0xFFU;
            break;
        case isa::Operator::Swap:
            if (instruction.swap.type == RegisterType::Scratch) {
                std::swap(scratch_[instruction.swap.a], scratch_[instruction.swap.b]);
            } else {
                std::swap(evaluation_[instruction.swap.a], evaluation_[instruction.swap.b]);
            }
            break;
        case isa::Operator::Clear:
            if (instruction.clear.type == RegisterType::Scratch) {
                for (size_t i = 0; i < scratch_.size(); ++i) {
                    if ((instruction.clear.mask & (1U << i)) != 0U) {
                        scratch_[i].as_u32[0] = 0U;
                    }
                }
            } else {
                for (size_t i = 0; i < evaluation_.size(); ++i) {
                    if ((instruction.clear.mask & (1U << i)) != 0U) {
                        evaluation_[i].value = 0U;
                    }
                }
            }
            break;
        case isa::Operator::LoadSingle: {
            const auto& load = instruction.loads;
            const Address address = scratch_[load.base].as_address + (load.off ? Address{load.imm} : Address{0});
            uint32_t value = 0U;
            if (Peek(address, value)) {
                scratch_[load.dst].as_u32[0] = value;
                if (load.inc) {
                    scratch_[load.base].as_address += load.imm;
                }
            } else {
                special_.exception_.bus_fault = 1;
            }
            break;
        }
        case isa::Operator::StoreSingle: {
            const auto& store = instruction.stores;
            const Address address = scratch_[store.base].as_address + (store.off ? Address{store.imm} : Address{0});
            uint32_t value = scratch_[store.src].as_u32[0];
            if (Poke(address, value)) {
                if (store.inc) {
                    scratch_[store.base].as_address += store.imm;
                }
            } else {
                special_.exception_.bus_fault = 1;
            }
            break;
        }
        case isa::Operator::Jump: {
            const auto& jump = instruction.jumps;
            bool allowed = true;  // default to allowed
            if (jump.cond) {
                // if the eval & mask is not zero, then perform the jump
                uint32_t eval = evaluation_[jump.eval].value;
                uint32_t mask = evaluation_[jump.mask].value;
                allowed = (eval & mask) != 0U;
            }
            if (allowed) {
                if (jump.save) {
                    // If the save bit is set, then we need to save the return address (i.e. the address of the next
                    // instruction) in the Return Address Special Register before we update the Program Address Register
                    // to jump to the target instruction.
                    special_.return_address_ = special_.program_address_ + sizeof(instructions::Instruction);
                }
                if (jump.imm > 0) {
                    // TODO Bus fault if jump wraps around the address space by exceeding the maximum address
                    special_.program_address_
                        = scratch_[jump.dst].as_address + Address{static_cast<isa::Address::StorageType>(jump.imm)};
                } else if (jump.imm < 0) {
                    // TODO Bus fault if jump wraps around the address space by going under zero
                    special_.program_address_
                        = scratch_[jump.dst].as_address - Address{static_cast<isa::Address::StorageType>(-jump.imm)};
                } else {
                    special_.program_address_ = scratch_[jump.dst].as_address;
                }
            }
            break;
        }
        case Operator::Compare: {
            const auto& compare = instruction.compare;
            ComparisonEvaluation cmp{};
            if (compare.s) {
                // Signed comparison
                int32_t a = scratch_[compare.a].as_s32[0];
                int32_t b = scratch_[compare.b].as_s32[0];
                cmp.equal = (a == b) ? 1U : 0U;
                cmp.less_than = (a < b) ? 1U : 0U;
                cmp.greater_than = (a > b) ? 1U : 0U;
                cmp.not_equal = (a != b) ? 1U : 0U;
            } else {
                // Unsigned comparison
                uint32_t a = scratch_[compare.a].as_u32[0];
                uint32_t b = scratch_[compare.b].as_u32[0];
                cmp.equal = (a == b) ? 1U : 0U;
                cmp.less_than = (a < b) ? 1U : 0U;
                cmp.greater_than = (a > b) ? 1U : 0U;
                cmp.not_equal = (a != b) ? 1U : 0U;
            }
            evaluation_[compare.e] = cmp;
            break;
        }
        case Operator::And: {
            const auto& and_instr = instruction.bitwise_and;
            uint32_t result = scratch_[and_instr.src1].as_u32[0] & scratch_[and_instr.src2].as_u32[0];
            scratch_[and_instr.dst].as_u32[0] = result;
            break;
        }
        case Operator::Or: {
            const auto& or_instr = instruction.bitwise_or;
            uint32_t result = scratch_[or_instr.src1].as_u32[0] | scratch_[or_instr.src2].as_u32[0];
            scratch_[or_instr.dst].as_u32[0] = result;
            break;
        }
        case Operator::Xor: {
            const auto& xor_instr = instruction.bitwise_xor;
            uint32_t result = scratch_[xor_instr.src1].as_u32[0] ^ scratch_[xor_instr.src2].as_u32[0];
            scratch_[xor_instr.dst].as_u32[0] = result;
            break;
        }
        case Operator::Not: {
            const auto& not_instr = instruction.bitwise_not;
            uint32_t result = ~scratch_[not_instr.src].as_u32[0];
            scratch_[not_instr.dst].as_u32[0] = result;
            break;
        }
        case Operator::Rsh: {
            const auto& rsh_instr = instruction.bitwise_rsh;
            uint32_t result = scratch_[rsh_instr.src].as_u32[0] >> rsh_instr.shift;
            scratch_[rsh_instr.dst].as_u32[0] = result;
            break;
        }
        case Operator::Lsh: {
            const auto& lsh_instr = instruction.bitwise_lsh;
            uint32_t result = scratch_[lsh_instr.src].as_u32[0] << lsh_instr.shift;
            scratch_[lsh_instr.dst].as_u32[0] = result;
            break;
        }
        case Operator::Count: {
            const auto& count_instr = instruction.bit_count;
            uint32_t value = scratch_[count_instr.src].as_u32[0];
            uint32_t result = 0U;
            while (value) {
                result += value & 1U;
                value >>= 1;
            }
            scratch_[count_instr.dst].as_u32[0] = result;
            break;
        }
        case Operator::SignedAdd: {
            const auto& sadd_instr = instruction.signed_add;
            int64_t result = static_cast<int64_t>(scratch_[sadd_instr.src1].as_s32[0])
                             + static_cast<int64_t>(scratch_[sadd_instr.src2].as_s32[0]);
            // TODO saturation and overflow detection
            scratch_[sadd_instr.dst].as_u32[0] = static_cast<uint32_t>(result & 0xFFFF'FFFFU);
            break;
        }
        case Operator::SignedSub: {
            const auto& ssub_instr = instruction.signed_sub;
            int64_t result = static_cast<int64_t>(scratch_[ssub_instr.src1].as_s32[0])
                             - static_cast<int64_t>(scratch_[ssub_instr.src2].as_s32[0]);
            // TODO saturation and overflow detection
            scratch_[ssub_instr.dst].as_u32[0] = static_cast<uint32_t>(result & 0xFFFF'FFFFU);
            break;
        }
        case Operator::SignedMultiply: {
            const auto& smul_instr = instruction.signed_multiply;
            int64_t result = static_cast<int64_t>(scratch_[smul_instr.src1].as_s32[0])
                             * static_cast<int64_t>(scratch_[smul_instr.src2].as_s32[0]);
            // TODO saturation and overflow detection
            scratch_[smul_instr.dst].as_u32[0] = static_cast<uint32_t>(result & 0xFFFF'FFFFU);
            break;
        }
        case Operator::SignedDivide: {
            const auto& sdiv_instr = instruction.signed_divide;
            int32_t divisor = scratch_[sdiv_instr.src2].as_s32[0];
            if (divisor == 0) {
                // Division by zero exception
                special_.exception_.instruction_fault = 1;
            } else {
                int32_t result = scratch_[sdiv_instr.src1].as_s32[0] / divisor;
                scratch_[sdiv_instr.dst].as_u32[0] = static_cast<uint32_t>(result);
            }
            break;
        }
        case Operator::SignedModulo: {
            const auto& smod_instr = instruction.signed_modulo;
            int32_t divisor = scratch_[smod_instr.src2].as_s32[0];
            if (divisor == 0) {
                // Division by zero exception
                special_.exception_.instruction_fault = 1;
            } else {
                int32_t result = scratch_[smod_instr.src1].as_s32[0] % divisor;
                scratch_[smod_instr.dst].as_u32[0] = static_cast<uint32_t>(result);
            }
            break;
        }
        case Operator::UnsignedAdd: {
            const auto& uadd_instr = instruction.unsigned_add;
            uint64_t result = static_cast<uint64_t>(scratch_[uadd_instr.src1].as_u32[0])
                              + static_cast<uint64_t>(scratch_[uadd_instr.src2].as_u32[0]);
            // TODO saturation and overflow detection
            scratch_[uadd_instr.dst].as_u32[0] = static_cast<uint32_t>(result & 0xFFFF'FFFFU);
            break;
        }
        case Operator::UnsignedSub: {
            const auto& usub_instr = instruction.unsigned_sub;
            uint64_t result = static_cast<uint64_t>(scratch_[usub_instr.src1].as_u32[0])
                              - static_cast<uint64_t>(scratch_[usub_instr.src2].as_u32[0]);
            // TODO saturation and overflow detection
            scratch_[usub_instr.dst].as_u32[0] = static_cast<uint32_t>(result & 0xFFFF'FFFFU);
            break;
        }
        case Operator::UnsignedMultiply: {
            const auto& umul_instr = instruction.unsigned_multiply;
            uint64_t result = static_cast<uint64_t>(scratch_[umul_instr.src1].as_u32[0])
                              * static_cast<uint64_t>(scratch_[umul_instr.src2].as_u32[0]);
            // TODO saturation and overflow detection
            scratch_[umul_instr.dst].as_u32[0] = static_cast<uint32_t>(result & 0xFFFF'FFFFU);
            break;
        }
        case Operator::UnsignedDivide: {
            const auto& udiv_instr = instruction.unsigned_divide;
            uint32_t divisor = scratch_[udiv_instr.src2].as_u32[0];
            if (divisor == 0) {
                // Division by zero exception
                special_.exception_.instruction_fault = 1;
            } else {
                uint32_t result = scratch_[udiv_instr.src1].as_u32[0    ] / divisor;
                scratch_[udiv_instr.dst].as_u32[0] = result;
            }
            break;
        }
        case Operator::UnsignedModulo: {
            const auto& umod_instr = instruction.unsigned_modulo;
            uint32_t divisor = scratch_[umod_instr.src2].as_u32[0];
            if (divisor == 0) {
                // Division by zero exception
                special_.exception_.instruction_fault = 1;
            } else {
                uint32_t result = scratch_[umod_instr.src1].as_u32[0] % divisor;
                scratch_[umod_instr.dst].as_u32[0] = result;
            }
            break;
        }
        default:
            special_.exception_.instruction_fault = 1;
            break;
    }
    if (special_.exception_.HasException()) {
        special_.program_address_ = GetHandler();
    } else {
        special_.program_address_ += sizeof(instructions::Instruction);
    }
    special_.performance_counter_ += 1;
}

Address Processor::GetHandler() const {
    Address handler{0U};
    if (special_.exception_.HasException()) {
        size_t index = 0;
        if (special_.exception_.unmaskable) {
            index = offsetof(VectorTable, unmaskable_handler) / sizeof(Address);
        } else if (special_.exception_.bus_fault) {
            index = offsetof(VectorTable, bus_fault_handler) / sizeof(Address);
        } else if (special_.exception_.instruction_fault) {
            index = offsetof(VectorTable, instruction_fault_handler) / sizeof(Address);
        } else if (special_.exception_.software_trigger) {
            index = offsetof(VectorTable, software_trigger_handler) / sizeof(Address);
        } else if (special_.exception_.deferred) {
            index = offsetof(VectorTable, deferred_handler) / sizeof(Address);
        } else if (special_.exception_.ticker) {
            index = offsetof(VectorTable, ticker_handler) / sizeof(Address);
        } else if (special_.exception_.external) {
            // For external exceptions, we need to look up the handler in the external handler table using the external
            // exception number as an index.
            uint32_t external_index = special_.exception_.external - 1;  // External exception numbers are 1-based
            Address external_table_address{0U};
            if (Peek(special_.vector_table_address_ + offsetof(VectorTable, external_handler_table),
                     external_table_address.value)) {
                Address entry_address = external_table_address + Address{external_index * sizeof(Address)};
                if (Peek(entry_address, handler.value)) {
                    return handler;
                }
            }
            // If we fail to fetch the handler for an external exception, we'll treat it as a reset exception and use
            // the reset handler.
            index = offsetof(VectorTable, reset_handler) / sizeof(Address);
        }

        if (Peek(special_.vector_table_address_ + index * sizeof(Address), handler.value)) {
            return handler;
        }
    }

    // If no exception is being handled or we fail to fetch the appropriate handler, return the reset handler.
    Address reset_handler{0U};
    if (Peek(special_.vector_table_address_ + offsetof(VectorTable, reset_handler), reset_handler.value)) {
        return reset_handler;
    }
    return Address{0U};  // TODO throw exception!
}

}  // namespace isa