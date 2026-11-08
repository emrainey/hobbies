/* verilator lint_off WIDTHEXPAND */
/* verilator lint_off WIDTHTRUNC */

// Common IEEE-754 binary32 helpers shared by the unary and binary FP units.
package fp_pkg;

    // Named special values (binary32).
    localparam logic [31:0] FP_NAN      = 32'h7fc00000;
    localparam logic [31:0] FP_POS_INF  = 32'h7f800000;
    localparam logic [31:0] FP_NEG_INF  = 32'hff800000;
    localparam logic [31:0] FP_POS_ZERO = 32'h00000000;
    localparam logic [31:0] FP_NEG_ZERO = 32'h80000000;


    function automatic logic        fp_sign(input logic [31:0] x);
        return x[31];
    endfunction

    function automatic logic [7:0]  fp_exponent(input logic [31:0] x);
        return x[30:23];
    endfunction

    function automatic logic [22:0] fp_mantissa(input logic [31:0] x);
        return x[22:0];
    endfunction

    function automatic real binary32_to_real(input logic [31:0] b);
        logic sign;
        int   expv;
        int   mant;
        real  result;
        sign = fp_sign(b);
        expv = fp_exponent(b);
        mant = fp_mantissa(b);
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
        if (r != r) return FP_NAN;
        dbits = $realtobits(r);
        sign = dbits[63];
        if (r == 0.0) return {sign, 31'b0};
        dbits = $realtobits(sign ? -r : r);
        dexp = dbits[62:52];
        dmant = dbits[51:0];
        if (dexp == 2047) return {sign, 8'hff, 23'b0};
        if (dexp == 0) return {sign, 31'b0};
        fexp = dexp - 1023 + 127;
        if (fexp >= 255) return {sign, 8'hff, 23'b0};
        if (fexp <= 0) return {sign, 31'b0};
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
        return {~fp_sign(in), in[30:0]};
    endfunction

    function automatic logic [31:0] fp_floor_bits(input logic [31:0] in);
        logic sign;
        int   expv;
        int   mant;
        int   e;
        int   mask;
        int   new_mant;
        sign = fp_sign(in);
        expv = fp_exponent(in);
        mant = fp_mantissa(in);
        if (expv == 255) return in;
        if (expv == 0) begin
            if (sign && mant != 0) return 32'hBF800000;
            return {sign, 31'b0};
        end
        if (expv < 127) begin
            if (sign) return 32'hBF800000;
            else return 32'h00000000;
        end
        if (expv >= 127 + 23) return in;
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