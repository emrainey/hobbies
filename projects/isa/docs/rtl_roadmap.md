# RTL Roadmap

How to bring the SystemVerilog model (`rtl/`) up to the same point as the C++ fast model
(`source/`, `include/isa/`), and then beyond it to the full documented design.

See @ref rtl_parity for the running list of model-to-model divergences.

## Scope and decisions

The plan below reflects these agreed decisions:

* **Microarchitecture:** single-cycle / multi-cycle *functional parity first*. The 4-stage pipeline in
  @ref pipeline is a later micro-architecture step (M12), layered over a proven golden harness.
* **Oracle:** the C++ model. When the two disagree we decide case-by-case which is wrong and record
  the call in @ref rtl_parity. The C++ model is the oracle for *the milestones in the order below*,
  not in perpetuity: where the RTL is deliberately more faithful to the documented design — a real
  byte-strobed write where C++ performs a read-modify-write — the ledger records `RTL-AHEAD` and
  parity is asserted one level up, at the architectural result, rather than on the transaction.
* **Floating point:** deferred. FP phases (M11) sit after the integer core, exception handling and
  caches.
* **Parity target:** *the documented design*, not merely what `Processor::Cycle()` happens to execute
  today. That includes the caches, the exception mask/mode machinery, privilege enforcement and
  interrupts. Because `Cycle()` does not implement those yet, the C++ model must grow too: those
  features are implemented **C++ first, RTL second**, so the oracle stays valid.
* **Memory ownership:** the RTL **exposes a bus port**; the testbench services it using the existing
  `isa::Bus` / `isa::Memory` semantics. Both models therefore run against the same memory model,
  address map and fault behaviour.

## Current state

| Model | Present | Missing |
| --- | --- | --- |
| C++ (`hobbies-isa`) | One instruction per `Processor::Cycle()`; full ISA datapath; vector-table boot in `Reset()`; direct `Peek`/`Poke` buses; breakpoints; persistence | Caches on the hot path; privilege enforcement; exception mask/mode; interrupts (ticker/deferred/external); arithmetic-fault routing |
| SV (`isa_rtl_sim`, `isa_rtl_processor_sim`) | Generated `isa` package (`rtl/isa.sv`): params, enums, opcodes, `is_defined_op`, instruction format structs and `instruction_t` union. Combinational `decoder` (`rtl/decoder.sv`). M1 `processor` top (`rtl/processor.sv`): reset/vector-table boot, word-bus port, fetch/execute FSM, the register-file instructions (`move`, `movei`, `copy`, `zero`, `halt`), `unsupported` strobe, architectural state exposed to the harness | Everything else: control flow, stack, loads/stores, ALU, exceptions, caches |

The single source of truth already exists: `docs/instructions.json` renders `rtl/isa.sv`,
`include/isa/operator.hpp` and `docs/instruction_formats.md` via `tools/generate_isa.py`, with a
staleness test (`isa_generated_sources_are_current`). Keep it that way — every new field or opcode
goes through the generator, never hand-edited in `isa.sv`.

The spec also carries the **special-register list**. Its word indices are emitted as
`SPECIAL_*` localparams in `isa.sv` and as `isa::special::*` in `operator.hpp`, and `cpu.hpp`
binds its own `isa::Special` field offsets to them with `static_assert`, so reordering a field
or an entry is a compile error rather than a silent RTL/C++ mismatch. The RTL therefore names
special registers (`special[SPECIAL_MODE]`) instead of using magic numbers.

## Cycle contract

`Processor::Cycle()` retires exactly one instruction. The RTL must expose the same boundary so the
two can be stepped in lockstep:

* One **retired instruction** produces exactly one cycle of `retire` high.
* The FSM may take several clock cycles per instruction (fetch, bus wait, execute); `retire` marks
  the commit edge.
* A **faulting** instruction still retires: it is consumed and redirects. `Cycle()` likewise executes
  one instruction, redirects, and still increments `performance_counter_`, so gating `retire` on
  success would desynchronise the models.
