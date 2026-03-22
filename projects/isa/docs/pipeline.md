# Pipeline

The CPU has a 4 stage pipeline: Fetch, Decode, Execute, and Writeback. Each stage performs a specific part of the instruction processing, allowing for multiple instructions to be processed simultaneously in different stages of the pipeline. This improves the overall performance and throughput of the CPU by allowing it to work on multiple instructions at the same time.

## Stages

### Fetch

In the Fetch stage, the CPU retrieves the instruction from memory at the address specified by the program address (PA). The instruction is then stored in an instruction register for further processing in the next stages.

In this stage, the CPU will attempt to fetch the instruction from the current program address, through the bus interface. If the fetch is successful, the instruction will be stored in the instruction register and the program address will be updated to point to the next instruction. If there is a bus fault during the fetch, such as an invalid memory access, a bus fault exception will be raised and the CPU will transition to the exception handling flow.

### Decode

