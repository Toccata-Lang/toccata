# Convert Test to Use Allocated Strings

## ⛔ CRITICAL: Commit after every change

**Commit after every meaningful change.** Do not batch changes. Each commit should be a single, focused change that builds and passes tests. This keeps history clean and makes rollback easy.

## ⛔ CRITICAL: Never use 42 as test data

**Never use 42 as a test integer value** — it's cliche and irritating. Pick something else.

## ⛔ CRITICAL: Don't remove BOOM calls in runtime3.c

**Never remove `BOOM()` calls in `runtime3.c` unless they actually get hit during testing.** They mark untested code paths. After conversion, check which BOOMs are still unused — those are the paths that still need test coverage.

`BOOM("test")` calls are **tripwires** on untested execution paths. When a conversion first exercises such a path, the Step 1 baseline run hits the tripwire — report and stop per the ⛔⛔⛔ rule. The user removes the tripwire, and the conversion proceeds. Tripwire removals belong in that test's commit (alongside the test file and plan update).

## ⛔ CRITICAL: Git Workflow

**When committing, ONLY stage and commit the files that were actually changed.**
Never run `git add -A` or `git add .`. Explicitly list only modified files:

```bash
git add regression-tests/test-hash-map.c
```

**Wait for explicit instruction before committing.**

## Overview

Tests in `regression-tests/test-hash-map.c` use I60 integers for keys and values. I60 terms are leaf terms — no heap allocation, no ref counting. The conversion replaces them with heap-allocated String keys and values so that `incRef`/`dec_and_free` paths are exercised.

**Each test must pass individually.** Tests must not rely on any previous test having allocated (or not allocated) any pool objects. Each test starts with `reset_counters()` and `countPoolObjects()` to establish a clean baseline.

**Any pools can be filled at the top of `main()`.** If a test needs a pool to already exist (so it pulls from the pool instead of creating a new one), allocate and free a value of the right type at the top of `main()` before any test runs.

**Strings under 100 chars are recycled from the same pool.** It's sufficient to pre-allocate the maximum number of strings any test uses at the same time — you don't need to pre-allocate the exact strings a test might use. Just allocate enough string slots at the top of `main()` so that `stringValue()` calls don't trigger new mallocs. Otherwise the first `stringValue()` call will allocate a new one (incrementing `malloc_count`).

One pattern for creating strings:

| Pattern | Use when |
|---|---|
| `stringValue("literal")` | Always — the only way to create strings |

**Hash control:** If a test requires a specific hash value (e.g., two keys must share the same bit position), you can set the string's `hashVal` field directly after creation:

```c
Term key = (Term)stringValue("mykey");
((String *)key)->hashVal = desired_hash;
```

This bypasses the SHA1 computation and lets you control the hash for testing structural paths.

**Automatic hash matching:** If a test computes a key's hash through the same `sha1` function pointer the runtime calls internally (e.g., `bmiReplaceCopied` re-hashes the existing key when deciding collision vs. branch), then passing that hash for a *different* key makes the collision/branch decision fall out automatically — no `hashVal` pinning needed. `hashVal` pinning is only required when you must force a specific bit position (branch tests) or a specific hash value the test does not compute from the key.

## Conversion Process

**⛔ CRITICAL: After converting ONE key or value, you MUST compile and run `make test-hash-map` and verify it passes before converting the next one.** Do not batch conversions. Do not convert multiple keys/values before running the test. Do not skip the verification step. Every conversion must be verified individually — this is how you catch memory errors early.

If the test fails after a conversion, fix the error before moving to the next key/value. If `check_counts` shows wrong expected numbers, update them and continue. If there's a real leak, fix it first.

**Never convert all I60 keys/values in a single pass.**

### Step 1: Isolate the test

Comment out all tests in `main()` except the one being converted.

