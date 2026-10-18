// Extracts the effective operation of an instruction word.
// The erased word decodes as a noop, a set bit 0 selects the immediate leap form, and
// anything that is not a defined opcode is flagged as invalid.
module decoder
    import isa::*;
(
    input  word_t instruction,
    output op_e   op,
    output logic  valid
);

    instruction_t decoded;
    always_comb begin
        decoded = instruction;
        valid = 1'b1;
        if (instruction == ERASED_WORD) begin
            op = OP_NONE;
        end else if (decoded.leap_immediate.immediate) begin
            op = OP_LEAP;
        end else begin
            op = decoded.noop.op;
            valid = is_defined_op(op);
        end
    end

endmodule
