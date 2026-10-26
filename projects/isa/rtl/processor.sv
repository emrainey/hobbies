// M1 processor top: reset/vector-table boot, fetch, the register-file
// instructions (move, movei, copy, zero, halt) and the bitwise operations, which
// run through the combinational bitwise_logic_unit.
//
// Fetch, decode (operand read) and commit are separate states. Scratch and
// evaluation state live in the register_file/evaluation_file submodules.
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

    // Special registers, exposed for the lockstep harness (scratch and evaluation
    // live in `u_scratch`/`u_evaluation` and are read hierarchically)
    output word_t      special[SPECIAL_COUNT],

    // Commit handshake
    output logic retire,
    output logic halted,
    output logic booted,

    // Set at the commit edge when the instruction is not implemented yet
    output logic unsupported
);

    // The vector table location (DEFAULT_VECTOR_TABLE_ADDRESS) comes from the isa package.

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

    // Vector-table word offsets (BOOT_*) come from the isa package.

    typedef enum logic [2:0] {
        S_BOOT,
        S_FETCH,
        S_DECODE,
        S_COMMIT,
        S_VECTOR,
        S_HALTED
    } state_e;

    state_e     state;
    logic       cond_pass;
    logic [2:0] boot_index;
    logic [3:0] vector_index;

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

    // What `copy Sd, Ss : Ee` writes: ComparisonEvaluation{false, true, false, false}
    // (isa::ComparisonEvaluation, include/isa/types.hpp).
    function automatic byte_t comparison_equal();
        evaluation_t e;
        e = '0;
        e.comparison.type_ = EVAL_TYPE_COMPARISON;
        e.comparison.equal = 1'b1;
        return e.raw;
    endfunction
    localparam byte_t EVAL_COMPARISON_EQUAL = comparison_equal();

    // The opcodes this milestone executes. Extending this function and the S_COMMIT
    // case below is what makes a milestone land.
    function automatic logic is_implemented(op_e code);
        case (code)
            OP_NONE, OP_COPY, OP_MOVE, OP_MOVE_IMMEDIATE, OP_ZERO, OP_HALT: return 1'b1;
            OP_COMPLEMENT, OP_RSH, OP_ASR, OP_LSH, OP_ROTATE, OP_COUNT1S, OP_COUNT_L0S,
            OP_NOT, OP_SET_BIT, OP_CLEAR_BIT, OP_TOGGLE_BIT, OP_REVERSE: return 1'b1;
            OP_AND, OP_OR, OP_XOR: return 1'b1;
            default: return 1'b0;
        endcase
    endfunction

    // A defined opcode that this milestone can execute.
    logic supported_op;
    assign supported_op = decoded_valid && is_implemented(decoded_op);

    always_comb begin
        mem_req   = (state == S_BOOT) || (state == S_FETCH) || (state == S_VECTOR);
        mem_write = 1'b0;
        mem_wdata = '0;
        mem_wstrb = '0;

        if (state == S_BOOT) begin
            mem_addr = DEFAULT_VECTOR_TABLE_ADDRESS + (address_t'(boot_index) << 2);
        end else if (state == S_FETCH) begin
            mem_addr = special[SPECIAL_PROGRAM_ADDRESS];
        end else if (state == S_VECTOR) begin
            mem_addr = special[SPECIAL_VECTOR_TABLE_ADDRESS] + (address_t'(vector_index) << 2);
        end else begin
            mem_addr = '0;
        end
    end


    // The register files. Reads are combinational from the instruction; writes land
    // at the S_COMMIT edge. A same-index double write resolves to port 0.
    scratch_control_t    s_read_control, s_write_control;
    scratch_index_t      s_read_index[0:1], s_write_index[0:1];
    word_t               s_read_data[0:1], s_write_data[0:1];
    logic [SCRATCH_COUNT-1:0] s_clear;
    evaluation_control_t e_read_control, e_write_control;
    evaluation_index_t   e_read_index[0:1], e_write_index[0:1];
    byte_t               e_read_data[0:1], e_write_data[0:1];
    logic [EVAL_COUNT-1:0]    e_clear;

    register_file u_scratch (
        .clock(clk), .reset(reset), .clear_mask(s_clear),
        .write_control(s_write_control), .write_index(s_write_index), .write_data(s_write_data),
        .read_control(s_read_control), .read_index(s_read_index), .read_data(s_read_data)
    );

    evaluation_file u_evaluation (
        .clock(clk), .reset(reset), .clear_mask(e_clear),
        .write_control(e_write_control), .write_index(e_write_index), .write_data(e_write_data),
        .read_control(e_read_control), .read_index(e_read_index), .read_data(e_read_data)
    );

    // Bitwise operations: the one-source forms (bitwise1) carry a shift or bit index,
    // the two-source forms (binary) read a second scratch register.
    logic is_bitwise2, is_bitwise1;
    always_comb begin
        is_bitwise2 = (decoded_op == OP_AND) || (decoded_op == OP_OR) || (decoded_op == OP_XOR);
        case (decoded_op)
            OP_COMPLEMENT, OP_RSH, OP_ASR, OP_LSH, OP_ROTATE, OP_COUNT1S, OP_COUNT_L0S,
            OP_NOT, OP_SET_BIT, OP_CLEAR_BIT, OP_TOGGLE_BIT, OP_REVERSE: is_bitwise1 = 1'b1;
            default: is_bitwise1 = 1'b0;
        endcase
    end

    word_t blu_a, blu_b, blu_result;
    byte_t blu_evaluation;
    /* verilator lint_off UNUSEDSIGNAL */
    logic  blu_valid;  // the same verdict as blu_fault, which is decoded without the data path
    /* verilator lint_on UNUSEDSIGNAL */

    assign blu_a = s_read_data[0];
    assign blu_b = is_bitwise2 ? s_read_data[1] : word_t'(view.bitwise1.shift);

    bitwise_logic_unit u_blu (
        .opcode(decoded_op), .a(blu_a), .b(blu_b),
        .result(blu_result), .evaluation(blu_evaluation), .valid(blu_valid)
    );

    // An invalid bitwise operation (REVERSE wider than a word) is an instruction fault,
    // whose handler is M8; until then it is reported like an unexecutable opcode and
    // writes nothing. It is decoded from the instruction alone, because the unit's own
    // `valid` sits on the operand data path and would close a loop through the
    // register file's write-before-read forwarding.
    logic blu_fault;
    assign blu_fault = (decoded_op == OP_REVERSE) && (view.bitwise1.shift > 2);

    // An instruction fault: an undefined opcode or an invalid operation. Nothing is
    // written, and the exception below redirects the program address like Cycle().
    logic instruction_fault;
    assign instruction_fault = (state == S_COMMIT) && !fetch_fault && (!decoded_valid || blu_fault);

    // The exception state after this commit; any set bit redirects through the table.
    word_t exception_next;
    always_comb begin
        exception_next = special[SPECIAL_EXCEPTION];
        if (instruction_fault) begin
            exception_next[EXCEPTION_INSTRUCTION_FAULT] = 1'b1;
        end
    end

    logic commit_valid;
    assign commit_valid = (state == S_COMMIT) && !fetch_fault && supported_op && !blu_fault;

    always_comb begin
        s_read_control  = SCRATCH_CONTROL_NONE;
        s_write_control = SCRATCH_CONTROL_NONE;
        e_read_control  = EVALUATION_CONTROL_NONE;
        e_write_control = EVALUATION_CONTROL_NONE;
        s_clear         = '0;
        e_clear         = '0;
        for (int i = 0; i < 2; i++) begin
            s_read_index[i]  = '0;
            s_write_index[i] = '0;
            s_write_data[i]  = '0;
            e_read_index[i]  = '0;
            e_write_index[i] = '0;
            e_write_data[i]  = '0;
        end

        // Operand read for the condition (S_DECODE).
        if (state == S_DECODE && decoded_op == OP_MOVE && view.move.cond) begin
            e_read_control  = EVALUATION_CONTROL_BOTH;
            e_read_index[0] = view.move.eval;
            e_read_index[1] = view.move.mask;
        end

        // Bitwise operands are read whenever the operation is about to commit; only the
        // write is gated by the operation being valid.
        if (state == S_COMMIT && !fetch_fault && supported_op) begin
            if (is_bitwise1) begin
                s_read_control  = SCRATCH_CONTROL_FIRST;
                s_read_index[0] = view.bitwise1.src;
            end
            if (is_bitwise2) begin
                s_read_control  = SCRATCH_CONTROL_BOTH;
                s_read_index[0] = view.binary.src1;
                s_read_index[1] = view.binary.src2;
            end
        end

        if (commit_valid) begin
            if (is_bitwise1 || is_bitwise2) begin
                s_write_control  = SCRATCH_CONTROL_FIRST;
                s_write_index[0] = is_bitwise2 ? view.binary.dst : view.bitwise1.dst;
                s_write_data[0]  = blu_result;
                if (is_bitwise2 ? view.binary.cond : view.bitwise1.cond) begin
                    e_write_control  = EVALUATION_CONTROL_FIRST;
                    e_write_index[0] = is_bitwise2 ? view.binary.eval : view.bitwise1.eval;
                    e_write_data[0]  = blu_evaluation;
                end
            end

            case (decoded_op)
                OP_COPY: begin
                    s_read_control   = SCRATCH_CONTROL_FIRST;
                    s_read_index[0]  = view.copy.src;
                    s_write_control  = SCRATCH_CONTROL_FIRST;
                    s_write_index[0] = view.copy.dst;
                    s_write_data[0]  = s_read_data[0];
                    if (view.copy.cond) begin
                        e_write_control  = EVALUATION_CONTROL_FIRST;
                        e_write_index[0] = view.copy.eval;
                        e_write_data[0]  = EVAL_COMPARISON_EQUAL;
                    end
                end
                OP_MOVE: begin
                    if (cond_pass && view.move.type_ == 1'b0) begin
                        // Shift the word, then clear the source. When source and
                        // destination match, port 0 (the value) wins.
                        s_read_control   = SCRATCH_CONTROL_FIRST;
                        s_read_index[0]  = view.move.src;
                        s_write_control  = SCRATCH_CONTROL_BOTH;
                        s_write_index[0] = view.move.dst;
                        s_write_data[0]  = view.move.dir
                                               ? word_t'($signed(s_read_data[0]) >>> view.move.shift)
                                               : (s_read_data[0] << view.move.shift);
                        s_write_index[1] = view.move.src;
                    end
                    if (cond_pass && view.move.type_ == 1'b1) begin
                        // The shift applies to the 8-bit value (cpu.cpp:336-347).
                        e_read_control   = EVALUATION_CONTROL_FIRST;
                        e_read_index[0]  = view.move.src;
                        e_write_control  = EVALUATION_CONTROL_BOTH;
                        e_write_index[0] = view.move.dst;
                        e_write_data[0]  = view.move.dir
                                               ? (e_read_data[0] >> view.move.shift)
                                               : (e_read_data[0] << view.move.shift);
                        e_write_index[1] = view.move.src;
                    end
                end
                OP_MOVE_IMMEDIATE: begin
                    if (view.move_immediate.eval) begin
                        e_write_control  = EVALUATION_CONTROL_FIRST;
                        e_write_index[0] = view.move_immediate.dst;
                        e_write_data[0]  = view.move_immediate.imm[7:0];
                    end else begin
                        // Partial-word write: read-modify-write on the destination.
                        s_read_control   = SCRATCH_CONTROL_FIRST;
                        s_read_index[0]  = view.move_immediate.dst;
                        s_write_control  = SCRATCH_CONTROL_FIRST;
                        s_write_index[0] = view.move_immediate.dst;
                        s_write_data[0]  = view.move_immediate.upper
                                               ? {view.move_immediate.imm, s_read_data[0][15:0]}
                                               : {s_read_data[0][31:16], view.move_immediate.imm};
                    end
                end
                OP_ZERO: begin
                    if (view.zero.type_ == REG_SCRATCH) begin
                        s_clear = view.zero.mask;
                    end else begin
                        e_clear = view.zero.mask;
                    end
                end
                default: ;
            endcase
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
            vector_index <= '0;

            cond_pass   <= 1'b0;
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
                    state       <= S_DECODE;
                end

                S_DECODE: begin
                    // The operand read stage: the evaluation file is read for the
                    // condition and the verdict is held for S_COMMIT.
                    cond_pass <= !view.move.cond || ((e_read_data[0] & e_read_data[1]) != 8'd0);
                    state     <= S_COMMIT;
                end

                S_COMMIT: begin
                    state <= S_FETCH;

                    if (fetch_fault) begin
                        // No instruction: Cycle() substitutes a noop and advances.
                    end else if (!decoded_valid) begin
                        // Undefined opcode: an instruction fault, handled below.
                    end else if (supported_op) begin
                        // Register writes are driven combinationally into the files below.
                        if (decoded_op == OP_HALT) begin
                            halted <= 1'b1;
                            state  <= S_HALTED;
                        end
                    end else begin
                        unsupported <= 1'b1;
                    end

                    special[SPECIAL_PERFORMANCE_COUNTER] <= special[SPECIAL_PERFORMANCE_COUNTER] + 32'd1;
                    special[SPECIAL_EXCEPTION]           <= exception_next;

                    if (exception_next != '0) begin
                        // Any pending exception sends the next fetch through the vector table.
                        vector_index <= VECTOR_INSTRUCTION_FAULT_HANDLER;
                        state        <= S_VECTOR;
                    end else begin
                        special[SPECIAL_PROGRAM_ADDRESS] <= special[SPECIAL_PROGRAM_ADDRESS] + 32'd4;
                        retire                           <= 1'b1;
                    end
                end

                S_VECTOR: begin
                    // GetHandler(): the handler, or the reset handler when that read faults.
                    if (!mem_fault) begin
                        special[SPECIAL_PROGRAM_ADDRESS] <= mem_rdata;
                        retire                           <= 1'b1;
                        state                            <= halted ? S_HALTED : S_FETCH;
                    end else if (vector_index != 4'(BOOT_RESET_HANDLER)) begin
                        vector_index <= 4'(BOOT_RESET_HANDLER);
                    end else begin
                        // Cycle() throws here: there is no handler to run.
                        halted <= 1'b1;
                        state  <= S_HALTED;
                    end
                end

                S_HALTED: begin
                    // Cycle() returns immediately once halted; hold until reset.
                end

                default: state <= S_FETCH;
            endcase
        end
    end

endmodule
