#pragma once

#include <isa/isa.hpp>

namespace isa {

/// For declining stacks the stack starts at the limit - unit size and "grows" down.
// If it exceeds the base, an exception will be issued.
struct Stack {
    Address limit{0};    ///< The highest numerical (exclusive) address of the stack. The first address is limit - unit
    Address current{0};  ///< The current address of the stack
    Address base{0};     ///< The lowest numerical address of the stack, used to judge stack overflow.
};

/// A simple CPU Processor class
class Processor {
public:
    Processor() = default;
    ~Processor() = default;

    scratch<16> const& GetScratch() const {
        return scratch_;
    }

    scratch<16>& GetScratch() {
        return scratch_;
    }

protected:
    icache<256> instruction_cache_;
    dcache<1024> data_cache_;
    scratch<16> scratch_;
    Evaluation evaluation_[16];
    // special registers
    Address program_address_{0};  ///< The current address of the instruction being executed
    Address return_address_{0};   ///< The address which the control flow will return to when a return is issued.
    Address current_address_{0};
    Stack stack_{};
    Stack exception_stack_{};
};

}  // namespace isa