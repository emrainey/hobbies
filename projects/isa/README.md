# Instruction Set Architecture (ISA) Project

## Design

### Overview

The ISA is designed to be a simple, RISC-like architecture with a focus on ease of implementation and understanding. It is intended to be used as a learning tool for those interested in computer architecture and assembly language programming.

### Features

* 16 Scratch Registers
* 16 Evaluation Registers
* 32-bit Instruction Word
* Simple Bus Emulation
    * L1 Instruction Cache (WIP)
    * L1 Data Cache (WIP)
    * Flash
    * SRAM
* Basic Arithmetic
    * Addition, Subtraction, Multiplication, Division, Modulo
* Floating Point Arithmetic
    * Addition, Subtraction, Multiplication, Division, Modulo, Abs, Floor, Ceil, Negate, etc.
* Logic Instructions
    * AND, OR, XOR, NOT
    * Complement, Negate
    * Shift Left/Right (Logical and Arithmetic)
* Comparisons
    * Logical Compare (Equal, Not Equal, Less Than, Greater Than, etc.)
    * Arithmetic Compare (Zero, Overflow, Underflow, etc.)
    * Floating Point Compare (Zero, Overflow, Underflow, etc.)
* Data Movement like Load/Save/Move/Copy/Swap
* Control Flow like Leap/Back/Halt
* System Calls and Exception Testing (Call/Trig)
* Simple Fault Handling (Vector Table, Safe Handler)
* Stack Bounds Checking on each alteration of the Stack Current Address
* Special Registers for:
    * Stack Base, Current and Limit Address (Exception variants)
    * Program Address and Return Address
    * Interrupt Enable and Flags
    * Perf Counter

### Future Plans

* Bus Cycle Emulation
* CPU Pipeline Emulation