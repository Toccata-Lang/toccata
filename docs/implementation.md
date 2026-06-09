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

### `makeLazyDup(Lab lb, Term arg)`

Creates a duplication structure for a given label and argument. Two variants:

- **Native value** (I60/F60/REF/VAL): Creates a `DUP` node with `arg` in both ports. No LAZ wrapping.
- **Other value** (LAM, SUP, etc.): Creates a `LAZ`/`DUP` self-referential chain:
  ```
  DUP port 1 ──→ LAZ (same location)
  DUP port 2 ──→ LAZ (same location)
  LAZ port 1 (negative) ──→ DUP (back to DUP's negative port)
  LAZ port 2 (positive) ──→ arg
  ```
  Returns the DUP node. The LAZ's context (port 2) carries `arg`.

## DUP/SUP Interaction

`dupSup` implements the DUP/SUP interaction with two label-dependent variants:

### Annihilation (same labels)
- `termLab(dup) == termLab(sup)`
- Take SUP aux ports (x, y), move them into DUP aux ports
- Both DUP and SUP nodes consumed

### Commutation (different labels)
- `termLab(dup) != termLab(sup)`
- Take SUP aux ports (x, y)
- Create two new DUP chains via `makeLazyDup(dupLab, x)` and `makeLazyDup(dupLab, y)`
- Create two new SUP nodes with VAR chains into the DUP chains
- Wire DUP aux ports to the new SUPs
- Original DUP and SUP consumed; new SUP + DUP chains remain

### ERA handling
- If DUP port 1 is ERA: take it, move SUP to DUP port 2
- If DUP port 2 is ERA: take it, move SUP to DUP port 1
- These branches are checked before the full expansion path

## Lazy Evaluation

`LAZ` nodes represent thunks. The `forceLazy()` function handles:

1. **Normal case**: Push the thunk body as a redex.
2. **Lazy DUP self-reference**: Detect circular lazy dups, replace self-pointing ports with `SUB` (deferred redexes), and set up the reduction graph.

### LAZ structural constraints

LAZ nodes are **positive-polarity** but can only be stored in **negative ports** (APP port 2, DUP ports). This breaks the standard interactive combinator model — LAZ nodes have **two incoming edges**, which means they can only be pointed to by **VAR nodes** (not by direct port connections in most cases).

### LAZ interactions — handled through VAR chains, not as jump table rules

LAZ nodes are **never the principal port of an interaction** in the reduction engine. The `interactions[16][16]` jump table has no entries for `[neg_tag][LAZ]` — all LAZ interactions are handled through VAR chains:

- **ERA → LAZ**: `eraVar` calls `take(termLoc(var))` which follows VAR chains. When `take` returns LAZ, `eraVar` calls `eraseLazy(laz)` which dispatches based on thunk type (DUP with cycle detection, APP/OP wildcard erasure).
- **Non-ERA negative → LAZ**: The negative term reaches LAZ through a VAR chain. The dispatch goes through `negVar` path where the VAR chain is followed and the appropriate handler is called based on the negative term's tag and LAZ's thunk type.

**Key insight:** The four LAZ interactions in the calculus (ERA/LAZ DUP, ERA/LAZ DUP loop, (APP/OPX/OPY/DUP)/LAZ DUP, ERA/LAZ (APP/OP), (APP/OP)/LAZ (APP/OP)) are **logical descriptions** of the reduct, not physical interaction rules. The physical implementation handles them through the VAR chain mechanism.

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

### Cycle detection for lazy DUP erasure

When erasing a LAZ whose thunk body is a DUP, we need to detect if there's a cycle: LAZ → context → ... → DUP → LAZ.