* `halted` is exposed so the harness can stop, mirroring `Processor::IsHalted()`.
* `booted` marks the end of the reset vector-table boot, so the harness can diff the reset image
  before the first instruction retires.
* `unsupported` is raised at the commit edge when the instruction cannot be executed yet: a defined
  opcode the milestone has not reached, or an undefined one. It is how a partially-implemented
  `processor` announces its own frontier instead of silently retiring an instruction it does not
  understand. Milestone tests assert it stays low, and it disappears as the opcode set fills in.
* A `mem_*` bus port (below) is held stable across the bus-wait cycles.

The testbench loop is then: drive the program into shared memory, reset both models, then for each
step clock the RTL until `retire` (or `halted`), call `Processor::Cycle()` once, and diff state. Any
mismatch is a @ref rtl_parity row, not an immediate failure.

## Reset contract

`Processor::Reset()` is not just "clear everything"; the RTL reset must reproduce it exactly:

* `scratch` = `0xFFFF_FFFF` in every register, `evaluation` = `0xFF` in every byte.
* `special` is zeroed, then `vector_table_address_` is set to `0x0010_0000`
  (`isa::DefaultVectorTableAddress`) and `mode_` to `0x0000_0004` (`Mode` defaults to
  `nonprivileged = 1`). This last one is easy to miss because `Special{}` is otherwise all zero.
  The RTL builds it from a `mode_t` packed struct with a named assignment pattern
  (`'{nonprivileged: 1'b1, default: '0}`); packed-struct members are listed most-significant
  first, the reverse of the C++ bitfield order.
* Five words are read from the vector table and patched in:
  `stack_initial` → stack `limit`/`current`, `stack_boundary` → stack `base`,
  `exception_stack_initial` → exception stack `limit`/`current`,
  `exception_stack_boundary` → exception stack `base`, `reset_handler` → `PA` and `RA`.
* A failed vector-table read leaves the corresponding field at its reset value.

M0 implements this boot in `rtl/processor.sv`; `RtlProcessorTest.ResetStateMatchesTheFastModel`
diffs the whole image before the first retire.

## Bus port

A single unified word-bus port, routed by the testbench through the same address map that
`Processor::Peek`/`Poke` use (`isa/map.hpp`). The flash and tightly-coupled buses are both
`BusAttributes<32, 8, 1>` (32-bit word transfers, 4-byte aligned), so a 32-bit port covers both.

| Signal | Dir | Width | Meaning |
| --- | --- | --- | --- |
| `mem_req` | out | 1 | A bus access starts this cycle |
| `mem_write` | out | 1 | 1 = write, 0 = read |
| `mem_addr` | out | 32 | Word-aligned address |
| `mem_wdata` | out | 32 | Write data |
| `mem_wstrb` | out | 4 | Write strobe: lane `i` is written; a clear lane is left unchanged |
| `mem_rdata` | in | 32 | Combinational response to `mem_addr`, sampled on the next rising edge |
| `mem_fault` | in | 1 | Unassigned/unaligned/faulting access |

Read data returns combinationally (SRAM-like) so a fetch closes in the following cycle; the
testbench routes `mem_addr` through `isa::memory::Map` and the attached `isa::Memory` objects and
raises `mem_fault` for unassigned addresses, matching `Bus::State::Fault`.

`mem_wstrb` is a real strobe, not a "write zero to the unselected lanes" mask: a clear lane must
survive the write unchanged. This is the first place the RTL is deliberately *ahead* of the C++
model — `Cycle()` writes only whole 32-bit words, implementing a sub-word `save` as a software
read-modify-write (`source/cpu.cpp:475` peeks the word, merges the size/shift-selected bits, pokes it
back), and `Bus::Poke` accepts only one whole `AddressableUnitType`, which on the 32-bit-unit flash
and tightly-coupled buses is a `uint32_t`. The C++ side therefore has no byte-granular write path at
all. The RTL drives strobes directly, so **parity for sub-word stores is asserted on the final memory
image after the step, not on the bus transactions** — P-022 in @ref rtl_parity.

