# FIT3143 One-Page Summary: Parallel Computing & Computer Architecture

## Topic 1: Introduction to Parallel Computing

**Parallel vs Distributed:** Parallel computing divides *the same task* to run concurrently across cores/components (one chip, one machine, or a network). Distributed computing runs the same or different tasks concurrently across cores spread over a network. Clusters and clouds are typically both.

**Flynn's Taxonomy (instruction streams × data streams):**
| Type | Instruction Streams | Data Streams | Example |
|---|---|---|---|
| SISD | Single | Single | Classic single-core CPU |
| SIMD | Single | Multiple | GPU shaders, vector units |
| MISD | Multiple | Single | Rare (e.g. fault-tolerant systems) |
| MIMD | Multiple | Multiple | Multi-core CPUs, clusters |

**Memory Architectures:**
- **Shared memory:** All processors access one global address space; a write by one is visible to all. Sub-classed as **UMA** (Uniform Memory Access) or **NUMA** (Non-Uniform Memory Access) by access-time uniformity.
- **Distributed memory:** Each processor has local memory only; no global address space, no cache coherency requirement; needs a communication network.
- **Hybrid:** Combines both (e.g. clusters of shared-memory SMP nodes).

**Amdahl's Law:** Speedup = 1 / (S + P/N), where S = serial fraction, P = parallel fraction, N = processors. The serial portion always caps maximum achievable speedup, even as N → ∞.

## Topic 2: Inter-Process Communication (IPC)

| Mechanism | Scope | Message Structure |
|---|---|---|
| Shared Memory | Single host only | None imposed |
| Signals | Single host | Discrete, structured |
| Message Passing | Single host | Discrete, structured |
| Sockets (BSD API) | Local or networked | Depends on protocol |
| ONC RPC | Networked | Structured procedure calls via stubs |

**TCP vs UDP:** TCP is connection-oriented and reliable (guarantees delivery/order); UDP is connectionless and unreliable, with lower overhead. Both originated in the 1980s for dumb-terminal and file-transfer traffic, and can underperform in modern high-speed data centre fabrics. A stream connection is only "reliable" when using TCP; UDP connections between hosts are always "unreliable."

**Sockets:** The BSD Socket API (`socket()`, `bind()`, `connect()`) is the defacto standard low-level networked IPC interface, sitting above the OSI Transport Layer.

## Topic 3: Pipelining & Superscalar Processing

**Pipelining fundamentals** (S = number of stages, T = stage/phase duration):
- **Latency** (time for one instruction, start to finish) = S × T — the *same* for pipelined and non-pipelined designs.
- **Pipelined throughput** = 1 / T (a new result every stage-cycle, once the pipeline is full).
- **Non-pipelined throughput** = 1 / (S × T) (must fully finish one instruction before starting the next).

**Data hazards & stalls:** A stall occurs when an instruction needs a result not yet ready. After a stall of *n* extra phases, that instruction's effective cycle time = T + (n × T). Once a stall ends, it takes at least one latency period before the CPU resumes completing instructions every phase. Solutions: avoid via compiler reordering, stall the pipeline, or use **data forwarding** (routes results directly between stages, extra hardware/control complexity required, but removes the stall entirely, restoring baseline throughput of 1/T).

**Superscalar CPUs** dispatch multiple instructions per cycle across multiple Execution Units (EUs), limited by **four performance-limiting factors:**
1. **Data (output) dependency** — an instruction needs a result from another.
2. **Procedural dependency** — instruction order/control flow (e.g. branches) constrains execution.
3. **Resource conflicts** — multiple instructions need the same EU/register/bus; reducible (not eliminable for free) by adding more hardware.
4. **Anti-dependency** — a later instruction would overwrite a register value an earlier instruction still needs to read; specific to out-of-order execution, not an issue in strictly in-order pipelines.

Concurrent execution in a given cycle is capped per instruction-type by however many EUs of that type exist (e.g. 3 FP instructions issued but only 2 FP EUs → only 2 execute that cycle).