**How `isCycle` works:**
- Entry: `isCycle(context, lazLoc)` where `context` = term at LAZ port 2 (positive), `lazLoc` = LAZ's location
- `findCycle` traverses from the context, following VAR chains and recursing into pair node ports
- For each port, checks two things:
  1. **VAR chain match:** port contains VAR → `lazLoc` (follows VAR chain to target)
  2. **Direct LAZ match:** port directly contains LAZ term at `lazLoc` (the cycle-closing point — DUP's ports contain LAZ directly)
- Returns 1 if cycle found, 0 otherwise

**Why both checks are needed:** The DUP's ports contain the LAZ **directly** (not via VAR). A VAR chain from the context leads to the DUP, but the DUP itself points to LAZ via a direct term reference. The cycle closes when `findCycle` encounters a port containing the LAZ term directly.

**Performance note:** The visited set uses a linear scan (`findCycleNode`), giving O(n²) worst case. The population of visited nodes is completely different between calls, so a hash set for O(1) lookups would be the right optimization when this becomes a real bottleneck. The TODO comment in `isCycle` tracks this.

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

## Testing

### Test Structure

Place test functions **before** `main()` (no forward declarations needed).

```c
void test_app_lam(void) {
  char msg[100];

  // Build terms
  Term lam = makePair(LAM, 0, SUB, newI60(8));
  Term app = makePair(APP, 0, newI60(7), SUB);

  // Trigger interaction (neg first, pos second)
  interact(app, lam);

  // Verify results
  Term result = take(portLoc(1, lam));
  if (termTag(result) != I60 || getI60(result) != 7) {
    sprintf(msg, "expected I60(7), got tag %s", tagStr(termTag(result)));
    BOOM(msg);
  }

  // Verify no leaks
  if (glblAlloced != 0) {
    BOOM("memory leak");
  }
}
```

### Key Conventions

- **`interact(neg, pos)`** — negative term first, positive term second
- **`glblAlloced == 0`** — always 0 before and after each test. Check after each test to verify no leaks.
- **`BOOM(msg)`** — prints message, file, and line number, then aborts. Use `sprintf` into a local `char msg[100]`.
- **Build command:** `make test-hvm` (not manual `clang`). Ensures correct flags (`CHECK_MEM_LEAK`, `SAFETY`, `STATS`) and source files.

### Cleanup Patterns

**Never use `freeLoc` directly in tests.** Use `take` to clean up ports and verify returned values.

**Exception:** When you expect SUB or LAZ at a port, use `get` + `freeLoc` instead of `take` (since `take` returns VAR for SUB/LAZ and doesn't free the location).

- **Ports with content** (leaf, I60, or pair body): use `take` to retrieve and free
- **Ports already VOID**: do not take — `glblAlloced` verifies all pairs are freed automatically
- **VAR return** (port had SUB or LAZ): location was NOT freed. Use `get` + `freeLoc` to verify and free

### Polarity Constraints

`makePair` validates port polarities when `SAFETY` is enabled:

| Tag | Port 1 | Port 2 |
|---|---|---|
| `SUB`, `LAM`, `LAZ` | negative | positive |
| `APP`, `OPX`, `OPY` | positive | negative |
| `DUP` | negative | negative |
| `SUP` | positive | positive |

Leaf terms (NUL, ERA, SUB, I60, VAL, etc.) are self-contained — no buffer location needed.

**⚠️ Lesson:** Never put a negative term (like SUB) in APP's port 1 — it must be positive. If you need to test `take` with SUB, use a VAR chain: create a separate location holding SUB, then put a VAR pointing to it in APP's port 1.

## Lessons Learned

1. **`take` on LAZ/SUB doesn't free** — returns a VAR pointing to the location. The location remains allocated.
2. **`swap` on ERA frees the location** — the new value is lost (overwritten by VOID). The value goes to `interact(ERA, pos)` instead.
3. **`move` with ERA does nothing** — `swap` handles ERA internally (frees location + calls interact), so `move`'s own logic is skipped.
4. **Never put negative terms in APP port 1** — APP port 1 must be positive. To test `take` with SUB, use a VAR chain pointing to a separate location.
5. **Don't depend on specific location values** — the free list reuses locations across tests, making absolute location checks fragile. Use relative checks (tag comparisons, `portLoc` results).
6. **`eraLeaf` must be registered per leaf type** — each positive leaf (NUL, I60, F60) needs its own entry in the interactions table. Constructors like LAM need dedicated handlers.
7. **`glblAlloced` is always 0 before and after each test** — check it after each test to verify no leaks.
8. **`move` rejects negative terms** — the SAFETY check in `move` aborts if `isNegative(pos)`. To put ERA (negative) into a location, use `swap` directly instead of `move`.
9. **`move` assumes old value is negative** — `move` calls `swap`, gets the old value, and if it's not SUB/ERA, pushes it as a redex. If the old value is positive (e.g., LAM body), this creates an invalid redex. Use `take` + `swap` pattern instead when the location may contain a positive term.
10. **Erased pair bodies must be freed** — when ERA erases a body that is a pair (like a nested LAM), `take` frees the port location but not the pair's own location. After `take`, call `freePair(termLoc(body) & 0xFFFFFFFE)` if `hasLocation(body)` is true.
11. **`swap` + `freePair` pattern for erasing bodies** — when implementing rules where a term is erased (like ERA/LAM), use: `take(port)` to free port location and get body, `freePair(termLoc(body) & 0xFFFFFFFE)` if body has location, then `swap(port, newVal)` to write the replacement.
12. **Always use `make test-hvm`** — not manual `clang`. Ensures correct flags (`CHECK_MEM_LEAK`, `SAFETY`, `STATS`) and source files.
13. **Handle old values BEFORE overwriting ports** — use `get` to read the old value, handle it based on tag (SUB → push redex, ERA → interact, other → consumed by the rule), then `swap` the new value. Don't silently discard old values.
14. **Never use 42 as a test integer value** — it's cliche and irritating. Pick something else.
15. **`eraseLazy` must not be called directly** — it is an internal helper invoked only by `eraVar` (when `take` returns a LAZ/SUB). To test `eraseLazy` behavior, always interact `ERA` with a `VAR` that points to a `LAZ` term in a port of the thunk term. Never call `eraseLazy(laz)` directly in tests — it bypasses the interaction dispatch and has known race conditions with `forceLazy`.
16. **ERA/VAR + lazy DUP: context and VAR must target different DUP ports** — When testing `eraVar` with a LAZ whose thunk is a DUP (lazy DUP pattern), the context (LAZ port 2) loops back to one port of DUP, and the VAR given to `interact(ERA, var)` must point to the *other* port of DUP. They must target different ports. In actual usage they will never be the same port — if both target the same DUP port the test configuration is invalid.
17. **DUP/SUP has two label-dependent variants** — Same label: annihilation (SUP aux values wired into DUP ports, both consumed). Different labels: commutation/expansion (creates new SUP + LAZ/DUP chains). The handler must check `termLab(dup) == termLab(sup)` to dispatch.
18. **`makeLazyDup` behavior depends on argument type** — For native values (I60/F60/REF/VAL), it puts the value directly in DUP ports. For other values (LAM, SUP, etc.), it creates a LAZ/DUP self-referential chain. Tests must account for both cases.
19. **Don't take ports you're about to overwrite** — When `move()` will overwrite a port (e.g., DUP aux ports in commutation), there's no need to `take()` them first. The `move`/`swap` handles the old value. Taking first adds no value and can introduce bugs.
20. **When SUP ports contain VARs into DUP chains, erase the DUP nodes** — `makeLazyDup` creates DUP nodes that remain allocated. To clean up in tests, follow VAR chains and call `interact(ERA, newTerm(VAR, 0, termLoc(var)))` for each VAR to erase the DUP nodes.
21. **Verify calculus against actual implementation** — The calculus description of DUP/SUP annihilation initially said `a → x, b → y` but the actual behavior is `x → a, y → b` (SUP values into DUP ports). Always verify the calculus matches the code with a test.
