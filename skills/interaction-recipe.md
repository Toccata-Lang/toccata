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

  // Pre-checks (e.g., glblAlloced == 0)

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
- Follows VAR chains
- Returns a positive value
- Special case: for `SUB` and `LAZ`, returns a `VAR` pointing back to the location

**`move(Location negLoc, Term pos)`**:
- Moves a **positive** term into a **negative** location
- Internally calls `swap`; handles `ERA` (triggers interact) and `SUB` (deferred redex)
- Requires: location must contain a negative term, term must be positive

### APP/LAM example

```c
void app_lam(Term neg, Term pos) {
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
interactions[APP][LAM] = &app_lam;
```

## Step 6: Verify results

Use `take()` to extract and verify rewired values:

```c
Term result1 = take(portLoc(2, app));
Term result2 = take(portLoc(1, lam));

if (termTag(result1) != I60 || getI60(result1) != 7) {
  sprintf(msg, "result1 should be I60(7), got tag %s", tagStr(termTag(result1)));
  BOOM(msg);
}
```

Check all nodes freed:

```c
if (glblAlloced != 0) {
  sprintf(msg, "glblAlloced should be 0, got %lld", (long long)glblAlloced);
  BOOM(msg);
}
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
- [ ] Results verified with `take()` and `glblAlloced == 0`
- [ ] Errors use `BOOM(msg)` with `sprintf` into local char array
