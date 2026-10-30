# RTL Parity Ledger

Running record of every known divergence or ambiguity between the C++ fast model
(`source/`, `include/isa/`) and the SystemVerilog model (`rtl/`), plus the parts of the documented
design that neither implements yet.

This is the artifact that makes "decide case-by-case" tractable — see @ref rtl_roadmap. Every time the
co-simulation harness reports a mismatch, add a row here before changing either model.

## How to use it

When the two models disagree, determine which side is wrong and record the call:

| Status | Meaning |
| --- | --- |
| `OPEN` | Divergence known, not yet adjudicated |
| `CPP-BUG` | The C++ model is wrong; RTL follows the spec/docs |
| `RTL-BUG` | The RTL is wrong; C++ is the reference |
| `SPEC` | Both are defensible; the docs are ambiguous and need a decision first |
| `DEFERRED` | Agreed to differ for now; revisit at the named milestone |
| `RTL-AHEAD` | The RTL is intentionally *more* faithful than the C++ model; parity is asserted at the architectural result, not the bus transaction |
| `MATCHED` | Verified matching, kept here as a regression anchor |

Milestone IDs refer to @ref rtl_roadmap.

## Index

| ID | Area | Status | Milestone |
| --- | --- | --- | --- |
| P-001 | `Grow` stack-overflow condition | `CPP-BUG` — fixed | M3 |
| P-002 | `Push`/`Pull` vs `Grow`/`Fall` bound style | `CPP-BUG` — fixed | M3 |
| P-003 | Divide-by-zero fault class | OPEN (suspected `CPP-BUG`) | M6/M8 |
| P-004 | Caches unused on hot path | DEFERRED | M10 |
| P-005 | Privilege not enforced | DEFERRED | M8 |
| P-006 | Exception mask never consulted | DEFERRED | M8 |
| P-007 | Interrupts not implemented | DEFERRED | M9 |
| P-008 | Floating-point precision semantics | DEFERRED | M11 |
| P-009 | `Load`/`Save` special-base encoding | OPEN | M2 |
| P-010 | `Reverse` unit selector reuses `shift` | OPEN | M5 |
| P-011 | Erased word / immediate-leap decode | MATCHED | M0 |
| P-012 | `Push`/`Pull` register ordering | `CPP-BUG` — fixed | M3 |
| P-013 | `Move` eval-to-eval semantics | MATCHED | M1/M4 |
| P-014 | `MoveImmediate` eval/upper selection | MATCHED | M1 |
| P-015 | `Swap` load-then-store atomicity | OPEN | M2 |
| P-016 | Bitwise evaluation flags | OPEN | M5/M7 |
| P-017 | Arithmetic `undefined` flag vs fault | OPEN (linked P-003) | M6/M8 |
| P-018 | `Negate` size encoding | OPEN | M7 |
| P-020 | `Trip` redirect timing | OPEN | M3/M8 |
| P-021 | Breakpoints / persistence | DEFERRED (not planned for RTL) | — |
| P-022 | Byte-strobed sub-word writes | `RTL-AHEAD` | M6 |
| P-023 | `Move` shift builder truncates | `CPP-BUG` fixed | M1 |
| P-024 | Taken control transfer resumes past its target | `CPP-BUG` — fixed | M3 |
| P-025 | `Leap` immediate applied in bytes | `CPP-BUG` — fixed | M3 |

## Details

### P-001 — `Grow` stack-overflow condition

`source/cpu.cpp:552-581`. The stack grows *down* from `limit` toward `base`, so growing subtracts
from `current`. The original code accepted the operation when `next <= base` and raised
`stack_fault` otherwise, which was inverted: `next <= base` is exactly the overflow case, and the
normal case (`next > base`) faulted. This also made the demo program of the time (`Grow{4}`) fault
against its configured stack (`current=0x10008000`, `base=0x10000000`); that program has since been
replaced, and the case is covered by the tests below.

