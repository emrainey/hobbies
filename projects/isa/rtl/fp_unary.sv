/* verilator lint_off WIDTHEXPAND */
/* verilator lint_off WIDTHTRUNC */
// Floating-point unary operations for M11.
// Implemented with explicit IEEE-754 binary32 bit manipulation so the modules
// are synthesizable and do not depend on Verilator's shortreal support.

package fp_pkg;

    function automatic real binary32_to_real(input logic [31:0] b);
        logic sign;
        int   expv;
        int   mant;
        real  result;
        sign = b[31];
        expv = b[30:23];
        mant = b[22:0];
        if (expv == 255) begin
            if (mant == 0) result = sign ? -1.0/0.0 : 1.0/0.0;
            else result = 0.0/0.0;
        end else if (expv == 0) begin
            result = (mant / 8388608.0) * (2.0 ** -126);
            if (sign) result = -result;
        end else begin
            result = (1.0 + mant / 8388608.0) * (2.0 ** (expv - 127));
            if (sign) result = -result;
        end
        return result;
    endfunction

    function automatic logic [31:0] real_to_binary32(input real r);
        logic sign;
        logic [63:0] dbits;
        int dexp;
        longint unsigned dmant;
        longint unsigned full;
        int fexp;
        longint unsigned m;
        longint unsigned rem;
        int shift;
        if (r != r) return 32'h7fc00000; // NaN
        dbits = $realtobits(r);
        sign = dbits[63];
        if (r == 0.0) return {sign, 31'b0}; // signed zero
        dbits = $realtobits(sign ? -r : r);
        dexp = dbits[62:52];
        dmant = dbits[51:0];
        if (dexp == 2047) return {sign, 8'hff, 23'b0}; // infinity
        if (dexp == 0) return {sign, 31'b0}; // double subnormal -> 0
        fexp = dexp - 1023 + 127;
        if (fexp >= 255) return {sign, 8'hff, 23'b0}; // overflow
        if (fexp <= 0) return {sign, 31'b0}; // underflow
        full = (64'd1 << 52) | dmant;
        shift = 52 - 23;
        rem = full & ((64'd1 << shift) - 1);
        m = full >> shift;
        if (rem > (64'd1 << (shift - 1)) ||
            (rem == (64'd1 << (shift - 1)) && m[0])) begin
            m = m + 1;
            if (m == (64'd1 << 24)) begin
                m = 64'd1 << 23;
                fexp = fexp + 1;
                if (fexp >= 255) return {sign, 8'hff, 23'b0};
            end
        end
        return {sign, fexp[7:0], m[22:0]};
    endfunction

    function automatic logic [31:0] fp_neg_bits(input logic [31:0] in);
        return {~in[31], in[30:0]};
    endfunction

    function automatic logic [31:0] fp_floor_bits(input logic [31:0] in);
        logic sign;
        int   expv;
        int   mant;
        int   e;
        int   mask;
        int   new_mant;
        sign = in[31];
        expv = in[30:23];
        mant = in[22:0];
        if (expv == 255) return in; // NaN / infinity
        if (expv == 0) begin
            if (sign && mant != 0) return 32'hBF800000; // -1.0
            return {sign, 31'b0}; // signed zero
        end
        if (expv < 127) begin
            if (sign) return 32'hBF800000; // -1.0
            else return 32'h00000000;
        end
        if (expv >= 127 + 23) return in; // already an integer
        e = expv - 127;
        mask = (1 << (23 - e)) - 1;
        if (sign) begin
            if ((mant & mask) != 0) begin
                new_mant = mant + (1 << (23 - e));
                if (new_mant[23]) begin
                    mant = 0;
                    expv = expv + 1;
                end else begin
                    mant = new_mant;
                end
            end
            mant = mant & ~mask;
        end else begin
            mant = mant & ~mask;
        end
        return {sign, expv[7:0], mant[22:0]};
    endfunction

    function automatic logic [31:0] fp_ceil_bits(input logic [31:0] in);
        return fp_neg_bits(fp_floor_bits(fp_neg_bits(in)));
    endfunction

    function automatic logic [31:0] fp_fractional_bits(input logic [31:0] in);
        real x, f;
        x = binary32_to_real(in);
        f = binary32_to_real(fp_floor_bits(in));
        return real_to_binary32(x - f);
    endfunction

endpackage

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
            // signed int -> float
            out = real_to_binary32(signed'(in));
        end else begin
            // float -> signed int (truncate, saturate, NaN -> 0)
            r = binary32_to_real(in);
            if (r != r) out = 32'd0;
            else if (r >= 2147483648.0) out = 32'h7fffffff;
            else if (r <= -2147483648.0) out = 32'h80000000;
            else out = 32'(signed'($rtoi(r)));
        end
    end
endmodule