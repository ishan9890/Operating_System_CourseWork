# ST5004CEM - Operating Systems and Security Coursework
## Task 1: Process Management and Threading
**Purpose:** Demonstrates a race condition on a shared `balance` variable.
3 threads each increment a shared long `balance` by 1, 100,000 times each
(expected final value: 300,000), with **no synchronization**.

**Compile:**
```bash
cd task1
gcc -o race_condition race_condition.c -pthread
```

**Run:**
```bash
./race_condition
```

**Result:** Final balance is inconsistent across runs (observed: 297530,
100003, 300000 on three consecutive runs) due to unsynchronized
read-modify-write access to `balance` from multiple threads. This
demonstrates the race condition concept before the mutex fix below.

### File: `task1/bank_sync.c` (Part A: Mutex Fix)
**Purpose:** Fixes the race condition from `race_condition.c` by protecting
the shared `balance` variable with a `pthread_mutex_t`. Only the critical
section (`balance += 1`) is locked, minimizing overhead.

**Compile:**
```bash
cd task1
gcc -o bank_sync bank_sync.c -pthread
```

**Run:**
```bash
./bank_sync
```

**Result:** Final balance is consistently 300,000 across all runs (unlike
the unsynchronized version), confirming the mutex eliminates the race
condition by enforcing mutual exclusion on the critical section.

### File: `task1/bank_sync.c` (Part B: Deadlock Demo - Unsafe)
**Purpose:** Demonstrates a classic circular-wait deadlock using two
separate mutexes (`account_A`, `account_B`). Thread 1 locks A then
attempts B; Thread 2 locks B then attempts A - opposite lock ordering.

**Compile/Run:** Same as above (functions uncommented in same file).

**Result:** Program hangs indefinitely after both threads acquire their
first lock and block waiting for the second. Required manual termination
(Ctrl+C). Confirms all four Coffman conditions for deadlock are present:
mutual exclusion, hold-and-wait, no preemption, circular wait.

### File: `task1/bank_sync.c` (Part B: Deadlock Fix - Safe)
**Purpose:** Fixes the deadlock by enforcing a consistent lock acquisition
order. Both `safe_transfer_A_to_B()` and `safe_transfer_B_to_A()` always
lock `account_A` before `account_B`, regardless of transfer direction.

**Result:** Both threads complete successfully with no hang. Since every
thread requests locks in the same global order, circular wait becomes
structurally impossible - one thread always acquires account_A first and
proceeds uncontested, while the other simply waits its turn.

**Design Decision:** Consistent lock ordering was chosen over alternatives
(e.g. try-lock with backoff, or a single global lock) because it completely
eliminates the possibility of circular wait at the design level, with no
retry logic or added complexity, at the minor cost of requiring careful
lock-order discipline across the whole codebase.

### File: `task1/scheduler.c`
**Purpose:** Simulates round-robin CPU scheduling for 4 processes with a
fixed time quantum of 2 units. Tracks waiting time and turnaround time
for each process.

**Compile:**
```bash
cd task1
gcc -o scheduler scheduler.c
```

**Run:**
```bash
./scheduler
```

**Result:** All 4 processes complete in cyclical 2-unit time slices.
Process with shortest burst time (P3, burst=3) finishes first (turnaround
13); process with longest burst time (P2, burst=8) has highest turnaround
(22), consistent with round-robin's fairness property - no process waits
indefinitely, but longer jobs naturally take longer to fully complete.
Average waiting time: 12.25 units; average turnaround time: 17.75 units.

### File: `task2/memory_sim.c` (FIFO Page Replacement)
**Purpose:** Simulates FIFO page replacement over a fixed reference string
{7,0,1,2,0,3,0,4,2,3,0,3,2} with 3 memory frames. Evicts the page that
has resided in memory the longest, regardless of recent usage.

**Compile:**
```bash
cd task2
gcc -o memory_sim memory_sim.c
```

