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
    /// SLA: The highest numerical (exclusive) address of the stack. The first usable address is limit - unit size.
    Address limit{0};
    /// SCA: The current address of the stack
    Address current{0};
    /// SBA: The lowest numerical address of the stack, used to judge stack overflow.
    Address base{0};

    friend std::ostream& operator<<(std::ostream& os, Stack stack) {
        os << "SLA: " << stack.limit << " SCA: " << stack.current << " SBA: " << stack.base;
        return os;
    }

    /// Returns the configured size of the stack in bytes.
    constexpr uint32_t Size() const {
        return static_cast<uint32_t>(limit - base);
    }

    /// Returns true if the given address is within the stack range, false otherwise. Note that the limit is exclusive,
    /// so the valid stack addresses are [base, limit).
    constexpr bool Contained(Address const& addr) const {
        return (base <= addr) and (addr < limit);
    }
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
        os << "E:" << mode.exception << " P:" << mode.privileged << " U:" << mode.nonprivileged;
        return os;
    }
};

using ExceptionUnit = uint32_t;

enum class ExceptionType : ExceptionUnit {
    Reset = 0,
    /// The unmaskble exception can not be mask and will be handled as soon as it is triggered.
    Unmaskable = 1,
    /// A fault occurred while trying to handle another exception, which can be used to detect faults in
    /// exception handlers and prevent infinite exception loops.
    Nested = 2,
    /// A bus fault occurred, such as accessing an invalid address, accessing a protected memory region, or violating access permissions.
    BusFault = 3,
    /// An instruction fault occurred, such as executing an invalid or undefined instruction.
    InstructionFault = 4,
    /// An arithmetic fault occurred, such as division by zero.
    ArithmeticFault = 5,
    /// A stack fault occurred, such as stack overflow or underflow.
    StackFault = 6,
    /// A trip instruction was issued in privileged mode, and the processor should transfer control to the trip handler.
    Tripped = 7,
    /// A system call instruction was issued in non-privileged mode, and the processor should transfer control to the system call handler.
    SystemCall = 8,
    /// A deferred system call instruction was issued, and the processor should transfer control to the deferred system call handler after higher priority exceptions are handled and the next instruction is executed.
    Deferred = 9,
    /// The tick interrupt fired.
    Ticker = 10,
    /// A privileged instruction was attempted in non-privileged mode, and the processor should transfer control to the privilege exception handler.
    Privilege = 11,
    /// A safe instruction was attempted, and the processor should transfer control to the safe exception handler.
    Safe = 12,
    /// Used by a top level peripheral handler to indicate that an external exception has occurred, and the processor should look up the appropriate handler in the external handler table using the external exception number as an index.
    External = 13,

    /// No Exception
    None = UINT16_MAX,
};
constexpr static size_t CountOfExceptionTypes{1024U};

constexpr static size_t CountOfExceptionTypeBits{log2(CountOfExceptionTypes)};

/// Holds the immediate number given to call and trip invocations.
struct Numeral {
    Numeral() : call{0}, trip{0} {
    }
    uint32_t call : 16;  ///< The system call number for system calls, or the trigger number for triggered exceptions.
    uint32_t trip : 16;  ///< The exception type to trip for a trip instruction, or the trigger number for triggered exceptions.
};

/// The Exception struct contains information about the current exception that occurred, which can be used by the exception handler to determine how to handle the exception and what actions to take.
struct Exception {
    /// 1 when the exception is a reset exception, meaning it is triggered by a reset signal and
    /// causes the processor to reset. 0 when the exception is a non-reset exception, meaning it
    /// is triggered by other events such as interrupts or faults.
    uint32_t reset : 1;

    /// 1 when the exception is unmaskable, meaning it cannot be ignored or handled by an exception handler,
    /// and will always cause the processor to enter an exception state. 0 when the exception is maskable,
    /// meaning it can be ignored or handled by an exception handler.
    uint32_t unmaskable : 1;

