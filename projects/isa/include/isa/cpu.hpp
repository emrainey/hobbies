#pragma once

#include <isa/types.hpp>
#include <isa/instructions.hpp>
#include <isa/bus.hpp>
#include <isa/map.hpp>
#include <isa/cache.hpp>
#include <isa/persistence.hpp>

#include <string>
#include <vector>

namespace isa {

/// Defining the Data Cache Size and Line Size
using DataCache = Cache<uint8_t, 1024, 16>;

/// Definining the Instruction Cache type
using InstructionCache = Cache<instructions::Instruction, 256, 1>;

/// For declining stacks the stack starts at the limit - unit size and "grows" down.
// If it exceeds the base, an exception will be issued.
struct Stack {
    /// SLA: The highest numerical (exclusive) address of the stack. The first address is limit - unit
    Address limit{0};
    /// SCA: The current address of the stack
    Address current{0};
    /// SBA: The lowest numerical address of the stack, used to judge stack overflow.
    Address base{0};
};

/// The Mode Register contains the current mode of the processor,
/// including whether the processor is in an exception handler, whether it's in privileged mode, etc.
struct Mode {
    constexpr Mode() : exception{0}, privileged{0}, nonprivileged{1} {
    }
    uint32_t exception : 1;      ///< 1 when the processor is in an exception handler, 0 otherwise
    uint32_t privileged : 1;     ///< 1 when the processor is in privileged mode, 0 otherwise
    uint32_t nonprivileged : 1;  ///< 1 when the processor is in nonprivileged (user) mode, 0 otherwise
    uint32_t : 29;

    friend std::ostream& operator<<(std::ostream& os, Mode mode) {
        os << " E:" << mode.exception << " P:" << mode.privileged << " U:" << mode.nonprivileged;
        return os;
    }
};

struct Exception {
    /// 1 when the exception is a reset exception, meaning it is triggered by a reset signal and
    /// causes the processor to reset. 0 when the exception is a non-reset exception, meaning it
    /// is triggered by other events such as interrupts or faults.
    uint32_t reset : 1;

    /// 1 when the exception is unmaskable, meaning it cannot be ignored or handled by an exception handler,
    /// and will always cause the processor to enter an exception state. 0 when the exception is maskable,
    /// meaning it can be ignored or handled by an exception handler.
    uint32_t unmaskable : 1;

    /// Bus fault exception, meaning it is triggered by a memory access violation, such as accessing an invalid address,
    /// accessing a protected memory region, or violating access permissions. 0 when the exception is a non-bus fault
    /// exception, meaning it is triggered by other events such as interrupts or software exceptions
    uint32_t bus_fault : 1;

    /// Instruction fault exception, meaning it is triggered by an invalid or undefined instruction,
    /// such as executing an instruction that is not supported by the processor or violates the instruction
    /// encoding. 0 when the exception is a non-instruction fault exception, meaning it is triggered by other events
    /// such as interrupts or software exceptions.
    uint32_t instruction_fault : 1;

    /// 1 when the exception is a software interrupt, meaning it is triggered by a software instruction such
    /// as an SWI (Software Interrupt) instruction, and can be used for making system calls or requesting
    /// services from the operating system. 0 when the exception is a non-software interrupt,
    /// meaning it is triggered by other events such as hardware interrupts or faults.
    uint32_t software_trigger : 1;

    /// 1 when the exception is a delayed software interrupt, meaning it is triggered by a software
    /// instruction such as a DSWI (Delayed Software Interrupt) instruction, and will be raised after the
    /// next instruction is executed, allowing for delayed handling of software interrupts. 0 when the
    /// exception is a non-delayed software interrupt, meaning it is triggered by other events such as
    /// hardware interrupts or faults, or by software instructions that do not have delayed behavior.
    uint32_t deferred : 1;

    /// 1 when the exception is a ticker exception, meaning it is triggered by a timer or clock event, and
    /// can be used for implementing preemptive multitasking or time-sharing. 0 when the exception is a
    /// non-ticker exception, meaning it is triggered by other events such as interrupts or faults.
    uint32_t ticker : 1;

    /// 1 when the exception is an external exception, meaning it is triggered by a
    /// peripheral device or I/O event, and can be used for handling input/output operations
    /// or device interactions. 0 when the exception is a non-external exception, meaning
    /// it is triggered by other events such as interrupts or faults.
    uint32_t external : 1;

    constexpr bool HasException() const {
        return reset or unmaskable or bus_fault or instruction_fault or software_trigger or deferred or ticker
               or external;
    }