**Fixed (C++).** `Grow` now moves `current` down by `imm * 4` and succeeds only when the result is
still at or above `base`; it sets `stack_fault` and leaves `current` unchanged when the result would
drop below `base`. The subtraction is guarded so a pointer near zero cannot wrap into the top of the
address space. `Grow{0}` remains a no-op. Covered by `GrowMovesStackPointerDownByImmediateWords`,
`GrowCanReachStackBase`, `GrowSetsStackFaultWhenGrowingBelowBase`,
`GrowSetsStackFaultWhenThePointerWouldWrap` and `GrowByZeroDoesNotMoveOrFault` in
`test/gtest_instruction.cpp`. The RTL must reproduce this predicate in M3.

### P-002 — `Push`/`Pull` vs `Grow`/`Fall` bound style

`source/cpu.cpp` stack operations. The original code used two different bounds idioms for the same
stack: `Contained(addr)` (`base <= addr < limit`) for `Push`/`Pull`, and explicit comparisons for
`Grow`/`Fall`. `Contained` is an *address* predicate (the storage is half-open) but it was applied to
post-move *stack pointers*, so the legal empty-stack pointer, `current == limit`, was rejected:
`Pull` up to the limit and `Push #0` at an empty stack both raised a spurious `stack_fault`.

**Fixed (C++).** `Stack` now exposes two predicates on 64-bit intermediates:

* `ContainsPointer(uint64_t)` — legal pointer range `[base, limit]` (closed at the exclusive top).
* `ContainsBlock(uint64_t start, uint32_t size)` — byte range `[start, start + size)` lies inside the
  storage `[base, limit)`.

`Grow`, `Fall`, `Push` and `Pull` all use `ContainsPointer`/`ContainsBlock`, with arithmetic widened to
64 bits so a pointer near zero or near the top of the address space cannot wrap into a plausible
value. `Grow{0}` and `Fall{0}` are no-ops, and a zero-register `Push`/`Pull` at an empty stack is a
success rather than a fault. Covered by the round-trip tests
(`GrowThenFallRoundTripsStackPointer`, `FallThenGrowRoundTripsStackPointer`,
`GrowToBaseThenFallReturnsToLimit`, `PushThenPullRoundTripsSelectedRegisters`), the asymmetric
accumulation test `GrowThenFallKeepsTrackOfMixedImmediates`, the top-boundary test
`FallReachesLimitExactlyButCannotPassIt`, and the empty-stack cases `PushByZeroAtEmptyStackIsNoOp`,
`FallByZeroAtEmptyStackIsNoOp` and `PullCanFillStackToLimit`.
The RTL must reproduce these two predicates exactly in M3.

### P-003 — Divide-by-zero fault class

`source/cpu.cpp:1016,1027,1041,1052,1066,1077`. Division/modulo by zero sets `instruction_fault`.
The ISA defines a distinct `arithmetic_fault` (`include/isa/cpu.hpp:80`, `ExceptionType::ArithmeticFault`)
and `docs/exceptions.md` lists arithmetic faults separately. Suspected wrong fault class in C++.
Determines the `GetHandler` vector taken, so it matters for M8.

### P-004 — Caches unused on hot path

`include/isa/cpu.hpp:17-20` declares `InstructionCache` (`Cache<Instruction, 256, 1>`) and
`DataCache` (`Cache<uint8_t, 1024, 16>`), but `Processor::Cycle()` reads and writes through
`Peek`/`Poke` (`cpu.cpp:290-1329`) and never touches them. Documented design includes the caches, so
per the roadmap they are in scope — C++ first (M10), then RTL.

### P-005 — Privilege not enforced

`include/isa/cpu.hpp:459-474` defines `PrivilegedInstructions` (`Halt`, `Trip`) and
`IsPrivilegedInstruction`, but `Cycle()` never calls it and never reads `Mode` (`cpu.hpp:51-63`). A
privileged instruction in nonprivileged mode should raise a privilege fault. C++ first (M8).

### P-006 — Exception mask never consulted

`Cycle()` sets individual exception flag bits directly; `ExceptionMask` (`cpu.hpp:222-294`) and the
`Mode`/`Exception` state machine are never applied. Masking and nested-exception behaviour are
documented (`docs/exceptions.md`). C++ first (M8).