**⛔⛔⛔ CRITICAL: Before doing any conversion work, compile and run `make test-hash-map`.** If the test fails at this point (with I60 values, before any changes), **under no circumstances are you to do anything else other than report it immediately**. No investigations. No checking. No debugging. No analysis. Just report the failure and stop. A pre-existing failure is a critical failure that must be handled by the user alone. Full stop.

### Step 2: Pick the first I60 key or value

Choose one `newI60(x)` that is used as a key or value (not a temporary). Do not touch the others yet.

### Step 3: Convert that one key or value to `stringValue()`

**Pattern A: `stringValue("literal")`** — for simple literals:

```c
// BEFORE
Term key = newI60(137);
Term val = newI60(251);

// AFTER
Term key = (Term)stringValue("key137");
Term val = (Term)stringValue("hello");
```

### Step 4: Don't try to calculate `check_counts()` expected values

It is too difficult to get right. Just leave the expected values as-is for now. After running `make test-hash-map`, the `check_counts` failure will tell you the actual numbers — update them to match and move on.

### Step 5: Compile and run

```bash
make test-hash-map
```

If `check_counts` fails with wrong expected numbers — update them to match the actual output and move on.
If `check_counts` fails with `unfreed != pool_delta` — there's a real leak. Fix it (see `skills/memory-leak-hunting.md`).
If a value's refs are decreased too many times (underflow), fix it (see `skills/memory-leak-hunting.md`).

### Step 8: Repeat

Go back to Step 2 and pick the next I60 key or value. Convert it. Verify. Repeat until all keys or values in this test are converted.

### Step 9: Final regression check

Uncomment ALL tests marked as [x] in the plan. Comment out the rest. Build, then loop the test binary 1000 times — tests run in randomized order each run, so repeated runs catch order-dependent pool issues:

```bash
make test-hash-map
fails=0
for i in $(seq 1 1000); do
  ./regression-tests/test-hash-map >/dev/null 2>&1 || fails=$((fails+1))
  [ "$fails" -gt 0 ] && echo "RUN $i FAILED"
done
echo "$fails failures out of 1000"
```

Verify nothing regressed: 0 failures out of 1000.

### Step 10: Update the plan

Mark the test as [x] in `docs/hash-map-plan.md`.

### Step 11: Commit

**One commit per test.** The first key or value conversion creates the commit. Every subsequent key or value conversion amends it.

```bash
git add regression-tests/test-hash-map.c docs/hash-map-plan.md
git commit -m "test: convert test<Name> to use String keys/values"
git add regression-tests/test-hash-map.c
git commit --amend --no-edit
```

Wait for explicit instruction before committing.

## Pool Accounting Quick Reference

| Allocation | `malloc_count` | `free_count` |
|---|---|---|
| `stringValue("...")` | +1 | 0 |
| `dec_and_free((Term)string, 1)` | 0 | +1 |
| `newI60(x)` | 0 | 0 |
| `malloc_bmiNode(N<20)` (first call) | +10 | 0 |
| `malloc_bmiNode(N<20)` (subsequent) | 0 | 0 |
| `freeBitmapNode(N<20)` | 0 | 0 (recycles) |
| `freeBitmapNode(N>=20)` | 0 | +1 |
| `malloc_arrayNode` (first) | +10 | 0 |
| `malloc_arrayNode` (subsequent) | 0 | 0 |
| `freeArrayNode` | 0 | 0 (recycles) |
| `malloc_hashCollisionNode(N)` | +1 | 0 |
| `freeHashCollisionNode` | 0 | +1 |

## Checklist

### During conversion
- [ ] All other tests commented out in `main()`
- [ ] One key or value converted at a time
- [ ] `make test-hash-map` passes after each conversion
- [ ] All keys or values in the test converted
- [ ] Final regression check: all [x] tests uncommented, 1000 runs pass

### After conversion
- [ ] Test marked [x] in `docs/hash-map-plan.md`
- [ ] One commit created by first key/value conversion
- [ ] Subsequent key/value conversions amend that commit
- [ ] Wait for explicit instruction before committing

