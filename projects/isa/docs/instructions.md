# Instructions

The set of instructions in the ISA generally fall into several categories including data movement, stack manipulation, arithmetic/logic operations, floating point operations, and control flow. Every attempt is made to use four letter mnemonics for instructions.

---

```asm
# One Term: d = op s
code Sd, Ss

# Two Inputs: d = a op b with side effects in e
code Sd, Sa, Sb : Ex
```

---

## Naming

* `Sx` - Scratch Register x
  * `Sd` - Scratch destination
  * `Sa` - Scratch source A (or address)
  * `Sb` - Scratch source B
* `Ex` - Evaluation Register x

There are 16 Scratch and 16 Evaluation registers. The Scratch registers are used for general purpose computation and can be read/written by most instructions.

## Operation Codes

```asm
noop -- no operation
```

This translates to:

```cpp
0b0000'0000'0000'0000'0000'0000'0000'0000 // noop
// or
0b1111'1111'1111'1111'1111'1111'1111'1111 // noop (when reading flash which is erased)
```

### Control Flow

```asm
halt                                - stop execution until reset
leap Sa                             - PA = Sa, RA unchanged
leap Es, Em, Sa                     - PA = Sa if (Es & Em) != 0, RA unchanged
leap Sa                             - PA = Sa, RA = PA + 4 (todo: special case of copy?)
leap Sa, #imm<16>                   - PA = Sa + offset (todo: special case of copy?)
leap Es, Em, Sa                     - PA = Sa, RA = PA + 4 if (Es & Em) != 0
leap #imm<30:2>                     - PA = #imm<30:2> << 2 (absolute immediate leap)
leap.s #imm<30:2>                   - RA = PA + 4, PA = #imm<30:2> << 2 (save return address)
back                                - PA = RA, RA = SAFE
call #imm<16>                       - system call with code #imm<16>
```

All non-immediate instructions reserve the two least-significant bits as `00`. Immediate leap uses those bits as mode
flags:

```cpp
0bxxxx'xxxx'xxxx'xxxx'xxxx'xxxx'xxxx'xx01 - immediate leap
0bxxxx'xxxx'xxxx'xxxx'xxxx'xxxx'xxxx'xx11 - immediate leap + save RA
```

### Data Movement

Instruction related to manipulation of data within a register file set.

```asm
copy Sd, Ss                         - d = s
move Sd, #imm<16>                   - d = #imm<16>
move Sd, Sa                         - d = a, a = 0
swap Sa, Sb                         - a swap b
zero {mask}                         - all registers set in mask are zeroed
```

### Memory Movement

Load and Save can both read/write to sub-word registers and specify an immediate  which can be used as an offset or a post-increment.

```asm
load.b/h/w Sd <- Sa << #0,1,2,3, #imm<10>               - d = *(a + imm<10>)
save.b/h/w Ss << #0,1,2,3 -> Ss, #imm<10>               - *(a + imm<10>) = s
```

### Stack Address

Implies SCA inc/dec, _can cause Hardware Faults_!

```asm
grow #imm<16>                       - SCA -= #imm<16> * sizeof(word)
undo #imm<16>                       - SCA += #imm<16> * sizeof(word)
push {mask}                         - SCA -= sizeof(word) * popcount(mask), store registers in mask to stack
pull {mask}                         - load registers in mask from stack, SCA += sizeof(word) * popcount(mask)
```

### Bitwise1 Operators

All bitwise1 mnemonics are four letters except for rotate which has to specify the direction.

```asm
bcpl Sd, Ss : Ex                    - d = ~s (bit invert)
brsh Sd, Ss, #imm<5> : Ex           - d = s >> #imm<5>
basr Sd, Ss, #imm<5> : Ex           - d = int((int)s >> #imm<5>)
blsh Sd, Ss, #imm<5> : Ex           - d = s << #imm<5>
brot.r/l Sd, Ss, #imm<5> : Ex       - d = (s << #imm<5>) | (s >> (32 - #imm<5>))
bcnt Sd, Ss : Ex                    - d = count 1's in s
bclz Sd, Ss : Ex                    - d = count leading zeros in s
bnot Sd, Ss : Ex                    - d = s ? 0 : 0xFFFFFFFF
bset Sd, Ss, #imm<5> : Ex           - d = s | 1 << #imm<5>
bclr Sd, Ss, #imm<5> : Ex           - d = s & ~(1 << #imm<5>)
btgl Sd, Ss, #imm<5> : Ex           - d = s ^ (1 << #imm<5>)
```

### Bitwise2 Operators

All bitwise mnemonics are four letters and effect eval register of the given index.

```asm
band Sd, Sa, Sb : Ex                - d = a & b
borr Sd, Sa, Sb : Ex                - d = a | b
bxor Sd, Sa, Sb : Ex                - d = a ^ b
```

### ALU Ops

All arithmetic logic operations effects eval register of the same index as scratch register destination. The signed or unsigned nature of the operation is determined by the mnemonic (e.g., `add.sXX` vs `add.uXX`) followed by the bit size (e.g., `add.s32` vs `add.s32`).

```asm
add.xXX Sd, Sa, Sb : Ex                      - d = a + b (e flags)
sub.xXX Sd, Sa, Sb : Ex                      - d = a - b (e flags)
mul.xXX Sd, Sa, Sb : Ex                      - d = a * b (e flags)
div.xXX Sd, Sa, Sb : Ex                      - d = a / b (e flags)
mod.xXX Sd, Sa, Sb : Ex                      - d = a % b (e flags)
```

### FPU Ops

```asm
fadd Sd, Sa, Sb : Ex                     - d = a + b (e flags)
fsub Sd, Sa, Sb : Ex                     - d = a - b (e flags)
fmul Sd, Sa, Sb : Ex                     - d = a * b (e flags)
fdiv Sd, Sa, Sb : Ex                     - d = a / b (e flags)
fflr Sd, Ss : Ex                         - d = floor(s)
fcel Sd, Ss : Ex                         - d = ceil(s)
fcvt.i Sd, Ss : Ex                       - d = int(s)
fcvt.f Sd, Ss : Ex                       - d = float(s)
```

### Comparisons

The Comparison instruction sets flags related to the type of comparison being performed. The flags are stored in the evaluation register of the given index. There are three types of comparisons:

* Logical compare which sets flags based on the result of a bitwise comparison of the two operands (e.g., equal, not equal, less than, greater than, etc.).
* Arithmetic compare which sets flags based on the result of an arithmetic comparison of the two operands (e.g., zero, overflow, underflow, etc.).
* Floating point compare which sets flags based on the result of a floating point comparison of the two operands (e.g., zero, overflow, underflow, etc.).

```asm
cmp.l Sa, Sb : Ex                        - Comparison of a and b, set flags in Ex
cmp.a Sa, Sb : Ex                        - Arithmetic compare a and b, set flags in Ex (zero, overflow, underflow)
cmp.f Sa, Sb : Ex                        - Floating Point compare a and b, set flags in Ex
```

### Future Instructions
