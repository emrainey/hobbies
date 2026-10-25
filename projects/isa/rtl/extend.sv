// Operand extender: keeps the low `bits` bits of `value` and widens them to `wide_t`
// with the sign bit (when `is_signed`) or with zeros. Purely combinational.
module operand_extend
    import isa::*;
(
    input  word_t value,
    input  int    bits,
    input  logic  is_signed,
    output wide_t extended
);
    logic [WIDE_SIZE-1:0] raw;
    logic                 fill;
    assign raw  = WIDE_SIZE'(value);
    assign fill = is_signed && value[bits-1];

    always_comb begin
        for (int i = 0; i < WIDE_SIZE; i++) begin
            extended[i] = (i < bits) ? raw[i] : fill;
        end
    end
endmodule
