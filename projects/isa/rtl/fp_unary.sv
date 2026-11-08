import fp_pkg::*;

// Stage 1
module fp_abs (
    input  logic [31:0] in,
    output logic [31:0] out
);
    assign out = {1'b0, in[30:0]};
endmodule

// Stage 2
module fp_neg (
    input  logic [31:0] in,
    output logic [31:0] out
);
    assign out = fp_neg_bits(in);
endmodule

// Stage 3
module fp_floor (
    input  logic [31:0] in,
    output logic [31:0] out
);
    always_comb out = fp_floor_bits(in);
endmodule

module fp_ceil (
    input  logic [31:0] in,
    output logic [31:0] out
);
    always_comb out = fp_ceil_bits(in);
endmodule

module fp_fractional (
    input  logic [31:0] in,
    output logic [31:0] out
);
    always_comb out = fp_fractional_bits(in);
endmodule

// Stage 3 convert
module fp_convert (
    input  logic [31:0] in,
    input  logic        dir,
    output logic [31:0] out
);
    always_comb begin
        real r;
        r = 0.0;
        out = in;
        if (dir == 1'b0) begin
            out = real_to_binary32(signed'(in));
        end else begin
            r = binary32_to_real(in);
            if (r != r) out = 32'd0; // NaN -> 0
            else if (r >= 2147483648.0) out = 32'h7fffffff;
            else if (r <= -2147483648.0) out = 32'h80000000;
            else out = 32'(signed'($rtoi(r)));
        end
    end
endmodule