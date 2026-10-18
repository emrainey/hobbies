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
| `MATCHED` | Verified matching, kept here as a regression anchor |

Milestone IDs refer to @ref rtl_roadmap.

## Index

| ID | Area | Status | Milestone |
| --- | --- | --- | --- |
| P-001 | `Grow` stack-overflow condition | `CPP-BUG` — fixed | M3 |
| P-002 | `Push`/`Pull` vs `Grow`/`Fall` bound style | OPEN | M3 |
| P-003 | Divide-by-zero fault class | OPEN (suspected `CPP-BUG`) | M6/M8 |
| P-004 | Caches unused on hot path | DEFERRED | M10 |
| P-005 | Privilege not enforced | DEFERRED | M8 |
| P-006 | Exception mask never consulted | DEFERRED | M8 |
| P-007 | Interrupts not implemented | DEFERRED | M9 |
| P-008 | Floating-point precision semantics | DEFERRED | M11 |
| P-009 | `Load`/`Save` special-base encoding | OPEN | M2 |
| P-010 | `Reverse` unit selector reuses `shift` | OPEN | M5 |
| P-011 | Erased word / immediate-leap decode | MATCHED | M0 |
| P-012 | `Push`/`Pull` register ordering | OPEN | M3 |
| P-013 | `Move` eval-to-eval semantics | OPEN | M1/M4 |
| P-014 | `MoveImmediate` eval/upper selection | OPEN | M1 |
| P-015 | `Swap` load-then-store atomicity | OPEN | M2 |
| P-016 | Bitwise evaluation flags | OPEN | M5/M7 |
| P-017 | Arithmetic `undefined` flag vs fault | OPEN (linked P-003) | M6/M8 |
| P-018 | `Negate` size encoding | OPEN | M7 |
| P-020 | `Trip` redirect timing | OPEN | M3/M8 |
| P-021 | Breakpoints / persistence | DEFERRED (not planned for RTL) | — |

## Details

### P-001 — `Grow` stack-overflow condition

`source/cpu.cpp:542-571`. The stack grows *down* from `limit` toward `base`, so growing subtracts
from `current`. The original code accepted the operation when `next <= base` and raised
`stack_fault` otherwise, which was inverted: `next <= base` is exactly the overflow case, and the
normal case (`next > base`) faulted. This also made `demo/program1.hpp`'s `Grow{4}` fault against
its configured stack (`current=0x10008000`, `base=0x10000000`).

**Fixed (C++).** `Grow` now moves `current` down by `imm * 4` and succeeds only when the result is
still at or above `base`; it sets `stack_fault` and leaves `current` unchanged when the result would
drop below `base`. The subtraction is guarded so a pointer near zero cannot wrap into the top of the
address space. `Grow{0}` remains a no-op. Covered by `GrowMovesStackPointerDownByImmediateWords`,
`GrowCanReachStackBase`, `GrowSetsStackFaultWhenGrowingBelowBase`,
`GrowSetsStackFaultWhenThePointerWouldWrap` and `GrowByZeroDoesNotMoveOrFault` in
`test/gtest_instruction.cpp`. The RTL must reproduce this predicate in M3.

### P-002 — `Push`/`Pull` vs `Grow`/`Fall` bound style

`source/cpu.cpp:586-637` (push/pull, via `Contained`) vs `:542-585` (grow/fall, via explicit compare).
Two different bounds idioms for the same stack. Not necessarily a bug, but the RTL must reproduce
whichever is intended. Resolve together with P-001.

### P-003 — Divide-by-zero fault class

`source/cpu.cpp:1005,1016,1030,1041,1055,1066`. Division/modulo by zero sets `instruction_fault`.
The ISA defines a distinct `arithmetic_fault` (`include/isa/cpu.hpp:80`, `ExceptionType::ArithmeticFault`)
and `docs/exceptions.md` lists arithmetic faults separately. Suspected wrong fault class in C++.
Determines the `GetHandler` vector taken, so it matters for M8.

### P-004 — Caches unused on hot path

