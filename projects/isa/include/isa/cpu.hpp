#pragma once

#include <isa/types.hpp>
#include <isa/instructions.hpp>
#include <isa/bus.hpp>
#include <isa/map.hpp>

#include <string>
#include <vector>

namespace isa {

/// For declining stacks the stack starts at the limit - unit size and "grows" down.
// If it exceeds the base, an exception will be issued.
struct Stack {
    Address limit{
        0};  ///< SLA: The highest numerical (exclusive) address of the stack. The first address is limit - unit
    Address current{0};  ///< SCA: The current address of the stack
    Address base{0};     ///< SBA: The lowest numerical address of the stack, used to judge stack overflow.
};

struct Special {
    // special registers
    Address program_address_{0};  ///< PA: The current address of the instruction being executed
    Address return_address_{0};   ///< RA: The address which the control flow will return to when a return is issued.
    Stack stack_{};               ///< The CPU Stack (growing down)
    Stack exception_stack_{};     ///< The Exception Stack (growing down and Privileged)
};

/// The evaluation registers
using Evaluations = std::array<Evaluation, CountOfRegisters>;

/// The scratch registers
using Scratch = std::array<word<CountOfDataBits>, CountOfRegisters>;

/// The Instruction Cache
using ICache = std::array<instructions::Instruction, 256>;

/// The Cache Line Unit Type.
using CacheLineUnit = uint8_t;

/// A simple Instruction Cache
using CacheLine = std::array<CacheLineUnit, CountOfUnitsPerCacheLine>;

/// The Data Cache
using DCache = std::array<CacheLine, 1024>;

struct PersistenceReport {
    bool success{false};
    std::string summary{};
    std::vector<std::string> files{};
};

/// A simple CPU Processor class
class Processor {
public:
    /// Default Constructor
    Processor();

    ~Processor() = default;

    /// The read-only view of the scratch registers
    Scratch const& ViewScratch() const {
        return scratch_;
    }

    /// The read-only view of the special registers
    Special const& ViewSpecial() const {
        return special_;
    }

    /// The read-only view of the evaluation registers
    Evaluations const& ViewEvaluations() const {
        return evaluation_;
    }

    /// The read-only view of the instruction cache
    ICache const& ViewInstructionCache() const {
        return instruction_cache_;
    }

    /// The read-only view of the data cache
    DCache const& ViewDataCache() const {
        return data_cache_;
    }

    /// The modifiable scratch registers
    Scratch& GetScratch() {
        return scratch_;
    }

    /// The modifiable special registers
    Special& GetSpecial() {
        return special_;
    }

    /// The modifiable evaluation registers
    Evaluations& GetEvaluations() {
        return evaluation_;
    }

    /// The instruction cache
    ICache& GetInstructionCache() {
        return instruction_cache_;
    }

    /// The data cache
    DCache& GetDataCache() {
        return data_cache_;
    }

    /// Resets the CPU to default state
    void Reset();

    /// Allows attaching memories to the memory bus of the CPU
    bool AddTightlyCoupledMemory(TightlyCoupledMemory& memory);

    /// [DEBUG] Allows Peeking at ANY memory address across the entire system through the CPU's view
    bool Peek(Address address, uint32_t& value) const;

    /// [DEBUG] Allows Poking at ANY memory address across the entire system through the CPU's view
    bool Poke(Address address, uint32_t value);

    /// Saves all the CPU state information to different files in the given folder
    PersistenceReport Save(std::string const& folder) const;

    /// Loads all the CPU state information from different files in the given folder
    PersistenceReport Load(std::string const& folder);

protected:
    ICache instruction_cache_;
    DCache data_cache_;
    Scratch scratch_;
    Evaluations evaluation_;
    Special special_;
    TightlyCoupledBus sram_bus_;
    std::vector<TightlyCoupledMemory*> tightly_coupled_memories_;
    // PeripheralMemoryBus peripheral_bus_;
};

}  // namespace isa