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

bool Processor::Load(program const& prog, Address load_address) {
    Address address = load_address;
    for (const auto& instr : prog) {
        if (not Poke(address, instr.raw)) {
            return false;
        }
        address += sizeof(instructions::Instruction);
    }
    return true;
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
        case isa::Operator::Copy: {
            const auto& copy = instruction.copy;
            scratch_[copy.dst].as_u32[0] = scratch_[copy.src].as_u32[0];
            if (copy.cond) {
                evaluation_[copy.eval] = ComparisonEvaluation{false, true, false, false};
            }
            break;
        }
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
                if (instruction.move.dst != instruction.move.src) {
                    scratch_[instruction.move.src].as_u32[0] = 0U;
                }
            }
            if (allowed and instruction.move.type == 1U) {
                EvaluationUnit value = evaluation_[instruction.move.src].value;
                if (instruction.move.dir) {
                    value = static_cast<EvaluationUnit>(static_cast<uint8_t>(value) >> instruction.move.shift);
                } else {
                    value <<= instruction.move.shift;
                }
                evaluation_[instruction.move.dst].value = value;
                if (instruction.move.dst != instruction.move.src) {
                    evaluation_[instruction.move.src].value = 0U;
                }
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
        case isa::Operator::Zero:
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
        case isa::Operator::Load: {
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
        case isa::Operator::Save: {
            const auto& save = instruction.save;
            const Address address = scratch_[save.base].as_address + (save.off ? Address{save.imm} : Address{0});
            uint32_t value = scratch_[save.src].as_u32[0];
            if (Poke(address, value)) {
                if (save.inc) {
                    scratch_[save.base].as_address += save.imm;
                }
            } else {
                special_.exception_.bus_fault = 1;
            }
            break;
        }
        case isa::Operator::Leap: {
            const auto& leap = instruction.leap;
            bool allowed = true;  // default to allowed
            if (leap.cond) {
                // if the eval & mask is not zero, then perform the leap
                uint32_t eval = evaluation_[leap.eval].value;
                uint32_t mask = evaluation_[leap.mask].value;
                allowed = (eval & mask) != 0U;
            }
            if (allowed) {
                if (leap.save) {
                    // If the save bit is set, then we need to save the return address (i.e. the address of the next
                    // instruction) in the Return Address Special Register before we update the Program Address Register
                    // to leap to the target instruction.
                    special_.return_address_ = special_.program_address_ + sizeof(instructions::Instruction);
                }
                if (leap.imm > 0) {
                    // TODO Bus fault if leap wraps around the address space by exceeding the maximum address
                    special_.program_address_
                        = scratch_[leap.dst].as_address + Address{static_cast<isa::Address::StorageType>(leap.imm)};
                } else if (leap.imm < 0) {
                    // TODO Bus fault if leap wraps around the address space by going under zero
                    special_.program_address_
                        = scratch_[leap.dst].as_address - Address{static_cast<isa::Address::StorageType>(-leap.imm)};
                } else {
                    special_.program_address_ = scratch_[leap.dst].as_address;
                }
            }
            break;
        }
        case Operator::Back:
            special_.program_address_ = special_.return_address_;
            if (instruction.back.zero) {
                Address safe_handler{0U};
                uint32_t offset = offsetof(VectorTable, safe_handler);
                if (Peek(special_.vector_table_address_ + offset, safe_handler.value)) {
                    special_.return_address_ = safe_handler;
                } else {
                    special_.exception_.bus_fault = 1;
                }
            }
            break;
        case Operator::Grow: {
            const auto& grow = instruction.grow;
            uint32_t eval = evaluation_[grow.eval].value;
            uint32_t mask = evaluation_[grow.mask].value;
            bool allowed = true;
            if (grow.cond) {
                // if the eval & mask is not zero, then perform the grow
                allowed = (eval & mask) != 0U;
            }
            if (allowed) {
                /// Grow the Stack Pointer by the immediate value * 4 (since the immediate value is in terms of 32-bit
                /// words) Growth of a Stack is _downwards_ in memory, so we subtract the immediate value from the
                /// current Stack Pointer to grow the stack. We also need to check for stack overflow by ensuring that
                /// we don't grow the stack beyond its limit.
                Address next = special_.stack_.current - Address{static_cast<isa::Address::StorageType>(grow.imm * 4U)};
                if (next <= special_.stack_.base) {
                    special_.stack_.current = next;
                } else {
                    special_.exception_.stack_fault = 1;
                }
            }
            break;
        }
        case Operator::Undo: {
            const auto& undo = instruction.undo;
            uint32_t eval = evaluation_[undo.eval].value;
            uint32_t mask = evaluation_[undo.mask].value;
            bool allowed = true;
            if (undo.cond) {
                // if the eval & mask is not zero, then perform the undo
                allowed = (eval & mask) != 0U;
            }
            if (allowed) {
                /// Undo the growth of the Stack Pointer by the immediate value * 4 (since the immediate value is in
                /// terms of 32-bit words) This is done by adding the immediate value to the current Stack Pointer, and
                /// we also need to check for stack underflow by ensuring that we don't undo the growth beyond the
                /// initial stack pointer.
                Address next = special_.stack_.current + Address{static_cast<isa::Address::StorageType>(undo.imm * 4U)};
                if (next <= special_.stack_.limit) {
                    special_.stack_.current = next;
                } else {
                    special_.exception_.stack_fault = 1;
                }
            }
            break;
        }
        case Operator::Call: {
            const auto& call = instruction.call;
            bool allowed = true;  // default to allowed
            if (call.cond) {
                // if the eval & mask is not zero, then perform the call
                uint32_t eval = evaluation_[call.eval].value;
                uint32_t mask = evaluation_[call.mask].value;
                allowed = (eval & mask) != 0U;
            }
            if (allowed) {
                // For a call instruction, we'll save the return address (i.e. the address of the next instruction) in
                // the Return Address Special Register before we update the Program Address Register to leap to the
                // target instruction.
                special_.return_address_ = special_.program_address_ + sizeof(instructions::Instruction);
                Address handler{0};
                // read the vector tabke to get the address of the system call handler corresponding to the system call
                // number in the immediate value of the instruction
                uint32_t offset = offsetof(VectorTable, system_call_handler);
                if (Peek(special_.vector_table_address_ + offset, handler.value)) {
                    special_.call_number_ = call.imm;
                    special_.program_address_ = handler;
                } else {
                    special_.exception_.bus_fault = 1;
                }
            }
            break;
        }
        case Operator::Trip: {
            const auto& trip = instruction.trip;
            bool allowed = true;  // default to allowed
            if (trip.cond) {
                // if the eval & mask is not zero, then perform the trip
                uint32_t eval = evaluation_[trip.eval].value;
                uint32_t mask = evaluation_[trip.mask].value;
                allowed = (eval & mask) != 0U;
            }
            if (allowed) {
                special_.exception_.type = static_cast<ExceptionType>(trip.imm);
                special_.exception_.tripped = 1;
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
            const auto& bitwise = instruction.bitwise2;
            uint32_t result = scratch_[bitwise.src1].as_u32[0] & scratch_[bitwise.src2].as_u32[0];
            scratch_[bitwise.dst].as_u32[0] = result;
            break;
        }
        case Operator::Or: {
            const auto& bitwise = instruction.bitwise2;
            uint32_t result = scratch_[bitwise.src1].as_u32[0] | scratch_[bitwise.src2].as_u32[0];
            scratch_[bitwise.dst].as_u32[0] = result;
            break;
        }
        case Operator::Xor: {
            const auto& bitwise = instruction.bitwise2;
            uint32_t result = scratch_[bitwise.src1].as_u32[0] ^ scratch_[bitwise.src2].as_u32[0];
            scratch_[bitwise.dst].as_u32[0] = result;
            break;
        }
        case Operator::Complement: {
            const auto& complement_instr = instruction.bitwise1;
            uint32_t result = ~scratch_[complement_instr.src].as_u32[0];
            scratch_[complement_instr.dst].as_u32[0] = result;
            break;
        }
        case Operator::Rsh: {
            const auto& rsh_instr = instruction.bitwise1;
            uint32_t result = scratch_[rsh_instr.src].as_u32[0] >> rsh_instr.shift;
            scratch_[rsh_instr.dst].as_u32[0] = result;
            break;
        }
        case Operator::Lsh: {
            const auto& lsh_instr = instruction.bitwise1;
            uint32_t result = scratch_[lsh_instr.src].as_u32[0] << lsh_instr.shift;
            scratch_[lsh_instr.dst].as_u32[0] = result;
            break;
        }
        case Operator::Count1s: {
            const auto& count_instr = instruction.bitwise1;
            uint32_t value = scratch_[count_instr.src].as_u32[0];
            uint32_t result = 0U;
            while (value) {
                result += value & 1U;
                value >>= 1;
            }
            scratch_[count_instr.dst].as_u32[0] = result;
            break;
        }
        case Operator::CountL0s: {
            const auto& count_instr = instruction.bitwise1;
            uint32_t value = scratch_[count_instr.src].as_u32[0];
            uint32_t result = 0U;
            uint32_t shift = 31;
            uint32_t mask = 1U << shift;
            // count the leading zeros by checking the most significant bit shifting right until we encounter a 1 or run
            // out of bits
            while ((value & mask) == 0U) {
                result++;
                if (shift == 0) {
                    break;
                } else {
                    shift--;
                    mask = 1U << shift;
                }
            }
            scratch_[count_instr.dst].as_u32[0] = result;
            break;
        }
        case Operator::Addition: {
            const auto& arithmetic = instruction.arithmetic;
            bool overflow = false;
            switch (static_cast<instructions::Arithmetic::Size>(arithmetic.size)) {
                case instructions::Arithmetic::Size::Byte:  // 8-bit
                    if (arithmetic.sign) {
                        scratch_[arithmetic.dst].as_s08[0] = operations::Addition<int8_t, int32_t>(
                            scratch_[arithmetic.src1].as_s08[0], scratch_[arithmetic.src2].as_s08[0], overflow);
                    } else {
                        scratch_[arithmetic.dst].as_u08[0] = operations::Addition<uint8_t, uint32_t>(
                            scratch_[arithmetic.src1].as_u08[0], scratch_[arithmetic.src2].as_u08[0], overflow);
                    }
                    break;
                case instructions::Arithmetic::Size::HalfWord:  // 16-bit
                    if (arithmetic.sign) {
                        scratch_[arithmetic.dst].as_s16[0] = operations::Addition<int16_t, int32_t>(
                            scratch_[arithmetic.src1].as_s16[0], scratch_[arithmetic.src2].as_s16[0], overflow);
                    } else {
                        scratch_[arithmetic.dst].as_u16[0] = operations::Addition<uint16_t, uint32_t>(
                            scratch_[arithmetic.src1].as_u16[0], scratch_[arithmetic.src2].as_u16[0], overflow);
                    }
                    break;
                case instructions::Arithmetic::Size::Word:  // 32-bit
                    if (arithmetic.sign) {
                        scratch_[arithmetic.dst].as_s32[0] = operations::Addition<int32_t, int64_t>(
                            scratch_[arithmetic.src1].as_s32[0], scratch_[arithmetic.src2].as_s32[0], overflow);
                    } else {
                        scratch_[arithmetic.dst].as_u32[0] = operations::Addition<uint32_t, uint64_t>(
                            scratch_[arithmetic.src1].as_u32[0], scratch_[arithmetic.src2].as_u32[0], overflow);
                    }
                    break;
                default: {
                    // Invalid size, set an appropriate exception
                    special_.exception_.instruction_fault = 1;
                    break;
                }
            }
            break;
        }
        case Operator::Subtract: {
            const auto& arithmetic = instruction.arithmetic;
            bool overflow = false;
            switch (static_cast<instructions::Arithmetic::Size>(arithmetic.size)) {
                case instructions::Arithmetic::Size::Byte:  // 8-bit
                    if (arithmetic.sign) {
                        scratch_[arithmetic.dst].as_s08[0] = operations::Addition<int8_t, int32_t>(
                            scratch_[arithmetic.src1].as_s08[0], -scratch_[arithmetic.src2].as_s08[0], overflow);
                    } else {
                        scratch_[arithmetic.dst].as_u08[0] = operations::Addition<uint8_t, uint32_t>(
                            scratch_[arithmetic.src1].as_u08[0], -scratch_[arithmetic.src2].as_u08[0], overflow);
                    }
                    break;
                case instructions::Arithmetic::Size::HalfWord:  // 16-bit
                    if (arithmetic.sign) {
                        scratch_[arithmetic.dst].as_s16[0] = operations::Addition<int16_t, int32_t>(
                            scratch_[arithmetic.src1].as_s16[0], -scratch_[arithmetic.src2].as_s16[0], overflow);
                    } else {
                        scratch_[arithmetic.dst].as_u16[0] = operations::Addition<uint16_t, uint32_t>(
                            scratch_[arithmetic.src1].as_u16[0], -scratch_[arithmetic.src2].as_u16[0], overflow);
                    }
                    break;
                case instructions::Arithmetic::Size::Word:  // 32-bit
                    if (arithmetic.sign) {
                        scratch_[arithmetic.dst].as_s32[0] = operations::Addition<int32_t, int64_t>(
                            scratch_[arithmetic.src1].as_s32[0], -scratch_[arithmetic.src2].as_s32[0], overflow);
                    } else {
                        scratch_[arithmetic.dst].as_u32[0] = operations::Addition<uint32_t, uint64_t>(
                            scratch_[arithmetic.src1].as_u32[0], -scratch_[arithmetic.src2].as_u32[0], overflow);
                    }
                    break;
                default: {
                    // Invalid size, set an appropriate exception
                    special_.exception_.instruction_fault = 1;
                    break;
                }
            }
            break;
        }
        case Operator::Multiply: {
            const auto& arithmetic = instruction.arithmetic;
            int64_t result = static_cast<int64_t>(scratch_[arithmetic.src1].as_s32[0])
                             * static_cast<int64_t>(scratch_[arithmetic.src2].as_s32[0]);
            // TODO saturation and overflow detection
            scratch_[arithmetic.dst].as_u32[0] = static_cast<uint32_t>(result & 0xFFFF'FFFFU);
            break;
        }
        case Operator::Divide: {
            const auto& arithmetic = instruction.arithmetic;
            int32_t divisor = scratch_[arithmetic.src2].as_s32[0];
            if (divisor == 0) {
                // Division by zero exception
                special_.exception_.instruction_fault = 1;
            } else {
                int32_t result = scratch_[arithmetic.src1].as_s32[0] / divisor;
                scratch_[arithmetic.dst].as_u32[0] = static_cast<uint32_t>(result);
            }
            break;
        }
        case Operator::Modulo: {
            const auto& arithmetic = instruction.arithmetic;
            int32_t divisor = scratch_[arithmetic.src2].as_s32[0];
            if (divisor == 0) {
                // Division by zero exception
                special_.exception_.instruction_fault = 1;
            } else {
                int32_t result = scratch_[arithmetic.src1].as_s32[0] % divisor;
                scratch_[arithmetic.dst].as_u32[0] = static_cast<uint32_t>(result);
            }
            break;
        }
        case Operator::FloatingAbs: {
            const auto& precision1 = instruction.precision1;
            float value = scratch_[precision1.src].as_float[0];
            scratch_[precision1.dst].as_float[0] = std::abs(value);
            break;
        }
        case Operator::FloatingNegate: {
            const auto& precision1 = instruction.precision1;
            float value = scratch_[precision1.src].as_float[0];
            scratch_[precision1.dst].as_float[0] = -value;
            break;
        }
        case Operator::FloatingCeil: {
            const auto& precision1 = instruction.precision1;
            float value = scratch_[precision1.src].as_float[0];
            scratch_[precision1.dst].as_float[0] = std::ceil(value);
            break;
        }
        case Operator::FloatingFloor: {
            const auto& precision1 = instruction.precision1;
            float value = scratch_[precision1.src].as_float[0];
            scratch_[precision1.dst].as_float[0] = std::floor(value);
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
        } else if (special_.exception_.nested) {
            index = offsetof(VectorTable, nested_handler) / sizeof(Address);
        } else if (special_.exception_.bus_fault) {
            index = offsetof(VectorTable, bus_fault_handler) / sizeof(Address);
        } else if (special_.exception_.instruction_fault) {
            index = offsetof(VectorTable, instruction_fault_handler) / sizeof(Address);
        } else if (special_.exception_.arithmetic_fault) {
            index = offsetof(VectorTable, arithmetic_fault_handler) / sizeof(Address);
        } else if (special_.exception_.stack_fault) {
            index = offsetof(VectorTable, stack_fault_handler) / sizeof(Address);
        } else if (special_.exception_.tripped) {
            index = offsetof(VectorTable, trip_handler) / sizeof(Address);
        } else if (special_.exception_.system_call) {
            index = offsetof(VectorTable, system_call_handler) / sizeof(Address);
        } else if (special_.exception_.deferred) {
            index = offsetof(VectorTable, deferred_handler) / sizeof(Address);
        } else if (special_.exception_.ticker) {
            index = offsetof(VectorTable, ticker_handler) / sizeof(Address);
        } else if (special_.exception_.privilege_fault) {
            index = offsetof(VectorTable, privilege_handler) / sizeof(Address);
        } else if (special_.exception_.safe) {
            index = offsetof(VectorTable, safe_handler) / sizeof(Address);
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