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

### Pattern 2: BMI pool exhaustion from bitmap-count-based recycling

**Location:** Pool system in `runtime3.c:malloc_bmiNode` / `freeBitmapNode`

**Test:** `testBmiCopyAssocSubNodeNoChange` (when run with other [x] tests)

**Cause:** `freeBitmapNode` returned nodes to pools indexed by the node's **actual bitmap count** at free time (`freeBMINodes[cnt]`), not the requested `itemCount`. Tests like `testEmptyBmiNode`, `testBmiDissoc`, `testBmiDissocEmpty` create nodes with `malloc_bmiNode(1)` that end up with bitmap count 0 when freed (empty nodes). These went to `freeBMINodes[0]` instead of `freeBMINodes[1]`. Over 16 tests, 8 nodes from itemCount=1 and 2 from itemCount=2 leaked into itemCount=0, leaving only 2 nodes in the itemCount=1 pool — not enough for `testBmiCopyAssocSubNodeNoChange` which needs 4 allocations.

**Fix:** Added `int32_t itemCount` field to `BitmapIndexedNode` struct (in `core.h` and `runtime3.h`). `malloc_bmiNode` stores the requested `itemCount` in the node. `freeBitmapNode` uses `node->itemCount` for the pool index instead of `__builtin_popcount(node->bitmap)`. The iteration loop still uses `__builtin_popcount(node->bitmap)` to only free actual entries.

---

### Pattern 3: bmiReplaceMutate orphans the outer node's refs on the moved entry

**Location:** `runtime3.c:bmiReplaceMutate` (createNode branch, 1e path of `bmiMutateAssoc`)

**Test:** `testBmiMutateAssocBranch` in `regression-tests/test-hash-map.c`

**Cause:** The 1e branch moves the existing entry (currKey/currVal) into a new sub-node via `createNode`, which stored `incRefVal(currKey, 1)` / `incRefVal(currVal, 1)` — extra refs on top of the outer node's original refs. It then zeroed the outer node's key slot (`bmiSetKey(node, bit, 0)`) without releasing the outer node's original refs. Unlike the copy path (`bmiReplaceCopied`), the node is mutated in place and never freed, so the original refs were orphaned — the strings never returned to the pool. The branch also passed `incRefVal(key, 1)` / `incRefVal(val, 1)` for the incoming key/val, unlike `bmiReplaceCopied` which transfers the caller's refs directly.

**Fix:** Pass all four values to `createNode` without incRef — the sub-node takes over the outer node's refs on currKey/currVal and the caller's refs on key/val (pure transfer, same pattern as the collision branch directly above it). No incRefs, no decs needed.

---

### Pattern 4: bmiMutateAssoc 1a path orphans parent's slot ref on replaced sub-node

**Location:** `runtime3.c:bmiMutateAssoc` (1a path — bit set, `child != 0`)

**Test:** `testBmiMutateAssocSubNodeRecurse` in `regression-tests/test-hash-map.c`

**Cause:** `bmiChild()` incRefs the child before returning it, so the old sub-node holds 2 refs (parent's slot ref + `bmiChild`'s temp ref). `mutateAssoc` on a sub-node with refs=2 takes the copy path, and `bmiClone` internally `dec_and_free`s the old sub-node once — consuming one ref. The 1a path then overwrote the parent's slot with the clone via `bmiSetVal`, orphaning the parent's remaining slot ref on the old sub-node. The old sub-node was left at refs=1, unreachable — never returned to the pool (`unfreed=0 pool_delta=-1`).

**Fix:** Added `dec_and_free(child, 1)` after `bmiSetVal(node, bit, n)` in the 1a path. When `n != child` (clone), this releases the orphaned parent slot ref (old sub-node: 2 → 1 via bmiClone → 0 via this dec → freed). When `n == child` (no-op/in-place), the callee consumed no refs, so this dec balances `bmiChild`'s temp ref (child: 2 → 1, still held by the parent's slot).

---

### Pattern 5: addMutateBMI promote path drops keys/values into empty sub-nodes

**Location:** `runtime3.c:addMutateBMI` (2a path — bit not set, n >= 16, promote to ArrayNode)

**Test:** `testBmiMutateAssocPromote` in `regression-tests/test-hash-map.c`

