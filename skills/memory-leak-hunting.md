# Memory Leak Hunting

## ⛔ CRITICAL: Commit after every change

**Commit after every meaningful change.** Do not batch changes. Each commit should be a single, focused change that builds and passes tests. This keeps history clean and makes rollback easy.

## ⛔ CRITICAL: Never use 42 as test data

**Never use 42 as a test integer value** — it's cliche and irritating. Pick something else.

## ⛔ CRITICAL: Don't remove BOOM calls in runtime3.c

**Never remove `BOOM()` calls in `runtime3.c` unless they actually get hit during testing.** They mark untested code paths. After conversion, check which BOOMs are still unused — those are the paths that still need test coverage.

## Overview

This project detects memory leaks via `CHECK_MEM_LEAK=1` compile flag. The hash-map unit tests (`make test-hash-map`) run C-level tests that verify malloc/free accounting. Each test calls `check_counts()` which asserts:

- `malloc_count == expected_malloc` — exact allocation count
- `free_count == expected_free` — exact free count
- `malloc_count - free_count == pool_delta` — unfreed allocations must match pool objects

When a test leaks, `check_counts()` calls `BOOM()` and aborts with a message like:

```
FAIL testFoo at line 123: expected malloc_count=5, got 6
BOOM: malloc_count mismatch
```

or:

```
FAIL testFoo at line 456: malloc=10 free=8 unfreed=2 pool_delta=1
BOOM: leak detected: unfreed allocations don't match pool objects
```

## Build & Run

```bash
# Build and run all hash-map unit tests
make test-hash-map
```

This compiles and runs `regression-tests/test-hash-map.c` with `CHECK_MEM_LEAK=1`. Each test calls `check_counts()` which will `BOOM()` on mismatch.

To run a single test, edit `test-hash-map.c` to call only that function, or look for the test's output in the full run.

## Leak Diagnosis Process

### Step 1: Identify the leak

```bash
make test-hash-map 2>&1 | grep -A2 "FAIL\|BOOM"
```

Look for:
- `expected malloc_count=X, got Y` — more allocations than expected
- `free_count` mismatch
- `unfreed != pool_delta` — unfreed allocations don't match pool objects

### Step 2: Find the leak source

Common patterns in this codebase:

1. **Missing `dec_and_free` after `incRef`** — reference created in one branch but not cleaned up in all paths
2. **`freePair()` not called** — pair bodies (the two slots of a pair) leaked when the pair itself is freed
3. **Pool accounting wrong** — pool created but expected count not updated in `check_counts()` call
4. **Old value not freed during mutation** — `mutateAssoc` updates in-place but doesn't `dec_and_free` the old value
5. **Sub-node clone doesn't free old reference** — `copyAssoc` clones a sub-node but leaves the original reference dangling
6. **Collision node not freed** — `HashCollisionNode` created via `malloc_hashCollisionNode()` needs `dec_and_free` (no pool)

### Step 3: Locate the code

Relevant files:

| File | Purpose |
|---|---|
| `runtime3.c` | Runtime support — `incRef`, `dec_and_free`, `freePair`, node allocators |
| `runtime3.h` | Type definitions, extern declarations |
| `regression-tests/test-hash-map.c` | All unit tests + `check_counts()` helper |
| `regression-tests/test-hash-map.c` | Pool accounting model comments at top of file

### Step 4: Trace the allocation

Look at the failing test in `regression-tests/test-hash-map.c`:

```bash
grep -n "check_counts\|malloc_count\|free_count" regression-tests/test-hash-map.c
```

The test file has a pool accounting model comment block at the top explaining:
- `malloc_bmiNode(<20)`: first call allocates 10 nodes via `my_malloc` (+1), explicit +9 spare = 10 mallocs. Subsequent calls pull from pool (no malloc change).
- `freeBitmapNode(<20)`: puts node back in pool, NO free_count increment.
- `freeBitmapNode(>=20)`: actually frees, free_count += 1.
- `malloc_arrayNode`: same pool pattern as BMI.
- `freeArrayNode`: always recycles, NO free_count increment.
- `malloc_hashCollisionNode`: direct `my_malloc`, malloc_count += 1.
- `freeHashCollisionNode`: direct free, free_count += 1.

### Step 5: Fix and verify