## Co-simulation harness

`test/gtest_rtl_processor.cpp` instantiates **both** models and diffs them. The DUT bus is serviced
from the oracle's own attached memory (`oracle.Peek`), so both sides see the same image and faults:

```cpp
isa::Processor oracle;                   // fast model
Vprocessor dut;                          // verilated RTL
oracle.Reset();

dut.reset = 1; tick(); dut.reset = 0;
while (!dut.booted) tick();              // reset vector-table boot

for (step : steps) {
    oracle.Cycle();
    while (!dut.retire && !dut.halted) tick();   // clock until commit
    assert_equal(oracle.ViewScratch(),      dut.scratch);
    assert_equal(oracle.ViewEvaluations(),  dut.evaluation);
    assert_equal(oracle.ViewSpecial(),      dut.special);
}
```

`tick()` first drives `dut.mem_rdata`/`dut.mem_fault` from `dut.mem_addr`, then toggles the clock, so
the combinational bus response is valid at the commit edge.

State is read straight from the Verilated model's public ports (`scratch`, `evaluation`, `special`),
so no debug ports are needed for M1–M7. A snapshot/debug port can be added later if the harness needs
to cross a process boundary.

Stimulus comes from a generated vector set (see below) plus hand-written directed programs in
`test/`.

## Generator extension

Add a fourth output to `tools/generate_isa.py`: a **test-vector generator** that emits random and
directed instruction streams with the architectural deltas they should produce. Both models consume
the identical stimulus, which turns the diff harness into a fuzz loop for free and keeps the
staleness test working. Vectors must respect the validity rules the decoder already encodes
(`is_defined_op`, the erased-word special case, the immediate-leap form).

## Milestones

| ID | Deliverable | Entry | Exit criteria |
| --- | --- | --- | --- |
| **M0** | Contract + harness: top `processor` with reset/vector-table boot, `retire`/`halted`/`booted`, bus port, stub retire (every fetched word retires unchanged), co-sim target, @ref rtl_parity, second Verilator target | — | Reset image diffs clean; an erased program retires 32 noops in lockstep |
| **M1** | Core state, no memory: register file (16×32, 16×8), `Special`, `move`, `movei`, `copy`, `zero`, `halt` | M0 | Directed programs diff clean against `Cycle()` |
| **M2** | Bus port live: decoder dispatch and real fetch/execute, `swap`, `load`, `save` (all size/shift/off/special modes) | M1 | Load/save addressing modes match |
| **M3** | Control flow + stack: `leap` (relative **and** immediate form), `back`, `grow`, `fall`, `push`, `pull`, `call`, `trip`; RA semantics | M2 | PA/RA/stack regs match per step, including the `Grow`/`Fall` bounds cases |
| **M4** | `comp` + eval-gated conditions used by move/leap/grow/fall/call/trip | M3 | Eval register bytes and gating match |
| **M5** | Bitwise: `bitwise1` (shift/rotate/count/clz/not/complement/set/clear/toggle/reverse) and `bitwise2` (and/or/xor) + bitwise eval flags | M4 | Scratch + eval match |
| **M6** | Integer arithmetic: add/sub/mul/div/mod × {b,h,w} × {s,u} × {wrap,sat}; overflow/zero/positive; div-by-zero | M5 | `operations::*` results + flags match |
| **M7** | `negate` (with `.sat`) and `extend` (sext/zext.b/h) | M6 | Scratch + flags match |
| **M8** | Exceptions + privilege: full `GetHandler()` ladder, exception mask, mode, privilege faults | M7 | *C++ first*, then RTL; fault redirects and mask/mode match |
| **M9** | Interrupts: ticker/deferred/external/unmaskable/nested, external handler table | M8 | *C++ first*, then RTL; injected events redirect identically |
| **M10** | Caches: I-cache 256×1, D-cache 1024×16 | M9 | *C++ first* (make `Cycle()` actually use them), then RTL; statistics and contents match |
| **M11** | Floating point: floor/ceil/abs/neg/fractional/convert, fadd/fsub/fmul/fdiv, precision flags | M10 | FP strategy chosen; bit-exact vectors pass |
| **M12** | Pipeline micro-architecture: split into the Fetch/Decode/Execute/Writeback stages of @ref pipeline | M1–M11 green | Same architectural results, retired one instruction per pipeline drain |

