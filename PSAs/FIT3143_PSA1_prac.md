# FIT3143 Practice Test: Parallel Computing and Computer Architecture

This practice test mirrors the style and topics of the questions covered earlier: IPC and shared memory, parallel architectures (SISD/SIMD/MISD/MIMD), SIMD computation timing, distributed process/memory calculations, TCP/UDP data transmission, pipelining, data hazards, and superscalar execution.

Try to answer each question before checking the answer key at the end.

---

## Question 1: IPC and Shared Memory

Alice has implemented a distributed logging program on a Linux machine with 8 processing cores. The program creates 8 processes, communicating through shared memory IPC. Every second, all processes send individual messages to all other processes (excluding itself).

Which of the following statements are correct?


A) If each message is 2 kilobytes long, the total memory needed for the program will be 2 kilobytes.

B) If each message is 2 kilobytes long, the total memory needed for the program will not exceed 112 kilobytes.

C) If each message is 2 kilobytes long, there will be at least 112 kilobytes of network data sent through the internet.

D) If each message is 2 kilobytes long and all processes consume all their messages from memory every second, the total memory needed in the worst case is 2 kilobytes.

E) None of the above statements are correct.

---

## Question 2: Parallel Architecture Speedup

Sarah has a computationally intensive task involving the multiplication of two large matrices, X and Y. Assume the memory for each machine is sufficiently large to store X and Y, and all processing units have the same computational efficiency.

Select the option that would yield the largest speedup for the task:


A) Four independent SISD architecture machines, each with 1 processing unit, without network connections.

B) One SIMD architecture machine with 6 processing units.

C) One MISD architecture machine with 6 processing units.

D) One MIMD architecture machine with 4 processing units.

E) Two MIMD architecture machines, each with 3 processing units, connected via LAN at 10Mbps.

---

## Question 3: SIMD Computation Timing

Suppose we want to perform element-wise addition of two floating point arrays, Y and Z, storing the result in array X.

Assume the machine has 16 execution units, and each unit takes 3 nanoseconds to compute an addition instruction and store the result. If arrays X, Y, and Z each contain 150 elements, what is the minimum computation time for the task in nanoseconds?


A) 3 nanoseconds

B) 16 nanoseconds

C) 30 nanoseconds

D) 150 nanoseconds

E) 450 nanoseconds

---

## Question 4: Distributed Process and Memory Calculation

Sam, a software developer, implements a program that runs with 6 processes, with each process consuming 2GB of memory. After running the program for 15 seconds, the program creates one more process for logging purposes, consuming 0.25GB of memory.

David from the deployment team dispatches Sam's program to 50 machines across the company's network. Which of the following statements are true?


A) The number of processes throughout the network stays at 300 for all time.

B) If 5 machines are down, the number of processes will drop from 350 to 300 eventually.

C) The total amount of memory consumed throughout the network will always be smaller than or equal to 100GB.

D) If Sam's program is deployed to 10 more machines, the total amount of memory consumed throughout the network will be less than 720GB.

E) None of the above statements are true.

---

## Question 5: TCP/UDP Data Transmission

A video conferencing application uses both TCP and UDP for different components of its network communication.

**Transmission details:**

- UDP is used for real-time audio commands, sent 4 times per second. Each UDP datagram has a payload of 150 bytes and a header of 8 bytes.
- TCP is used for sending control signals, transmitted 2 times per second. Each TCP segment has a payload of 400 bytes and a header of 20 bytes.
- TCP is used for transmitting chat messages, sent 3 times per second. Each message is divided into 2 chunks, with each chunk containing a payload of 600 bytes and a 20-byte header.
- UDP is used for video frames, sending 10 frames per second. Each UDP packet holds a payload of 1000 bytes and a header of 8 bytes.

Calculate the total amount of data (in bytes) transmitted per second across all protocols.

---

## Question 6: Pipelined vs Non-Pipelined Architecture

Assume a time phase duration of 4 nanoseconds and 6 stages to execute an instruction.

Which of the following statements are true? (There may be more than one correct answer, or none.)


A) The latency of a pipelined CPU will be 4 nanoseconds.

B) The latency of a pipelined CPU will be 6 nanoseconds.

C) The latency of a pipelined CPU will be 24 nanoseconds.

D) The throughput of a pipelined CPU will be 1/4 (one quarter) instructions per nanosecond.

E) The throughput of a pipelined CPU will be 4 instructions per nanosecond.

F) The throughput of a non-pipelined CPU will be 1/4 (one quarter) instructions per nanosecond.

G) The throughput of a non-pipelined CPU will be 1/24 (one twenty-fourth) instructions per nanosecond.

---

## Question 7: Data Hazards in a Pipeline

Assume a time phase duration of 3 nanoseconds and 4 stages to execute an instruction. Assume the CPU stalls for the minimum possible time during each stall, and a data hazard produces a stall after two time phases.

Which of the following are true?


A) During the stall period, the throughput of the CPU is reduced to 1/9 (one ninth) instructions per nanosecond.