**Run:**
```bash
./memory_sim
```

**Result:** 10 page faults, 3 hits out of 13 requests (23.08% hit ratio).
Frame eviction order follows strict insertion order (oldest-loaded page
evicted first), verified by manual trace.

### File: `task2/memory_sim.c` (LRU Page Replacement)
**Purpose:** Simulates LRU page replacement over the same reference string
and frame count as the FIFO test, for direct comparison. Evicts the page
that has not been accessed for the longest time, tracked via a logical
clock updated on every access (including hits).

**Compile/Run:** Same file as FIFO (`memory_sim.c`) — both algorithms run
sequentially in `main()`.

**Result:** 9 page faults, 4 hits (30.77% hit ratio) - outperforming FIFO
(10 faults, 23.08% hit ratio) on the identical reference string. LRU's
advantage comes from tracking recency of use: e.g. page 0, accessed
recently before eviction was needed, was retained by LRU but had already
been evicted by FIFO earlier in the sequence, causing FIFO an extra fault.

### File: `task2/memory_sim.c` (Configurable Page Size)
**Purpose:** Extends the paging simulation with a configurable page size
(`PAGE_SIZE_KB`) and process size (`PROCESS_SIZE_KB`), calculating the
number of pages a process actually requires via ceiling division, and
comparing this against available physical memory (frames x page size).

**Result:** With page size = 4KB and process size = 50KB, the process
requires 13 pages, but only 3 frames (12KB) of physical memory are
available - confirming that page replacement is necessary, which the
FIFO/LRU simulations below directly demonstrate.

### File: `task3/secure_fs.c` (Authentication System)
**Purpose:** Implements user authentication using a custom djb2-based
password hashing function (passwords are never stored or compared in
plaintext). Includes a 3-attempt lockout to mitigate brute-force guessing.

**Compile:**
```bash
cd task3
gcc -o secure_fs secure_fs.c
```

**Run:**
```bash
./secure_fs
```

**Test credentials:** username `ishan`, password `password123` (or `faker`
with their respective password, as registered in `main()`).

**Result:** Correct credentials log in successfully; incorrect password
decrements remaining attempts (3 total); unknown username or exceeding
3 attempts results in login failure/lockout.

### File: `task3/secure_fs.c` (File Operations + Audit Logging)
**Purpose:** Adds a menu-driven interface for file create/write/read/delete
operations, with every successful operation automatically logged to
`audit.log` (timestamp, username, action, filename).

**Compile/Run:** Same file as authentication (`secure_fs.c`).

**Result:** Full create -> write -> read -> delete -> verify-deleted
sequence tested successfully. Reading a deleted file correctly fails with
a clear error message. `audit.log` accurately records all four operations
with timestamps and the acting username, providing a traceable record of
file system activity.

### File: `task3/secure_fs.c` (File Permission System)
**Purpose:** Implements a Unix-style owner/group/others permission model
per file, enforced before every read/write/delete operation. Permissions
persist across program runs via `permissions.dat`.

**Result:** Two-session test confirmed: owner (`ishan`) has full read/write
access; a different user (`faker`) is correctly denied write access but
retains default read access, matching the default permission scheme
(owner: rwx, group/others: r--). Permission records persist correctly
between separate program executions.

**Bug Fixed:** An earlier version opened the file in write mode (`"w"`,
which truncates on open) *before* checking permissions, causing silent
data loss even when a write was subsequently denied. Fixed by moving the
permission check before any file is opened - a general security principle
that access checks must precede, never follow, a state-changing action.

### File: `task3/secure_fs.c` (Encryption/Decryption)
**Purpose:** Adds XOR-cipher based encryption/decryption for sensitive
files, using a fixed shared key (`ENCRYPTION_KEY`). Encryption requires
write permission; decryption requires read permission on the target file.

