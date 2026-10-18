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
0b1111'1111'1111'1111'1111'1111'1111'1111 // noop (the erased flash word; not an immediate leap)
```

### Control Flow

#### `halt`

```asm
halt                                - stop execution until reset
```

#### `leap`

Register-based leap uses a signed 10-bit immediate measured in 4-byte instructions, giving a byte offset from `Sd`
between -2048 and +2044. An immediate of zero leaps to the address in `Sd`. Conditional register leaps do not change
`PA` or `RA` when the condition is false. The `.s` forms save the address of the instruction following the leap
(`PA + 4`) to `RA` only when the leap is taken.

```asm
leap Sd, #simm<10>                  - PA = Sd + (sign_extend(#simm<10>) * 4)
leap Ee, Em, Sd, #simm<10>          - same leap only if (Ee & Em) != 0
leap.s Sd, #simm<10>                - same as leap, and RA = PA + 4
leap.s Ee, Em, Sd, #simm<10>        - conditional leap, and RA = PA + 4 if taken
leap #addr<30:2>                    - PA = #addr<30:2> << 2 (absolute, 4-byte-aligned address)
leap.s #addr<30:2>                  - same absolute leap, and RA = PA + 4
```

Register-based leaps reserve the two least-significant bits as `00`. Absolute immediate leap requires a 4-byte-aligned
target and uses those bits as mode flags:

```cpp
0bxxxx'xxxx'xxxx'xxxx'xxxx'xxxx'xxxx'xx01 - immediate leap
0bxxxx'xxxx'xxxx'xxxx'xxxx'xxxx'xxxx'xx11 - immediate leap + save RA
```

#### `back`

```asm
back                                - PA = RA
back zero                           - PA = RA, then RA = SAFE (the `safe_handler` address from the vector table)
```

#### `call`

```asm
call #imm<10>                       - system call with code #imm<10>
call #imm<10> : Ee, Em              - system call only if (Ee & Em) != 0
```

#### `trip`

`trip.dbg` also signals debugger clients, but does not automatically halt the CPU. The signal remains latched until
cleared through the CPU debugger API. The trip exception is raised and dispatched normally.

```asm
trip #imm<10>                       - raise the software exception identified by #imm<10>
trip #imm<10> : Ee, Em              - raise the exception only if (Ee & Em) != 0
trip.dbg #imm<10>                   - raise the exception and signal a debugger break
trip.dbg #imm<10> : Ee, Em          - raise the exception and signal a debugger break only if (Ee & Em) != 0
```

### Data Movement

Instruction related to manipulation of data within a register file set.

```asm
copy Sd, Ss                         - d = s
copy Sd, Ss : Ee                    - d = s, and Ee is set to a Comparison evaluation with only `equal` set
move Sd, Sa                         - d = a, a = 0 (no zeroing when Sd == Sa)
move Sd, Sa << #imm                 - d = a << #imm, a = 0
move Sd, Sa >> #imm                 - d = a >> #imm (arithmetic, sign preserving), a = 0
move Sd, Sa [<< | >> #imm] : Ee, Em - same move, only performed if (Ee & Em) != 0
move Ed, Ea                         - same move on evaluation registers, when both operands are evaluation registers
move Sd, #imm<16>                   - low half of d = #imm<16>, high half unchanged
move Sd, #imm<16> << 16U            - high half of d = #imm<16>, low half unchanged
move Ed, #imm<16>                   - d = low 8 bits of #imm<16>
swap Sa, Sb                         - tmp = *Sb, *Sb = Sa, Sa = tmp (Sb holds the address of a memory word; bus fault if inaccessible)
zero.s {mask}                       - all scratch registers set in mask are zeroed
zero.e {mask}                       - all evaluation registers set in mask are zeroed
sext.b Sd, Ss : Ex                  - d = sign extend the low 8 bits of s to 32 bits
sext.h Sd, Ss : Ex                  - d = sign extend the low 16 bits of s to 32 bits
zext.b Sd, Ss : Ex                  - d = zero extend the low 8 bits of s to 32 bits
zext.h Sd, Ss : Ex                  - d = zero extend the low 16 bits of s to 32 bits
```

`move` shifts and the conditional form apply to both register files. The shift distance is limited to the shift field of the instruction. The upper-half form (`<< 16U`) is not allowed for evaluation registers since they are only 8 bits.

`sext` and `zext` widen a byte or half-word to a word. Narrowing needs no instruction, since the low bits of a register are already the narrow value. With `: Ex`, an Arithmetic evaluation is written from the result: `positive` is set when bit 31 is clear, `zero` when the result is zero; `overflow` and `undefined` are clear.

### Memory Movement

Load and Save can both read/write to sub-word registers and specify an immediate which can be used as an offset or a post-increment.

```asm
load.b Sd <- Sa << #0,1,2,3                  - d = byte lane (0-3) of the word at *a (zero extended)
load.h Sd <- Sa << #0,1                      - d = half-word lane (0-1) of the word at *a (zero extended)
load.w Sd <- Sa                              - d = *a
load.x Sd <- Sa [<< #n] + #imm<10>           - offset form: address = a + #imm<10>, a is unchanged
load.x Sd <- Sa [<< #n] += #imm<10>          - post-increment form: address = a, then a += #imm<10>
save.b Ss << #0,1,2,3 -> Sa                  - the byte lane (0-3) of the word at *a = low 8 bits of s
save.h Ss << #0,1 -> Sa                      - the half-word lane (0-1) of the word at *a = low 16 bits of s
save.w Ss -> Sa                              - *a = s
save.x Ss [<< #n] -> Sa + #imm<10>           - offset form: address = a + #imm<10>, a is unchanged
save.x Ss [<< #n] -> Sa += #imm<10>          - post-increment form: address = a, then a += #imm<10>
```

`x` stands for `b`, `h` or `w`. The shift selects the byte or half-word lane within the addressed word; a word access must have a shift of 0 and a half-word shift is at most 1. An immediate of 0 is always the post-increment form (a += 0). A sub-word `save` reads the word, replaces only the selected lane and writes the word back. A failed bus access raises a bus fault and leaves the registers unchanged.

There are also forms whose base is a special register rather than a scratch register: the current stack address (`SCA`) and the program address (`PA`). These read or write the word at `SCA` or `PA` directly.

```asm
load Sd <- SCA                               - d = *SCA
load Sd <- PA                                - d = *PA
save Ss -> SCA                               - *SCA = s
save Ss -> PA                                - *PA = s
```

### Stack Address

Implies SCA inc/dec, _can cause Hardware Faults_!

```asm
grow #imm<10>                       - SCA -= #imm<10> * sizeof(word)
grow #imm<10> : Ee, Em              - same, only if (Ee & Em) != 0
fall #imm<10>                       - SCA += #imm<10> * sizeof(word)
fall #imm<10> : Ee, Em              - same, only if (Ee & Em) != 0
push {mask}                         - SCA -= sizeof(word) * popcount(mask), store registers in mask to stack
pull {mask}                         - load registers in mask from stack, SCA += sizeof(word) * popcount(mask)
```

### Bitwise1 Operators

All bitwise1 mnemonics are four letters except for reverse, which carries a size suffix.

The optional `: Ex` form writes an Arithmetic evaluation to `Ex` from the result: `positive` is set when bit 31 is clear, `zero` is set when the result is zero, and `overflow` and `undefined` are always clear. Without `: Ex` the evaluation registers are not modified.

```asm
bcpl Sd, Ss : Ex                    - d = ~s (bit invert)
brsh Sd, Ss, #imm<5> : Ex           - d = s >> #imm<5>
basr Sd, Ss, #imm<5> : Ex           - d = int((int)s >> #imm<5>)
blsh Sd, Ss, #imm<5> : Ex           - d = s << #imm<5>
brot Sd, Ss, #imm<5> : Ex           - d = (s << #imm<5>) | (s >> (32 - #imm<5>)) (rotate left, no bits are lost)
bcnt Sd, Ss : Ex                    - d = count 1's in s
bclz Sd, Ss : Ex                    - d = count leading zeros in s
bnot Sd, Ss : Ex                    - d = s ? 0 : 0xFFFFFFFF
bset Sd, Ss, #imm<5> : Ex           - d = s | 1 << #imm<5>
bclr Sd, Ss, #imm<5> : Ex           - d = s & ~(1 << #imm<5>)
btgl Sd, Ss, #imm<5> : Ex           - d = s ^ (1 << #imm<5>)
brev.b Sd, Ss : Ex                  - d = s with the bits of each byte reversed
brev.h Sd, Ss : Ex                  - d = s with the bits of each half-word reversed
brev.w Sd, Ss : Ex                  - d = s with the bits of the word reversed
```

### Bitwise2 Operators

All bitwise mnemonics are four letters. The optional `: Ex` form sets flags in `Ex` as described for Bitwise1.

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
neg.sXX Sd, Ss : Ex                          - d = -s (signed two's complement; XX is 8, 16 or 32)
neg.sXX.sat Sd, Ss : Ex                      - d = -s, clamped to the signed range instead of wrapping
```

`neg` is signed only. Negating the most negative value overflows: it wraps to itself, or saturates to the maximum with `.sat`, and the `overflow` flag is set in `Ex`.

### Float Unary Operators

Floating point operations work on the 32-bit `float` view of scratch registers. The one-source forms take `Sd, Ss` and have no evaluation operand.

```asm
fabs Sd, Ss                              - d = |s|
fneg Sd, Ss                              - d = -s
fflr Sd, Ss                              - d = floor(s)
fcel Sd, Ss                              - d = ceil(s)
ffrc Sd, Ss                              - d = s - floor(s)
fcvt.i2f Sd, Ss                          - d = float(signed int s)
fcvt.f2i Sd, Ss                          - d = int(float s), truncated, saturating, NaN -> 0
```

### Float Binary Operators

The two-source forms take `Sd, Sa, Sb` and may name an evaluation register (`: Ex`) for the conditional form.

```asm
fadd Sd, Sa, Sb : Ex                     - d = a + b (precision flags)
fsub Sd, Sa, Sb : Ex                     - d = a - b (precision flags)
fmul Sd, Sa, Sb : Ex                     - d = a * b (precision flags)
fdiv Sd, Sa, Sb : Ex                     - d = a / b (precision flags; instruction fault if |b| < FLT_EPSILON)
```

When the `: Ex` form is used, the result flags are written to `Ex` as a Precision evaluation (type 3). Without `: Ex` the evaluation register is not modified. The flags are:

* inexact - the result is not exactly representable and was rounded
* rounded - set along with inexact (the result was rounded)
* subnormal - the result is a subnormal (denormal) value
* underflow - the result was tiny and inexact

### Comparisons

The `comp` instruction sets flags related to the type of comparison being performed. The flags are stored in the evaluation register of the given index. The evaluation type is encoded in the instruction and is one of:

* Comparison (logical) which sets flags based on the result of a bitwise comparison of the two operands (e.g., equal, not equal, less than, greater than, etc.).
* Arithmetic which sets flags based on the result of an arithmetic comparison of the two operands (e.g., zero, overflow, underflow, etc.).
* Precision (floating point) which sets flags based on the result of a floating point comparison of the two operands.

```asm
comp Sa, Sb : Ex                         - Compare a and b, set flags in Ex (type and signedness are encoded)
```

### Future Instructions
