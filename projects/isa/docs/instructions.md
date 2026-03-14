# Instructions

The set of instructions in the ISA generall fall into several categories including data movement, stack manipulation, arithmetic/logic operations, floating point operations, and control flow.

---

```asm
# Unary d = op s
op d, s
```

---

```asm
# Binary: d = a op b with side effects in e
op d, a, b
```

---

## Naming

* `Sx` - Scratch Register x
  * `Sd` - Scratch destination
  * `Sa` - Scratch source A (or address)
  * `Sb` - Scratch source B
* `Ex` - Evaluation Register x

## Operators

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
jump Sa                             - PA = Sa, RA unchanged
jump Es, Em, Sa                     - PA = Sa if (Es & Em) != 0, RA unchanged
call Sa                             - PA = Sa, RA = PA + 4 (todo: special case of copy?)
call Sa, #imm<16>                   - PA = Sa + offset (todo: special case of copy?)
call #imm<30>                       - PA = #imm<30> << 2
call Es, Em, Sa                     - PA = Sa, RA = PA + 4 if (Es & Em) != 0
return                              - PA = RA (todo: special case of copy?)
sys #imm<16>                        - syscall with code #imm<16>
```

This translates to an instruction word of:

```c++
0bxxxx'xxxx'xxxx'xxxx'xxxx'xxxx'xxxx'xx01 - jump #imm<32> (last 2 bits of imm must be zero)
0bxxxx'xxxx'xxxx'xxxx'xxxx'xxxx'xxxx'xx11 = call #imm<32> (last 2 bits of imm must be zero)
//-------------------------------------^ set RA or not
```

### Data Movement

```asm
copy Sd, Ss                         - d = s
copy Sd, #imm<16>                   - d = #imm<16>
swap Sa, Sb                         - a swap b
zero {mask}                         - all registers set in mask are zeroed
load Sd, Sa                         - d = *(a)
load Sa, {mask}                     - Sx,y,z = *Sa, *Sa+4, etc
store Sa, Ss                        - *(a) = s
store Sa, {mask}                    - *Sa, *(Sa+4), etc = Sx,y,z
```

This translates to:

```cpp
0bxxxx'xxxx'xxxx'xxxx'xxxx'xxxx'xxxx'x100 - copy Sd, Ss
```



### Stack Address

Implies SCA inc/dec, _can cause Hardware Faults_!

```asm
grow #imm<16>                       - SCA -= #imm<16> * word_size
shrink #imm<16>                     - SCA += #imm<16> * word_size
load Sd, SCA, Ro                    - d = *(SCA + o)
store SCA, Ro, Ss                   - *(SCA + o) = s
save {flags}                        - store multiple from SCA, SCA -= count_of_flags * word_size
restore {flags}                     - load multiple to SCA, SCA += count_of_flags * word_size
copy Sd, SCA                        - d = SCA
copy Sd, SLA                        - d = SLA
copy Sd, SBA                        - d = SBA
copy SCA, Ss                        - SCA = s
copy SLA, Ss                        - SLA = s
copy SBA, Ss                        - SBA = s
```

### Logic Operators

```asm
and  Sd, Sa, Sb                     - d = a & b
or   Sd, Sa, Sb                     - d = a | b
xor  Sd, Sa, Sb                     - d = a ^ b
nand Sd, Sa, Sb                     - d = !(a & b)
nor  Sd, Sa, Sb                     - d = !(a | b)
nxor Sd, Sa, Sb                     - d = !(a ^ b)

bis  Sd, Ss, #imm<5>                - d = s | 1 << #imm<5>
bic  Sd, Ss, #imm<5>                - d = s & ~(1 << #imm<5>)
bit  Sd, Ss, #imm<5>                - d = s ^ (1 << #imm<5>)

cmpl Sd, Ss                         - d = ~s (bit invert)
lsh  Sd, Ss, #imm<5>                - d = s << #imm<5>
rsh  Sd, Ss, #imm<5>                - d = s >> #imm<5>
arsh Sd, Ss, #imm<5>                - d = int((int)s >> #imm<5>)
```

### ALU Ops

All arithmetic logic operations effects eval register of same index as scratch register destintation.

```asm
add Sd, Sa, Sb                      - d = a + b (e flags)
sub Sd, Sa, Sb                      - d = a - b (e flags)
mul Sd, Sa, Sb                      - d = a * b (e flags)
div Sd, Sa, Sb                      - d = a / b (e flags)
mod Sd, Sa, Sb                      - d = a % b (e flags)
```

### FPU Ops

Effects eval register of the same index as scratch register.

```asm
fadd Sd, Sa, Sb                     - d = a + b (e flags)
fsub Sd, Sa, Sb                     - d = a - b (e flags)
fmul Sd, Sa, Sb                     - d = a * b (e flags)
fdiv Sd, Sa, Sb                     - d = a / b (e flags)
fflr Sd, Ss                         - d = floor(s)
fcel Sd, Ss                         - d = ceil(s)
fcvt Sd, Ss                         - d = int(s)
fcvt Sd, Sd                         - d = float(s)
```

### Comparisons

```asm
cmp Ed, Sa, Sb (parallel compare and sets flags in e)
```