### P-007 — Interrupts not implemented

`Ticker`, `Deferred`, `External`, `Unmaskable` and `Nested` have vector-table slots and `Exception`
flag bits (`cpu.hpp:67-105`) and a `GetHandler` branch each (`cpu.cpp:1331-1387`), but no trigger
path exists in `Cycle()`. Note `deferred` is documented as "raised after the next instruction is
executed", which needs a pending latch rather than an immediate redirect. C++ first (M9).

### P-008 — Floating-point precision semantics

`source/cpu.cpp:1229-1318`. FP uses the host `float` and `fenv` (`FE_INEXACT`/`FE_UNDERFLOW`), and
sets `flags.rounded = flags.inexact` (`:1301`) as an approximation. `FloatingDivision` also treats
`|divisor| < epsilon` as divide-by-zero (`:1274`). None of this is bit-exactly reproducible in RTL;
the FP strategy (real IEEE-754 unit vs. third-party core vs. independent reference vectors) is
deferred to M11.

### P-009 — `Load`/`Save` special-base encoding

`source/cpu.cpp:392-399` (load) and `:438-445` (save). When `special==1` the `base` scratch index is
reused as a selector: `base==4` → stack current, `base==0` → program address, anything else →
`instruction_fault`. Mirror exactly, including the fault case.

### P-010 — `Reverse` unit selector reuses `shift`

`source/cpu.cpp:792-806`. For `Reverse`, the `shift` field is reinterpreted as a unit selector
(`0`=byte, `1`=half, `2`=word); a value `> 2` sets `instruction_fault`. The RTL decode must apply the
same reinterpretation per-opcode.

### P-011 — Erased word / immediate-leap decode

`include/isa/instructions.hpp:76-86`, `rtl/decoder.sv:16-23`, `rtl/isa.sv:31,111`. An erased word
(`0xFFFFFFFF`) decodes as `noop`; bit 0 of the low byte selects the immediate-leap form; otherwise
the opcode is used and `is_defined_op` decides validity. Already verified by `gtest_rtl_decoder`;
kept as a regression anchor.

### P-012 — `Push`/`Pull` register ordering

`source/cpu.cpp` `Push`/`Pull`. Registers are walked in ascending scratch order and the stack address
walks ascending in both directions. The original `Push` walked the address *descending*
(`address -= 4` while `Pull` used `address += 4`), so a multi-register `Push` followed by an identical
`Pull` did not round-trip: the memory image was mirrored.

**Fixed (C++).** `Push` now writes the newly allocated block `[next, current)` from low to high, the
same order `Pull` reads it, so the two are inverses for any register mask. Covered by
`PushThenPullRoundTripsSelectedRegisters`, and the existing `PushStoresSelectedRegistersAndMovesStackPointer`
now expects the higher-numbered register at the higher address (`0x10000FFC`). The RTL must match this
order for the resulting memory image to be identical.

### P-013 — `Move` eval-to-eval semantics

`source/cpu.cpp:339-350`. When both `dst` and `src` are Evaluation registers, `type==1`; the shift
applies to the 8-bit eval value, and the source is zeroed when `dst != src` (as it is for scratch).
Match the zeroing behaviour exactly.

**MATCHED (M1).** `rtl/processor.sv` implements the 8-bit shift and the conditional source zeroing,
and `RtlProcessorTest.MoveShiftsEvaluationWithinEightBits` diffs each step — including a shift of 4,
where a left shift truncates at the byte (`0xFF << 4 == 0xF0`) and a shift of 8 produces zero.

### P-014 — `MoveImmediate` eval/upper selection

`source/cpu.cpp:353-360`. The `eval` bit selects an Evaluation destination and masks the immediate to
8 bits; otherwise `upper` picks the upper 16-bit half of the scratch register. The `upper`+eval
combination is rejected at construction (`instructions.hpp:249`), so the RTL need not define it.

