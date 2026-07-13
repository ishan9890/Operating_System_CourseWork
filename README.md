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