1. Comment out all tests in `main()` except the one you're fixing
2. Apply the fix (e.g., add `dec_and_free` for leaked value)
3. Re-run: `make test-hash-map`
4. If `check_counts` fails with wrong expected numbers — **this is normal**. `check_counts` is there to prevent regressions. Just update the expected values to match reality and move on.
5. Verify the test passes (no `BOOM`)
6. **Final regression check** — uncomment ALL tests marked as [x] in the plan, comment out the rest. Re-run: `make test-hash-map`. Verify nothing regressed.
7. Check `git diff` — only the fix and possibly `test-hash-map.c` expected values should have changed

## Known Leak Patterns

<!-- FILL THIS IN AS YOU DISCOVER LEAKS -->

<!--
Format:

### Pattern N: [One-line title]

**Location:** `runtime3.c:LINE` or `test-hash-map.c:LINE`

**Test:** `test<Name>` in `regression-tests/test-hash-map.c`

**Cause:** [What caused the leak — e.g., "bmiMutateAssoc 1c path doesn't dec_and_free old value"]

**Fix:** [What changed — e.g., "Added dec_and_free(oldVal) before storing newVal in bmiMutateAssoc"]

---
-->

### Pattern 1: Double incRef on bmiChild

**Location:** `runtime3.c:2271`

**Test:** `testBmiCopyAssocSubNodeNoChange`

**Cause:** `bmiChild()` already `incRef`s the child before returning it. But `bmiCopyAssoc` was calling `incRefVal(child, 1)` again, causing a double incRef. The extra ref was never decremented, leaking the sub-node.

**Fix:** Removed `incRefVal(child, 1)` wrapper — `bmiChild` already handles the incRef. Changed from `copyAssoc(incRefVal(child, 1), ...)` to `copyAssoc((Value *)child, ...)`.

---

## Common Fix Patterns

### Missing `dec_and_free` after `incRef`

```c
// BEFORE — leaks when branch not taken
Value *tmp = (Value *)incRef((Term)(Value *)someTerm, 1);
if (condition) {
  doSomething(tmp);
}
// tmp leaked if condition was false

// AFTER
Value *tmp = (Value *)incRef((Term)(Value *)someTerm, 1);
if (condition) {
  doSomething(tmp);
}
dec_and_free(termVal((Term)(Value *)tmp), 1);
```

### Missing `freePair` on pair body

```c
// BEFORE — pair body (2 slots) leaked
freeValue((Value *)somePair);

// AFTER — free the pair AND its body
freePair((Value *)somePair);
```

### Old value not freed during in-place mutation

```c
// BEFORE — bmiMutateAssoc 1c path: keys equal, values different
// oldVal is still referenced, leaked
((BitmapIndexedNode *)node)->array[2 * idx + 1] = newVal;

// AFTER — dec_and_free old value before storing new
Term oldVal = ((BitmapIndexedNode *)node)->array[2 * idx + 1];
dec_and_free(termVal((Term)(Value *)oldVal), 1);
((BitmapIndexedNode *)node)->array[2 * idx + 1] = newVal;
```

### Pool accounting wrong

The pool model in `test-hash-map.c` comments:
- First call to `malloc_bmiNode(N)` where N < 20: creates pool, `malloc_count += 10` (1 for pool array + 9 explicit)
- Subsequent calls to same pool: `malloc_count += 0` (pulls from pool)
- `freeBitmapNode(N < 20)`: `free_count += 0` (recycles to pool)
- `freeBitmapNode(N >= 20)`: `free_count += 1` (actual free)
- `malloc_hashCollisionNode`: `malloc_count += 1` (no pool)
- `freeHashCollisionNode`: `free_count += 1` (no pool)

`check_counts()` is a regression guard — it asserts exact expected numbers. If you fix a real leak (e.g., added a `dec_and_free`), the expected `free_count` will go up. **Just update the expected values in `check_counts()` and move on.** Don't second-guess it — if the code is correct and the numbers change, the numbers are right.

### Using `prefs` to debug ref counts

When a test fails with `pool_delta != unfreed`, use `prefs("name", (Term)value)` to print a value's current ref count. Call it on terms at key points (after creation, after storage in nodes, before free) to trace where refs are going wrong. `prefs` only prints — it doesn't modify refs. It's a read-only diagnostic tool.