    /// 1 when the exception is nested, meaning it is triggered while the processor is already handling another
    /// exception, and can be used to detect faults in exception handlers and prevent infinite exception loops. 0 when
    /// the exception is non-nested, meaning it is not triggered during the handling of another exception.
    uint32_t nested : 1;

    /// Bus fault exception, meaning it is triggered by a memory access violation, such as accessing an invalid address,
    /// accessing a protected memory region, or violating access permissions. 0 when the exception is a non-bus fault
    /// exception, meaning it is triggered by other events such as interrupts or software exceptions
    uint32_t bus_fault : 1;

    /// Instruction fault exception, meaning it is triggered by an invalid or undefined instruction,
    /// such as executing an instruction that is not supported by the processor or violates the instruction
    /// encoding. 0 when the exception is a non-instruction fault exception, meaning it is triggered by other events
    /// such as interrupts or software exceptions.
    uint32_t instruction_fault : 1;

    /// Arithmetic fault exception, meaning it is triggered by an arithmetic error, such as division by zero.
    uint32_t arithmetic_fault : 1;

    /// 1 When the Stack has overflow or underflowed, meaning it is triggered when the stack pointer exceeds the stack
    /// boundary in either direction, and can be used to detect stack overflows and underflows. 0 when the exception is
    /// a non-stack fault exception, meaning it is triggered by other events such as interrupts or software exceptions.
    uint32_t stack_fault : 1;

    /// 1 when the exception is a trip exception, meaning it is tripped by a software instruction such
    /// as a trip instruction, and can be used for making system calls or requesting
    /// services from the operating system. 0 when the exception is a non-trip exception,
    /// meaning it is triggered by other events such as hardware interrupts or faults.
    uint32_t tripped : 1;

    /// 1 when the exception is a system call, meaning it is triggered by a software instruction such as a
    /// SYSCALL instruction, and can be used for making system calls or requesting services from the operating
    /// system. 0 when the exception is a non-system call exception, meaning it is triggered by other events such as
    /// hardware interrupts or faults.
    uint32_t system_call : 1;

    /// 1 when the exception is a delayed software interrupt, meaning it is triggered by a software
    /// instruction such as a deferred system call, and will be raised after the
    /// next instruction is executed, allowing for delayed handling of software interrupts. 0 when the
    /// exception is a non-delayed software interrupt, meaning it is triggered by other events such as
    /// hardware interrupts or faults, or by software instructions that do not have delayed behavior.
    uint32_t deferred : 1;

    /// 1 when the exception is a ticker exception, meaning it is triggered by a timer or clock event, and
    /// can be used for implementing preemptive multitasking or time-sharing. 0 when the exception is a
    /// non-ticker exception, meaning it is triggered by other events such as interrupts or faults.
    uint32_t ticker : 1;

    /// 1 when the exception is a privilege exception, meaning a privileged operation was attempted without
    /// sufficient access level.
    uint32_t privilege_fault : 1;

    /// 1 when the exception is a safe exception, meaning the processor should transfer control to a known-safe
    /// handler.
    uint32_t safe : 1;

    /// 1 when the exception is an external exception, meaning it is triggered by a
    /// peripheral device or I/O event, and can be used for handling input/output operations
    /// or device interactions. 0 when the exception is a non-external exception, meaning
    /// it is triggered by other events such as interrupts or faults.
    uint32_t external : 1;

    uint32_t : 18;  ///< Unused bits for future expansion

    bool HasException() const {
        return as_word().as_u32[0] != 0U;
    }

    friend std::ostream& operator<<(std::ostream& os, Exception exc) {
        word<CountOfDataBits> fields = *reinterpret_cast<word<CountOfDataBits>*>(&exc);
        os << std::hex << fields.as_u32[0];
        // os << " R:" << exc.reset << " U:" << exc.unmaskable << "N:" << exc.nested << " B:" << exc.bus_fault
        //    << " I:" << exc.instruction_fault << " A:" << exc.arithmetic_fault << " S:" << exc.stack_fault
        //    << "C:" << exc.system_call << " D:" << exc.deferred << " T:" << exc.ticker << " P:" << exc.privilege_fault
        //    << " H:" << exc.safe << " E:" << exc.external << " SE:" << static_cast<uint32_t>(exc.type);
        return os;
    }

