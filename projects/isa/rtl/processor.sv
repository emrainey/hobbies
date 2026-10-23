// M1 processor top: reset/vector-table boot, fetch, and the register-file
// instructions (move, movei, copy, zero, halt).
//
// Fetch and execute are separate states. `mem_req`/`mem_addr` are driven from
// S_FETCH and the returned word is latched at the edge into S_COMMIT, where the
// instruction commits: the register writes, the program-address advance, the
// performance counter and the `retire` pulse all land on that one edge. Splitting
// them now means M2 can insert bus-wait states for load/save without reshaping the
// FSM -- only `is_implemented` and the S_COMMIT case grow.
//
// `unsupported` pulses at the commit edge when the fetched instruction cannot be
// executed: a defined opcode this milestone has not reached, or an undefined one.
// M0 retired those silently, which makes an accidentally out-of-scope instruction
// look like a state mismatch; the harness asserts this stays low. Successive
// milestones shrink the set until only undefined opcodes remain, which are an
// `instruction_fault` rather than "not implemented" (M8).
//
// An instruction with no implementation behind it still retires and advances the
// program address, so the harness keeps making progress; its architectural effect
// is simply not modelled yet.
//
// Contract recap (docs/rtl_roadmap.md):
//   * one `retire` pulse per consumed instruction, including a faulting fetch,
//   * the unified word-bus handshake,
//   * reset boot from the vector table, matching Processor::Reset(),
//   * architectural state readable by the lockstep harness.
//
// Bus timing: `mem_req` starts an access in the current cycle, `mem_addr` is stable
// while it is high, and `mem_rdata`/`mem_fault` are combinational responses to
// `mem_addr` that are sampled on the next rising edge. This matches Bus::Peek.
module processor
    import isa::*;
