# Banking Concurrency Simulation

This project demonstrates multithreading concepts in C using `pthread`.  
It simulates bank accounts and teller threads performing transfers, showing how concurrency issues can arise and how to resolve them.

---

Four phases:

1. **Phase 1 – No Synchronization**
   - Multiple threads update accounts simultaneously without protection.
   - Leads to **race conditions** and corrupted balances.
   - Fast, but results are inconsistent across runs.

2. **Phase 2 – Mutex Synchronization**
   - Each account has a mutex lock.
   - Threads must lock an account before updating its balance.
   - Fixes race conditions → balances are always correct.
   - Slight performance slowdown due to threads waiting on locks.

3. **Phase 3 – Deadlock Creation**
   - Transfers require two accounts to be locked.
   - If two threads lock different accounts and then wait on each other, a **deadlock** occurs.
   - Program stalls: no progress is made and CPU usage drops near zero.

4. **Phase 4 – Deadlock Resolution**
   - Solved by enforcing **lock ordering**: always lock the lower account ID first.
   - Prevents circular waiting → no deadlocks.
   - Balances remain correct, and unrelated transfers can run in parallel.

---


### Compile
```bash
gcc Wall -pthread phaseX.c -o phaseX
./phaseX