    Word as_word() const {
        return *reinterpret_cast<const Word*>(this);
    }
};
static_assert(sizeof(Exception) == sizeof(word<CountOfDataBits>),
              "Exception struct must be the size of a single word for efficient storage and manipulation");

/// Mirrors the Exception struct but does not allow masking of reset, unmaskable and the type.
/// This is used for checking whether an exception should be handled or not based on the exception mask.
/// @note Each bit should be in the same position as Exception.
struct ExceptionMask {
    uint32_t : 2;  ///< Inaccessible bits for reset and unmaskable exceptions, which cannot be masked and will always cause the processor to enter an exception state when they occur.

    /// 1 when the exception is nested, meaning it is triggered while the processor is already handling another
    /// exception, and can be used to detect faults in exception handlers and prevent infinite exception loops. 0 when
    /// the exception is non-nested, meaning it is not triggered during the handling of another exception.
    uint32_t nested : 1;

    /// Bus fault exception, meaning it is triggered by a memory access violation, such as accessing an invalid address,
    /// accessing a protected memory region, or violating access permissions. 0 when the exception is a non-bus fault
    /// exception, meaning it is triggered by other events such as interrupts or software exceptions
    uint32_t bus_fault : 1;

    /// Instruction fault exception, meaning it is triggered by an invalid or undefined instruction,
    /// such as executing an instruction that is not supported by the processor or violates the instruction
    /// encoding. 0 when the exception is a non-instruction fault exception, meaning it is triggered by other events
    /// such as interrupts or software exceptions.
    uint32_t instruction_fault : 1;

    /// Arithmetic fault exception, meaning it is triggered by an arithmetic error, such as division by zero.
    uint32_t arithmetic_fault : 1;

    /// 1 When the Stack has overflow or underflowed, meaning it is triggered when the stack pointer exceeds the stack
    /// boundary in either direction, and can be used to detect stack overflows and underflows. 0 when the exception is
    /// a non-stack fault exception, meaning it is triggered by other events such as interrupts or software exceptions.
    uint32_t stack_fault : 1;

    /// 1 when the exception is a software interrupt, meaning it is triggered by a software instruction such
    /// as an SWI (Software Interrupt) instruction, and can be used for making system calls or requesting
    /// services from the operating system. 0 when the exception is a non-software interrupt,
    /// meaning it is triggered by other events such as hardware interrupts or faults.
    uint32_t tripped : 1;

    /// 1 when the exception is a system call, meaning it is triggered by a software instruction such as a
    /// SYSCALL instruction, and can be used for making system calls or requesting services from the operating
    /// system. 0 when the exception is a non-system call exception, meaning it is triggered by other events such as
    /// hardware interrupts or faults.
    uint32_t system_call : 1;

    /// 1 when the exception is a delayed software interrupt, meaning it is triggered by a software
    /// instruction such as a deferred system call, and will be raised after the
    /// next instruction is executed, allowing for delayed handling of software interrupts. 0 when the
    /// exception is a non-delayed software interrupt, meaning it is triggered by other events such as
    /// hardware interrupts or faults, or by software instructions that do not have delayed behavior.
    uint32_t deferred : 1;

    /// 1 when the exception is a ticker exception, meaning it is triggered by a timer or clock event, and
    /// can be used for implementing preemptive multitasking or time-sharing. 0 when the exception is a
    /// non-ticker exception, meaning it is triggered by other events such as interrupts or faults.
    uint32_t ticker : 1;

    /// 1 when the exception is a privilege exception, meaning a privileged operation was attempted without
    /// sufficient access level.
    uint32_t privilege_fault : 1;

    /// 1 when the exception is a safe exception, meaning the processor should transfer control to a known-safe
    /// handler.
    uint32_t safe : 1;

