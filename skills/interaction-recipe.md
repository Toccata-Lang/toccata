# Implementing HVM Interaction Rules

This recipe documents the process for implementing and testing HVM interaction rules.

## ⛔ CRITICAL: Never use 42 as test data

**Never use 42 as a test integer value** — it's cliche and irritating. Pick something else.

**⛔ CRITICAL: When committing, ONLY stage and commit the files that were actually changed.**
Never run `git add -A` or `git add .` — it will pick up untracked files (backup files, generated files, dependencies, build artifacts) that are not part of your changes. Instead, explicitly list only the files you modified:

```bash
git add test-hvm.c new.c graph.c
```

Then commit. If you see untracked files (`??`) in `git status`, they must NOT be included in the commit.

## Prerequisites

- `test-hvm.c` with `hvmInit()` / `hvmFree()` in `main()`
- `new.c` with `interactions[16][16]` jump table initialized to `&badrdx`
- `new.h` exposing `glblAlloced`, `nodeCount`, and all term construction functions

## Files to Read for Context

Before implementing or modifying interaction rules, read these files in order:

| # | File | Purpose |
|---|---|---|
| 1 | `skills/interaction-recipe.md` | This recipe — how to implement & test interaction rules |
| 2 | `docs/calculus.md` | Formal calculus — node types, polarities, all 15 interaction rules |
| 3 | `docs/interactions.dot` | Visual before/after diagrams for each interaction rule |
| 4 | `docs/implementation.md` | Architecture reference — term layout, memory, reduction engine |
| 5 | `new.h` | Type definitions, tag constants, function declarations |
| 6 | `new.c` | Core implementation — `take`, `swap`, `move`, `interact`, existing rules |
| 7 | `test-hvm.c` | Test suite — patterns for building terms, triggering interactions, verifying results |
| 8 | `graph.c` | DOT graph generation for debugging (called by `test-hvm.c` via `graphDown`) |
| 9 | `runtime3.c` | Runtime support — reference counting, memory management, free lists, native functions, vector/hash-map ops |
| 10 | `Makefile` | Build command for `test-hvm` — flags (`SAFETY`, `CHECK_MEM_LEAK`, `STATS`) and source files |

Read in order: the calculus defines the rules, the implementation shows how they work, the tests show how to exercise them, and the graph/debug/runtime files help diagnose issues.

## Step 1: Create the test function

Place test functions **before** `main()` (no forward declarations needed).

```c
void test_app_lam(void) {
  char msg[100];

  // Build terms

  // Trigger interaction

  // Post-checks
}
```

Call from `main()`:

```c
int main(int argc, char *argv[]) {
  hvmInit(1024);
  test_app_lam();
  hvmFree();
  return 0;
}
```

## Step 2: Set up terms in the buffer

Use `makePair(tag, label, port1, port2)` to create pair nodes. It allocates a location, writes both children, and returns the tagged term.

**Polarity constraints** (enforced by `SAFETY`):

| Tag | Port 1 | Port 2 |
|---|---|---|
| `LAM` | negative | positive |
| `APP` | positive | negative |
| `SUP` | positive | positive |
| `DUP` | negative | negative |
| `LAZ` | negative | positive |
| `SUB` | negative | positive |

Leaf terms (NUL, ERA, SUB, I60, VAL, etc.) are self-contained — no buffer location needed.

```c
Term lam = makePair(LAM, 0, SUB, newI60(7));
Term app = makePair(APP, 0, newI60(7), SUB);
```

**⚠️ Lesson:** Never put a negative term (like SUB) in APP's port 1 — it must be positive. If you need to test `take` with SUB, use a VAR chain: create a separate location holding SUB, then put a VAR pointing to it in APP's port 1.

## Step 3: Trigger the interaction

```c
interact(app, lam);  // neg=APP, pos=LAM
```

This will hit `badrdx` until the rule is implemented.

## Step 4: Implement the interaction rule

Place the function **before** the `interactions` jump table declaration in `new.c`.

### Using `take` and `move` (preferred over `get`/`free`)

**`take(Location loc)`**:
- Extracts the term at `loc`, freeing the location
- Follows VAR chains (freeing each location in the chain)
- Returns a positive value
- **Special case:** for `SUB` and `LAZ`, returns a `VAR` pointing to the location **without freeing it**

**`move(Location negLoc, Term pos)`**:
- Moves a **positive** term into a **negative** location
- Internally calls `swap`; handles `ERA` (triggers interact) and `SUB` (deferred redex)
- Requires: location must contain a negative term, term must be positive

**⚠️ Lesson:** `swap` handles ERA by calling `freeLoc(loc)` **after** the atomic exchange. This overwrites the new value with VOID. When `move` encounters ERA, the location is freed and the new value is lost — the value is passed to `interact(ERA, pos)` instead.

### APP/LAM example

```c
void appLam(Term neg, Term pos) {
  // neg = APP, pos = LAM

  // Take APP's port 1 (positive argument)
  Term arg = take(portLoc(1, neg));

  // Take LAM's port 2 (positive body)
  Term body = take(portLoc(2, pos));

  // Move body to APP's port 2 location (negative)
  move(portLoc(2, neg), body);

  // Move argument to LAM's port 1 location (negative)
  move(portLoc(1, pos), arg);
}
```

