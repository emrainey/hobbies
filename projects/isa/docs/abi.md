# ABI

The Calling Convention defines how functions receive parameters from a caller and how they return a result in most languages.

## Stack Discipline

Declining Stack with downward growth. Stack (SCA) starts at high address (SLA - alignment) and grows to low address (SBA).

## Example

### Function Call

For normal function calls, a `leap` is used.

```asm
--  Site, Address in S4 already
0xxxxx move S0, #arg1
0xxxxx move S1, #arg2
0xxxxx move S2, #arg3
0xxxxx move S3, #arg4
0xxxxx leap S4
0xxxxx -- S0 has return value or pointer to structure on local stack

### Inside the Function

#### Prologue

```asm
0x1000 push #0003     -- for saving caller's scratch registers for the value's we'll overwrite
0x1008 grow #20         -- for an automatic structure (32 bytes)
```

#### Epilogue

```asm
0xxxxx undo #20         -- clean up local structure off stack
0xxxxx pull #0003     -- restore caller's scratch registers
0xxxxx move S0, #X      -- set return value
0xxxxx back             -- return to caller
```

### System Call

A system call would look like this in assembly which invokes a specific system call handler based on the immediate argument:

```asm
-- Call Site, Address in S4 already
0xxxxx move S0, #arg1
0xxxxx move S1, #arg2
0xxxxx move S2, #arg3
0xxxxx move S3, #arg4
0xxxxx call #imm<16>
0xxxxx -- S0 has return value or pointer to structure on local stack
```
Inside the system call handler, the system call number can be read from the `call` field of the `Numeral` struct in the special register, and the arguments can be read from S0-S3. The return value can be written back to S0 before returning from the handler.

```asm
-- System Call Handler, Address in S4 already
0xxxxx push #000F     -- for saving caller's scratch registers for the value
0xxxxx grow #20         -- for an automatic structure (32 bytes)
0xxxxx -- Read system call number from special register's numeral.call field
0xxxxx -- Read arguments from S0-S3
0xxxxx -- Perform system call logic based on system call number and arguments
0xxxxx -- Write return value to S0
0xxxxx undo #20         -- clean up local structure off stack
0xxxxx pull #000F     -- restore caller's scratch registers
0xxxxx back             -- return to caller (sensitive to call and trip instruction returns)
```

An C++ wrapper can be constructed like this:

```cpp
/// System call with code #imm<16> and arguments arg1, arg2, arg3, arg4.
/// @return A word which can be a value or a pointer to a structure in memory.
/// @warning Returning a pointer to privileged memory will cause a privilege fault if the caller is unprivileged. Use with caution!
template <uint16_t CODE>
word call(word arg1, word arg2, word arg3, word arg4);

// Example usage:
auto result = call<0x1234>(arg1, arg2, arg3, arg4);
```

### Tripped Exception

A triggered exception would look like this in assembly which invokes a specific exception handler based on the immediate argument:

```asm
trip #imm<16>
```

Inside the handler, it is expected that the handler will read the exception type from the `trip` field of the `Numeral` struct in the special register, and handle the exception accordingly. The handler can also read any relevant information about the exception from the other fields in the special register, such as the program address where the exception occurred, and can write any necessary information back to the special register before returning.

```asm
-- Exception Handler, Address in S4 already
0xxxxx push #000F     -- for saving caller's scratch registers for the value
0xxxxx grow #20         -- for an automatic structure (32 bytes)
0xxxxx -- Read exception type from special register's numeral.trip field
0xxxxx -- Read other relevant information about the exception from special register fields
0xxxxx -- Perform exception handling logic based on exception type and relevant information
0xxxxx -- Write any necessary information back to special register fields
0xxxxx undo #20         -- clean up local structure off stack
0xxxxx pull #000F     -- restore caller's scratch registers
0xxxxx back             -- return to caller (sensitive to call and trip instruction returns)
```
