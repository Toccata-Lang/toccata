# HVM Implementation (`new.c`)

This document summarizes the Higher-Order Virtual Machine runtime implemented in `new.c`.

## Term Representation

A `Term` is a single 64-bit word encoding three fields:

```
[ location (high) | label | tag (low) ]
```

| Field | Bits | Description |
|---|---|---|
| **Tag** | low bits | Node type (16 possible values) |
| **Label** | middle bits | Symbolic identifier (e.g. variable name) |
| **Location** | high bits | Index into `nodeBuff` (even addresses only) |

### Tags

| Tag | Polarity | Description |
|---|---|---|
| `VAL` / `VL1` | positive | Constant value |
| `VAR` | positive | Variable reference (points to a location) |
| `SUB` | negative | Deferred redex (pair of terms waiting to be reduced) |
| `NUL` | positive | Null / erasure target |
| `ERA` | negative | Eraser node |
| `LAM` | positive | Lambda abstraction |
| `APP` | negative | Application node |
| `REF` | negative | Function reference (stores an `interactionFn` pointer) |
| `SUP` | positive | Superposition (duplicating positive term) |
| `DUP` | negative | Duplication trigger (duplicates a negative term) |
| `OPX` / `OPY` | negative | Arithmetic/logic operators (x and y operands) |
| `I60` | positive | 60-bit signed integer |
| `F60` | positive | 60-bit float |
| `LAZ` | positive | Lazy thunk (delays evaluation) |

### Polarity

```c
bool isPositive(Term term);   // VAL, VAR, NUL, LAM, REF, SUP, I60, F60, LAZ
bool isNegative(Term term);   // SUB, ERA, APP, DUP, OPX, OPY
```

Redexes always form between one negative-polarity and one positive-polarity node.

## Memory Management

### Node Buffer

`nodeBuff` is an array of `Term` entries. Pairs always occupy even/odd location slots `(loc, loc+1)`.

- **`buffEnd`**: Monotonically increasing counter for new allocations (atomic).
- **`buffSize`**: Total buffer capacity (set at initialization).

### Free List

A lock-free (or mutex-protected) linked list stored *inside* the buffer for O(1) pair allocation:

- Free entries store `NUL` in port 1, with the next free location as the location field.
- Port 2 is cleared to `VOID`.
- Three states for the free list head: `EMPTY_FREE_LIST`, `LOCK_FREE_LIST` (lock in progress), or a valid location.

### Allocation

```c
Location allocPair(void);  // O(1) pop from free list or extend buffEnd
void freePair(Location loc);  // O(1) push onto free list
void freeLoc(Location loc);   // Free a single cell (coalesces into pair if both free)
```

## Reduction Engine

### Redex Stack

Each thread has a per-thread `Pairs` struct (stack of term pairs):

```c
void pushRedex(Term neg, Term pos);
bool popRedex(Term* neg, Term* pos);
```

When the stack overflows, the system aborts (designed for bounded depth).

### Core Operations

#### `take(Location loc)`

Removes and returns the term at a location, following `VAR` chains. Each location in the chain is freed.

- **Normal terms**: Frees the location, returns the term.
- **`VAR` chains**: Follows each link, freeing each location, until a non-VAR term is found.
- **`SUB` and `LAZ`**: Does **not** free the location. Returns `newTerm(VAR, 0, loc)` — a VAR pointing to the location.

**⚠️ Important:** `take` on LAZ/SUB leaves the location allocated. The returned VAR can be moved elsewhere, but the original location remains in the buffer.

#### `swap(Location loc, Term term)`

Atomically swaps a term into a location. If the old value was:

- **`SUB`** (deferred redex, and not exactly the SUB literal): Extracts the stored pair, pushes it as a new redex, frees the pair, returns `SUB`.
- **`SUB`** (exactly the SUB literal): Returns `SUB` unchanged. No freeing or redex pushing.
- **`ERA`** (eraser): **Frees the location after the exchange**, then triggers `interact()` with the old ERA and the new term. **The new term is lost** — the location is overwritten with VOID.
- **Other**: Returns the old term. The new term remains at the location.

**⚠️ Important:** When `swap` encounters ERA, the incoming `term` is passed to `interact(ERA, term)` but the location itself is freed (set to VOID). The location does **not** contain the new term.

#### `move(Location negLoc, Term pos)`

Moves a positive term into a negative location. Internally calls `swap(negLoc, pos)`:

- If old value is **`ERA`**: `swap` already freed the location and called `interact(ERA, pos)`. `move` does nothing further.
- If old value is **`SUB`**: No special handling. `move` returns.
- If old value is **another negative term**: Frees the location, then calls `interact(neg, pos)` if `pos == NUL`, otherwise `pushRedex(neg, pos)`.