    /// 1 when the exception is an external exception, meaning it is triggered by a
    /// peripheral device or I/O event, and can be used for handling input/output operations
    /// or device interactions. 0 when the exception is a non-external exception, meaning
    /// it is triggered by other events such as interrupts or faults.
    uint32_t external : 1;

    uint32_t : 18; ///< Unused bits for the type, which is not used in the mask since the type of the exception should not affect whether it is masked or not.

    Word as_word() const {
        return *reinterpret_cast<const Word*>(this);
    }
};

// struct StackInitializer {
//     /// The top most address of the stack. The current address will start here and grow downwards as data is pushed onto the stack. This can be used to detect stack underflow when the current address exceeds the initial address.
//     Address initial{0};
//     /// The boundary address of the stack, which can be used to detect stack overflow.
//     Address boundary{0};
// };

// struct ExceptionHandlers {
//     /// The address of the reset handler, which is the entry point for the processor when it is reset.
//     Address reset_handler{0};
//     /// The address of the unmaskable handler, which is the entry point for handling unmaskable exceptions.
//     Address unmaskable_handler{0};
//     /// The address of the nested handler, which is the entry point for handling nested exceptions.
//     Address nested_handler{0};
//     /// The address of the bus fault handler, which is the entry point for handling bus fault exceptions
//     Address bus_fault_handler{0};
//     /// The address of the instruction fault handler, which is the entry point for handling instruction fault
//     /// exceptions.
//     Address instruction_fault_handler{0};
//     /// The address of the arithmetic fault handler, which is the entry point for handling arithmetic fault exceptions.
//     Address arithmetic_fault_handler{0};
//     /// The address of the stack fault handler, which is the entry point for handling stack fault exceptions.
//     Address stack_fault_handler{0};
//     /// The address of the software trip handler, which is the entry point for handling software trip exceptions
//     Address trip_handler{0};
//     /// The address of the system call handler, which is the entry point for handling functions which rely on elevated
//     /// privileges, such as I/O operations, memory management, etc. This can be used for implementing system calls in an
//     /// operating system.
//     Address system_call_handler{0};
//     /// The address of the deferred handler, which is the entry point for handling deferred exceptions.
//     Address deferred_handler{0};
//     /// The address of the ticker handler, which is the entry point for handling ticker exceptions.
//     Address ticker_handler{0};
//     /// The address of the privilege handler, which handles privilege exceptions.
//     Address privilege_handler{0};
//     /// The address of a safe fallback handler for low-priority safety recovery.
//     Address safe_handler{0};
//     /// The address of the external handler table, which is the entry point for handling external exceptions.
//     Address external_handler_table{0};
// };

// struct InitializerTable {
//     StackInitializer user_stack;
//     StackInitializer exception_stack;
//     ExceptionHandlers handlers;
// };

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
    /// The address of the nested handler, which is the entry point for handling nested exceptions.
    Address nested_handler{0};
    /// The address of the bus fault handler, which is the entry point for handling bus fault exceptions
    Address bus_fault_handler{0};
    /// The address of the instruction fault handler, which is the entry point for handling instruction fault
    /// exceptions.
    Address instruction_fault_handler{0};
    /// The address of the arithmetic fault handler, which is the entry point for handling arithmetic fault exceptions.
    Address arithmetic_fault_handler{0};
    /// The address of the stack fault handler, which is the entry point for handling stack fault exceptions.
    Address stack_fault_handler{0};
    /// The address of the software trip handler, which is the entry point for handling software trip exceptions
    Address trip_handler{0};
    /// The address of the system call handler, which is the entry point for handling functions which rely on elevated
    /// privileges, such as I/O operations, memory management, etc. This can be used for implementing system calls in an
    /// operating system.
    Address system_call_handler{0};
    /// The address of the deferred handler, which is the entry point for handling deferred exceptions.
    Address deferred_handler{0};
    /// The address of the ticker handler, which is the entry point for handling ticker exceptions.
    Address ticker_handler{0};
    /// The address of the privilege handler, which handles privilege exceptions.
    Address privilege_handler{0};
    /// The address of a safe fallback handler for low-priority safety recovery.
    Address safe_handler{0};
    /// The address of the external handler table, which is the entry point for handling external exceptions.
    Address external_handler_table{0};

    /// Used to copy the vector table to another location in memory, such as from ROM to RAM, which can be useful for
    /// allowing the processor to use a different vector table for handling exceptions.
    Address& operator[](size_t index) {
        // cast this into a uint32_t pointer for easier indexing and copying
        Address* ptr = reinterpret_cast<Address*>(this);
        return ptr[index];
    }
};
constexpr static size_t VectorTableCount = sizeof(VectorTable) / sizeof(Address);