B) During the stall period, the throughput of the CPU is reduced to 1/3 (one third) instructions per nanosecond.

C) If we employ data forwarding to prevent a stall, the throughput of the CPU equals 1/12 (one twelfth) instructions per nanosecond.

D) If we employ data forwarding to prevent a stall, the throughput of the CPU equals 1/9 (one ninth) instructions per nanosecond.

E) If we employ data forwarding to prevent a stall, the throughput of the CPU equals 1/3 (one third) instructions per nanosecond.

---

## Question 8: Superscalar Architecture

Assume a 14-way superscalar CPU core has 5 integer Execution Units (EU), 3 floating point (FP) EUs, and 6 Address Arithmetic (AA) EUs.

**Scenario 1:** 14 consecutive instructions with no dependencies comprise 4 integer instructions, 9 FP instructions, and 1 address arithmetic instruction.

**Scenario 2:** 14 consecutive integer instructions have no dependencies.

Which of the following statements are true?


A) (Scenario 1) The CPU core can execute at least 6 instructions concurrently.

B) (Scenario 1) The CPU core can execute all 14 instructions concurrently.

C) (Scenario 1) Only the integer instructions can be executed concurrently.

D) (Scenario 2) The CPU core will not be able to execute any of the instructions concurrently.

E) (Scenario 2) The CPU core can execute at least 4 instructions concurrently.

---
---

# Answer Key

## Question 1: B

- Each of the 8 processes sends to 7 others → 8 × 7 = 56 messages per second.
- Total memory = 56 × 2KB = **112KB** in the worst case, so total memory needed will not exceed 112KB.
- A is wrong (that's a single message size). C is wrong (shared memory, no network involved). D is wrong (worst case still allows all messages to exist simultaneously before consumption, so up to 112KB, not 2KB).

## Question 2: B

- SIMD machine with 6 processing units gives the best speedup: matrix multiplication is a data-parallel task well suited to SIMD, with the most usable processing units (6) and no communication overhead.
- SISD machines cannot cooperate (no network). MISD does not fit the task pattern. The single MIMD machine only has 4 units (less than SIMD's 6). The two-machine MIMD option, despite 6 total units, suffers from LAN communication overhead over a slow 10Mbps link, likely offsetting parallel gains.

## Question 3: C (30 nanoseconds)

- Batches needed = ceil(150 / 16) = ceil(9.375) = 10 batches.
- Time = 10 batches × 3ns = **30 nanoseconds**.

## Question 4: E (None of the above are true)

- Per machine before 15s: 6 × 2GB = 12GB, 6 processes.
- Per machine after 15s: 12GB + 0.25GB = 12.25GB, 7 processes.
- 50 machines: before 15s = 300 processes, 600GB; after 15s = 350 processes, 612.5GB.
- A is false (process count rises to 350 after 15s). B is false (45 machines remaining would eventually run 7 processes each = 315, not 300). C is false (600GB alone exceeds 100GB). D is false (60 machines × 12GB = 720GB before 15s, which is not less than 720GB, and rises further after 15s).

## Question 5

Per-unit sizes:
- UDP audio: 150 + 8 = 158 bytes × 4/sec = 632 bytes
- TCP control: 400 + 20 = 420 bytes × 2/sec = 840 bytes
- TCP chat: 2 × (600 + 20) = 1,240 bytes × 3/sec = 3,720 bytes
- UDP video: 1000 + 8 = 1,008 bytes × 10/sec = 10,080 bytes

**Total = 632 + 840 + 3,720 + 10,080 = 15,272 bytes per second**

## Question 6: C and D

- Latency = 6 stages × 4ns = **24ns** (Statement C).
- Pipelined throughput = 1 instruction / 4ns = **1/4 instructions per nanosecond** (Statement D).
- Non-pipelined throughput = 1 instruction / 24ns = 1/24 instructions per nanosecond. This matches Statement G, so G is also true.
- A and B are false (they understate the latency). E is false (throughput cannot exceed 1 instruction per stage-cycle). F is false (1/4 is the pipelined throughput, not the non-pipelined one).

**Correct answers: C, D, and G.**

## Question 7: A and E

- Normal cycle time = 3ns, stall adds 2 × 3ns = 6ns extra.
- During stall: 3ns + 6ns = 9ns per instruction → throughput = 1/9 instructions per nanosecond (Statement A).
- With data forwarding, the stall is eliminated, returning to baseline throughput = 1 instruction / 3ns = 1/3 instructions per nanosecond (Statement E).

## Question 8: A and E

**Scenario 1:**
- Integer: 4 instructions, 5 EUs available → all 4 run concurrently.
- FP: 9 instructions, only 3 FP EUs available → only 3 run concurrently.
- AA: 1 instruction, 6 EUs available → runs concurrently.
- Total concurrent = 4 + 3 + 1 = 8 instructions, satisfying "at least 6" (Statement A). B and C are false.

**Scenario 2:**
- 14 integer instructions, only 5 integer EUs available → at most 5 run concurrently, satisfying "at least 4" (Statement E). D is false.