#### `forceLazy(Term z)`

Forces a lazy thunk (`LAZ`):

1. Takes both ports of the `LAZ` node (negative thunk body, positive context).
2. If the thunk body is a `DUP` with a self-reference (circular lazy dup), replaces the self-pointing port with `SUB` (deferred redex).
3. Pushes the thunk body as a redex, or swaps it into the context location.

### Interaction Dispatch

```c
interactionFn interactions[16][16];  // Jump table indexed by [neg_tag][pos_tag]
void interact(Term neg, Term pos);   // Looks up and calls the appropriate rule
```

The 16×16 table is statically initialized to `&badrdx` for all entries. Rules are installed as they are implemented; unhandled combinations abort via `badrdx`.

### Reduction Loop

```c
void *normalize(void *v);  // Thread entry point
```

Each thread pops redexes from its local stack and reduces them until empty. Supports multi-threaded execution via pthreads.

## Term Construction

### `makePair(Tag tag, Lab lab, Term fst, Term snd)`

Creates a new pair node. Validates port polarities:

| Tag | Port 1 | Port 2 |
|---|---|---|
| `SUB`, `LAM`, `LAZ` | negative | positive |
| `APP`, `OPX`, `OPY` | positive | negative |
| `DUP` | negative | negative |
| `SUP` | positive | positive |

Allocates a location, stores both terms, and returns the new term.

### `makeOp(Lab op, Term x, Term y)`

Creates an operator term: builds an `OPX` pair with the operation code, wraps it in a `LAZ` thunk, and returns a variable reference to the second port.

## Lazy Evaluation

`LAZ` nodes represent thunks. The `forceLazy()` function handles:

1. **Normal case**: Push the thunk body as a redex.
2. **Lazy DUP self-reference**: Detect circular lazy dups, replace self-pointing ports with `SUB` (deferred redexes), and set up the reduction graph.

### LAZ structural constraints

LAZ nodes are **positive-polarity** but can only be stored in **negative ports** (APP port 2, DUP ports). This breaks the standard interactive combinator model — LAZ nodes have **two incoming edges**, which means they can only be pointed to by **VAR nodes** (not by direct port connections in most cases).

**Lazy DUP pattern:**
```
DUP port 1 ──→ LAZ (same location)
DUP port 2 ──→ LAZ (same location)
LAZ port 1 (negative) ──→ DUP (back to DUP's negative port)
LAZ port 2 (positive) ──→ VAR chain ──→ DUP
```
Both DUP ports contain the **same LAZ term** (same location). The LAZ's thunk body (port 1) points back to the DUP's negative port, forming a self-referential loop. The LAZ's context (port 2) is a **positive** term, so it cannot directly contain the DUP (negative). It must be a **VAR** that chains to the DUP. Cycle detection from the context follows VAR chains to find the DUP.

**Lazy APP pattern:**
```
APP port 2 (negative) ──→ LAZ
LAZ port 1 (negative) ──→ APP (back to APP node)
LAZ port 2 (positive) ──→ positive context value
```
The LAZ's thunk body points back to the APP node, and the LAZ's context carries the positive value the APP will interact with when forced.

## Thread Safety

- **Non-atomic mode** (`NON_ATOMIC`): Single-threaded, direct array access.
- **Atomic mode** (default): Uses `atomic_load_explicit` / `atomic_exchange_explicit` / `atomic_fetch_add_explicit` with `memory_order_relaxed`.
- **Mutex**: `redexMutex` for thread-safe operations on shared state.
- **Per-thread**: `__thread Pairs pairs` (redex stack), `__thread Location freeList` (free list head).

## Debugging & Diagnostics

| Function | Purpose |
|---|---|
| `printTerm()` | Pretty-prints a term with tag, label, location, and port contents |
| `printRawTerm()` | Compact term representation |
| `printBuff()` | Dumps node buffer contents in a range |
| `printFreeList()` | Dumps the free list chain |
| `pb()` / `pr()` | Print buffer / print redexes (convenience wrappers) |
| `check_buff()` | Checks for leaked pairs (non-NUL entries still in use) |
| `isCycle()` | Detects cycles in term graphs (for `CHECK_MEM_LEAK`) |

## Initialization

```c
void hvmInit(u64 size);   // Allocate buffer, init mutex/condvar, set buffSize, open graphs.dot
void hvmReset(void);       // Clear buffer, reset counters
void hvmFree(void);        // Free buffer and destroy mutex
void spawn_threads();      // Create worker threads
```

**⚠️ Important:** `hvmInit` must set `buffSize` (for bounds checking in `allocPair`) and open `dotFile` (for debug output in `boom`). Both are required for correct operation.
