import isa::*;

module evaluation_file(
    input logic clock,
    input logic reset,
    input evaluation_control_t write_control,
    input evaluation_index_t write_index[0:1],
    input byte_t write_data[0:1],
    input evaluation_control_t read_control,
    input evaluation_index_t read_index[0:1],
    output byte_t read_data[0:1]
);
    byte_t evaluation [(EVAL_COUNT-1):0];
    always_ff @(posedge clock or posedge reset) begin
        if (reset) begin
            for (int i = 0; i < EVAL_COUNT; i++) begin
                evaluation[i] <= '0;
            end
        end else begin
            case (write_control)
                EVALUATION_CONTROL_FIRST: begin
                    evaluation[write_index[0]] <= write_data[0];
                end
                EVALUATION_CONTROL_SECOND: begin
                    evaluation[write_index[1]] <= write_data[1];
                end
                EVALUATION_CONTROL_BOTH: begin
                    if (write_index[0] != write_index[1]) begin
                        evaluation[write_index[0]] <= write_data[0];
                        evaluation[write_index[1]] <= write_data[1];
                    end else begin
                        evaluation[write_index[0]] <= write_data[0];
                    end
                end
                default: begin
                    // Do nothing
                end
            endcase
        end
    end
    always_comb begin
        read_data[0] = '0;
        read_data[1] = '0;
        if (!reset) begin
            case (read_control)
                EVALUATION_CONTROL_FIRST: begin
                    read_data[0] = evaluation[read_index[0]];
                end
                EVALUATION_CONTROL_SECOND: begin
                    read_data[1] = evaluation[read_index[1]];
                end
                EVALUATION_CONTROL_BOTH: begin
                    read_data[0] = evaluation[read_index[0]];
                    read_data[1] = evaluation[read_index[1]];
                end
                default: begin
                    // Both outputs stay zero
                end
            endcase
        end
    end
endmodule

module register_file(
    input logic clock,
    input logic reset,
    input scratch_control_t write_control,
    input scratch_index_t write_index[0:1],
    input word_t write_data[0:1],
    input scratch_control_t read_control,
    input scratch_index_t read_index[0:1],
    output word_t read_data[0:1]
);
    reg [WORD_SIZE-1:0] scratch [(SCRATCH_COUNT-1):0];
    always_ff @(posedge clock or posedge reset) begin
        if (reset) begin
            for (int i = 0; i < SCRATCH_COUNT; i++) begin
                scratch[i] <= '0;
            end
        end else begin
            case (write_control)
                SCRATCH_CONTROL_FIRST: begin
                    scratch[write_index[0]] <= write_data[0];
                end
                SCRATCH_CONTROL_SECOND: begin
                    scratch[write_index[1]] <= write_data[1];
                end
                SCRATCH_CONTROL_BOTH: begin
                    if (write_index[0] != write_index[1]) begin
                        scratch[write_index[0]] <= write_data[0];
                        scratch[write_index[1]] <= write_data[1];
                    end else begin
                        scratch[write_index[0]] <= write_data[0];
                    end
                end
                default: begin
                    // Do nothing
                end
            endcase
        end
    end
    always_comb begin
        if (reset) begin
            read_data[0] = '0;
            read_data[1] = '0;
        end else begin
            case (read_control)
                SCRATCH_CONTROL_FIRST: begin
                    read_data[0] = scratch[read_index[0]];
                    read_data[1] = '0;
                end
                SCRATCH_CONTROL_SECOND: begin
                    read_data[0] = '0;
                    read_data[1] = scratch[read_index[1]];
                end
                SCRATCH_CONTROL_BOTH: begin
                    read_data[0] = scratch[read_index[0]];
                    read_data[1] = scratch[read_index[1]];
                end
                default: begin
                    read_data[0] = '0;
                    read_data[1] = '0;
                end
            endcase
        end
    end
endmodule
