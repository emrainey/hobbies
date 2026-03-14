#pragma once

#include <isa/isa.hpp>

namespace isa {

/// For declining stacks the stack starts at the limit - unit size and "grows" down.
// If it exceeds the base, an exception will be issued.
struct Stack {
    Address limit{0};    ///< SLA: The highest numerical (exclusive) address of the stack. The first address is limit - unit
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
using Evaluations = std::array<Evaluation, 16>;

/// The scratch registers
using Scratch = std::array<word<32>, 16>;

/// The Instruction Cache
using ICache = std::array<instructions::Instruction, 256>;

/// A simple Instruction Cache
using CacheLine = std::array<uint8_t, 16>;

/// The Data Cache
using DCache = std::array<CacheLine, 1024>;

/// A simple CPU Processor class
class Processor {
public:
    Processor() = default;
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

protected:
    ICache instruction_cache_;
    DCache data_cache_;
    Scratch scratch_;
    Evaluations evaluation_;
    Special special_;
};

}  // namespace isa