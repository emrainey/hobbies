// Processor top: reset/vector-table boot, register-file and integer ALU
// instructions, bitwise operations, and variable-latency LOAD.
//
// Fetch, decode (operand read) and commit are separate states. Scratch and
// evaluation state live in the register_file/evaluation_file submodules.
//
// Fetch, decode and commit are serialized. Bus requests use a decoupled
// request/response handshake: request fields remain stable until accepted, and the
// processor waits for a response before advancing. Memory latency is unbounded by
// the RTL; a system may report a timeout as a fault response.
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
// Request and response handshakes occur on rising edges when valid and ready are
// both high. A response may arrive any number of cycles after its request.
/* verilator lint_off UNOPTFLAT */

// Non-architectural debug state.
typedef struct packed {
    logic [31:0] external_pending;
    logic [4:0]  last_exception_type;
} debug_t;

module processor
    import isa::*;
    import fp_pkg::*;
(
    input  logic clk,
    input  logic reset,

    // Unified word bus
    output logic       mem_req_valid,
    input  logic       mem_req_ready,
    output logic       mem_write,
    output address_t   mem_addr,
    output word_t      mem_wdata,
    output logic [3:0] mem_wstrb,
    input  logic       mem_rsp_valid,
    output logic       mem_rsp_ready,
    input  word_t      mem_rsp_data,
    input  logic       mem_rsp_fault,

    // External interrupt injection (one bit per source)
    input  logic [31:0] external_irq,

    // Special registers, exposed for the lockstep harness (scratch and evaluation
    // live in `u_scratch`/`u_evaluation` and are read hierarchically)
    output word_t      special[SPECIAL_COUNT],

    // Commit handshake
    output logic retire,
    output logic halted,
    output logic booted,

    // Debug (non-architectural)
    output debug_t dbg,

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

    typedef enum logic [4:0] {
        S_BOOT_REQUEST,
        S_BOOT_RESPONSE,
        S_FETCH_REQUEST,
        S_FETCH_RESPONSE,
        S_DECODE,
        S_COMMIT,
        S_LOAD_RESPONSE,
        S_SAVE_RESPONSE,
        S_SWAP_READ_RESPONSE,
        S_SWAP_WRITE_REQUEST,
        S_SWAP_WRITE_RESPONSE,
        S_BACK_RESPONSE,
        S_PUSH_PULL_REQUEST,
        S_PUSH_PULL_RESPONSE,
        S_CALL_RESPONSE,
        S_VECTOR_REQUEST,
        S_VECTOR_RESPONSE,
        S_HALTED
    } state_e;

    state_e     state;
    logic       cond_pass;
    logic       push_pull_is_push;
    logic [15:0] push_pull_mask;
    logic [4:0]  push_pull_index;
    logic [31:0] push_pull_address;
    logic [31:0] push_pull_end;
    logic       push_pull_bus_fault;
    logic [2:0] boot_index;
    logic [3:0] vector_index;

    assign booted = (state != S_BOOT_REQUEST) && (state != S_BOOT_RESPONSE);

    // The instruction latched after a fetch response, and whether that response faulted.
    // A failed fetch is replayed as a noop, matching the Cycle() fetch fallback
    // (source/cpu.cpp:296-299).
    word_t instruction;
    logic  fetch_fault;

    // Latched swap operands and read data, held across the two bus transactions.
    address_t swap_address_reg;
    word_t    swap_wdata_reg;
    word_t    swap_read_data_reg;

    // The instruction viewed as any of the generated formats, and the effective
    // operation from the shared combinational decoder.
    instruction_t view;
    op_e          decoded_op;
    logic         decoded_valid;

    always_comb view = instruction;

    function automatic int first_set_bit(input [15:0] mask);
        for (int i = 0; i < 16; i++)
            if (mask[i]) return i;
        return 16;
    endfunction

    function automatic int next_set_bit(input [15:0] mask, input int after);
        for (int i = after + 1; i < 16; i++)
            if (mask[i]) return i;
        return 16;
    endfunction

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

    // Manual constants (not part of the generated isa package).
    localparam int EXCEPTION_PRIVILEGE_FAULT = 11;
    localparam int EXCEPTION_EXTERNAL = 13;
    localparam logic [31:0] EXCEPTION_MASK_PRESERVED_BITS = 32'd3;
    localparam logic [3:0] VECTOR_BUS_FAULT_HANDLER = 4'd7;
    localparam logic [3:0] VECTOR_ARITHMETIC_FAULT_HANDLER = 4'd9;
    localparam logic [3:0] VECTOR_STACK_FAULT_HANDLER = 4'd10;
    localparam logic [3:0] VECTOR_TRIPPED_HANDLER = 4'd11;
    localparam logic [3:0] VECTOR_PRIVILEGE_HANDLER = 4'd15;
    localparam logic [4:0] VECTOR_EXTERNAL_HANDLER_TABLE = 5'd17;
    localparam logic [31:0] MODE_EXCEPTION = 32'd3;

    // Privileged instructions (mirrors isa::IsPrivilegedInstruction).
    function automatic logic is_privileged_op(op_e code);
        return (code == OP_HALT) || (code == OP_TRIP);
    endfunction

    // The opcodes this milestone executes. Extending this function and the S_COMMIT
    // case below is what makes a milestone land.
    function automatic logic is_implemented(op_e code);
        case (code)
            OP_NONE, OP_COPY, OP_MOVE, OP_MOVE_IMMEDIATE, OP_ZERO, OP_LOAD, OP_SAVE, OP_HALT,
            OP_LEAP, OP_BACK, OP_GROW, OP_FALL, OP_PUSH, OP_PULL, OP_CALL, OP_TRIP: return 1'b1;
            OP_FLOATING_ABS: return 1'b1;
            OP_FLOATING_NEGATE: return 1'b1;
            OP_FLOATING_FLOOR: return 1'b1;
            OP_FLOATING_CEIL: return 1'b1;
            OP_FLOATING_FRACTIONAL: return 1'b1;
            OP_FLOATING_CONVERT: return 1'b1;
            OP_FLOATING_ADDITION: return 1'b1;
            OP_FLOATING_SUBTRACTION: return 1'b1;
            OP_FLOATING_MULTIPLICATION: return 1'b1;
            OP_FLOATING_DIVISION: return 1'b1;
            OP_SWAP, OP_COMPLEMENT, OP_RSH, OP_ASR, OP_LSH, OP_ROTATE, OP_COUNT1S, OP_COUNT_L0S,
            OP_NOT, OP_SET_BIT, OP_CLEAR_BIT, OP_TOGGLE_BIT, OP_REVERSE: return 1'b1;
            OP_AND, OP_OR, OP_XOR, OP_COMPARE: return 1'b1;
            OP_ADDITION, OP_SUBTRACT, OP_MULTIPLY, OP_DIVIDE, OP_MODULO, OP_NEGATE, OP_EXTEND: return 1'b1;
            default: return 1'b0;
        endcase
    endfunction

    // A defined opcode that this milestone can execute.
    logic supported_op;
    assign supported_op = decoded_valid && is_implemented(decoded_op);

    logic memory_encoding_valid;
    always_comb begin
        memory_encoding_valid = 1'b1;
        if (decoded_op == OP_LOAD || decoded_op == OP_SAVE) begin
            case (view.memory.size)
                SIZE_BYTE: memory_encoding_valid = 1'b1;
                SIZE_HALF: memory_encoding_valid = view.memory.shift <= 1;
                SIZE_WORD: memory_encoding_valid = view.memory.shift == 0;
                default:   memory_encoding_valid = 1'b0;
            endcase
            if (view.memory.special && view.memory.base != 4 && view.memory.base != 0) begin
                memory_encoding_valid = 1'b0;
            end
        end
    end

    address_t load_address;
    always_comb begin
        if (view.memory.special) begin
            load_address = view.memory.base == 4 ? special[SPECIAL_STACK_CURRENT] : special[SPECIAL_PROGRAM_ADDRESS];
        end else begin
            load_address = s_read_data[0] + (view.memory.off ? address_t'(view.memory.imm) : '0);
        end
    end

    // Swap addresses the word in Sb; Sa is the value written.
    address_t swap_address;
    always_comb begin
        swap_address = s_read_data[0];
    end
    always_comb begin
        mem_req_valid = !reset && ( (state == S_BOOT_REQUEST) || (state == S_FETCH_REQUEST) ||
                                    (state == S_VECTOR_REQUEST) ||
                                    (state == S_COMMIT && commit_valid &&
                                     (decoded_op == OP_LOAD || decoded_op == OP_SAVE || decoded_op == OP_SWAP ||
                                      (decoded_op == OP_BACK && view.back.zero))) ||
                                    (state == S_SWAP_WRITE_REQUEST) ||
                                    (state == S_PUSH_PULL_REQUEST) ||
                                    (state == S_COMMIT && commit_valid && decoded_op == OP_CALL && cond_pass) );
        mem_rsp_ready = !reset && ((state == S_BOOT_RESPONSE) || (state == S_FETCH_RESPONSE) ||
                                   (state == S_VECTOR_RESPONSE) || state == S_LOAD_RESPONSE ||
                                   state == S_SAVE_RESPONSE || state == S_SWAP_READ_RESPONSE ||
                                   state == S_SWAP_WRITE_RESPONSE || state == S_BACK_RESPONSE ||
                                   state == S_PUSH_PULL_RESPONSE || state == S_CALL_RESPONSE);
        mem_write = 1'b0;
        mem_wdata = '0;
        mem_wstrb = '0;

        if (state == S_COMMIT && commit_valid && decoded_op == OP_SAVE) begin
            mem_write = 1'b1;
            case (view.memory.size)
                SIZE_BYTE: begin
                    mem_wstrb = 4'b0001 << view.memory.shift;
                    mem_wdata = word_t'(s_read_data[1][7:0]) << (view.memory.shift * 8);
                end
                SIZE_HALF: begin
                    mem_wstrb = 4'b0011 << (view.memory.shift * 2);
                    mem_wdata = word_t'(s_read_data[1][15:0]) << (view.memory.shift * 16);
                end
                default: begin
                    mem_wstrb = 4'b1111;
                    mem_wdata = s_read_data[1];
                end
            endcase
        end

        if (state == S_SWAP_WRITE_REQUEST) begin
            mem_write = 1'b1;
            mem_wstrb = 4'b1111;
            mem_wdata = swap_wdata_reg;
        end

        if (state == S_PUSH_PULL_REQUEST) begin
            if (push_pull_is_push) begin
                mem_write = 1'b1;
                mem_wstrb = 4'b1111;
                mem_wdata = s_read_data[0];
            end
        end

        if (state == S_BOOT_REQUEST) begin
            mem_addr = DEFAULT_VECTOR_TABLE_ADDRESS + (address_t'(boot_index) << 2);
        end else if (state == S_FETCH_REQUEST) begin
            mem_addr = special[SPECIAL_PROGRAM_ADDRESS];
        end else if (state == S_VECTOR_REQUEST) begin
            if (external_fetch_state == 2'd1)
                mem_addr = special[SPECIAL_VECTOR_TABLE_ADDRESS] + (address_t'(VECTOR_EXTERNAL_HANDLER_TABLE) << 2);
            else if (external_fetch_state == 2'd2)
                mem_addr = external_table_base + (address_t'(special[SPECIAL_EXTERNAL_SOURCE]) << 2);
            else
                mem_addr = special[SPECIAL_VECTOR_TABLE_ADDRESS] + (address_t'(vector_index) << 2);
        end else if (state == S_COMMIT && (decoded_op == OP_LOAD || decoded_op == OP_SAVE || decoded_op == OP_SWAP)) begin
            if (decoded_op == OP_SWAP) begin
                mem_addr = swap_address;
            end else begin
                mem_addr = load_address;
            end
        end else if (state == S_COMMIT && commit_valid && decoded_op == OP_BACK && view.back.zero) begin
            mem_addr = special[SPECIAL_VECTOR_TABLE_ADDRESS] + 32'd64;
        end else if (state == S_PUSH_PULL_REQUEST) begin
            mem_addr = push_pull_address;
        end else if (state == S_COMMIT && commit_valid && decoded_op == OP_CALL && cond_pass) begin
            mem_addr = special[SPECIAL_VECTOR_TABLE_ADDRESS] + 32'd48;
        end else if (state == S_SWAP_WRITE_REQUEST) begin
            mem_addr = swap_address_reg;
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

    // Stage 1 of M11: floating-point absolute value (combinational).
    logic [WORD_SIZE-1:0] fp_abs_result;
    fp_abs u_fp_abs (
        .in (s_read_data[0]),
        .out(fp_abs_result)
    );

    // Stage 2 of M11: floating-point negation (combinational).
    logic [WORD_SIZE-1:0] fp_neg_result;
    fp_neg u_fp_neg (
        .in (s_read_data[0]),
        .out(fp_neg_result)
    );

    // Stage 3 of M11: floor, ceil, fractional, convert (combinational).
    logic [WORD_SIZE-1:0] fp_floor_result;
    fp_floor u_fp_floor (
        .in (s_read_data[0]),
        .out(fp_floor_result)
    );

    logic [WORD_SIZE-1:0] fp_ceil_result;
    fp_ceil u_fp_ceil (
        .in (s_read_data[0]),
        .out(fp_ceil_result)
    );

    logic [WORD_SIZE-1:0] fp_fractional_result;
    fp_fractional u_fp_fractional (
        .in (s_read_data[0]),
        .out(fp_fractional_result)
    );

    logic [WORD_SIZE-1:0] fp_convert_result;
    fp_convert u_fp_convert (
        .in (s_read_data[0]),
        .dir(view.float_unary.dir),
        .out(fp_convert_result)
    );

    // Stage 4 of M11: floating-point addition (combinational).
    logic [WORD_SIZE-1:0] fp_add_result;
    logic fp_add_inexact, fp_add_rounded, fp_add_subnormal, fp_add_underflow;
    fp_add u_fp_add (
        .a         (s_read_data[0]),
        .b         (s_read_data[1]),
        .result    (fp_add_result),
        .inexact   (fp_add_inexact),
        .rounded   (fp_add_rounded),
        .subnormal (fp_add_subnormal),
        .underflow (fp_add_underflow)
    );

    logic [WORD_SIZE-1:0] fp_sub_result;
    logic fp_sub_inexact, fp_sub_rounded, fp_sub_subnormal, fp_sub_underflow;
    fp_sub u_fp_sub (
        .a         (s_read_data[0]),
        .b         (s_read_data[1]),
        .result    (fp_sub_result),
        .inexact   (fp_sub_inexact),
        .rounded   (fp_sub_rounded),
        .subnormal (fp_sub_subnormal),
        .underflow (fp_sub_underflow)
    );

    logic [WORD_SIZE-1:0] fp_mul_result;
    logic fp_mul_inexact, fp_mul_rounded, fp_mul_subnormal, fp_mul_underflow;
    fp_mul u_fp_mul (
        .a         (s_read_data[0]),
        .b         (s_read_data[1]),
        .result    (fp_mul_result),
        .inexact   (fp_mul_inexact),
        .rounded   (fp_mul_rounded),
        .subnormal (fp_mul_subnormal),
        .underflow (fp_mul_underflow)
    );

    // External interrupts: lowest enabled pending source.
    logic external_interrupt_pending;
    logic [4:0] external_source_sel;
    logic external_found;
    logic [31:0] external_pending_effective;

    // Edge detection for level-sensitive IRQ inputs.
    logic [31:0] external_irq_prev;
    always_comb begin
        external_pending_effective = special[SPECIAL_EXTERNAL_PENDING] | external_irq;
        external_interrupt_pending = (((external_pending_effective & special[SPECIAL_EXTERNAL_MASK]) != 0) &&
                                     (mode_t'(special[SPECIAL_MODE]).nonprivileged || mode_t'(special[SPECIAL_MODE]).privileged));
        external_source_sel = 5'd0;
        external_found = 1'b0;
        for (int i = 0; i < 32; i++) begin
            if (!external_found &&
                ((external_pending_effective & special[SPECIAL_EXTERNAL_MASK] & (32'd1 << i)) != 0)) begin
                external_source_sel = i[4:0];
                external_found = 1'b1;
            end
        end
    end

    // Floating division by a value whose magnitude is below FLT_EPSILON raises
    // an instruction fault (matches C++).
    /* verilator lint_off UNOPTFLAT */
    logic fp_divisor_too_small;
    always_comb begin
        real absv;
        absv = 0.0;
        fp_divisor_too_small = 1'b0;
        if (decoded_op == OP_FLOATING_DIVISION) begin
            absv = binary32_to_real(s_read_data[1]);
            if (absv < 0.0) absv = -absv;
            if (absv < FP_EPSILON) fp_divisor_too_small = 1'b1;
        end
    end
    /* verilator lint_on UNOPTFLAT */

    logic [WORD_SIZE-1:0] fp_div_result;
    logic fp_div_inexact, fp_div_rounded, fp_div_subnormal, fp_div_underflow;
    fp_div u_fp_div (
        .a         (s_read_data[0]),
        .b         (s_read_data[1]),
        .result    (fp_div_result),
        .inexact   (fp_div_inexact),
        .rounded   (fp_div_rounded),
        .subnormal (fp_div_subnormal),
        .underflow (fp_div_underflow)
    );

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

    // The arithmetic operations (add, sub, mul, div, mod, neg, sext/zext). Their
    // operands are read in S_DECODE and held in registers, so the unit's fault (a zero
    // divisor, a bad size) does not depend on the register file in the commit cycle.
    // The three formats share the size field but place `sign`, `sat`, `cond` and
    // `eval` differently, so each is picked out by operation.
    logic is_alu, is_alu_binary, is_extend, is_negate;
    always_comb begin
        is_alu_binary = decoded_op == OP_ADDITION || decoded_op == OP_SUBTRACT || decoded_op == OP_MULTIPLY
                        || decoded_op == OP_DIVIDE || decoded_op == OP_MODULO;
        is_negate     = decoded_op == OP_NEGATE;
        is_extend     = decoded_op == OP_EXTEND;
        is_alu        = is_alu_binary || is_negate || is_extend;
    end

    word_t op_a, op_b;  // the operands latched at the end of S_DECODE

    logic alu_sign, alu_sat, alu_cond;
    evaluation_index_t alu_eval_index;
    scratch_index_t alu_dst;
    always_comb begin
        alu_sign       = 1'b0;
        alu_sat        = 1'b0;
        alu_cond       = 1'b0;
        alu_eval_index = view.arithmetic.eval;
        alu_dst        = view.arithmetic.dst;
        if (is_alu_binary) begin
            alu_sign       = view.arithmetic.sign;
            alu_sat        = view.arithmetic.sat;
            alu_cond       = view.arithmetic.cond;
        end else if (is_negate) begin
            alu_sat        = view.negate.sat;
            alu_cond       = view.negate.cond;
            alu_eval_index = view.negate.eval;
            alu_dst        = view.negate.dst;
        end else begin
            alu_sign       = view.extend.sign;
            alu_cond       = view.extend.cond;
            alu_eval_index = view.extend.eval;
            alu_dst        = view.extend.dst;
        end
    end

    word_t       alu_result;
    evaluation_t alu_eva;
    logic        alu_fault_raw;
    /* verilator lint_off UNUSEDSIGNAL */
    logic        alu_valid;  // is_alu already says whether the unit executes this opcode
    /* verilator lint_on UNUSEDSIGNAL */

    alu u_alu (
        .op(decoded_op), .size(view.arithmetic.size), .sign(alu_sign), .sat(alu_sat),
        .a(op_a), .b(op_b),
        .result(alu_result), .eva(alu_eva), .fault(alu_fault_raw), .valid(alu_valid)
    );

    logic alu_fault;
    assign alu_fault = is_alu && alu_fault_raw;

    // The instruction writes only the low `size` bits of the destination (extend writes
    // the whole word), so the old destination, read at commit, supplies the rest.
    word_t alu_mask, alu_write_data;
    always_comb begin
        case (view.arithmetic.size)
            SIZE_BYTE: alu_mask = 32'h0000_00FF;
            SIZE_HALF: alu_mask = 32'h0000_FFFF;
            default:   alu_mask = 32'hFFFF_FFFF;
        endcase
        if (is_extend) begin
            alu_mask = 32'hFFFF_FFFF;
        end
        alu_write_data = (s_read_data[0] & ~alu_mask) | (alu_result & alu_mask);
    end

    word_t load_value;
    always_comb begin
        case (view.memory.size)
            SIZE_BYTE: load_value = (mem_rsp_data >> (view.memory.shift * 8)) & 32'h0000_00FF;
            SIZE_HALF: load_value = (mem_rsp_data >> (view.memory.shift * 16)) & 32'h0000_FFFF;
            default:   load_value = mem_rsp_data;
        endcase
    end

    evaluation_t comp_eval;
    always_comb begin
        comp_eval = '0;
        comp_eval.comparison.type_ = EVAL_TYPE_COMPARISON;
        if (view.compare.signed_) begin
            if ($signed(s_read_data[0]) == $signed(s_read_data[1])) comp_eval.comparison.equal = 1'b1;
            if ($signed(s_read_data[0]) <  $signed(s_read_data[1])) comp_eval.comparison.less_than = 1'b1;
            if ($signed(s_read_data[0]) >  $signed(s_read_data[1])) comp_eval.comparison.greater_than = 1'b1;
            if ($signed(s_read_data[0]) != $signed(s_read_data[1])) comp_eval.comparison.not_equal = 1'b1;
        end else begin
            if (s_read_data[0] == s_read_data[1]) comp_eval.comparison.equal = 1'b1;
            if (s_read_data[0] <  s_read_data[1]) comp_eval.comparison.less_than = 1'b1;
            if (s_read_data[0] >  s_read_data[1]) comp_eval.comparison.greater_than = 1'b1;
            if (s_read_data[0] != s_read_data[1]) comp_eval.comparison.not_equal = 1'b1;
        end
    end

    // Leap helpers (register form). Immediate form is handled separately.
    logic leap_is_immediate;
    assign leap_is_immediate = view.leap_immediate.immediate;

    logic signed [63:0] leap_byte_offset;
    assign leap_byte_offset = $signed({{54{view.leap.imm[9]}}, view.leap.imm}) * 64'sd4;

    word_t leap_imm_target;
    assign leap_imm_target = {view.leap_immediate.imm, 2'b00};

    word_t leap_base;
    assign leap_base = s_read_data[0];

    logic leap_overflow;
    always_comb begin
        leap_overflow = 1'b0;
        if (decoded_op == OP_LEAP && !leap_is_immediate && cond_pass) begin
            if (leap_byte_offset > 0) begin
                if (leap_base > 32'hFFFFFFFF - leap_byte_offset[31:0])
                    leap_overflow = 1'b1;
            end else if (leap_byte_offset < 0) begin
                if (leap_base < (32'd0 - leap_byte_offset[31:0]))
                    leap_overflow = 1'b1;
            end
        end
    end

    word_t leap_target;
    always_comb begin
        if (leap_byte_offset > 0)
            leap_target = leap_base + leap_byte_offset[31:0];
        else if (leap_byte_offset < 0)
            leap_target = leap_base - (32'd0 - leap_byte_offset[31:0]);
        else
            leap_target = leap_base;
    end

    logic [63:0] grow_fall_next64;
    logic stack_fault_this;
    always_comb begin
        grow_fall_next64 = 64'd0;
        stack_fault_this = 1'b0;
        if ((decoded_op == OP_GROW || decoded_op == OP_FALL) && cond_pass) begin
            if (decoded_op == OP_GROW)
                grow_fall_next64 = 64'(special[SPECIAL_STACK_CURRENT]) - (64'(view.frame.imm) * 64'd4);
            else
                grow_fall_next64 = 64'(special[SPECIAL_STACK_CURRENT]) + (64'(view.frame.imm) * 64'd4);
            if (grow_fall_next64 < 64'(special[SPECIAL_STACK_BASE]) || grow_fall_next64 > 64'(special[SPECIAL_STACK_LIMIT]))
                stack_fault_this = 1'b1;
        end
    end

    logic [3:0]  push_pull_count;

    // External interrupt table fetch state.
    logic [31:0] external_table_base;
    logic [1:0]  external_fetch_state;
    logic [31:0] push_pull_size;
    logic        push_pull_bounds_ok;
    logic [63:0] push_pull_end64;
    always_comb begin
        push_pull_count = 4'($countones(view.push_pull.mask));
        push_pull_size = 32'(push_pull_count) * 32'd4;
        if (decoded_op == OP_PUSH) begin
            push_pull_end64 = 64'(special[SPECIAL_STACK_CURRENT]) - 64'(push_pull_size);
            push_pull_bounds_ok = (64'(special[SPECIAL_STACK_BASE]) <= push_pull_end64) &&
                                  (push_pull_end64 <= 64'(special[SPECIAL_STACK_LIMIT])) &&
                                  (64'(push_pull_size) <= 64'(special[SPECIAL_STACK_LIMIT]) - push_pull_end64);
        end else begin
            push_pull_end64 = 64'(special[SPECIAL_STACK_CURRENT]) + 64'(push_pull_size);
            push_pull_bounds_ok = (64'(special[SPECIAL_STACK_BASE]) <= 64'(special[SPECIAL_STACK_CURRENT])) &&
                                  (64'(special[SPECIAL_STACK_CURRENT]) <= 64'(special[SPECIAL_STACK_LIMIT])) &&
                                  (64'(push_pull_size) <= 64'(special[SPECIAL_STACK_LIMIT]) - 64'(special[SPECIAL_STACK_CURRENT]));
        end
    end

    logic control_transferred;
    always_comb begin
        control_transferred = 1'b0;
        if (decoded_op == OP_LEAP) begin
            if (leap_is_immediate)
                control_transferred = 1'b1;
            else if (cond_pass)
                control_transferred = 1'b1;
        end else if (decoded_op == OP_BACK) begin
            control_transferred = 1'b1;
        end
    end

    // An instruction fault: an undefined opcode or an invalid operation. Nothing is
    // written, and the exception below redirects the program address like Cycle().
    logic instruction_fault;
    assign instruction_fault = (state == S_COMMIT) && !fetch_fault &&
                               (!decoded_valid || blu_fault ||
                                ((decoded_op == OP_LOAD || decoded_op == OP_SAVE) && !memory_encoding_valid));

    logic data_bus_fault;
    assign data_bus_fault = ((state == S_LOAD_RESPONSE) || (state == S_SAVE_RESPONSE) ||
                            (state == S_SWAP_READ_RESPONSE) || (state == S_SWAP_WRITE_RESPONSE)) &&
                            mem_rsp_valid && mem_rsp_ready && mem_rsp_fault;

    // The exception state after this commit; any set bit redirects through the table.
    word_t exception_next;
    /* verilator lint_off UNOPTFLAT */
    always_comb begin
        exception_next = special[SPECIAL_EXCEPTION];
        if (data_bus_fault) begin
            exception_next[EXCEPTION_BUS_FAULT] = 1'b1;
        end
        if (instruction_fault) begin
            exception_next[EXCEPTION_INSTRUCTION_FAULT] = 1'b1;
        end
        if (alu_fault) begin
            if (alu_eva.arithmetic.undefined) begin
                exception_next[EXCEPTION_ARITHMETIC_FAULT] = 1'b1;
            end else begin
                exception_next[EXCEPTION_INSTRUCTION_FAULT] = 1'b1;
            end
        end
        if (decoded_op == OP_FLOATING_DIVISION && fp_divisor_too_small) begin
            exception_next[EXCEPTION_ARITHMETIC_FAULT] = 1'b1;
        end
        if (is_privileged_op(decoded_op) &&
            mode_t'(special[SPECIAL_MODE]).nonprivileged) begin
            exception_next[EXCEPTION_PRIVILEGE_FAULT] = 1'b1;
        end
        if (decoded_op == OP_LEAP && !leap_is_immediate && cond_pass && leap_overflow) begin
            exception_next[EXCEPTION_BUS_FAULT] = 1'b1;
        end
        if ((decoded_op == OP_GROW || decoded_op == OP_FALL) && cond_pass && stack_fault_this) begin
            exception_next[EXCEPTION_STACK_FAULT] = 1'b1;
        end
        if ((decoded_op == OP_PUSH || decoded_op == OP_PULL) && !push_pull_bounds_ok) begin
            exception_next[EXCEPTION_STACK_FAULT] = 1'b1;
        end
        if ((decoded_op == OP_PUSH || decoded_op == OP_PULL) && push_pull_bus_fault) begin
            exception_next[EXCEPTION_BUS_FAULT] = 1'b1;
        end
        if (decoded_op == OP_TRIP && cond_pass && !mode_t'(special[SPECIAL_MODE]).nonprivileged) begin
            exception_next[EXCEPTION_TRIPPED] = 1'b1;
        end
        if (external_interrupt_pending) begin
            exception_next[EXCEPTION_EXTERNAL] = 1'b1;
        end

        // Apply the exception mask (reset/unmaskable bits 0/1 are not maskable).
        exception_next = exception_next & (special[SPECIAL_EXCEPTION_MASK] | EXCEPTION_MASK_PRESERVED_BITS);
    end
    /* verilator lint_on UNOPTFLAT */

    logic [3:0] exception_vector_index;
    always_comb begin
        if (exception_next[EXCEPTION_BUS_FAULT])
            exception_vector_index = VECTOR_BUS_FAULT_HANDLER;
        else if (exception_next[EXCEPTION_INSTRUCTION_FAULT])
            exception_vector_index = VECTOR_INSTRUCTION_FAULT_HANDLER;
        else if (exception_next[EXCEPTION_ARITHMETIC_FAULT])
            exception_vector_index = VECTOR_ARITHMETIC_FAULT_HANDLER;
        else if (exception_next[EXCEPTION_STACK_FAULT])
            exception_vector_index = VECTOR_STACK_FAULT_HANDLER;
        else if (exception_next[EXCEPTION_TRIPPED])
            exception_vector_index = VECTOR_TRIPPED_HANDLER;
        else if (exception_next[EXCEPTION_PRIVILEGE_FAULT])
            exception_vector_index = VECTOR_PRIVILEGE_HANDLER;
        else
            exception_vector_index = VECTOR_INSTRUCTION_FAULT_HANDLER;
    end

    logic commit_valid;
    assign commit_valid = (state == S_COMMIT) && !fetch_fault && supported_op && !blu_fault && !alu_fault &&
                          ((decoded_op != OP_LOAD && decoded_op != OP_SAVE) || memory_encoding_valid);

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
        if (state == S_DECODE && decoded_op == OP_LEAP && !view.leap_immediate.immediate && view.leap.cond) begin
            e_read_control  = EVALUATION_CONTROL_BOTH;
            e_read_index[0] = view.leap.eval;
            e_read_index[1] = view.leap.mask;
        end
        if (state == S_DECODE && (decoded_op == OP_GROW || decoded_op == OP_FALL) && view.frame.cond) begin
            e_read_control  = EVALUATION_CONTROL_BOTH;
            e_read_index[0] = view.frame.eval;
            e_read_index[1] = view.frame.mask;
        end
        if (state == S_DECODE && (decoded_op == OP_CALL || decoded_op == OP_TRIP) && view.frame.cond) begin
            e_read_control  = EVALUATION_CONTROL_BOTH;
            e_read_index[0] = view.frame.eval;
            e_read_index[1] = view.frame.mask;
        end

        // The ALU operands are read in S_DECODE and latched.
        if (state == S_DECODE && is_alu) begin
            s_read_control  = is_alu_binary ? SCRATCH_CONTROL_BOTH : SCRATCH_CONTROL_FIRST;
            s_read_index[0] = is_alu_binary ? view.arithmetic.src1 : (is_negate ? view.negate.src : view.extend.src);
            s_read_index[1] = view.arithmetic.src2;
        end

        // Bitwise operands are read whenever the operation is about to commit; only the
        // write is gated by the operation being valid.
        if (state == S_COMMIT && !fetch_fault && supported_op) begin
            if (decoded_op == OP_LOAD) begin
                s_read_control  = SCRATCH_CONTROL_FIRST;
                s_read_index[0] = view.memory.base;
            end
            if (decoded_op == OP_SAVE) begin
                s_read_control  = SCRATCH_CONTROL_BOTH;
                s_read_index[0] = view.memory.base;
                s_read_index[1] = view.memory.reg_;
            end
            if (decoded_op == OP_SWAP) begin
                s_read_control  = SCRATCH_CONTROL_BOTH;
                s_read_index[0] = view.swap.b;
                s_read_index[1] = view.swap.a;
            end
            if (is_alu) begin
                // The old destination, merged with the sized result.
                s_read_control  = SCRATCH_CONTROL_FIRST;
                s_read_index[0] = alu_dst;
            end
            if (is_bitwise1) begin
                s_read_control  = SCRATCH_CONTROL_FIRST;
                s_read_index[0] = view.bitwise1.src;
            end
            if (is_bitwise2) begin
                s_read_control  = SCRATCH_CONTROL_BOTH;
                s_read_index[0] = view.binary.src1;
                s_read_index[1] = view.binary.src2;
            end
            if (decoded_op == OP_LEAP && !leap_is_immediate) begin
                s_read_control  = SCRATCH_CONTROL_FIRST;
                s_read_index[0] = view.leap.dst;
            end
            if (state == S_PUSH_PULL_REQUEST && push_pull_is_push) begin
                s_read_control  = SCRATCH_CONTROL_FIRST;
                s_read_index[0] = 4'(push_pull_index);
            end
            if (decoded_op == OP_COMPARE) begin
                s_read_control  = SCRATCH_CONTROL_BOTH;
                s_read_index[0] = view.compare.a;
                s_read_index[1] = view.compare.b;
            end
            if (decoded_op == OP_FLOATING_ABS || decoded_op == OP_FLOATING_NEGATE ||
                decoded_op == OP_FLOATING_FLOOR || decoded_op == OP_FLOATING_CEIL ||
                decoded_op == OP_FLOATING_FRACTIONAL || decoded_op == OP_FLOATING_CONVERT) begin
                s_read_control  = SCRATCH_CONTROL_FIRST;
                s_read_index[0] = view.float_unary.src;
            end
            if (decoded_op == OP_FLOATING_ADDITION || decoded_op == OP_FLOATING_SUBTRACTION ||
                decoded_op == OP_FLOATING_MULTIPLICATION || decoded_op == OP_FLOATING_DIVISION) begin
                s_read_control  = SCRATCH_CONTROL_BOTH;
                s_read_index[0] = view.binary.src1;
                s_read_index[1] = view.binary.src2;
            end
        end

        if (state == S_LOAD_RESPONSE && mem_rsp_valid && mem_rsp_ready && !mem_rsp_fault) begin
            s_read_control  = SCRATCH_CONTROL_FIRST;
            s_read_index[0] = view.memory.base;
            if (view.memory.off == 1'b0) begin
                s_write_control  = SCRATCH_CONTROL_BOTH;
                s_write_index[0] = view.memory.base;
                s_write_data[0]  = s_read_data[0] + address_t'(view.memory.imm);
                s_write_index[1] = view.memory.reg_;
                s_write_data[1]  = load_value;
            end else begin
                s_write_control  = SCRATCH_CONTROL_FIRST;
                s_write_index[0] = view.memory.reg_;
                s_write_data[0]  = load_value;
            end

        end

        if (state == S_SAVE_RESPONSE && mem_rsp_valid && mem_rsp_ready && !mem_rsp_fault &&
            view.memory.off == 1'b0) begin
            s_read_control  = SCRATCH_CONTROL_FIRST;
            s_read_index[0] = view.memory.base;
            s_write_control = SCRATCH_CONTROL_FIRST;
            s_write_index[0] = view.memory.base;
            s_write_data[0] = s_read_data[0] + address_t'(view.memory.imm);
        end

        if (state == S_SWAP_WRITE_RESPONSE && mem_rsp_valid && mem_rsp_ready && !mem_rsp_fault) begin
            s_write_control  = SCRATCH_CONTROL_FIRST;
            s_write_index[0] = view.swap.a;
            s_write_data[0]  = swap_read_data_reg;
        end

        if (state == S_PUSH_PULL_RESPONSE && mem_rsp_valid && mem_rsp_ready && !mem_rsp_fault &&
            !push_pull_is_push) begin
            s_write_control  = SCRATCH_CONTROL_FIRST;
            s_write_index[0] = 4'(push_pull_index);
            s_write_data[0]  = mem_rsp_data;
        end

        if (commit_valid) begin
            if (is_alu) begin
                s_write_control  = SCRATCH_CONTROL_FIRST;
                s_write_index[0] = alu_dst;
                s_write_data[0]  = alu_write_data;
                if (alu_cond) begin
                    e_write_control  = EVALUATION_CONTROL_FIRST;
                    e_write_index[0] = alu_eval_index;
                    e_write_data[0]  = alu_eva.raw;
                end
            end

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
                OP_COMPARE: begin
                    e_write_control  = EVALUATION_CONTROL_FIRST;
                    e_write_index[0] = view.compare.e;
                    e_write_data[0]  = comp_eval.raw;
                end
                OP_FLOATING_ABS: begin
                    // Stage 1 of M11: clear the sign bit of the source float.
                    s_write_control  = SCRATCH_CONTROL_FIRST;
                    s_write_index[0] = view.float_unary.dst;
                    s_write_data[0]  = fp_abs_result;
                end
                OP_FLOATING_NEGATE: begin
                    // Stage 2 of M11: flip the sign bit of the source float.
                    s_write_control  = SCRATCH_CONTROL_FIRST;
                    s_write_index[0] = view.float_unary.dst;
                    s_write_data[0]  = fp_neg_result;
                end
                OP_FLOATING_FLOOR: begin
                    s_write_control  = SCRATCH_CONTROL_FIRST;
                    s_write_index[0] = view.float_unary.dst;
                    s_write_data[0]  = fp_floor_result;
                end
                OP_FLOATING_CEIL: begin
                    s_write_control  = SCRATCH_CONTROL_FIRST;
                    s_write_index[0] = view.float_unary.dst;
                    s_write_data[0]  = fp_ceil_result;
                end
                OP_FLOATING_FRACTIONAL: begin
                    s_write_control  = SCRATCH_CONTROL_FIRST;
                    s_write_index[0] = view.float_unary.dst;
                    s_write_data[0]  = fp_fractional_result;
                end
                OP_FLOATING_CONVERT: begin
                    s_write_control  = SCRATCH_CONTROL_FIRST;
                    s_write_index[0] = view.float_unary.dst;
                    s_write_data[0]  = fp_convert_result;
                end
                OP_FLOATING_ADDITION: begin
                    s_write_control  = SCRATCH_CONTROL_FIRST;
                    s_write_index[0] = view.binary.dst;
                    s_write_data[0]  = fp_add_result;
                    if (view.binary.cond) begin
                        e_write_control  = EVALUATION_CONTROL_FIRST;
                        e_write_index[0] = view.binary.eval;
                        e_write_data[0]  = {2'b00, fp_add_underflow, fp_add_subnormal,
                                           fp_add_rounded, fp_add_inexact, 2'b11};
                    end
                end
                OP_FLOATING_SUBTRACTION: begin
                    s_write_control  = SCRATCH_CONTROL_FIRST;
                    s_write_index[0] = view.binary.dst;
                    s_write_data[0]  = fp_sub_result;
                    if (view.binary.cond) begin
                        e_write_control  = EVALUATION_CONTROL_FIRST;
                        e_write_index[0] = view.binary.eval;
                        e_write_data[0]  = {2'b00, fp_sub_underflow, fp_sub_subnormal,
                                           fp_sub_rounded, fp_sub_inexact, 2'b11};
                    end
                end
                OP_FLOATING_MULTIPLICATION: begin
                    s_write_control  = SCRATCH_CONTROL_FIRST;
                    s_write_index[0] = view.binary.dst;
                    s_write_data[0]  = fp_mul_result;
                    if (view.binary.cond) begin
                        e_write_control  = EVALUATION_CONTROL_FIRST;
                        e_write_index[0] = view.binary.eval;
                        e_write_data[0]  = {2'b00, fp_mul_underflow, fp_mul_subnormal,
                                           fp_mul_rounded, fp_mul_inexact, 2'b11};
                    end
                end
                OP_FLOATING_DIVISION: begin
                    if (!fp_divisor_too_small) begin
                        s_write_control  = SCRATCH_CONTROL_FIRST;
                        s_write_index[0] = view.binary.dst;
                        s_write_data[0]  = fp_div_result;
                        if (view.binary.cond) begin
                            e_write_control  = EVALUATION_CONTROL_FIRST;
                            e_write_index[0] = view.binary.eval;
                            e_write_data[0]  = {2'b00, fp_div_underflow, fp_div_subnormal,
                                               fp_div_rounded, fp_div_inexact, 2'b11};
                        end
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
            state       <= S_BOOT_REQUEST;
            boot_index  <= '0;
            halted      <= 1'b0;
            instruction <= ERASED_WORD;
            fetch_fault <= 1'b0;
            vector_index <= '0;
            external_table_base <= '0;
            external_fetch_state <= 2'd0;
            external_irq_prev <= '0;
            dbg <= '0;

            cond_pass   <= 1'b0;
            push_pull_is_push  <= 1'b0;
            push_pull_mask     <= '0;
            push_pull_index    <= '0;
            push_pull_address  <= '0;
            push_pull_end      <= '0;
            push_pull_bus_fault <= 1'b0;
            op_a        <= '0;
            op_b        <= '0;
            swap_address_reg <= '0;
            swap_wdata_reg   <= '0;
            swap_read_data_reg <= '0;
            for (int i = 0; i < SPECIAL_COUNT; i++) begin
                special[i] <= '0;
            end
            special[SPECIAL_VECTOR_TABLE_ADDRESS] <= DEFAULT_VECTOR_TABLE_ADDRESS;
            special[SPECIAL_MODE] <= MODE_RESET;
            special[SPECIAL_EXCEPTION_MASK] <= 32'hFFFFFFFF;
        end else begin
            // Latch external interrupts on rising edges.
            special[SPECIAL_EXTERNAL_PENDING] <= special[SPECIAL_EXTERNAL_PENDING] | (external_irq & ~external_irq_prev);
            external_irq_prev <= external_irq;
            case (state)
                S_BOOT_REQUEST: begin
                    if (mem_req_ready) begin
                        state <= S_BOOT_RESPONSE;
                    end
                end

                S_BOOT_RESPONSE: begin
                    if (mem_rsp_valid && mem_rsp_ready) begin
                        // A faulting read leaves the field at its reset value, mirroring Reset().
                        if (!mem_rsp_fault) begin
                            case (boot_index)
                                BOOT_STACK_INITIAL: begin
                                    special[SPECIAL_STACK_LIMIT]   <= mem_rsp_data;
                                    special[SPECIAL_STACK_CURRENT] <= mem_rsp_data;
                                end
                                BOOT_STACK_BOUNDARY: begin
                                    special[SPECIAL_STACK_BASE] <= mem_rsp_data;
                                end
                                BOOT_EXC_STACK_INITIAL: begin
                                    special[SPECIAL_EXCEPTION_STACK_LIMIT]   <= mem_rsp_data;
                                    special[SPECIAL_EXCEPTION_STACK_CURRENT] <= mem_rsp_data;
                                end
                                BOOT_EXC_STACK_BOUNDARY: begin
                                    special[SPECIAL_EXCEPTION_STACK_BASE] <= mem_rsp_data;
                                end
                                default: begin
                                    special[SPECIAL_PROGRAM_ADDRESS] <= mem_rsp_data;
                                    special[SPECIAL_RETURN_ADDRESS] <= mem_rsp_data;
                                end
                            endcase
                        end

                        if (boot_index == BOOT_RESET_HANDLER) begin
                            boot_index <= '0;
                            state      <= S_FETCH_REQUEST;
                        end else begin
                            boot_index <= boot_index + 1'b1;
                            state      <= S_BOOT_REQUEST;
                        end
                    end
                end

                S_FETCH_REQUEST: begin
                    if (mem_req_ready) begin
                        state <= S_FETCH_RESPONSE;
                    end
                end

                S_FETCH_RESPONSE: begin
                    if (mem_rsp_valid && mem_rsp_ready) begin
                        instruction <= mem_rsp_data;
                        fetch_fault <= mem_rsp_fault;
                        state       <= S_DECODE;
                    end
                end

                S_DECODE: begin
                    // The operand read stage: the evaluation file is read for the
                    // condition and the verdict is held for S_COMMIT.
                    if (decoded_op == OP_LEAP && !view.leap_immediate.immediate) begin
                        if (view.leap.cond)
                            cond_pass <= ((e_read_data[0] & e_read_data[1]) != 8'd0);
                        else
                            cond_pass <= 1'b1;
                    end else if (decoded_op == OP_MOVE) begin
                        if (view.move.cond)
                            cond_pass <= ((e_read_data[0] & e_read_data[1]) != 8'd0);
                        else
                            cond_pass <= 1'b1;
                    end else if (decoded_op == OP_GROW || decoded_op == OP_FALL) begin
                        if (view.frame.cond)
                            cond_pass <= ((e_read_data[0] & e_read_data[1]) != 8'd0);
                        else
                            cond_pass <= 1'b1;
                    end else if (decoded_op == OP_CALL || decoded_op == OP_TRIP) begin
                        if (view.frame.cond)
                            cond_pass <= ((e_read_data[0] & e_read_data[1]) != 8'd0);
                        else
                            cond_pass <= 1'b1;
                    end else begin
                        cond_pass <= 1'b1;
                    end
                    op_a      <= s_read_data[0];
                    op_b      <= s_read_data[1];
                    state     <= S_COMMIT;
                end

                S_COMMIT: begin
                    if (commit_valid && (decoded_op == OP_LOAD || decoded_op == OP_SAVE || decoded_op == OP_SWAP)) begin
                        if (decoded_op == OP_SWAP) begin
                            if (mem_req_ready) begin
                                swap_address_reg <= swap_address;
                                swap_wdata_reg  <= s_read_data[1];
                                state           <= S_SWAP_READ_RESPONSE;
                            end
                        end else if (mem_req_ready) begin
                            state <= decoded_op == OP_LOAD ? S_LOAD_RESPONSE : S_SAVE_RESPONSE;
                        end
                    end else if (commit_valid && decoded_op == OP_BACK && view.back.zero) begin
                        special[SPECIAL_PROGRAM_ADDRESS] <= special[SPECIAL_RETURN_ADDRESS];
                        if (mem_req_ready) begin
                            state <= S_BACK_RESPONSE;
                        end else begin
                            state <= S_COMMIT;
                        end
                    end else if (commit_valid && (decoded_op == OP_PUSH || decoded_op == OP_PULL) &&
                                 push_pull_bounds_ok && (push_pull_count != 0)) begin
                        push_pull_is_push   <= (decoded_op == OP_PUSH);
                        push_pull_mask      <= view.push_pull.mask;
                        push_pull_index     <= 5'(first_set_bit(view.push_pull.mask));
                        push_pull_address   <= (decoded_op == OP_PUSH) ? push_pull_end64[31:0]
                                                                   : special[SPECIAL_STACK_CURRENT];
                        push_pull_end       <= push_pull_end64[31:0];
                        push_pull_bus_fault <= 1'b0;
                        state               <= S_PUSH_PULL_REQUEST;
                    end else if (commit_valid && decoded_op == OP_CALL && cond_pass) begin
                        special[SPECIAL_RETURN_ADDRESS] <= special[SPECIAL_PROGRAM_ADDRESS] + 32'd4;
                        if (mem_req_ready) begin
                            state <= S_CALL_RESPONSE;
                        end else begin
                            state <= S_COMMIT;
                        end
                    end else begin
                        state <= S_FETCH_REQUEST;

                        if (fetch_fault) begin
                            // No instruction: Cycle() substitutes a noop and advances.
                        end else if (!decoded_valid) begin
                            // Undefined opcode: an instruction fault, handled below.
                        end else if (supported_op) begin
                            // Register writes are driven combinationally into the files below.
                            if (decoded_op == OP_HALT && !mode_t'(special[SPECIAL_MODE]).nonprivileged) begin
                                halted <= 1'b1;
                                state  <= S_HALTED;
                            end
                            if (decoded_op == OP_LEAP) begin
                                if (leap_is_immediate) begin
                                    if (view.leap_immediate.save)
                                        special[SPECIAL_RETURN_ADDRESS] <= special[SPECIAL_PROGRAM_ADDRESS] + 32'd4;
                                    special[SPECIAL_PROGRAM_ADDRESS] <= leap_imm_target;
                                end else if (cond_pass) begin
                                    if (view.leap.save)
                                        special[SPECIAL_RETURN_ADDRESS] <= special[SPECIAL_PROGRAM_ADDRESS] + 32'd4;
                                    if (!leap_overflow)
                                        special[SPECIAL_PROGRAM_ADDRESS] <= leap_target;
                                end
                            end
                            if (decoded_op == OP_BACK && !view.back.zero) begin
                                special[SPECIAL_PROGRAM_ADDRESS] <= special[SPECIAL_RETURN_ADDRESS];
                            end
                            if (decoded_op == OP_GROW || decoded_op == OP_FALL) begin
                                if (cond_pass && !stack_fault_this) begin
                                    if (decoded_op == OP_GROW)
                                        special[SPECIAL_STACK_CURRENT] <= special[SPECIAL_STACK_CURRENT] - (32'(view.frame.imm) << 2);
                                    else
                                        special[SPECIAL_STACK_CURRENT] <= special[SPECIAL_STACK_CURRENT] + (32'(view.frame.imm) << 2);
                                end
                            end

                            if (decoded_op == OP_TRIP && cond_pass && !mode_t'(special[SPECIAL_MODE]).nonprivileged) begin
                                special[SPECIAL_NUMERAL] <= (special[SPECIAL_NUMERAL] & 32'h0000FFFF) |
                                                             (32'(view.trip.imm) << 16);
                            end
                        end else begin
                            unsupported <= 1'b1;
                        end

                        special[SPECIAL_PERFORMANCE_COUNTER] <= special[SPECIAL_PERFORMANCE_COUNTER] + 32'd1;
                        special[SPECIAL_EXCEPTION]           <= exception_next;
                        special[SPECIAL_MODE]               <= (exception_next != '0) ? MODE_EXCEPTION : special[SPECIAL_MODE];

                        if (exception_next != '0) begin
                            // Any pending exception sends the next fetch through the vector table.
                            special[SPECIAL_MODE] <= MODE_EXCEPTION; // exception + privileged
                            dbg <= '{external_pending: special[SPECIAL_EXTERNAL_PENDING],
                                     last_exception_type: 5'(exception_vector_index)};
                            if (exception_next[EXCEPTION_EXTERNAL]) begin
                                special[SPECIAL_EXTERNAL_SOURCE] <= 32'(external_source_sel);
                                special[SPECIAL_EXTERNAL_PENDING] <=
                                    special[SPECIAL_EXTERNAL_PENDING] & ~(32'd1 << external_source_sel);
                                external_fetch_state <= 2'd1;
                            end else begin
                                external_fetch_state <= 2'd0;
                            end
                            vector_index <= exception_vector_index;
                            state        <= S_VECTOR_REQUEST;
                        end else if (!control_transferred) begin
                            special[SPECIAL_PROGRAM_ADDRESS] <= special[SPECIAL_PROGRAM_ADDRESS] + 32'd4;
                            retire                           <= 1'b1;
                        end else begin
                            retire <= 1'b1;
                        end
                    end
                end

                S_LOAD_RESPONSE: begin
                    if (mem_rsp_valid && mem_rsp_ready) begin
                        special[SPECIAL_PERFORMANCE_COUNTER] <= special[SPECIAL_PERFORMANCE_COUNTER] + 32'd1;
                        special[SPECIAL_EXCEPTION]           <= exception_next;
                        special[SPECIAL_MODE]               <= (exception_next != '0) ? MODE_EXCEPTION : special[SPECIAL_MODE];
                        if (exception_next != '0) begin
                            vector_index <= exception_vector_index;
                            state        <= S_VECTOR_REQUEST;
                        end else begin
                            special[SPECIAL_PROGRAM_ADDRESS] <= special[SPECIAL_PROGRAM_ADDRESS] + 32'd4;
                            retire                           <= 1'b1;
                            state                            <= S_FETCH_REQUEST;
                        end
                    end
                end

                S_SAVE_RESPONSE: begin
                    if (mem_rsp_valid && mem_rsp_ready) begin
                        special[SPECIAL_PERFORMANCE_COUNTER] <= special[SPECIAL_PERFORMANCE_COUNTER] + 32'd1;
                        special[SPECIAL_EXCEPTION]           <= exception_next;
                        special[SPECIAL_MODE]               <= (exception_next != '0) ? MODE_EXCEPTION : special[SPECIAL_MODE];
                        if (exception_next != '0) begin
                            vector_index <= exception_vector_index;
                            state        <= S_VECTOR_REQUEST;
                        end else begin
                            special[SPECIAL_PROGRAM_ADDRESS] <= special[SPECIAL_PROGRAM_ADDRESS] + 32'd4;
                            retire                           <= 1'b1;
                            state                            <= S_FETCH_REQUEST;
                        end
                    end
                end

                S_SWAP_READ_RESPONSE: begin
                    if (mem_rsp_valid && mem_rsp_ready) begin
                        if (mem_rsp_fault) begin
                            special[SPECIAL_PERFORMANCE_COUNTER] <= special[SPECIAL_PERFORMANCE_COUNTER] + 32'd1;
                            special[SPECIAL_EXCEPTION]           <= exception_next;
                            special[SPECIAL_MODE]               <= (exception_next != '0) ? MODE_EXCEPTION : special[SPECIAL_MODE];
                            if (exception_next != '0) begin
                                vector_index <= exception_vector_index;
                                state        <= S_VECTOR_REQUEST;
                            end else begin
                                special[SPECIAL_PROGRAM_ADDRESS] <= special[SPECIAL_PROGRAM_ADDRESS] + 32'd4;
                                retire                           <= 1'b1;
                                state                            <= S_FETCH_REQUEST;
                            end
                        end else begin
                            swap_read_data_reg <= mem_rsp_data;
                            state              <= S_SWAP_WRITE_REQUEST;
                        end
                    end
                end

                S_SWAP_WRITE_REQUEST: begin
                    if (mem_req_ready) begin
                        state <= S_SWAP_WRITE_RESPONSE;
                    end
                end

                S_SWAP_WRITE_RESPONSE: begin
                    if (mem_rsp_valid && mem_rsp_ready) begin
                        special[SPECIAL_PERFORMANCE_COUNTER] <= special[SPECIAL_PERFORMANCE_COUNTER] + 32'd1;
                        special[SPECIAL_EXCEPTION]           <= exception_next;
                        special[SPECIAL_MODE]               <= (exception_next != '0) ? MODE_EXCEPTION : special[SPECIAL_MODE];
                        if (exception_next != '0) begin
                            vector_index <= exception_vector_index;
                            state        <= S_VECTOR_REQUEST;
                        end else begin
                            special[SPECIAL_PROGRAM_ADDRESS] <= special[SPECIAL_PROGRAM_ADDRESS] + 32'd4;
                            retire                           <= 1'b1;
                            state                            <= S_FETCH_REQUEST;
                        end
                    end
                end

                S_BACK_RESPONSE: begin
                    if (mem_rsp_valid && mem_rsp_ready) begin
                        special[SPECIAL_PERFORMANCE_COUNTER] <= special[SPECIAL_PERFORMANCE_COUNTER] + 32'd1;
                        special[SPECIAL_EXCEPTION]           <= exception_next;
                        special[SPECIAL_MODE]               <= (exception_next != '0) ? MODE_EXCEPTION : special[SPECIAL_MODE];
                        if (exception_next != '0) begin
                            vector_index <= exception_vector_index;
                            state        <= S_VECTOR_REQUEST;
                        end else begin
                            special[SPECIAL_RETURN_ADDRESS] <= mem_rsp_data;
                            retire                          <= 1'b1;
                            state                           <= S_FETCH_REQUEST;
                        end
                    end
                end

                S_PUSH_PULL_REQUEST: begin
                    if (mem_req_ready) begin
                        state <= S_PUSH_PULL_RESPONSE;
                    end
                end

                S_PUSH_PULL_RESPONSE: begin
                    if (mem_rsp_valid && mem_rsp_ready) begin
                        if (mem_rsp_fault) begin
                            push_pull_bus_fault <= 1'b1;
                        end
                        push_pull_address <= push_pull_address + 32'd4;
                        push_pull_index   <= 5'(next_set_bit(push_pull_mask, int'(push_pull_index)));
                        if (next_set_bit(push_pull_mask, int'(push_pull_index)) >= 16) begin
                            special[SPECIAL_STACK_CURRENT] <= push_pull_end;
                            special[SPECIAL_PERFORMANCE_COUNTER] <= special[SPECIAL_PERFORMANCE_COUNTER] + 32'd1;
                            special[SPECIAL_EXCEPTION]           <= exception_next;
                            special[SPECIAL_MODE]               <= (exception_next != '0) ? MODE_EXCEPTION : special[SPECIAL_MODE];
                            if (exception_next != '0) begin
                                vector_index <= exception_vector_index;
                                state        <= S_VECTOR_REQUEST;
                            end else begin
                                special[SPECIAL_PROGRAM_ADDRESS] <= special[SPECIAL_PROGRAM_ADDRESS] + 32'd4;
                                retire <= 1'b1;
                                state  <= S_FETCH_REQUEST;
                            end
                        end else begin
                            state <= S_PUSH_PULL_REQUEST;
                        end
                    end
                end

                S_CALL_RESPONSE: begin
                    if (mem_rsp_valid && mem_rsp_ready) begin
                        special[SPECIAL_PERFORMANCE_COUNTER] <= special[SPECIAL_PERFORMANCE_COUNTER] + 32'd1;
                        special[SPECIAL_EXCEPTION]           <= exception_next;
                        special[SPECIAL_MODE]               <= (exception_next != '0) ? MODE_EXCEPTION : special[SPECIAL_MODE];
                        if (exception_next != '0) begin
                            vector_index <= exception_vector_index;
                            state        <= S_VECTOR_REQUEST;
                        end else begin
                            special[SPECIAL_NUMERAL] <= (special[SPECIAL_NUMERAL] & 32'hFFFF0000) |
                                                         32'(view.frame.imm);
                            special[SPECIAL_PROGRAM_ADDRESS] <= mem_rsp_data;
                            retire                          <= 1'b1;
                            state                           <= S_FETCH_REQUEST;
                        end
                    end
                end

                S_VECTOR_REQUEST: begin
                    if (mem_req_ready) begin
                        state <= S_VECTOR_RESPONSE;
                    end
                end

                S_VECTOR_RESPONSE: begin
                    if (mem_rsp_valid && mem_rsp_ready) begin
                        // GetHandler(): the handler, or the reset handler when that read faults.
                        if (!mem_rsp_fault) begin
                            if (external_fetch_state == 2'd1) begin
                                external_table_base <= mem_rsp_data;
                                external_fetch_state <= 2'd2;
                                state <= S_VECTOR_REQUEST;
                            end else if (external_fetch_state == 2'd2) begin
                                special[SPECIAL_PROGRAM_ADDRESS] <= mem_rsp_data;
                                retire                           <= 1'b1;
                                external_fetch_state             <= 2'd0;
                                state                            <= halted ? S_HALTED : S_FETCH_REQUEST;
                            end else begin
                                special[SPECIAL_PROGRAM_ADDRESS] <= mem_rsp_data;
                                retire                           <= 1'b1;
                                state                            <= halted ? S_HALTED : S_FETCH_REQUEST;
                            end
                        end else if (vector_index != 4'(BOOT_RESET_HANDLER)) begin
                            vector_index <= 4'(BOOT_RESET_HANDLER);
                            state        <= S_VECTOR_REQUEST;
                        end else begin
                            // Cycle() throws here: there is no handler to run.
                            halted <= 1'b1;
                            state  <= S_HALTED;
                        end
                    end
                end

                S_HALTED: begin
                    // Cycle() returns immediately once halted; hold until reset.
                end

                default: state <= S_FETCH_REQUEST;
            endcase
        end
    end

endmodule