**MATCHED (M1).** `rtl/processor.sv` writes `evaluation[dst] <= imm[7:0]`, or the selected 16-bit half
of the scratch register; `RtlProcessorTest.MoveImmediateWritesHalvesAndMasksEvaluationDestination`
pins the surviving half and the 8-bit mask.

### P-015 — `Swap` load-then-store atomicity

`source/cpu.cpp:361-370`. `Swap` peeks the address and pokes the exchanged value; if either fails it
sets `bus_fault` and leaves the register untouched. The RTL must not partially commit the exchange
on a fault.

### P-016 — Bitwise evaluation flags

`source/cpu.cpp:280-288` (`BitwiseEvaluation`): `positive` is set when the MSB is clear, `zero` when
the result is zero; overflow/undefined are not set. Shared by `bitwise1`, `bitwise2` and `extend`.

### P-017 — Arithmetic `undefined` flag vs fault

`include/isa/types.hpp:234-251` defines `ArithmeticEvaluation.undefined` for divide-by-zero, but
`Cycle()` routes divide-by-zero to a fault instead of writing an evaluation (linked to P-003).
Decide together whether the flag, the fault, or both apply.

### P-018 — `Negate` size encoding

`source/cpu.cpp:1111-1144`. `neg.sXX` carries a 2-bit `size` in the top bits of the word and is
implemented as `Subtraction(0, src)` per width; the evaluation is written only when `cond` and no
instruction fault occurred.

### P-020 — `Trip` redirect timing

`source/cpu.cpp:676-694` sets `numeral.trip` and the `tripped` flag (and optionally the debug
breakpoint) but does not redirect inline; the redirect happens in the `Cycle()` tail via
`GetHandler()` (`:1312-1313`). The RTL FSM must place the redirect at the same commit point.

### P-021 — Breakpoints / persistence

`source/cpu.cpp:96-141,143-278` (16 breakpoints, `Save`/`Load` of scratch/special/eval/cache/memory
images). Debugger-side functionality, not architectural; not planned for RTL parity.

### P-022 — Byte-strobed sub-word writes

`rtl/processor.sv` `mem_wstrb`. A sub-word `save` is a real strobed write in hardware: the lanes
covered by `size`/`shift` are written and the remaining lanes are left unchanged. The C++ model does
not model this. `Cycle()` writes only whole words and implements a sub-word `save` as a software
read-modify-write — `source/cpu.cpp:478` peeks the word, merges the `size`/`shift`-selected bits, and
pokes the result — and `Bus::Poke` accepts only one whole `AddressableUnitType`, which on the
32-bit-unit flash and tightly-coupled buses is a `uint32_t`. There is no byte-granular write path in
C++ at all.

**RTL-AHEAD.** The RTL behaviour is the faithful one and is kept; this is a case of the C++ shortcut
being an implementation detail rather than a specification. The consequence is that a sub-word store
step cannot be compared as an identical bus transaction. The current RTL tests assert each store's
address, write data and byte strobes. They do not yet independently diff the final memory image:
the M0 harness services the DUT bus from `oracle.Peek`, so the oracle's memory is not an independent
DUT memory model and can mask a missing DUT write. Revisit at M6, where `Load`/`Save` make it concrete.

The intended closure is the bus seam in the structure track (S1, @ref rtl_roadmap): once the C++ model
issues a request carrying byte strobes and the memory backends honour them, both models perform the
same transaction and this row can move to `MATCHED` without changing either model's architectural
behaviour.

### P-023 — `Move` shift builder truncated

`include/isa/instructions.hpp:150`. `Move`'s shift operand was built from
`ImmediateType = isa::Immediate<log2(CountOfDataShiftBits)>`, while the `shift` field it populates is
`CountOfDataShiftBits` bits wide. `CountOfDataShiftBits` is *already* `log2(CountOfDataBits)` = 5, so
the extra `log2` narrowed the immediate to 2 bits. `Immediate`'s constructor takes a `uint32_t` into a
2-bit bitfield, so `Move{S1, S2, false, 4}` silently encoded a shift of **zero** rather than being
rejected — `Move::ImmediateType{4}.value == 0`.

