# Exceptions

## Hardware Induced

* Unassigned Address - attempted access of address that is not assigned to any bus/target
* Unaligned Address - address was not aligned for the assigned bus/memory
* Permission - attempted access that is not allowed (e.g. write to ROM, execute in unmapped, etc)
* InstructionDecodeFailure - instruction failed to decode
* Stack Overflow - push from CSA + imm >= SLA
* Stack Underflow - pop - imm would make CSA < SBA

## Software Induced

* Breakpoint - executed breakpoint instruction
* Semihosting - execute external semihosting feature in place of normal instruction. Requires debugger support.
* Invalid Operation - executed invalid operation instruction
* Syscall - executed syscall instruction to elevate privilege

