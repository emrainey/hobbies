// M0 processor top: reset/vector-table boot, fetch and a stub retire.
//
// The instruction body is intentionally empty: every fetched word is retired with no
// architectural effect. M1/M2 replace that stub with real execution and decoder
// dispatch. Until then this module exists to prove the co-simulation contract:
//
//   * one `retire` pulse per consumed instruction,
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
    output word_t      special[16],

    // Commit handshake
    output logic retire,
    output logic halted,
    output logic booted
);

    // Mirrors isa::DefaultVectorTableAddress (include/isa/map.hpp, Map[1].start).
    localparam address_t DEFAULT_VECTOR_TABLE_ADDRESS = 32'h0010_0000;

    // Special-register word indices, matching isa::Special (include/isa/cpu.hpp).
    localparam int SPECIAL_PA = 0;
    localparam int SPECIAL_RA = 1;
    localparam int SPECIAL_VTA = 2;
    localparam int SPECIAL_STACK_LIMIT = 3;
    localparam int SPECIAL_STACK_CURRENT = 4;
    localparam int SPECIAL_STACK_BASE = 5;
    localparam int SPECIAL_EXC_STACK_LIMIT = 6;
    localparam int SPECIAL_EXC_STACK_CURRENT = 7;
    localparam int SPECIAL_EXC_STACK_BASE = 8;
    localparam int SPECIAL_MODE = 11;
    localparam int SPECIAL_PERFORMANCE_COUNTER = 13;

    // Mode at reset: Exception=0, Privileged=0, NonPrivileged=1 (isa::Mode's default).
    localparam word_t MODE_RESET = 32'h0000_0004;

    // Vector-table word offsets, matching isa::VectorTable.
    localparam logic [2:0] BOOT_STACK_INITIAL = 3'd0;
    localparam logic [2:0] BOOT_STACK_BOUNDARY = 3'd1;
    localparam logic [2:0] BOOT_EXC_STACK_INITIAL = 3'd2;
    localparam logic [2:0] BOOT_EXC_STACK_BOUNDARY = 3'd3;
    localparam logic [2:0] BOOT_RESET_HANDLER = 3'd4;

    typedef enum logic [1:0] {
        S_BOOT,
        S_FETCH,
        S_COMMIT
    } state_e;

    state_e     state;
    logic [2:0] boot_index;

    assign booted = (state != S_BOOT);

    always_comb begin
        mem_req   = (state == S_BOOT) || (state == S_FETCH);
        mem_write = 1'b0;
        mem_wdata = '0;
        mem_wstrb = '0;

        if (state == S_BOOT) begin
            mem_addr = DEFAULT_VECTOR_TABLE_ADDRESS + (address_t'(boot_index) << 2);
        end else if (state == S_FETCH) begin
            mem_addr = special[SPECIAL_PA];
        end else begin
            mem_addr = '0;
        end
    end

    always_ff @(posedge clk) begin
        retire <= 1'b0;

        if (reset) begin
            state      <= S_BOOT;
            boot_index <= '0;
            halted     <= 1'b0;

            for (int i = 0; i < SCRATCH_COUNT; i++) begin
                scratch[i] <= ERASED_WORD;
            end
            for (int i = 0; i < EVAL_COUNT; i++) begin
                evaluation[i] <= '1;
            end
            for (int i = 0; i < 16; i++) begin
                special[i] <= '0;
            end
            special[SPECIAL_VTA]  <= DEFAULT_VECTOR_TABLE_ADDRESS;
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
                                special[SPECIAL_EXC_STACK_LIMIT]   <= mem_rdata;
                                special[SPECIAL_EXC_STACK_CURRENT] <= mem_rdata;
                            end
                            BOOT_EXC_STACK_BOUNDARY: begin
                                special[SPECIAL_EXC_STACK_BASE] <= mem_rdata;
                            end
                            default: begin
                                special[SPECIAL_PA] <= mem_rdata;
                                special[SPECIAL_RA] <= mem_rdata;
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
                    // M0 stub: an unfetchable word behaves like the C++ noop fallback.
                    state <= S_COMMIT;
                end

                S_COMMIT: begin
                    special[SPECIAL_PA]                  <= special[SPECIAL_PA] + 32'd4;
                    special[SPECIAL_PERFORMANCE_COUNTER] <= special[SPECIAL_PERFORMANCE_COUNTER] + 32'd1;
                    retire                               <= 1'b1;
                    state                                <= S_FETCH;
                end

                default: state <= S_FETCH;
            endcase
        end
    end

endmodule
