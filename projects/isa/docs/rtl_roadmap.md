# RTL Roadmap

How to bring the SystemVerilog model (`rtl/`) up to the same point as the C++ fast model
(`source/`, `include/isa/`), and then beyond it to the full documented design.

See @ref rtl_parity for the running list of model-to-model divergences.

## Scope and decisions

The plan below reflects these agreed decisions:

* **Microarchitecture:** single-cycle / multi-cycle *functional parity first*. The 4-stage pipeline in
  @ref pipeline is a later micro-architecture step (M12), layered over a proven golden harness.
* **Oracle:** the C++ model. When the two disagree we decide case-by-case which is wrong and record
  the call in @ref rtl_parity.
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
| SV (`isa_rtl_sim`) | Generated `isa` package (`rtl/isa.sv`): params, enums, opcodes, `is_defined_op`, instruction format structs and `instruction_t` union. Combinational `decoder` (`rtl/decoder.sv`) | Everything else: register file, special registers, ALU, control flow, stack, buses, exceptions, caches, top-level `processor` |

The single source of truth already exists: `docs/instructions.json` renders `rtl/isa.sv`,
`include/isa/operator.hpp` and `docs/instruction_formats.md` via `tools/generate_isa.py`, with a
staleness test (`isa_generated_sources_are_current`). Keep it that way — every new field or opcode
goes through the generator, never hand-edited in `isa.sv`.

## Cycle contract

`Processor::Cycle()` retires exactly one instruction. The RTL must expose the same boundary so the
two can be stepped in lockstep:

* One **retired instruction** produces exactly one cycle of `retire` high.
* The FSM may take several clock cycles per instruction (fetch, bus wait, execute); `retire` marks
  the commit edge.
* `halted` is exposed so the harness can stop, mirroring `Processor::IsHalted()`.
* A `mem_*` bus port (below) is held stable across the bus-wait cycles.

The testbench loop is then: drive the program into shared memory, reset both models, then for each
step clock the RTL until `retire` (or `halted`), call `Processor::Cycle()` once, and diff state. Any
mismatch is a @ref rtl_parity row, not an immediate failure.

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
| `mem_wstrb` | out | 4 | Byte enables for sub-word writes |
| `mem_rdata` | in | 32 | Read data, valid the cycle after `mem_req` |
| `mem_fault` | in | 1 | Unassigned/unaligned/faulting access |

Read data returns combinationally (SRAM-like) so a fetch closes in the following cycle; the
testbench routes `mem_addr` through `isa::memory::Map` and the attached `isa::Memory` objects and
raises `mem_fault` for unassigned addresses, matching `Bus::State::Fault`.

## Co-simulation harness

New test target that instantiates **both** models and diffs them:

```cpp
isa::Processor oracle;                  // fast model
Vprocessor dut;                          // verilated RTL
// load the same isa::program into both, reset both
for (step : steps) {
    oracle.Cycle();
    while (!dut.retire && !dut.halted) tick(dut);   // clock until commit
    assert_equal(oracle.ViewScratch(),      dut.scratch);
    assert_equal(oracle.ViewEvaluations(),  dut.evaluation);
    assert_equal(oracle.ViewSpecial(),      dut.special);
}
```

State is read straight from the Verilated model's public members (declare scratch/eval/special as
top-level signals), so no debug ports are needed for M1–M7. A snapshot/debug port can be added later
if the harness needs to cross a process boundary.

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
| **M0** | Contract + harness: top `processor`, `retire`/`halted`, bus port, co-sim test skeleton, @ref rtl_parity, second Verilator target | — | Empty program runs on both models and state diffs clean |
| **M1** | Core state, no memory: register file (16×32, 16×8), `Special`, `move`, `movei`, `copy`, `zero`, `halt` | M0 | Directed programs diff clean against `Cycle()` |
| **M2** | Bus port live: fetch, `Reset()` vector-table boot, `swap`, `load`, `save` (all size/shift/off/special modes) | M1 | Load/save addressing modes and `Reset()` match |
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

## Build and target changes

`projects/isa/CMakeLists.txt` currently builds one Verilator target:

```cmake
verilate(isa_rtl_sim PREFIX Vdecoder TOP_MODULE decoder
    SOURCES rtl/isa.sv rtl/decoder.sv)
```

Keep that (it backs `gtest_isa_rtl`) and add a second target for the processor plus a co-sim test:

```cmake
add_library(isa_rtl_processor_sim)
verilate(isa_rtl_processor_sim
    PREFIX Vprocessor
    TOP_MODULE processor
    SOURCES rtl/isa.sv rtl/decoder.sv rtl/processor.sv rtl/*.sv)

add_executable(gtest_isa_rtl_processor test/gtest_rtl_processor.cpp)
target_link_libraries(gtest_isa_rtl_processor
    PRIVATE isa_rtl_processor_sim hobbies-isa GTest::gtest GTest::gtest_main Threads::Threads)
```

Gate both behind the existing `BUILD_VERILOG` option.

## Risks

* **No oracle for the documented-only features.** Mitigated by implementing M8–M10 C++ first.
* **Floating point bit-exactness (M11).** Host `float` + `fenv` cannot be mirrored in RTL directly;
  the eventual FP unit may require independent reference vectors, not the C++ model.
* **Wide vs. narrow stores.** Sub-word `save` uses byte enables (`mem_wstrb`); the C++ model does
  read-modify-write via `Poke`. Both must agree on endianness and byte placement.
* **`Grow`/`Fall` bound conditions.** P-001 (the inverted `Grow` predicate) is fixed in C++; see
  @ref rtl_parity. P-002 (whether to unify the `Grow`/`Fall` and `Push`/`Pull` bound idioms) is still
  open. The RTL must reproduce the corrected `Grow` predicate in M3.
* **Pipeline hazard work (M12)** can reintroduce divergences; keep the M1–M11 diff harness as the
  regression gate.