`include/isa/cpu.hpp:17-20` declares `InstructionCache` (`Cache<Instruction, 256, 1>`) and
`DataCache` (`Cache<uint8_t, 1024, 16>`), but `Processor::Cycle()` reads and writes through
`Peek`/`Poke` (`cpu.cpp:290-1318`) and never touches them. Documented design includes the caches, so
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
flag bits (`cpu.hpp:67-105`) and a `GetHandler` branch each (`cpu.cpp:1320-1376`), but no trigger
path exists in `Cycle()`. Note `deferred` is documented as "raised after the next instruction is
executed", which needs a pending latch rather than an immediate redirect. C++ first (M9).

### P-008 — Floating-point precision semantics

`source/cpu.cpp:1218-1307`. FP uses the host `float` and `fenv` (`FE_INEXACT`/`FE_UNDERFLOW`), and
sets `flags.rounded = flags.inexact` (`:1301`) as an approximation. `FloatingDivision` also treats
`|divisor| < epsilon` as divide-by-zero (`:1274`). None of this is bit-exactly reproducible in RTL;
the FP strategy (real IEEE-754 unit vs. third-party core vs. independent reference vectors) is
deferred to M11.

### P-009 — `Load`/`Save` special-base encoding

`source/cpu.cpp:389-396` (load) and `:435-442` (save). When `special==1` the `base` scratch index is
reused as a selector: `base==4` → stack current, `base==0` → program address, anything else →
`instruction_fault`. Mirror exactly, including the fault case.

### P-010 — `Reverse` unit selector reuses `shift`

`source/cpu.cpp:782-796`. For `Reverse`, the `shift` field is reinterpreted as a unit selector
(`0`=byte, `1`=half, `2`=word); a value `> 2` sets `instruction_fault`. The RTL decode must apply the
same reinterpretation per-opcode.

### P-011 — Erased word / immediate-leap decode

`include/isa/instructions.hpp:76-86`, `rtl/decoder.sv:16-23`, `rtl/isa.sv:31,111`. An erased word
(`0xFFFFFFFF`) decodes as `noop`; bit 0 of the low byte selects the immediate-leap form; otherwise
the opcode is used and `is_defined_op` decides validity. Already verified by `gtest_rtl_decoder`;
kept as a regression anchor.

### P-012 — `Push`/`Pull` register ordering

`source/cpu.cpp:596-604` (push) and `:621-631` (pull). Registers are walked in ascending scratch
order while the stack address walks descending (push) / ascending (pull). The RTL must match this
order for the resulting memory image to be identical.

### P-013 — `Move` eval-to-eval semantics

`source/cpu.cpp:336-347`. When both `dst` and `src` are Evaluation registers, `type==1`; the shift
applies to the 8-bit eval value, and the source is zeroed when `dst != src` (as it is for scratch).
Match the zeroing behaviour exactly.

### P-014 — `MoveImmediate` eval/upper selection

`source/cpu.cpp:350-357`. The `eval` bit selects an Evaluation destination and masks the immediate to
8 bits; otherwise `upper` picks the upper 16-bit half of the scratch register. The `upper`+eval
combination is rejected at construction (`instructions.hpp:249`), so the RTL need not define it.

### P-015 — `Swap` load-then-store atomicity

`source/cpu.cpp:358-367`. `Swap` peeks the address and pokes the exchanged value; if either fails it
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

`source/cpu.cpp:1100-1133`. `neg.sXX` carries a 2-bit `size` in the top bits of the word and is
implemented as `Subtraction(0, src)` per width; the evaluation is written only when `cond` and no
instruction fault occurred.

### P-020 — `Trip` redirect timing

`source/cpu.cpp:665-683` sets `numeral.trip` and the `tripped` flag (and optionally the debug
breakpoint) but does not redirect inline; the redirect happens in the `Cycle()` tail via
`GetHandler()` (`:1312-1313`). The RTL FSM must place the redirect at the same commit point.

### P-021 — Breakpoints / persistence

`source/cpu.cpp:96-141,143-278` (16 breakpoints, `Save`/`Load` of scratch/special/eval/cache/memory
images). Debugger-side functionality, not architectural; not planned for RTL parity.

## Open questions

* P-002: the intended bound idiom for `Fall` (whether to align it with the corrected `Grow`
  predicate or leave the explicit `next <= limit` check).
* P-003/P-017: whether divide-by-zero is an instruction fault, an arithmetic fault, an evaluation
  bit, or some combination.
* P-007: whether `deferred` needs an explicit pending latch in the architectural state.
