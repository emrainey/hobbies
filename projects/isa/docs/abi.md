# ABI

The Calling Convention defines how functions receive parameters from a caller and how they return a result in most languages.

## Stack Discipline

Declining Stack with downward growth. Stack (SCA) starts at high address (SLA - alignment) and grows to low address (SBA).

## Example

A function call would look like

```asm
-- Call Site, Address in S4 already
0xxxxx move S0, #arg1
0xxxxx move S1, #arg2
0xxxxx move S2, #arg3
0xxxxx move S3, #arg4
0xxxxx call S4
0xxxxx -- S0 has return value or pointer to structure on local stack
```

### Function Prologue

```asm
0x1000 save 5,6         -- for saving caller's scratch registers
0x1004 copy S5, CSA     -- load parameter from stack
0x1008 grow #20         -- for an automatic structure (32 bytes)
0x100C mov  S6, #...    -- use scratch register
```

### Function Epilogue

```asm
0xxxxx move S0, #X      -- set return value
0xxxxx shrink #20       -- clean up local structure off stack
0xxxxx restore 5,6      -- restore caller's scratch registers
0xxxxx return           -- return to caller
```