    friend std::ostream& operator<<(std::ostream& os, Exception exc) {
        os << " R:" << exc.reset << " U:" << exc.unmaskable << " B:" << exc.bus_fault << " I:" << exc.instruction_fault
           << " S:" << exc.software_trigger << " D:" << exc.deferred << " T:" << exc.ticker << " E:" << exc.external;
        return os;
    }
};

struct VectorTable {
    /// The Initial Stack Address to be used by the user mode, which can be different from the main stack used by the
    /// processor.
    Address stack_initial{0};
    /// The boundary of the stack, which can be used to detect stack overflows and underflows.
    Address stack_boundary{0};
    /// The Stack to be used when handling exceptions
    Address exception_stack_initial{0};
    /// The boundary of the exception stack, which can be used to detect stack overflows and underflows in exception
    /// handlers.
    Address exception_stack_boundary{0};
    /// The address of the reset handler, which is the entry point for the processor when it is reset.
    Address reset_handler{0};
    /// The address of the unmaskable handler, which is the entry point for handling unmaskable exceptions.
    Address unmaskable_handler{0};
    /// The address of the bus fault handler, which is the entry point for handling bus fault exceptions
    Address bus_fault_handler{0};
    /// The address of the instruction fault handler, which is the entry point for handling instruction fault
    /// exceptions.
    Address instruction_fault_handler{0};
    /// The address of the software trigger handler, which is the entry point for handling software trigger exceptions
    /// (like SWI/SVC)
    Address software_trigger_handler{0};
    /// The address of the deferred handler, which is the entry point for handling deferred exceptions.
    Address deferred_handler{0};
    /// The address of the ticker handler, which is the entry point for handling ticker exceptions.
    Address ticker_handler{0};
    /// The address of the external handler table, which is the entry point for handling external exceptions.
    Address external_handler_table{0};

    /// Used to copy the vector table to another location in memory, such as from ROM to RAM, which can be useful for
    /// allowing the processor to use a different vector table for handling exceptions.
    Address& operator[](size_t index) {
        // cast this into a uint32_t pointer for easier indexing and copying
        uint32_t* ptr = reinterpret_cast<uint32_t*>(this);
        return reinterpret_cast<Address&>(ptr[index]);
    }
};
constexpr static size_t VectorTableCount = sizeof(VectorTable) / sizeof(Address);

struct Special {
    // special registers
    Address program_address_{0};  ///< PA: The current address of the instruction being executed
    Address return_address_{0};   ///< RA: The address which the control flow will return to when a return is issued.
    Address vector_table_address_{
        DefaultVectorTableAddress};  ///< VTA: The address of the vector table of handler to pick the appropriate
                                     ///< handler when an exception is raised.
    Stack stack_{};                  ///< The CPU Stack (growing down)
    Stack exception_stack_{};        ///< The Exception Stack (growing down and Privileged)
    Exception exception_{};          ///< The current exception being handled, if any
    Mode mode_{};                    ///< The current mode of the processor
    /// A simple performance counter that can be used for counting events, measuring time, etc.
    uint32_t performance_counter_{0};
};

/// The evaluation registers
using Evaluations = std::array<Evaluation, CountOfRegisters>;

/// The scratch registers
using Scratch = std::array<word<CountOfDataBits>, CountOfRegisters>;

struct PersistenceReport {
    bool success{false};
    std::string summary{};
    std::vector<std::string> files{};
};

/// A simple CPU Processor class.
/// This Model has a 4 Stage Pipeline (Fetch, Decode, Execute and Writeback) and a Harvard Architecture with separate
/// instruction and data caches.
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
    InstructionCache const& ViewInstructionCache() const {
        return instruction_cache_;
    }

    /// The read-only view of the data cache
    DataCache const& ViewDataCache() const {
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
    InstructionCache& GetInstructionCache() {
        return instruction_cache_;
    }

    /// The data cache
    DataCache& GetDataCache() {
        return data_cache_;
    }

    /// Resets the CPU to default state
    void Reset();

    /// Returns whether the CPU is currently halted, which can be used for determining whether to execute a cycle or
    /// not.
    bool IsHalted() const {
        return halted_;
    }

    /// Allows attaching flash memories to the flash bus of the CPU
    bool AttachFlashMemory(FlashMemory& memory);

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

    /// Performs a single cycle of the CPU which will execute each stage of the pipeline.
    void Cycle();

protected:
    /// Returns the handler which corresponds to the current exception being handled, if any, by looking it up in the
    /// vector table. If no exception is being handled, returns the reset handler.
    Address GetHandler() const;

    // === Data Members ===
    InstructionCache instruction_cache_;
    DataCache data_cache_;
    Scratch scratch_;
    Evaluations evaluation_;
    Special special_;
    FlashBus flash_bus_;
    std::vector<FlashMemory*> flash_memories_;
    TightlyCoupledBus sram_bus_;
    std::vector<TightlyCoupledMemory*> tightly_coupled_memories_;
    // PeripheralMemoryBus peripheral_bus_;

    // === Control Variables for Pipeline Stages ===
    bool halted_;
};

}  // namespace isa