/// The number of special registers is the size of the Special struct divided by the size of an Address, since each
/// special register is the size of an Address.
constexpr static size_t CountOfSpecialRegisters{16U};

/// The Special Registers of the processor, which include the Program Address Register, the Stack Pointer, the Mode
/// Register, etc.
struct Special {
    /// [0] PA: The current address of the instruction being executed
    Address program_address_{0};
    /// [1] RA: The address which the control flow will return to when a return is issued.
    Address return_address_{0};
    /// [2] VTA: The address of the vector table of handler to pick the appropriate
    /// handler when an exception is raised.
    Address vector_table_address_{DefaultVectorTableAddress};
    /// [3,4,5] CPU Stack (growing down)
    Stack stack_{};
    /// [6,7,8] Exception Stack (growing down and Privileged)
    Stack exception_stack_{};
    /// [9] The current exception being handled, if any
    Exception exception_{};
    /// [10] A simple mask register that can be used for various purposes, such as masking interrupts,
    /// controlling access to resources, etc.
    ExceptionMask exception_mask_{};
    /// [11] The current mode of the processor
    Mode mode_{};
    /// [12] The current call and trip immediate values.
    Numeral numeral_{};
    /// [13] A simple performance counter that can be used for counting events, measuring time, etc.
    uint32_t performance_counter_{0};
    /// [14,15] Reserved for future expansion
    uint32_t reserved_[2]{0};

    constexpr Word as_word(size_t index) const {
        if (index < CountOfSpecialRegisters) {
            const Word* ptr = reinterpret_cast<const Word*>(this);
            return ptr[index];
        } else {
            throw std::out_of_range("Special register index out of range");
        }
    }
};
static_assert(sizeof(Special) == CountOfSpecialRegisters * sizeof(Address),
              "Special union must be the size of the number of special registers times the size of an Address for "
              "efficient storage and manipulation");

constexpr static size_t ProgramAddressIndex{0U};
constexpr static size_t StackCurrentIndex{4U};

/// The evaluation registers
using Evaluations = std::array<Evaluation, CountOfEvaluationRegisters>;

/// The scratch registers
using Scratch = std::array<word<CountOfDataBits>, CountOfScratchRegisters>;

struct PersistenceReport {
    bool success{false};
    std::string summary{};
    std::vector<std::string> files{};
};

constexpr static Operator PrivilegedInstructions[] = {
    Operator::Halt,
    Operator::Trip,
};

/// A helper function to determine whether an instruction is a privileged instruction, which requires elevated
/// privileges to execute, and will cause a privileged instruction exception if attempted to be executed without the
/// required privileges.
constexpr static bool IsPrivilegedInstruction(Operator op) {
    for (auto privileged_op : PrivilegedInstructions) {
        if (op == privileged_op) {
            return true;
        }
    }
    return false;
}

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

    /// Loads a program into memory starting at the given address. This can be used for loading a program into flash/RAM
    bool Load(program const& prog, Address load_address);

    /// Performs a single cycle of the CPU which will execute each stage of the pipeline.
    void Cycle();

    /// Indicates if a breakpoint has been hit, which can be used for debugging and tooling purposes to determine
    /// whether to halt the CPU or not.
    bool HasBreakPoint() const;

    /// Sets a break point at the given address, which will cause the CPU to halt when the program counter reaches that
    /// address.
    bool SetBreakPoint(Address address);

    /// Clears the break point at the given address.
    bool ClearBreakPoint(Address address);

    /// Returns the address of the current break point that has been hit, if any.
    /// Note: This function should only be called when HasBreakPoint() returns true, otherwise the return value is
    /// undefined.
    Address GetBreakPoint() const;

    /// Returns the address of the break point at the given index, which can be used for debugging and tooling purposes
    /// to inspect the set break points.
    Address GetBreakPoint(size_t index) const;

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

    // === Debugging and Tooling Aids ===
    bool break_point_hit_;
    constexpr static size_t kMaxBreakPoints{16U};
    std::array<Address, kMaxBreakPoints> break_points_;
};