**Cause:** The promote path built the 17 one-entry sub-nodes with `cloneBitmapIndexedNode(&emptyBMI, idx, key, val)`. `emptyBMI` has bitmap 0, so the clone's copy loop never runs — the sub-node comes back empty (bitmap 0, no entries) and the key/val are silently dropped. The dropped strings' refs never reach 0, so they never return to the string pool (`unfreed=150 pool_delta=134` — 16 leaked key strings). The k/v branch also passed `incRef(key, 2)` / `incRef(val, 1)` into the drop, compounding the leak. The I60-only version of the test never caught this: I60s have no refs, and the test only counted non-zero ArrayNode slots (17 empty sub-nodes still count as 17).

**Fix:** Build each one-entry sub-node directly: `malloc_bmiNode(1)`, set `bitmap = bitpos(hash, newShift)`, store the key/val in slots 0/1 (pure ref transfer — the sub-node takes over the caller's refs on the new key/val and the old node's refs on each moved pair, with the old slots zeroed before the old node is freed). Existing pairs are re-hashed with `sha1((FnArity *)0, incRef(key, 1))` to find their bit position at `shift + 5`.

---

### Pattern 6: vectGet returns an incRef'd copy the caller must free

**Location:** `regression-tests/test-hash-map.c:testCollisionVec` (test cleanup); API contract in `runtime3.c:vectGet`

**Test:** `testCollisionVec` in `regression-tests/test-hash-map.c`

**Cause:** `vectGet(vect, i)` returns `dupeVal(&array[i])` — an incRef'd copy of the element. The caller owns that ref and must `dec_and_free` it. The test's verification loop called `vectGet` once per pair but never freed the results, so each pair vector was left at refs=2. Freeing the result vector then dropped the pairs to refs=1 — never returned to the Vector pool (`unfreed=0 pool_delta=-2`). The test also initially never freed the result vector itself (`pool_delta=-3`). Both are test-cleanup gaps, not runtime bugs.

**Fix:** Added `dec_and_free(pairTerm, 1)` after each `vectGet` use, and `dec_and_free((Term)vecResult, 1)` before `check_counts`. Both return the vectors to the pool, restoring the baseline.

---

### Pattern 7: bmiKey/bmiVal return borrowed refs, Toccata bmiCopyAssoc consumes them as owned (double-free)

**Location:** `hvm-core.toc` (bmiKey/bmiVal inline-C wrappers); root cause split between `runtime3.c:bmiKey`/`bmiVal` (borrowed) and the Toccata `bmiCopyAssoc` (consumes as owned)

**Test:** `test-bmi` (regression test, `make test-bmi`)

**Cause:** `bmiChild()` returns an **owned** ref (`incRef` before returning), but `bmiKey()`/`bmiVal()` return a **borrowed** pointer into the node's array (no `incRef`). The Toccata `bmiCopyAssoc` treats `currKey`/`currVal` as owned — it consumes them via `(= currKey k)` / `(= currVal v)` (whose `type-num` does `dec_and_free`). Because `bmiKey`/`bmiVal` never incremented the refcount, that consumption was an **extra decrement** that drove the BMI node's own reference to the value to 0 prematurely. The value was freed (refs → -10), its memory reused, and when the BMI node was later freed, `freeBitmapNode` decremented the stale slot → **double-free** (`decRefs: refs too small: 1 -11`).

**Fix:** Made the Toccata inline-C wrappers for `bmiKey`/`bmiVal` `incRef` the result, so callers get an owned reference (consistent with `bmiChild`). Fixed the wrappers rather than the C functions because the C callers (`bmiCopyAssoc`/`bmiMutateAssoc` in runtime3.c) depend on the borrowed-ref behavior and `incRefVal` before use — fixing the C functions would have double-incRef'd there.

**Key insight:** This is a **double-free**, not a leak. The value was over-decremented (refs went negative), not under-decremented (refs stuck above 0). See the "Double-Free / Use-After-Free Diagnosis" section below.

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

## Double-Free / Use-After-Free Diagnosis

A **double-free** is the opposite of a leak: instead of refs never reaching 0 (leak), refs go **below** the freed marker. This is a use-after-free / over-decrement.

### Failure signature

A leak shows up as `malloc_count != free_count` or `pool_delta != unfreed` at the end of a test. A **double-free** aborts mid-test with:

```
failure in decRefs, refs too small: <delta> <refs> <ptr>
Aborted (core dumped)
```

The freed marker is `refsError = -10`. When a value is freed, its refs is set to -10. A double-free is detected when `decRefs` is called on a value whose refs is **already -10** — the `fetch_sub` drives it to -11 (or lower), and `decRefs` aborts because the old value (`newRefs`) is `< deltaRefs`. So `refs too small: 1 -11` means "this value was already freed, and something decremented it again."

### The crash point is NOT the bug location

The abort happens when a **container** is freed and iterates its array, decrementing a stale slot. In Pattern 7 the crash was in `freeBitmapNode` (freeing a BMI node) — but the actual extra decrement happened **earlier**, when the value was first over-decremented. The backtrace at the crash tells you **which container** is being freed, not **why** the slot is stale. You must trace **backwards** from the crash to find the first over-decrement.

### Memory reuse confuses the diagnosis

A freed value's memory is **reused** for a new value (same pointer, new contents). So the `type` you see at the crash point may be the **new** value's type, not the original's. In Pattern 7 the double-freed value showed `type=43` (SomeType) at the crash, but it was originally a `Val` (type 49) — the Val was freed, its memory reused for a Some, and the stale BMI slot pointed at the reused memory. Don't trust the type at the crash; trust the refcount trajectory.

### Technique: refcount trajectory logging

The most effective tool is to log **every** `incRef`/`decRefs` with `(pointer, type, delta, before, after)` to a file, then find the value whose refcount hits 0 (freed) **while a container still holds a reference**, and trace who did the extra decrement.

Add to `incRef` and `decRefs` in `runtime3.c` (tag all debug code with a unique prefix so cleanup is a single grep):

```c
static FILE *dbg_log = (FILE *)0;
static long dbg_seq = 0;
static void dbg_logref(const char *op, Value *v, int delta, REFS_SIZE before, REFS_SIZE after) {
  if (!dbg_log) dbg_log = fopen("/tmp/refs.log", "w");
  fprintf(dbg_log, "%ld %s %p type=%ld delta=%d before=%ld after=%ld\n",
          ++dbg_seq, op, (void *)v, (long)v->type, delta, (long)before, (long)after);
}
// in decRefs, after the fetch_sub:  dbg_logref("dec", v, deltaRefs, newRefs, newRefs - deltaRefs);
// in incRef,  after the CAS loop:   dbg_logref("inc", v, deltaRefs, refs, newRefs);
// in the decRefs failure path, before abort: fflush(dbg_log);
```

**Flush before the abort** — the log is buffered and lost on `abort()` otherwise.

Then read the log and find the double-freed value's trajectory. Look for the value that goes `before=1 after=0` (freed) and then reappears later as `before=-10 after=-11` (the double-free). The decrements **between** those two points are the over-decrements — trace each one back to its source.

### Technique: backtrace at the failure point

Add a backtrace to the `decRefs` failure path to see **which container** is being freed:

```c
#include <execinfo.h>
// in the decRefs failure path, before abort:
void *bt[64]; int n = backtrace(bt, 64); backtrace_symbols_fd(bt, n, 2);
```

Resolve the offsets with `addr2line -e <binary> -f -C <offset>`. In Pattern 7 this showed `freeBitmapNode → dec_and_free → decValRef → decRefs` — identifying the BMI node as the container being freed, which pointed me at the BMI's array slots.

### Root-cause class: borrowed vs owned reference convention

A common root cause is an **inconsistent ownership convention** across sibling functions. In Pattern 7, `bmiChild` returned an owned ref (`incRef`) but `bmiKey`/`bmiVal` returned borrowed refs (no `incRef`). Callers written to the "owned" convention over-decremented the borrowed results. When you find an over-decrement, check whether the function that produced the value `incRef`s its result — and compare it against its siblings.

**Fix location matters.** When fixing, understand **all** callers before deciding where to fix. In Pattern 7 I fixed the Toccata inline-C wrappers (not the C `bmiKey`/`bmiVal`) because the C callers (`bmiCopyAssoc`/`bmiMutateAssoc`) rely on the borrowed-ref behavior and `incRefVal` before use. Fixing the C functions would have double-incRef'd in the C callers.

### The `dupeArg` under-count

