// Bitwise logic unit: a purely combinational mirror of the bitwise ops in
// Processor::Cycle (source/cpu.cpp). The pipeline stage registers `result`.
//
// `b` is the second source for AND/OR/XOR and the shift amount / bit index for the
// one-source operations (only its low SHIFT_W bits are used, like the instruction's
// shift field). `valid` is low for an undefined operation or a REVERSE width above
// word, which the processor reports as an instruction fault.
//
// `evaluation` is what the instruction's `cond` writes: an arithmetic evaluation
// carrying the sign and zero state of the result (never overflow), matching
// BitwiseEvaluation() in the C++ model.
module bitwise_logic_unit
    import isa::*;
(
    input  op_e   opcode,
    input  word_t a,
    input  word_t b,
    output word_t result,
    output byte_t evaluation,
    output logic  valid
);
    logic [SHIFT_W-1:0] shift;
    assign shift = b[SHIFT_W-1:0];

    // Reverse the bits within each unit of `unit_bits` bits.
    function automatic word_t reverse_units(word_t value, int unit_bits);
        word_t reversed;
        reversed = '0;
        for (int base = 0; base < WORD_SIZE; base += unit_bits) begin
            for (int bit_index = 0; bit_index < unit_bits; bit_index++) begin
                reversed[base+unit_bits-1-bit_index] = value[base+bit_index];
            end
        end
        return reversed;
    endfunction

    function automatic word_t count_leading_zeros(word_t value);
        int count;
        count = 0;
        while (count < WORD_SIZE && !value[WORD_SIZE-1-count]) begin
            count++;
        end
        return word_t'(count);
    endfunction

    always_comb begin
        result = '0;
        valid  = 1'b1;

        case (opcode)
            OP_COMPLEMENT: result = ~a;
            OP_RSH:        result = a >> shift;
            OP_ASR:        result = word_t'($signed(a) >>> shift);
            OP_LSH:        result = a << shift;
            OP_ROTATE:     result = (shift == '0) ? a : ((a << shift) | (a >> (WORD_SIZE - int'(shift))));
            OP_COUNT1S:    result = word_t'($countones(a));
            OP_COUNT_L0S:  result = count_leading_zeros(a);
            OP_NOT:        result = (a == '0) ? '1 : '0;
            OP_SET_BIT:    result = a | (word_t'(1) << shift);
            OP_CLEAR_BIT:  result = a & ~(word_t'(1) << shift);
            OP_TOGGLE_BIT: result = a ^ (word_t'(1) << shift);
            OP_REVERSE: begin
                if (shift > 2) begin
                    valid = 1'b0;
                end else begin
                    result = reverse_units(a, 8 << shift);
                end
            end
            OP_AND:        result = a & b;
            OP_OR:         result = a | b;
            OP_XOR:        result = a ^ b;
            default:       valid = 1'b0;
        endcase
    end

    // Arithmetic evaluation of the result: sign and zero only, never overflow.
    evaluation_t eval_view;
    always_comb begin
        eval_view                   = '0;
        eval_view.arithmetic.type_    = EVAL_TYPE_ARITHMETIC;
        eval_view.arithmetic.positive = !result[WORD_SIZE-1];
        eval_view.arithmetic.zero     = (result == '0);
        evaluation                  = eval_view.raw;
    end
endmodule
