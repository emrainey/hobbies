# Processor FSM

Mermaid view of the serialized processor FSM in `rtl/processor.sv`. Update this diagram whenever the FSM changes.

```mermaid
stateDiagram-v2
    [*] --> S_BOOT_REQUEST : reset
    S_BOOT_REQUEST --> S_BOOT_RESPONSE : mem_req_ready
    S_BOOT_RESPONSE --> S_BOOT_REQUEST : boot_index != BOOT_RESET_HANDLER
    S_BOOT_RESPONSE --> S_FETCH_REQUEST : boot_index == BOOT_RESET_HANDLER

    S_FETCH_REQUEST --> S_FETCH_RESPONSE : mem_req_ready
    S_FETCH_RESPONSE --> S_DECODE : mem_rsp_valid && mem_rsp_ready
    S_DECODE --> S_COMMIT : (always)

    S_COMMIT --> S_LOAD_RESPONSE : commit_valid && OP_LOAD && mem_req_ready
    S_COMMIT --> S_SAVE_RESPONSE : commit_valid && OP_SAVE && mem_req_ready
    S_COMMIT --> S_SWAP_READ_RESPONSE : commit_valid && OP_SWAP && mem_req_ready
    S_COMMIT --> S_FETCH_REQUEST : otherwise (retire, or redirect on exception)

    S_LOAD_RESPONSE --> S_FETCH_REQUEST : mem_rsp_valid && mem_rsp_ready (retire or redirect)
    S_SAVE_RESPONSE --> S_FETCH_REQUEST : mem_rsp_valid && mem_rsp_ready (retire or redirect)

    S_SWAP_READ_RESPONSE --> S_SWAP_WRITE_REQUEST : read response OK
    S_SWAP_READ_RESPONSE --> S_FETCH_REQUEST : read response faults (retire or redirect)
    S_SWAP_WRITE_REQUEST --> S_SWAP_WRITE_RESPONSE : mem_req_ready
    S_SWAP_WRITE_RESPONSE --> S_FETCH_REQUEST : mem_rsp_valid && mem_rsp_ready (retire or redirect)

    S_COMMIT --> S_VECTOR_REQUEST : exception pending
    S_LOAD_RESPONSE --> S_VECTOR_REQUEST : exception pending
    S_SAVE_RESPONSE --> S_VECTOR_REQUEST : exception pending
    S_SWAP_READ_RESPONSE --> S_VECTOR_REQUEST : exception pending
    S_SWAP_WRITE_RESPONSE --> S_VECTOR_REQUEST : exception pending

    S_VECTOR_REQUEST --> S_VECTOR_RESPONSE : mem_req_ready
    S_VECTOR_RESPONSE --> S_FETCH_REQUEST : handler read OK
    S_VECTOR_RESPONSE --> S_VECTOR_REQUEST : handler read faults (retry with reset handler)
    S_VECTOR_RESPONSE --> S_HALTED : reset handler read faults

    S_COMMIT --> S_HALTED : OP_HALT
    S_HALTED --> S_HALTED : (hold until reset)
```

Notes:

- Reset boots from the vector table through `S_BOOT_REQUEST`/`S_BOOT_RESPONSE`.
- `S_COMMIT` issues bus requests for `load`, `save`, and `swap`; other instructions retire in the same edge.
- `swap` performs read then write; `Sa` commits only on a successful write response.
- `S_VECTOR_*` services exception handler fetches.
- `halted` holds in `S_HALTED` until reset.