Example:
```c
prefs("key1 after creation", key1);       // should show refs=1
prefs("key1 after bmiMutateAssoc", key1); // should show refs=1 (stored but not incRef'd)
prefs("key1 before freeing result", key1); // check refs haven't gone to 0 prematurely
```

### Verifying strings are properly freed

After converting a test to use `stringValue()`, add a `prefs` call **right before `check_counts`** to verify the test's own strings are properly freed:

```c
// Clean up
dec_and_free((Term)branchResult, 1);

prefs("key1", key1);

check_counts("testBmiCopyAssocBranch", 0, 0, __LINE__);
```

Expected ref counts at this point:
- **`refs=1`** — string was created with refs=1, and all node references have been cleaned up. The string is still alive (not freed) because refs hasn't reached 0, but it's properly accounted for.
- **`refs > 1`** — the string is still referenced by something that hasn't been freed. The original node may not have been freed, or a node clone still holds a reference.
- **`refs=0`** — the string was over-freed. A `dec_and_free` was called too many times.

If `pool_delta != unfreed` and `prefs` shows `refs > 1` for a string the test owns, the string's reference chain is the leak source. Trace back to find which node still holds the reference and needs to be freed.

## Checklist

### During diagnosis
- [ ] `make test-hash-map 2>&1 | grep -A2 "FAIL\|BOOM"` identifies the failing test
- [ ] Test function identified in `test-hash-map.c`
- [ ] All other tests commented out in `main()` (only fix target active)
- [ ] Pool accounting model reviewed (comments at top of test-hash-map.c)
- [ ] Relevant runtime3.c code traced
- [ ] Leak pattern identified and documented above
- [ ] Fix applied, expected values updated, test passes in isolation

### After fix
- [ ] Final regression check: all [x] tests from plan uncommented, rest commented out, `make test-hash-map` passes
- [ ] Pattern documented in "Known Leak Patterns" section
- [ ] `git diff` shows only the fix (and possibly `test-hash-map.c` expected value updates)

## Argument Passing Semantics

**When a value is passed as an argument to a function, its refs are automatically decremented.** If the refs become 0, the value is freed and refs is set to **-10** (the freed marker). You can verify this by calling `prefs` right after the function call.

```c
prefs("key1 before bmiMutateAssoc", key1);   // refs=1
Value *result = bmiMutateAssoc(node, key1, val1, hash1, 0);
prefs("key1 after bmiMutateAssoc", key1);    // refs=-10 (freed)
```

**Exception:** If the function returns that same arg value back as the result, the ref is NOT decremented. The value survives the call with its original ref count.

This is the most common source of pool_delta mismatches: a string passed to a function is freed automatically, but the test still holds a variable pointing to it. The string is gone, but the pool slot is lost.

## Value Lifetime Rules

### A value passed as an argument may be freed by the function

A function receiving a value as an argument may free it — either directly, or through a called subroutine. The function is not required to return the value for it to be freed.

```c
// bmiReplaceCopied frees its 'node' argument internally
Value *bmiReplaceCopied(BitmapIndexedNode *node, ...) {
    dec_and_free((Term)node, 1);  // node is freed here
    return newNode;               // node is NOT returned
}
```

After the call returns, the original value is dead (refs=-10). Any further use is a use-after-free.

### incRef(N-1) when passing an argument to N functions

If a function needs to pass one of its arguments to multiple functions, it must incRef (N-1) times before the calls. Each function call decrements refs by 1, so without extra incRef's the value would be freed after the first call.

```c
// BEFORE — arg freed after func1, crashes or corrupts in func2
func1(arg);  // arg freed here
func2(arg);  // use-after-free!

// AFTER — arg survives both calls
incRef(arg, 1);  // refs 1→2
func1(arg);      // refs 2→1
func2(arg);      // refs 1→0 → freed
```

### Each execution path must cause exactly 1 total decrement

If an argument is used in different execution paths (if/else, switch cases), each path must cause the total decrements of the argument to be exactly 1. If one path decrements 0 times and another decrements 2+, the value will either leak (refs never hits 0) or double-free (refs goes negative).

