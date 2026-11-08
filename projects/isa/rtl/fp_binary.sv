/* verilator lint_off WIDTHEXPAND */
/* verilator lint_off WIDTHTRUNC */

import fp_pkg::*;

// Stage 4 of M11: IEEE-754 binary32 addition.
module fp_add (
    input  logic [31:0] a,
    input  logic [31:0] b,
    output logic [31:0] result,
    output logic       inexact,
    output logic       rounded,
    output logic       subnormal,
    output logic       underflow
);
    real ra, rb, rexact;
    logic [31:0] rbits;

    always_comb begin
        ra = binary32_to_real(a);   // widen: binary32 -> real (double)
        rb = binary32_to_real(b);   // widen: binary32 -> real (double)
        rexact = ra + rb;          // exact operation in double precision
        rbits = real_to_binary32(rexact); // narrow: real (double) -> binary32

        // NaN: propagate NaN, no flags.
        if (ra != ra || rb != rb) begin
            result = FP_NAN;
            inexact = 1'b0;
            rounded = 1'b0;
            subnormal = 1'b0;
            underflow = 1'b0;
        end else begin
            result = rbits;
            // inexact if the rounded float differs from the exact real result.
            if (rbits == FP_POS_INF || rbits == FP_NEG_INF) begin
                // infinity result: inexact only if inputs were finite (overflow)
                inexact = (fp_exponent(a) != 8'hff && fp_exponent(b) != 8'hff) ? 1'b1 : 1'b0;
            end else begin
                inexact = (binary32_to_real(rbits) != rexact) ? 1'b1 : 1'b0;
            end
            rounded = inexact;
            subnormal = (fp_exponent(rbits) == 8'd0 && fp_mantissa(rbits) != 0) ? 1'b1 : 1'b0;
            underflow = subnormal && inexact;
        end
    end
endmodule

// Stage 4 of M11: IEEE-754 binary32 subtraction.
module fp_sub (
    input  logic [31:0] a,
    input  logic [31:0] b,
    output logic [31:0] result,
    output logic       inexact,
    output logic       rounded,
    output logic       subnormal,
    output logic       underflow
);
    real ra, rb, rexact;
    logic [31:0] rbits;

    always_comb begin
        ra = binary32_to_real(a);   // widen: binary32 -> real (double)
        rb = binary32_to_real(b);   // widen: binary32 -> real (double)
        rexact = ra - rb;          // exact operation in double precision
        rbits = real_to_binary32(rexact); // narrow: real (double) -> binary32

        // NaN: propagate NaN, no flags.
        if (ra != ra || rb != rb) begin
            result = FP_NAN;
            inexact = 1'b0;
            rounded = 1'b0;
            subnormal = 1'b0;
            underflow = 1'b0;
        end else begin
            result = rbits;
            if (rbits == FP_POS_INF || rbits == FP_NEG_INF) begin
                inexact = (fp_exponent(a) != 8'hff && fp_exponent(b) != 8'hff) ? 1'b1 : 1'b0;
            end else begin
                inexact = (binary32_to_real(rbits) != rexact) ? 1'b1 : 1'b0;
            end
            rounded = inexact;
            subnormal = (fp_exponent(rbits) == 8'd0 && fp_mantissa(rbits) != 0) ? 1'b1 : 1'b0;
            underflow = subnormal && inexact;
        end
    end
endmodule