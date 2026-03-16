# Bar on the Path to Nemea -- Concurrent Systems Simulation

A concurrent systems simulation written in **C** that models a small
outdoor bar visited by hikers on the path from **Petri to Nemea**.

The system demonstrates **process synchronization**, **shared memory
communication**, and **POSIX semaphore coordination** between multiple
independent programs.

This project was developed for the **Operating Systems course** at the
**University of Athens -- Department of Informatics and
Telecommunications**.

------------------------------------------------------------------------

# What This Implementation Contains

current codebase builds five executables:

- `initializer`: creates and initializes shared memory/semaphores, starts
    receptionist and visitors
- `receptionist`: serves visitors in FCFS order and assigns seats/orders
- `visitor`: waits in queue, gets seated, orders, eats, then leaves
- `monitor`: prints a snapshot of current bar state and statistics
- `closing`: marks the bar as closed for new arrivals

------------------------------------------------------------------------

# Bar Model

- 3 tables
- 4 chairs per table
- a FCFS waiting buffer with capacity `MAX_VISITORS` (100)
- one receptionist process
- many visitor processes

Rules enforced by the simulation:

- Visitors enter a shared waiting buffer (FCFS queue).
- A table is considered occupied once the first visitor sits.
- New visitors are seated at a table until its 4 chairs are filled.
- If all 3 tables are occupied, receptionist waits for a table to become
    free.
- A table becomes available again only when all seated visitors at that
    table have left.
- If the bar is closed (`is_closed = true`), new visitors leave without
    entering the queue.

------------------------------------------------------------------------

# Shared Memory Layout

The shared segment stores:

- Queue state (`front`, `rear`, `length`, waiting PIDs)
- Per-visitor queue synchronization semaphores
- Buffer-capacity semaphore
- Table occupancy/chair PIDs (3 x 4)
- Per-chair semaphore used to notify visitors when order-taking is done
- Global mutex semaphore for protected updates
- Aggregated statistics:
    - total/average wait time
    - total/average stay time
    - wine/water/cheese/salad counters
    - visitors served
- Closing flag (`is_closed`)

Defined in `shared_mem.h`.

------------------------------------------------------------------------

# Build

Compile everything:

```bash
make
```

Clean binaries/objects:

```bash
make clean
```

Also available in the Makefile:

```bash
make run
```

`make run` executes:

```bash
./initializer 212 0 0 12347
```

------------------------------------------------------------------------

# Run Commands

Important: in this implementation, the shared memory key is fixed to
`12347` (`SHM_KEY` in `shared_mem.h`). The initializer validates this.

## Recommended (full simulation)

```bash
./initializer <num_visitors> <order_time> <rest_time> 12347
```

Example:

```bash
./initializer 120 5 10 12347
```

Meaning:

- `num_visitors`: number of visitor processes to spawn
- `order_time`: maximum receptionist ordering delay (seconds)
- `rest_time`: maximum visitor eating/stay delay (seconds)

## Manual tools during execution

Print a state snapshot:

```bash
./monitor -s 12347
```

Close bar for new arrivals (existing queued/seated visitors continue):

```bash
./closing 12347
```

------------------------------------------------------------------------

# Outputs

At the end, receptionist prints:

- Average wait time
- Average stay time
- Wine consumed
- Water consumed
- Cheese consumed
- Salads consumed
- Total visitors served

Runtime events are appended to `log.txt` via `log_event()`.

------------------------------------------------------------------------

# Requirements

- Linux environment
- GCC compiler
- POSIX semaphore support
- System V shared memory support