// Arithmetic logic unit: a purely combinational mirror of the arithmetic in
// Processor::Cycle (source/cpu.cpp, include/isa/arithmetic.hpp) -- add, sub, mul,
// div, mod, neg and sext/zext. The pipeline stage registers the outputs.
//
// Operands are the low `size` bits of `a` and `b`, read as signed or unsigned per
// `sign`. NEG is always signed (`0 - a`, `b` is ignored) and EXTEND widens the low
// `size` bits of `a` to the whole word (`b` is ignored).
//
// `result` holds the answer in its low `size` bits and zero above; the instruction
// only writes those low bits of the destination, so the writer merges them.
// EXTEND is the exception: its result is the whole word.
//
// `eva` is the evaluation the instruction's `cond` writes. Arithmetic flags come from
// the exact (wider) result: `overflow` when it does not fit the size, `positive` when
// it is strictly greater than zero. With `sat` the result is clamped instead of wrapped,
// though the flags still describe the exact result. EXTEND reports the sign and zero of
// the extended word, like the bitwise operations.
//
// `fault` asks for an instruction fault, in which case nothing is written: division or
// modulo by zero (`eva.arithmetic.undefined` is set too), or a size the operation does
// not support. `valid` is low for an opcode this unit does not execute.
module alu
    import isa::*;
(
    input  op_e         op,
    input  size_e       size,
    input  logic        sign,
    input  logic        sat,
    input  word_t       a,
    input  word_t       b,
    output word_t       result,
    output evaluation_t eva,
    output logic        fault,
    output logic        valid
);
    int    bits;
    logic  size_ok;
    logic  is_signed;
    wide_t wide_a, wide_b;
    wide_t min_value, max_value;

    always_comb begin
        bits      = size_bits(size);
        size_ok   = size_is_operand(size);
        is_signed = sign || (op == OP_NEGATE);
        min_value = range_min(bits, is_signed);
        max_value = range_max(bits, is_signed);
    end

    operand_extend u_extend_a (.value(a), .bits(bits), .is_signed(is_signed), .extended(wide_a));
    operand_extend u_extend_b (.value(b), .bits(bits), .is_signed(is_signed), .extended(wide_b));

    wide_t wide;       // the exact result before wrapping or saturating
    logic  undefined;  // division or modulo by zero
    logic  min_by_neg_one;

    always_comb begin
        wide           = '0;
        undefined      = 1'b0;
        valid          = 1'b1;
        min_by_neg_one = is_signed && (wide_a == min_value) && (wide_b == wide_t'(-1));

        case (op)
            OP_ADDITION: wide = wide_a + wide_b;
            OP_SUBTRACT: wide = wide_a - wide_b;
            OP_MULTIPLY: wide = wide_a * wide_b;
            OP_NEGATE:   wide = -wide_a;
            OP_DIVIDE: begin
                if (wide_b == '0) begin
                    undefined = 1'b1;
                end else if (min_by_neg_one) begin
                    wide = max_value;
                end else begin
                    wide = wide_a / wide_b;
                end
            end
            OP_MODULO: begin
                if (wide_b == '0) begin
                    undefined = 1'b1;
                end else if (!(is_signed && wide_b == wide_t'(-1))) begin
                    wide = wide_a % wide_b;
                end
            end
            OP_EXTEND: wide = wide_a;
            default:   valid = 1'b0;
        endcase
    end

    logic is_extend;
    assign is_extend = (op == OP_EXTEND);

    logic saturating;  // only add, sub, mul and neg carry a saturate flag
    assign saturating = sat && (op == OP_ADDITION || op == OP_SUBTRACT || op == OP_MULTIPLY || op == OP_NEGATE);

    wide_t clamped;
    always_comb begin
        clamped = wide;
        if (saturating) begin
            if (wide < min_value) begin
                clamped = min_value;
            end else if (wide > max_value) begin
                clamped = max_value;
            end
        end
    end

    always_comb begin
        // Extend widens to the word, so its size is the source and not the destination.
        fault = !valid ? 1'b0 : (!size_ok || undefined || (is_extend && size == SIZE_WORD));

        result = '0;
        if (is_extend) begin
            result = wide[WORD_SIZE-1:0];
        end else begin
            for (int i = 0; i < WORD_SIZE; i++) begin
                result[i] = (i < bits) ? clamped[i] : 1'b0;
            end
        end

        eva                      = '0;
        eva.arithmetic.type_     = EVAL_TYPE_ARITHMETIC;
        eva.arithmetic.undefined = valid && undefined;
        if (is_extend) begin
            eva.arithmetic.positive = !result[WORD_SIZE-1];
            eva.arithmetic.zero     = (result == '0);
        end else if (valid && !undefined) begin
            eva.arithmetic.positive = (wide > 0);
            eva.arithmetic.zero     = (wide == 0);
            eva.arithmetic.overflow = (wide < min_value) || (wide > max_value) || (op == OP_DIVIDE && min_by_neg_one);
        end
    end
endmodule