namespace operations {
template <typename OPERAND_TYPE, typename INTERMEDIATE_TYPE>
constexpr OPERAND_TYPE Addition(OPERAND_TYPE a, OPERAND_TYPE b, bool& overflow) {
    constexpr auto pos_med = static_cast<INTERMEDIATE_TYPE>(std::numeric_limits<OPERAND_TYPE>::max() / 2);
    constexpr auto neg_med = static_cast<INTERMEDIATE_TYPE>(std::numeric_limits<OPERAND_TYPE>::min() / 2);
    overflow = false;
    if (static_cast<INTERMEDIATE_TYPE>(a) < neg_med and static_cast<INTERMEDIATE_TYPE>(b) < neg_med) {
        overflow = true;
        return std::numeric_limits<OPERAND_TYPE>::min();
    }
    if (static_cast<INTERMEDIATE_TYPE>(a) > pos_med and static_cast<INTERMEDIATE_TYPE>(b) > pos_med) {
        overflow = true;
        return std::numeric_limits<OPERAND_TYPE>::max();
    }
    return a + b;
}

template <typename OPERAND_TYPE, typename INTERMEDIATE_TYPE>
constexpr OPERAND_TYPE Subtraction(OPERAND_TYPE a, OPERAND_TYPE b, bool& overflow) {
    constexpr auto pos_med = static_cast<INTERMEDIATE_TYPE>(std::numeric_limits<OPERAND_TYPE>::max() / 2);
    constexpr auto neg_med = static_cast<INTERMEDIATE_TYPE>(std::numeric_limits<OPERAND_TYPE>::min() / 2);
    overflow = false;
    if (static_cast<INTERMEDIATE_TYPE>(a) < neg_med and static_cast<INTERMEDIATE_TYPE>(b) < neg_med) {
        overflow = true;
        return std::numeric_limits<OPERAND_TYPE>::min();
    }
    if (static_cast<INTERMEDIATE_TYPE>(a) > pos_med and static_cast<INTERMEDIATE_TYPE>(b) > pos_med) {
        overflow = true;
        return std::numeric_limits<OPERAND_TYPE>::max();
    }
    return a - b;
}

template <typename OPERAND_TYPE, typename INTERMEDIATE_TYPE>
constexpr OPERAND_TYPE Multiply(OPERAND_TYPE a, OPERAND_TYPE b, bool& overflow) {
    constexpr auto pos_med = static_cast<INTERMEDIATE_TYPE>(std::numeric_limits<OPERAND_TYPE>::max() / 2);
    constexpr auto neg_med = static_cast<INTERMEDIATE_TYPE>(std::numeric_limits<OPERAND_TYPE>::min() / 2);
    overflow = false;
    if (static_cast<INTERMEDIATE_TYPE>(a) < neg_med and static_cast<INTERMEDIATE_TYPE>(b) < neg_med) {
        overflow = true;
        return std::numeric_limits<OPERAND_TYPE>::min();
    }
    if (static_cast<INTERMEDIATE_TYPE>(a) > pos_med and static_cast<INTERMEDIATE_TYPE>(b) > pos_med) {
        overflow = true;
        return std::numeric_limits<OPERAND_TYPE>::max();
    }
    return a * b;
}

}  // namespace operations

}  // namespace isa