```c
// BEFORE — path A leaks, path B double-frees
if (condition) {
    func1(arg);  // 1 decrement — correct
} else {
    func2(arg);  // 1 decrement
    func3(arg);  // 2nd decrement — double-free!
}

// AFTER — both paths cause exactly 1 decrement
if (condition) {
    func1(arg);  // 1 decrement
} else {
    incRef(arg, 1);  // refs 1→2
    func2(arg);  // refs 2→1
    func3(arg);  // refs 1→0 → freed
}
```

### createNode doesn't incRef

`createNode` stores pointers directly into the node's array without calling `incRef`. **The caller is responsible for ensuring keys/values have been incRef'd before passing them in.**

```c
// createNode signature
Value *createNode(int shift, int64_t key1hash, Value *key1, Value *val1,
                  int64_t key2hash, Value *key2, Value *val2);

// Caller must incRef before passing
Value *newLeaf = createNode(shift, hash1, incRefVal(key1, 1), val1,
                            hash2, incRefVal(key2, 1), val2);
```

## Debugging Techniques

### Use prefs at strategic points

Call `prefs("tag", term)` after creation, after function calls, and before freeing. Without it, you can't tell whether a value was freed prematurely, over-freed, or is still alive when it should be dead.

### Trace through every call site

A function may receive an argument, pass it to another function, and that nested function may free it. You need to follow the chain — don't stop at the first function boundary.

### Verify freeBitmapNode actually iterates the array

`freeBitmapNode` does call `dec_and_free` on each array entry, but you need to verify the node's bitmap is non-zero and entries are non-null at the time of free. Add debug prints to confirm the loop runs.

### Watch for incRef's that are never undone

An `incRef` in a function argument (e.g., `sha1((FnArity *)0, incRef(key, 1))`) may never be decremented if the function pointer call doesn't trigger argument passing semantics. Every incRef must have a matching decrement somewhere.

## Common Leak Patterns

### Strings allocated by tests

When converting from I60 to `stringValue()`, the test's strings may not be properly freed. Add `prefs` right before `check_counts` to verify string ref counts are sane.

### Node contents aren't auto-freed by the caller

When a node is freed via `dec_and_free`, `freeBitmapNode` iterates the array and calls `dec_and_free` on each entry. But if the caller already incRef'd values before passing to `createNode`, those incRef's must be accounted for — otherwise refs won't reach 0 and the strings leak.

## Lessons Learned

1. **Pool objects are not leaks** — objects sitting in central free-list pools are accounted for. `check_counts()` verifies `malloc_count - free_count == pool_delta`. A leak is when unfreed allocations DON'T match pool objects.
2. **I60 terms don't allocate** — I60 integers are leaf terms, no heap allocation. Leaks only involve VAL terms (String, Vector, BMI, ArrayNode, HashCollisionNode, etc.).
3. **Every `incRef` needs a matching `dec_and_free`** — track ref counts through ALL control flow paths. If a reference is created in one branch but not cleaned up in another, it leaks.
4. **`freePair()` is not `freeValue()`** — `freeValue()` frees the struct but not the pair body. `freePair()` frees both. Use `freePair` for pair-typed terms.
5. **Pool recycling doesn't increment `free_count`** — `freeBitmapNode(<20)` and `freeArrayNode` recycle into pools, `free_count += 0`. Only `freeBitmapNode(>=20)` and `freeHashCollisionNode` increment `free_count`.
6. **First pool creation counts as malloc** — first call to `malloc_bmiNode(N)` or `malloc_arrayNode()` creates the pool: `malloc_count += 10` (1 for pool array + 9 explicit allocations). Subsequent calls: `malloc_count += 0`.
7. **In-place mutation must free old values** — `bmiMutateAssoc` 1c path (same key, different value) must `dec_and_free` the old value before storing the new one.
8. **`check_counts()` is a regression guard, not a leak detector** — it asserts exact expected numbers to catch future regressions. If you fix a real bug (e.g., add a missing `dec_and_free`), the expected numbers will change. Just update them and move on. Don't waste time trying to make the numbers "work" when the code is already correct.
9. **Different `itemCount` = different pool index** — `malloc_bmiNode(1)` and `malloc_bmiNode(2)` use different pool slots. Each first-call creates a new pool.
10. **Always verify compilation after each change** — don't wait until all functions are done. Compile frequently to catch type errors early.
