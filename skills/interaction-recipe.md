# Implementing HVM Interaction Rules

This recipe documents the process for implementing and testing HVM interaction rules.

## Prerequisites

- `test-hvm.c` with `hvmInit()` / `hvmFree()` in `main()`
- `new.c` with `interactions[16][16]` jump table initialized to `&badrdx`
- `new.h` exposing `glblAlloced`, `nodeCount`, and all term construction functions

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

## Step 5: Register the rule

In `hvmInit()`, set the jump table entry:

```c
interactions[APP][LAM] = &appLam;
```

Register `eraLeaf` for all positive leaf types that ERA should consume:

```c
interactions[ERA][NUL] = &eraLeaf;
interactions[ERA][I60] = &eraLeaf;
interactions[ERA][F60] = &eraLeaf;
interactions[ERA][LAM] = &eraLeaf;
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

**⚠️ Lesson:** Don't check `glblAlloced == 0` across multiple tests. Tests accumulate dangling pairs (LAZ/SUB locations aren't freed by `take`, VAR chain targets aren't freed). Check `glblAlloced` relative to what's expected given the test's setup.

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

## Lessons Learned

1. **`take` on LAZ/SUB doesn't free** — returns a VAR pointing to the location. The location remains allocated.
2. **`swap` on ERA frees the location** — the new value is lost (overwritten by VOID). The value goes to `interact(ERA, pos)` instead.
3. **`move` with ERA does nothing** — `swap` handles ERA internally (frees location + calls interact), so `move`'s own logic is skipped.
4. **Never put negative terms in APP port 1** — APP port 1 must be positive. To test `take` with SUB, use a VAR chain pointing to a separate location.
5. **Don't depend on specific location values** — the free list reuses locations across tests, making absolute location checks fragile. Use relative checks (tag comparisons, `portLoc` results).
6. **`eraLeaf` must be registered per leaf type** — each positive leaf (NUL, I60, F60, LAM, etc.) needs its own entry in the interactions table.
7. **Tests accumulate state** — dangling pairs from LAZ/SUB and VAR chain targets mean `glblAlloced` doesn't reset to 0 between tests.
