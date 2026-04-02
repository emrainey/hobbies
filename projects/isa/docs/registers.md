# Register File

The Register File is composed of two distinct types of registers: Scratch Registers and Evaluation Registers.

## Scratch Registers

The scratch registers are general-purpose registers used for temporary storage during instruction execution. There are 16 scratch registers, named `s0-s15`. Each register is a processor word sized and can hold any data type that fits within that size.

```cpp
namespace registers {
union word {
    constexpr word() : u32(0) {}

    uint32_t u32;
    int32_t  i32;
    float    f32;
    address  addr;
};
static_assert(sizeof(word) == 4, "word must be 4 bytes");

word scratch[16];

}  // namespace registers
```

Each register index in an instruction only takes up 4 bits then to index 0-15 inclusive.

## Special Registers

### Stack

* SLA - Stack Limit Address (aligned by bus limit)
* SCA - Stack Current address (aligned by bus limit)
* SBA - Stack Base Address (aligned by bus limit)

### Function

* RA - return address (aligned by 4)
* PA - program address (aligned by 4)

### Control of Processor

* MD - mode register (various flags)

### Exception Handling

* ETA - exception table address (aligned by 4)
* ESBA - exception stack base address (aligned by bus limit)
* ESCA - exception stack current address (aligned by bus limit)
* ESLA - exception stack limit address (aligned by bus limit)

## Evaluation Registers

The evaluation registers are used to hold intermediate results of comparison results which can then be used to make conditional logic from. Instead of a CPU needing to have _one_ carry bit, _one_ zero bit, _one_ etc, _each_ evaluation register contains one.

@ref Evaluation