(
    input  logic clk,
    input  logic reset,

    // Unified word bus
    output logic       mem_req,
    output logic       mem_write,
    output address_t   mem_addr,
    output word_t      mem_wdata,
    output logic [3:0] mem_wstrb,
    input  word_t      mem_rdata,
    input  logic       mem_fault,

    // Architectural state, exposed for the lockstep harness
    output word_t      scratch[SCRATCH_COUNT],
    output byte_t      evaluation[EVAL_COUNT],
    output word_t      special[SPECIAL_COUNT],

    // Commit handshake
    output logic retire,
    output logic halted,
    output logic booted,

    // Set at the commit edge when the instruction is not implemented yet
    output logic unsupported
);

    // Mirrors isa::DefaultVectorTableAddress (include/isa/map.hpp, Map[1].start).
    // Unsized so the width is taken from `address_t`; a parameter cannot be used as
    // the size of a based literal (`ADDRESS_SIZE'h...` is a syntax error).
    localparam address_t DEFAULT_VECTOR_TABLE_ADDRESS = 'h0010_0000;

    // Special-register word indices (SPECIAL_*) come from the generated isa package,
    // which the C++ model binds to its own field offsets with static_asserts.

    // Mode register layout. Packed structs list the most significant field first,
    // the reverse of the C++ bitfield order (isa::Mode allocates from bit 0).
    typedef struct packed {
        logic [WORD_SIZE-3-1:0] reserved;
        logic                   nonprivileged;
        logic                   privileged;
        logic                   exception;
    } mode_t;

    // Mode at reset: Exception=0, Privileged=0, NonPrivileged=1 (isa::Mode's default).
    localparam mode_t MODE_RESET = '{nonprivileged: 1'b1, default: '0};

    // Vector-table word offsets, matching isa::VectorTable.
    localparam logic [2:0] BOOT_STACK_INITIAL = 3'd0;
    localparam logic [2:0] BOOT_STACK_BOUNDARY = 3'd1;
    localparam logic [2:0] BOOT_EXC_STACK_INITIAL = 3'd2;
    localparam logic [2:0] BOOT_EXC_STACK_BOUNDARY = 3'd3;
    localparam logic [2:0] BOOT_RESET_HANDLER = 3'd4;

    typedef enum logic [1:0] {
        S_BOOT,
        S_FETCH,
        S_COMMIT,
        S_HALTED
    } state_e;

    state_e     state;
    logic [2:0] boot_index;

    assign booted = (state != S_BOOT);

    // The instruction latched at the end of S_FETCH, and whether fetching it faulted.
    // A failed fetch is replayed as a noop, matching the Cycle() fetch fallback
    // (source/cpu.cpp:296-299).
    word_t instruction;
    logic  fetch_fault;

    // The instruction viewed as any of the generated formats, and the effective
    // operation from the shared combinational decoder.
    instruction_t view;
    op_e          decoded_op;
    logic         decoded_valid;

    always_comb view = instruction;

    decoder u_decoder (
        .instruction(instruction),
        .op(decoded_op),
        .valid(decoded_valid)
    );

    // What `copy Sd, Ss : Ee` writes: ComparisonEvaluation{false, true, false, false},
    // i.e. type = Comparison in bits 1:0 and equal in bit 3 (isa::ComparisonEvaluation,
    // include/isa/types.hpp).
    localparam byte_t EVAL_COMPARISON_EQUAL = 8'h09;

    // The opcodes this milestone executes. Extending this function and the S_COMMIT
    // case below is what makes a milestone land.
    function automatic logic is_implemented(op_e code);
        case (code)
            OP_NONE, OP_COPY, OP_MOVE, OP_MOVE_IMMEDIATE, OP_ZERO, OP_HALT: return 1'b1;
            default: return 1'b0;
        endcase
    endfunction

    // A defined opcode that this milestone can execute.
    logic supported_op;
    assign supported_op = decoded_valid && is_implemented(decoded_op);

    always_comb begin
        mem_req   = (state == S_BOOT) || (state == S_FETCH);
        mem_write = 1'b0;
        mem_wdata = '0;
        mem_wstrb = '0;

        if (state == S_BOOT) begin
            mem_addr = DEFAULT_VECTOR_TABLE_ADDRESS + (address_t'(boot_index) << 2);
        end else if (state == S_FETCH) begin
            mem_addr = special[SPECIAL_PROGRAM_ADDRESS];
        end else begin
            mem_addr = '0;
        end
    end

    always_ff @(posedge clk) begin
        retire      <= 1'b0;
        unsupported <= 1'b0;

        if (reset) begin
            state       <= S_BOOT;
            boot_index  <= '0;
            halted      <= 1'b0;
            instruction <= ERASED_WORD;
            fetch_fault <= 1'b0;

            for (int i = 0; i < SCRATCH_COUNT; i++) begin
                scratch[i] <= '0;
            end
            for (int i = 0; i < EVAL_COUNT; i++) begin
                evaluation[i] <= '0;
            end
            for (int i = 0; i < SPECIAL_COUNT; i++) begin
                special[i] <= '0;
            end
            special[SPECIAL_VECTOR_TABLE_ADDRESS] <= DEFAULT_VECTOR_TABLE_ADDRESS;
            special[SPECIAL_MODE] <= MODE_RESET;
        end else begin
            case (state)
                S_BOOT: begin
                    // A faulting read leaves the field at its reset value, mirroring Reset().
                    if (!mem_fault) begin
                        case (boot_index)
                            BOOT_STACK_INITIAL: begin
                                special[SPECIAL_STACK_LIMIT]   <= mem_rdata;
                                special[SPECIAL_STACK_CURRENT] <= mem_rdata;
                            end
                            BOOT_STACK_BOUNDARY: begin
                                special[SPECIAL_STACK_BASE] <= mem_rdata;
                            end
                            BOOT_EXC_STACK_INITIAL: begin
                                special[SPECIAL_EXCEPTION_STACK_LIMIT]   <= mem_rdata;
                                special[SPECIAL_EXCEPTION_STACK_CURRENT] <= mem_rdata;
                            end
                            BOOT_EXC_STACK_BOUNDARY: begin
                                special[SPECIAL_EXCEPTION_STACK_BASE] <= mem_rdata;
                            end
                            default: begin
                                special[SPECIAL_PROGRAM_ADDRESS] <= mem_rdata;
                                special[SPECIAL_RETURN_ADDRESS] <= mem_rdata;
                            end
                        endcase
                    end

                    if (boot_index == BOOT_RESET_HANDLER) begin
                        boot_index <= '0;
                        state      <= S_FETCH;
                    end else begin
                        boot_index <= boot_index + 1'b1;
                    end
                end

                S_FETCH: begin
                    // The bus response is combinational, so the word is present now.
                    instruction <= mem_rdata;
                    fetch_fault <= mem_fault;
                    state       <= S_COMMIT;
                end

                S_COMMIT: begin
                    state <= S_FETCH;

                    if (fetch_fault) begin
                        // No instruction: Cycle() substitutes a noop and advances.
                    end else if (supported_op) begin
                        case (decoded_op)
                            OP_NONE: ;  // retire and advance only
                            OP_HALT: begin
                                halted <= 1'b1;
                                state  <= S_HALTED;
                            end
                            OP_COPY: begin
                                scratch[view.copy.dst] <= scratch[view.copy.src];
                                if (view.copy.cond) begin
                                    evaluation[view.copy.eval] <= EVAL_COMPARISON_EQUAL;
                                end
                            end
                            OP_MOVE: begin
                                logic allowed;
                                allowed = 1'b1;
                                if (view.move.cond) begin
                                    allowed = (evaluation[view.move.eval] & evaluation[view.move.mask]) != 8'd0;
                                end
                                if (allowed && (view.move.type_ == 1'b0)) begin
                                    // Scratch to scratch: shift the word, then clear the
                                    // source unless it is the destination (cpu.cpp:324-335).
                                    word_t value;
                                    value = view.move.dir
                                                ? word_t'($signed(scratch[view.move.src]) >>> view.move.shift)
                                                : (scratch[view.move.src] << view.move.shift);
                                    scratch[view.move.dst] <= value;
                                    if (view.move.dst != view.move.src) begin
                                        scratch[view.move.src] <= '0;
                                    end
                                end
                                if (allowed && (view.move.type_ == 1'b1)) begin
                                    // Evaluation to evaluation: the shift applies to the 8-bit
                                    // value, so a left shift keeps only the low byte
                                    // (cpu.cpp:336-347).
                                    byte_t value;
                                    value = view.move.dir
                                                ? (evaluation[view.move.src] >> view.move.shift)
                                                : (evaluation[view.move.src] << view.move.shift);
                                    evaluation[view.move.dst] <= value;
                                    if (view.move.dst != view.move.src) begin
                                        evaluation[view.move.src] <= '0;
                                    end
                                end
                            end
                            OP_MOVE_IMMEDIATE: begin
                                if (view.move_immediate.eval) begin
                                    evaluation[view.move_immediate.dst] <= view.move_immediate.imm[7:0];
                                end else if (view.move_immediate.upper) begin
                                    scratch[view.move_immediate.dst][31:16] <= view.move_immediate.imm;
                                end else begin
                                    scratch[view.move_immediate.dst][15:0] <= view.move_immediate.imm;
                                end
                            end
                            OP_ZERO: begin
                                if (view.zero.type_ == REG_SCRATCH) begin
                                    for (int i = 0; i < SCRATCH_COUNT; i++) begin
                                        if (view.zero.mask[i]) begin
                                            scratch[i] <= '0;
                                        end
                                    end
                                end else begin
                                    for (int i = 0; i < EVAL_COUNT; i++) begin
                                        if (view.zero.mask[i]) begin
                                            evaluation[i] <= '0;
                                        end
                                    end
                                end
                            end
                            default: ;  // unreachable: supported_op excludes it
                        endcase
                    end else begin
                        unsupported <= 1'b1;
                    end

                    special[SPECIAL_PROGRAM_ADDRESS]     <= special[SPECIAL_PROGRAM_ADDRESS] + 32'd4;
                    special[SPECIAL_PERFORMANCE_COUNTER] <= special[SPECIAL_PERFORMANCE_COUNTER] + 32'd1;
                    retire                               <= 1'b1;
                end

                S_HALTED: begin
                    // Cycle() returns immediately once halted; hold until reset.
                end

                default: state <= S_FETCH;
            endcase
        end
    end

endmodule