## String Ref Counting Contract

**⛔ CRITICAL: When you pass a String to a function that stores it in a node, you are giving up ownership of that String.** The function does NOT increment the String's ref count. When the function frees the node, it decrements refs on all entries. If refs reaches 0, the String is freed.

This means:
- **Never use a String in a function call after it's been stored in a node that gets freed.** The String may already be at `refsError` (-10), and any `incRef`/`dec_and_free` will fail.
- **If you need to look up a key after it's been stored, create a fresh `stringValue()` for the lookup.** The new String has its own ref count and won't be affected by the node's lifecycle.
- **`strSha1(incRefVal(key, 1))` leaves the ref unchanged** — it increments then decrements (strSha1 decrements when the hash is already cached). So after this call, the String has its original refs.
- **`equal(a, b)` on two Strings consumes exactly one ref of *each* argument, in all cases.** On a match, `strCmp` decrements only the second argument and returns `some(first)`; `equal` then frees that Some wrapper, and `decValRef` (runtime3.c:741) decrements all impls of a ReifiedVal — including the first argument. On a mismatch, `strCmp` decrements both directly (the `nothing()` wrapper has no impls). Either way: one of each. Never use a String in an `equal()` comparison after it's been stored in a freed node.
- **Never clean up a String that was stored in a node the test frees.** The node's free handles it. Cleaning it up again causes a double-decrement.

### Example: Correct pattern for lookup after storage

```c
// WRONG: key1's refs is exhausted after bmiMutateAssoc stores it
// and the old node is freed. Using key1 again will fail.
Term key1 = (Term)stringValue("key137");
Value *result = bmiMutateAssoc(node, key1, val, hash, 0);
Value *clone = bmiCopyAssoc(bm, key1, newVal, hash, 0);  // FAILS!

// CORRECT: create a fresh String for the lookup
Term key1 = (Term)stringValue("key137");
Value *result = bmiMutateAssoc(node, key1, val, hash, 0);
Term lookupKey = (Term)stringValue("key137");
int64_t lookupHash = strSha1(incRefVal(lookupKey, 1));
Value *clone = bmiCopyAssoc(bm, lookupKey, newVal, lookupHash, 0);  // OK!
```

### Example: Correct pattern for verification

```c
// WRONG: newVal was stored in the clone, freed when cloneResult is freed.
// Comparing with newVal in an equal() call will fail.
if (equal((Value *)cloneSub->array[1], (Value *)newVal)) { ... }

// CORRECT: compare values within the node, not with external Strings
if (!equal((Value *)cloneSub->array[1], (Value *)cloneSub->array[3])) { ... }
```

## Common Pitfalls

1. **Forgetting to free String keys/values at cleanup** — a string only needs explicit `dec_and_free` if it's returned from a function and not used again as an argument in another call. If it's passed as an argument to another function, that function's cleanup handles it.
2. **Double-freeing** — if `bmiGet` or `bmiDissoc` frees the node internally, don't also `dec_and_free` the node. Only free what the test owns.
3. **Using `stringValue()` for the same string twice** — `stringValue("key")` may return different heap allocations each call. If you need the same pointer, store it in a variable.
4. **Using `newI60` for temporary values that are NOT stored** — temps like `newI60(999)` passed to a function that frees them internally don't need test-level cleanup. Only count what the test allocates.
5. **Using a String after its node was freed** — once a String is stored in a node and that node is freed, the String's refs is decremented (possibly to `refsError`). Any further use (including `equal()` comparisons) will fail. Create a fresh `stringValue()` for any subsequent use.

6. **In-place mutate paths may replace the stored pointer** — `bmiMutateAssoc`'s 1c path (same key, different value) stores the *passed* key pointer and frees the old key. After the call, the node holds the fresh key; the original key variable points at freed memory. Pointer-identity checks must compare against the fresh key (e.g., `array[2*idx] != updateKey`), not the original.
