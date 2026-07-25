# Convert Test to Use Allocated Strings

## ⛔ CRITICAL: Commit after every change

**Commit after every meaningful change.** Do not batch changes. Each commit should be a single, focused change that builds and passes tests. This keeps history clean and makes rollback easy.

## ⛔ CRITICAL: Never use 42 as test data

**Never use 42 as a test integer value** — it's cliche and irritating. Pick something else.

## ⛔ CRITICAL: Don't remove BOOM calls in runtime3.c

**Never remove `BOOM()` calls in `runtime3.c` unless they actually get hit during testing.** They mark untested code paths. After conversion, check which BOOMs are still unused — those are the paths that still need test coverage.

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

## Conversion Process

**⛔ CRITICAL: After converting ONE key or value, you MUST compile and run `make test-hash-map` and verify it passes before converting the next one.** Do not batch conversions. Do not convert multiple keys/values before running the test. Do not skip the verification step. Every conversion must be verified individually — this is how you catch memory errors early.

If the test fails after a conversion, fix the error before moving to the next key/value. If `check_counts` shows wrong expected numbers, update them and continue. If there's a real leak, fix it first.

**Never convert all I60 keys/values in a single pass.**

### Step 1: Isolate the test

Comment out all tests in `main()` except the one being converted.

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

Uncomment ALL tests marked as [x] in the plan. Comment out the rest. Re-run:

```bash
make test-hash-map
```

Verify nothing regressed.

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
- [ ] Final regression check: all [x] tests uncommented, passes

### After conversion
- [ ] Test marked [x] in `docs/hash-map-plan.md`
- [ ] One commit created by first key/value conversion
- [ ] Subsequent key/value conversions amend that commit
- [ ] Wait for explicit instruction before committing

## Common Pitfalls

1. **Forgetting to free String keys/values at cleanup** — a string only needs explicit `dec_and_free` if it's returned from a function and not used again as an argument in another call. If it's passed as an argument to another function, that function's cleanup handles it.
2. **Double-freeing** — if `bmiGet` or `bmiDissoc` frees the node internally, don't also `dec_and_free` the node. Only free what the test owns.
3. **Using `stringValue()` for the same string twice** — `stringValue("key")` may return different heap allocations each call. If you need the same pointer, store it in a variable.
4. **Using `newI60` for temporary values that are NOT stored** — temps like `newI60(999)` passed to a function that frees them internally don't need test-level cleanup. Only count what the test allocates.