When a Toccata variable is used multiple times, the compiler emits `dupeArg` calls that `incRef` for each duplicate. But `dupeArg` only counts the **duplicates** — the original reference is only counted if it was owned when it arrived. If the original was a **borrowed** ref (not counted), the total refcount is short by one, and consuming all the uses over-decrements. This is how Pattern 7's borrowed `currVal` became a double-free: the BMI's ref (1) + `dupeArg` (1) = 2, but the code held 3 logical refs (BMI + original + dupe), so the final consumes drove it to 0 while the BMI still held it.

## lldb Workflow

**lldb is available** (`/usr/bin/lldb`, v18.1.3). It's the best tool for *live* debugging of the C runtime — especially double-frees, where you want to stop at the exact over-decrement instead of reconstructing it from logs. It complements the trajectory-logging technique above: logging is better for "trace the whole history", lldb is better for "stop me right here and show me why".

### Core loop

```bash
lldb ./regression-tests/test-bmi
(bmi) b decRefs                 # breakpoint by function name
(bmi) b runtime3.c:173          # ...or by file:line
(bmi) r                         # run (add args if needed: command import -a party-pooper)
(bmi) n / s / c                 # step over / step into / continue
(bmi) p v->refs                 # print an expression
(bmi) frame var                 # locals in the current frame
(bmi) bt                        # backtrace
```

### Catch a double-free at the first over-decrement

The freed marker is `refsError = -10`. Break the moment `decRefs` is about to decrement an already-freed value — this stops you at the *root cause*, not the later container-free crash:

```
(bmi) b decRefs -c 'v->refs == -10'
(bmi) r
...stops on the first over-decrement...
(bmi) bt                        # who is decrementing the stale ref
(bmi) p v                      # inspect the (reused) value
```

This directly resolves the "crash point ≠ bug location" problem: the backtrace here shows the owner of the stale reference.

### Conditional breakpoints (avoid flooding)

```
(bmi) b decRefs -c 'v == (Value *)0xADDR'      # one specific value
(bmi) b decRefs -c 'newRefs < deltaRefs'       # any over-decrement
(bmi) b incRef  -c 'v->type == 43'             # only Some values
```

### Watchpoints on a refcount field

```
(bmi) p &v->refs
(bmi) watchpoint set variable v->refs          # stop on any read/write
```

**Caveat:** this runtime uses atomics and a free-list/pool, so `refs` gets many *legal* writes. Watchpoints are noisy here — prefer a conditional breakpoint on the failure predicate (`v->refs == -10`) over a raw watchpoint.

### Memory inspection

```
(bmi) memory read <addr>                     # raw bytes
(bmi) memory histogram -a <addr>             # identify what owns/points at a value
(bmi) p *(Value *)0xADDR                     # interpret memory as a Value
```

### Batch / scripted runs (no TTY)

Useful for non-interactive debugging:

```bash
lldb -b -o 'b decRefs' -o 'r' -o 'bt' ./regression-tests/test-bmi
```

### Core dumps

```bash
lldb ./regression-tests/test-bmi -c core     # inspect the abort state
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
11. **Double-free vs leak are different failure modes** — a leak is refs stuck above 0 (`malloc_count != free_count` at the end); a double-free is refs below the freed marker (-10), which aborts mid-test with `decRefs: refs too small`. Don't apply leak-hunting techniques (pool accounting) to a double-free — the value was over-freed, not under-freed.
12. **Crash point ≠ bug location for double-frees** — a double-free aborts when a container is freed and touches a stale slot. The extra decrement happened earlier. Trace backwards with refcount trajectory logging to find the first over-decrement.
13. **Memory reuse lies about types** — a freed value's memory is reused, so the `type` at the crash may be the new value's type, not the original's. Trust the refcount trajectory, not the crash-time type.
14. **Check ownership-convention consistency** — when a value is over-decremented, check whether the producing function `incRef`s its result and compare against its siblings (e.g., `bmiChild` incRefs but `bmiKey`/`bmiVal` did not). An inconsistent convention across sibling accessors is a common root cause.
15. **Fix where the convention is broken, considering all callers** — understand all callers before choosing the fix location. Fixing the wrong layer (e.g., the C function instead of the Toccata wrapper) double-incRef's in callers that already manage the ref manually.