M0–M7 reproduce `Processor::Cycle()` exactly. M8–M10 reach the documented design, with the C++
model extended first. M12 is the microarchitecture.

### M1 notes

* **One FSM, one edge.** `S_FETCH` drives the bus and latches the word; `S_COMMIT` performs the
  register writes, the program-address advance, the counter and `retire` on a single edge. The
  cycle count per instruction is an implementation detail — the harness only compares state at the
  retire boundary — but keeping fetch and execute separate is what lets M2 add bus-wait states
  without reshaping the FSM.
* **Fields come from the union.** The `instruction_t` view (`view.copy.dst`, `view.move.shift`, …)
  is used rather than shifting the raw word by hand, so the field positions stay owned by
  `rtl/isa.sv` and are already covered by `gtest_layout`.
* **The implemented set is data, not control flow.** `is_implemented()` lists the opcodes this
  milestone executes; the `S_COMMIT` case and this function grow together, and `default` sets
  `unsupported`.
* **Tests read the generated layout.** `gtest_layout` extracts fields by name from the generated
  `isa::layout` tables instead of restating bit positions, so a format change is caught there rather
  than in every test. `LayoutTest.MoveShiftOperandEncodesEveryShiftAmount` is the regression guard for
  P-023, which had been silently encoding large `move` shifts as zero.
* **Reset-image stimulus.** The M1 tests do not need a preamble to seed registers: the reset image
  (scratch all ones, evaluations 0xFF) is already pinned by `ResetStateMatchesTheFastModel`, and a
  cleared source register is a distinguishable outcome.

## Build and target changes

`projects/isa/CMakeLists.txt` builds two Verilator targets, both gated behind `BUILD_VERILOG`:

```cmake
# decoder-only target, backs gtest_isa_rtl
verilate(isa_rtl_sim PREFIX Vdecoder TOP_MODULE decoder
    SOURCES rtl/isa.sv rtl/decoder.sv)

# processor + co-simulation target, backs gtest_isa_rtl_processor
add_library(isa_rtl_processor_sim)
verilate(isa_rtl_processor_sim
    PREFIX Vprocessor
    TOP_MODULE processor
    SOURCES rtl/isa.sv rtl/decoder.sv rtl/processor.sv)

add_executable(gtest_isa_rtl_processor test/gtest_rtl_processor.cpp)
target_link_libraries(gtest_isa_rtl_processor
    PRIVATE isa_rtl_processor_sim hobbies-isa GTest::gtest GTest::gtest_main Threads::Threads)
```

## Risks

* **No oracle for the documented-only features.** Mitigated by implementing M8–M10 C++ first.
* **Floating point bit-exactness (M11).** Host `float` + `fenv` cannot be mirrored in RTL directly;
  the eventual FP unit may require independent reference vectors, not the C++ model.
* **Wide vs. narrow stores.** Sub-word `save` uses byte enables (`mem_wstrb`); the C++ model does
  read-modify-write via `Poke`. Both must agree on endianness and byte placement.
* **Stack bound conditions.** P-001 (the inverted `Grow` predicate), P-002 (the split bounds idioms)
  and P-012 (`Push`/`Pull` ordering) are fixed in C++; see @ref rtl_parity. `Stack::ContainsPointer`
  and `Stack::ContainsBlock` are the reference predicates, and `Push`/`Pull` both walk ascending. The
  RTL must reproduce these in M3.
* **Pipeline hazard work (M12)** can reintroduce divergences; keep the M1–M11 diff harness as the
  regression gate.