This was not theoretical: the demo program of the time wrote `Move{..., true, Move::ImmediateType{4}}`
(it has since been replaced), which had been encoding a shift of 0.

**CPP-BUG (fixed, M1).** Both models read the 5-bit field identically — `rtl/isa.sv`'s `move_t` and
the C++ bitfield agree, which `gtest_layout` already verified — so this was a defect in the C++
*builder*, not a model divergence. The fix is the field width itself:

```cpp
using ImmediateType = isa::Immediate<CountOfDataShiftBits>;
static_assert(ImmediateType::Bits == CountOfDataShiftBits, "...");
```

The `static_assert` is not tautological: it constrains the *expression* rather than the template
parameter, so reintroducing `log2(...)` fails the build. `LayoutTest.MoveShiftOperandEncodesEveryShiftAmount`
walks shifts 0..31 and checks the encoded field, and `LayoutTest.MoveFieldsMatchCppBitfields` pins the
rest of the format; `Move::ImmediateType` is the only builder alias that did not take its width from
the field it fills (`MoveImmediate`, `Zero` and the rest do).

A shift literal of 32 or more still truncates to the low 5 bits, which is inherent to the encoding
and now consistent with every other immediate.

### P-024 — A taken control transfer resumed one instruction past its target

`source/cpu.cpp`. `Cycle()` ends by advancing the Program Address to the instruction which comes next
(`program_address_ += sizeof(Instruction)`). `Leap`, `LeapImmediate`, `Back` and `Call` had already
written the address to run next, so the shared tail advanced them a second time and every taken
control transfer landed **four bytes past its target**.

`docs/instructions.md:57` (`PA = Sd + (sign_extend(#simm<10>) * 4)`) and `:61`
(`PA = #addr<30:2> << 2`) are unambiguous that the target *is* the next Program Address, so the model
was wrong rather than the documentation. No RTL depended on it yet — M1 implements no control
transfer — so it surfaced through the demo program rather than through co-simulation: the fill loop
restarted on its `save` instead of its `add`, the running value never advanced, and every generated
word came out identical.

**CPP-BUG (fixed, M3 prep).** `Cycle()` now records whether the instruction chose its own next address
(`control_transferred`) and skips the tail increment when it did. The exception path is unchanged and
still takes priority, and a `leap` whose condition is false still falls through. Six tests in
`test/gtest_instruction.cpp` had frozen the wrong behaviour and were updated
(`LeapJumpsToTargetAddress`, `ImmediateLeapJumpsToAbsoluteAddress`,
`ImmediateLeapWithSaveStoresReturnAddress`, `BackJumpsToReturnAddress`,
`BackWithZeroSetsReturnAddressToSafeHandler`, `CallJumpsToSystemCallHandlerAndStoresCallNumber`);
`LeapThatIsNotTakenFallsThrough` was added so the incremental path stays pinned. The M3 RTL must land
the target address directly.

### P-025 — `Leap` immediate measured in bytes

`source/cpu.cpp`. The same block added `leap.imm` to the base address as a raw byte count, while
`docs/instructions.md:51` and the `Leap` field comment (`include/isa/instructions.hpp:555`) define it
as a count of instructions which is scaled by four to become a byte offset. The two agree only for
`imm == 0`, which is all the tests exercised, so nothing caught it.

**CPP-BUG (fixed, M3 prep).** The offset is now
`int64_t(leap.imm) * static_cast<int64_t>(sizeof(Instruction))`, computed in 64 bits so that neither
the sign nor the scaling can wrap, with the existing overflow and underflow bus-fault guards applied
to the scaled value. `LeapImmediateIsMeasuredInInstructions` and
`LeapBackwardsScalesTheImmediateAndLandsOnTheTarget` pin `+2` and `-2` as eight bytes either way, which
settles the immediate-scaling conflict recorded while the demo program was being written.

## Open questions

* P-003/P-017: whether divide-by-zero is an instruction fault, an arithmetic fault, an evaluation
  bit, or some combination.
* P-007: whether `deferred` needs an explicit pending latch in the architectural state.