### ERA/LAM example — erasing a body and rewiring ports

```c
void eraLam(Term neg, Term pos) {
  // neg = ERA, pos = LAM
  // After: ERA → port1, NUL → port2, body is erased

  Term body = take(portLoc(2, pos));
  interact(ERA, body);
  move(portLoc(1, pos), NUL);
  return;
}
```

**How it works:**
1. **`take(portLoc(2, pos))`** — gets the body from LAM port 2, frees the port location, returns the body term
2. **`interact(ERA, body)`** — erases the body recursively; handles ERA/SUB chains automatically
3. **`move(portLoc(1, pos), NUL)`** — moves NUL into port 1; `swap` handles the old value (ERA → `interact(ERA, NUL)`, SUB literal → no-op)

## Step 5: Register the rule

In `hvmInit()`, set the jump table entry:

```c
interactions[APP][LAM] = &appLam;
```

Register `eraLeaf` for all positive leaf types that ERA should consume (leaf-only erasure):

```c
interactions[ERA][NUL] = &eraLeaf;
interactions[ERA][I60] = &eraLeaf;
interactions[ERA][F60] = &eraLeaf;
```

For rules where ERA connects to a constructor (like LAM), register the dedicated handler:

```c
interactions[ERA][LAM] = &eraLam;
```

## Step 6: Verify results

Use `take()` to extract and verify rewired values:

```c
Term result = take(portLoc(1, lam));
if (termTag(result) != I60 || getI60(result) != 7) {
  sprintf(msg, "LAM port 1 should be I60(7), got tag %s", tagStr(termTag(result)));
  BOOM(msg);
}
```

**⚠️ Lesson:** `glblAlloced` is always 0 before and after each individual test. Check it after each test to verify no leaks — there's no need to check before each test.

## Build command

Always use `make test-hvm` (not manual `clang`): this ensures correct flags and source files.

```bash
make test-hvm
```

## Error handling

Use the `BOOM(msg)` macro (requires `#include "new.h"`):

```c
char msg[100];
sprintf(msg, "expected X, got Y");
BOOM(msg);
```

`BOOM` prints the message, file, and line number, then aborts.

## Checklist

- [ ] Test function placed before `main()`, no forward declaration
- [ ] Terms built with `makePair()` respecting polarity constraints
- [ ] `interact(neg, pos)` called with correct argument order
- [ ] Rule uses `take`/`move` (not `get`/`free`)
- [ ] Port polarities verified: `move` needs positive term → negative location
- [ ] Rule registered in `interactions` table in `hvmInit()`
- [ ] Results verified with `take()` and tag comparisons
- [ ] Errors use `BOOM(msg)` with `sprintf` into local char array
- [ ] Test checks `glblAlloced == 0` after the interaction to verify no leaks
- [ ] Uses `take` (not `freeLoc`) to clean up ports after tests, verifying returned values
  - Exception: for SUB/LAZ ports, use `get` + `freeLoc` since `take` returns VAR without freeing
- [ ] Uses `make test-hvm` to build (not manual clang)
- [ ] Uses `swap` not `move` when writing negative terms (ERA)
- [ ] Uses `take` + `freePair` pattern when erasing pair bodies
- [ ] Uses `hasLocation(body)` before calling `freePair` on body
- [ ] Handles old values at ports BEFORE overwriting (via `get` + tag dispatch)

## Cleanup Pattern

Never use `freeLoc` directly in tests. Use `take` to clean up ports and verify returned values.

**Exception:** When you expect SUB or LAZ at a port, use `get` + `freeLoc` instead of `take` (since `take` returns VAR for SUB/LAZ and doesn't free the location).

**Pattern for each port:**

- **Ports with content** (leaf, I60, or pair body): use `take` to retrieve and free:

```c
Term r = take(portLoc(n, term));
if (termTag(r) != EXPECTED_TAG) BOOM("expected EXPECTED_TAG");
```

The port location was freed by `take`'s internal `freeLoc`, which coalesces into `freePair` if both ports are now VOID.

- **Ports already VOID**: do not take — `glblAlloced` verifies all pairs are freed automatically.

- **VAR return** (port had SUB or LAZ): the location was NOT freed. Two options:

  **Option A — swap + take** (verifies SUB was replaced):

  ```c
  // Port had SUB — take returns VAR
  r = take(portLoc(n, term));
  if (termTag(r) != VAR) BOOM("expected VAR");

  // Replace SUB with NUL
  r = swap(portLoc(n, term), NUL);
  if (termTag(r) != SUB) BOOM("expected SUB from swap");

  // Free the location
  r = take(portLoc(n, term));
  if (termTag(r) != NUL) BOOM("expected NUL");
  ```

  **Option B — get + freeLoc** (simpler when you just need to verify and free):

  ```c
  // Verify SUB, then free
  r = get(portLoc(n, term));
  if (termTag(r) != SUB) BOOM("expected SUB");
  freeLoc(portLoc(n, term));
  ```

**⚠️ Lesson:** `move` uses `swap` internally, so after a `move`, the target port may contain the moved-in value (not the old SUB). Always check what `take` returns rather than assuming SUB.

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