**Result:** `sense.txt` (plaintext: "top sense") was encrypted, verified
unreadable via direct `cat` (binary garbage output), then decrypted in a
separate program execution, restoring the exact original content. Confirms
XOR's self-inverse property (same operation both encrypts and decrypts)
works correctly and persists correctly to disk across runs.

**Security Note:** XOR cipher with a static repeating key is used here to
demonstrate the encryption/decryption *concept* clearly, but is NOT
cryptographically secure for real use - it is vulnerable to frequency
analysis and known-plaintext attacks, particularly when the key is shorter
than the data. A production system should use a vetted algorithm (e.g.
AES-256) via a library such as OpenSSL's libcrypto.

### Files: `task4/server.c`, `task4/client.c` (Basic Socket Communication)
**Purpose:** Establishes a minimal TCP client-server connection using
BSD sockets on port 8080 (localhost). Server accepts one connection,
exchanges a single message with the client, then exits.

**Compile:**
```bash
cd task4
gcc -o server server.c
gcc -o client client.c
```

**Run (two terminals required):**
```bash
# Terminal 1
./server

# Terminal 2 (after server is running)
./client
```

**Result:** Client successfully connects, sends "Hello from client!",
and receives "Hello from server!" in response. Server correctly logs
the connection and received message before exiting.

### Files: `task4/server.c`, `task4/client.c` (Command Protocol)
**Purpose:** Implements a simple text-based command protocol. Client sends
`LOGIN <user> <pass>`, `MSG <text>`, or `QUIT`; server parses the command
prefix and responds with `OK ...` or `ERROR ...`.

**Compile/Run:** Same as basic socket test (see above), now interactive -
client prompts for commands with `>` after connecting.

**Result:** Full command sequence (LOGIN, MSG, QUIT) tested successfully.
Server correctly parses each command type and responds appropriately;
QUIT cleanly terminates the connection on both sides without errors.

### Files: `task4/server.c`, `task4/client.c` (Authentication)
**Purpose:** Adds real credential checking to the LOGIN command using the
same djb2 password hashing approach as Task 3. MSG commands are rejected
with an error unless the client has successfully authenticated first.

**Result:** Verified two scenarios - (1) sending MSG before LOGIN is
correctly rejected with a clear error; after successful LOGIN, MSG
commands succeed and are tagged with the authenticated username. (2)
LOGIN with an incorrect password is correctly rejected with "Invalid
credentials", and the connection remains unauthenticated.

### Files: `task4/server.c`, `task4/client.c` (Multi-Client Concurrency)
**Purpose:** Server now loops `accept()` continuously and spawns a new
detached pthread per connected client, allowing multiple clients to be
served simultaneously without blocking one another.

**Compile:**
```bash
cd task4
gcc -o server server.c -pthread
gcc -o client client.c
```

**Result:** Verified with 2 simultaneous clients (fd=4, fd=5) - both
connected, authenticated as different users, and exchanged messages
independently and concurrently, with neither blocking the other.
Demonstrates correct application of POSIX threading (from Task 1) to a
networked, multi-client server context.

### Files: `task4/server.c`, `task4/client.c` (Input Validation)
**Purpose:** Adds robust validation for malformed input - empty commands,
incomplete LOGIN/MSG arguments, and unrecognized command words - so the
server never crashes and always responds with a clear, appropriate error.

**Result:** All edge cases tested successfully: empty input, incomplete
LOGIN/MSG arguments, and unrecognized commands each produce a distinct,
correct error message. QUIT and successful LOGIN/MSG continue to work
correctly alongside the new validation.

**Bug Fixed:** An earlier version was missing the QUIT and "unknown
command" branches entirely from the if/else-if chain. Since `response`
was declared but left uninitialized when no branch matched, the server
sent back stale data from a previous loop iteration instead of a correct
response or crash - a real example of undefined behavior from using an
uninitialized variable, only fixed by ensuring every possible input path
explicitly sets `response` before it is sent.