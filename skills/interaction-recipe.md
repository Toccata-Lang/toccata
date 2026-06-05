# Implementing HVM Interaction Rules

This recipe documents the process for implementing and testing HVM interaction rules.

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
| 9 | `Makefile` | Build command for `test-hvm` — flags (`SAFETY`, `CHECK_MEM_LEAK`, `STATS`) and source files |

Read in order: the calculus defines the rules, the implementation shows how they work, the tests show how to exercise them, and the graph/debug files help diagnose issues.

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

**⚠️ Lesson:** Don't check `glblAlloced == 0` across multiple tests. Tests accumulate dangling pairs (LAZ/SUB locations aren't freed by `take`, VAR chain targets aren't freed). Save `u64 initialAlloced = glblAlloced;` at the start of each test and compare at the end:

```c
u64 initialAlloced = glblAlloced;
// ... build terms, interact, take ...
if (glblAlloced != initialAlloced) {
  sprintf(msg, "glblAlloced should be %lld, got %lld", (long long)initialAlloced, (long long)glblAlloced);
  BOOM(msg);
}
```

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
- [ ] Test saves `u64 initialAlloced = glblAlloced;` and compares at end
- [ ] Uses `make test-hvm` to build (not manual clang)
- [ ] Uses `swap` not `move` when writing negative terms (ERA)
- [ ] Uses `take` + `freePair` pattern when erasing pair bodies
- [ ] Uses `hasLocation(body)` before calling `freePair` on body
- [ ] Handles old values at ports BEFORE overwriting (via `get` + tag dispatch)

## Lessons Learned

1. **`take` on LAZ/SUB doesn't free** — returns a VAR pointing to the location. The location remains allocated.
2. **`swap` on ERA frees the location** — the new value is lost (overwritten by VOID). The value goes to `interact(ERA, pos)` instead.
3. **`move` with ERA does nothing** — `swap` handles ERA internally (frees location + calls interact), so `move`'s own logic is skipped.
4. **Never put negative terms in APP port 1** — APP port 1 must be positive. To test `take` with SUB, use a VAR chain pointing to a separate location.
5. **Don't depend on specific location values** — the free list reuses locations across tests, making absolute location checks fragile. Use relative checks (tag comparisons, `portLoc` results).
6. **`eraLeaf` must be registered per leaf type** — each positive leaf (NUL, I60, F60) needs its own entry in the interactions table. Constructors like LAM need dedicated handlers.
7. **Tests accumulate state** — dangling pairs from LAZ/SUB and VAR chain targets mean `glblAlloced` doesn't reset to 0 between tests. Save `u64 initialAlloced = glblAlloced;` at test start and compare at end.
8. **`move` rejects negative terms** — the SAFETY check in `move` aborts if `isNegative(pos)`. To put ERA (negative) into a location, use `swap` directly instead of `move`.
9. **`move` assumes old value is negative** — `move` calls `swap`, gets the old value, and if it's not SUB/ERA, pushes it as a redex. If the old value is positive (e.g., LAM body), this creates an invalid redex. Use `take` + `swap` pattern instead when the location may contain a positive term.
10. **Erased pair bodies must be freed** — when ERA erases a body that is a pair (like a nested LAM), `take` frees the port location but not the pair's own location. After `take`, call `freePair(termLoc(body) & 0xFFFFFFFE)` if `hasLocation(body)` is true.
11. **`swap` + `freePair` pattern for erasing bodies** — when implementing rules where a term is erased (like ERA/LAM), use: `take(port)` to free port location and get body, `freePair(termLoc(body) & 0xFFFFFFFE)` if body has location, then `swap(port, newVal)` to write the replacement.
12. **Always use `make test-hvm`** — not manual `clang`. Ensures correct flags (`CHECK_MEM_LEAK`, `SAFETY`, `STATS`) and source files.
13. **Handle old values BEFORE overwriting ports** — use `get` to read the old value, handle it based on tag (SUB → push redex, ERA → interact, other → consumed by the rule), then `swap` the new value. Don't silently discard old values.
14. **Never use 42 as a test integer value** — it's cliche and irritating. Pick something else.
