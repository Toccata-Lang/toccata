# Hash-Map Implementation Plan

## Overview

The hash-map is an immutable key-value store based on Clojure's bitmap trie data structure. The C-level data structure and operations exist in both `runtime3.c` (all stubs that abort). The main task is to activate the implementations in `runtime3.c` by uncommenting the commented-out code, then wire the Toccata protocol layer.

## Important Notes

**runtime3.c uses `Term` (unsigned long) for most operations, not `Value *`.**
- `incRef(Term)` takes Term, not Value *
- `dec_and_free(Term)` takes Term, not Value *
- `integerSha1(Term)` takes Term, not Value *
- `BitmapIndexedNode->array` is `Value **`
- `ArrayNode->array` is `Term *`

**Types that DON'T exist in runtime3.c:**
- `Integer` struct — integers are I60 terms
- `ListType` — not defined
- `HashedValue` struct — no hash caching
- `new_num`/`new_i24` — use `newI60(x)` instead

**Protocol functions NOT wired up yet:**
- `count` — returns count of sequence
- `sha1` — computes SHA1 hash
- `get` — polymorphic get dispatch
- `baseDissoc` — polymorphic dissoc dispatch
- `hashSeq` — flatten to sequence
- `vals` — get all values

## Progress

### Done
- [x] `clone_BitmapIndexedNode` — type casts for incRef
- [x] `createNode` — no incRef calls (caller handles ref counting)
- [x] `bmiHashVec` — type casts for incRef/hashVec/mutateVectConj/dec_and_free
- [x] `nakedSha1` — reworked to take Term, use termTag() dispatch

## Architecture Reference

### Node Types (defined in `runtime3.h` / `core.h`)

| Type | ID | Purpose |
|---|---|---|
| `BitmapIndexedNode` | 11 | Sparse bitmap trie — efficient for small-to-medium maps |
| `ArrayNode` | 12 | Fallback when BMI gets too wide (>16 entries) |
| `HashCollisionNode` | 13 | Stores entries that hash-collide |
| `HashMapType` | 14 | Abstract type number for `instance?` checks |

## Runtime3.c vs Core.c — Side-by-Side

### Memory Management

| Function | runtime3.c | core.c | Notes |
|---|---|---|---|
| `malloc_bmiNode()` | ✅ | ✅ | Identical |
| `freeBitmapNode()` | ⚠️ Stub | ✅ | runtime3.c: aborts; core.c: uncommented, works |
| `malloc_hashCollisionNode()` | ✅ | ✅ | Identical |
| `freeHashCollisionNode()` | ⚠️ Stub | ✅ | runtime3.c: aborts; core.c: uncommented, works |
| `malloc_arrayNode()` | ✅ | ✅ | Identical |
| `freeArrayNode()` | ✅ | ✅ | Identical |
| `emptyBMI` global | ✅ | ✅ | Identical |

### Helper Functions

| Function | runtime3.c | core.c | Notes |
|---|---|---|---|
| `mask()` | ✅ | ✅ | Hash bit extraction |
| `bitpos()` | ✅ | ✅ | Bit position from hash |
| `nakedSha1()` | ⚠️ Stub | ⚠️ Stub | Both abort; full impl commented out in runtime3.c |
| `notFoundPtr` | ✅ | ✅ | Sentinel value |

### Operations — runtime3.c (ALL STUBS)

| Function | runtime3.c | core.c |
|---|---|---|
| `clone_BitmapIndexedNode()` | Stub | ✅ |
| `createNode()` | Stub | ✅ |
| `bmiHashVec()` | Stub | ✅ |
| `bmiCount()` | Stub | ✅ |
| `bmiCopyAssoc()` | Stub | ✅ |
| `bmiMutateAssoc()` | Stub | ✅ |
| `bmiGet()` | Stub | ✅ |
| `bmiDissoc()` | Stub | ✅ |
| `arrayNodeCopyAssoc()` | Stub | ✅ |
| `arrayNodeMutateAssoc()` | Stub | ✅ |
| `arrayNodeGet()` | Stub | ✅ |
| `arrayNodeCount()` | Stub | ✅ |
| `collisionAssoc()` | Stub | ✅ |
| `arrayNodeDissoc()` | Stub | ✅ |
| `collisionCount()` | Stub | ✅ |
| `collisionVec()` | Stub | ✅ |
| `collisionDissoc()` | Stub | ✅ |
| `collisionGet()` | Stub | ✅ |
| `arrayNodeVec()` | Stub | ✅ |
| `get()` | Stub | ✅ |
| `baseDissoc()` | Stub | ✅ |
| `hashVec()` | Stub | ✅ |
| `copyAssoc()` | Stub | ✅ |
| `mutateAssoc()` | Stub | ✅ |
| `hashMapGet()` | Stub | ✅ |
| `hashMapAssoc()` | Stub | ✅ |

### Operations — core.c Only (not in runtime3.c at all)

| Function | Notes |
|---|---|
| `bmiHashSeq()` | Flatten BMI to `List` of pairs (and recursively sequence sub-nodes) |
| `collisionSeq()` | Flatten collision node to `List` of pairs |
| `arrayNodeSeq()` | Flatten array node to `List` of pairs |
| `equal()` | Value equality — dispatches to `integer_EQ` or `equalSTAR` |

## What Needs to Be Done

### Phase 1: Activate runtime3.c Implementations

All 27 operation stubs in runtime3.c have their real code commented out immediately after the `abort()`. Each stub follows the pattern:

```c
Value *bmiGet(...) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
  <real implementation here>
  // */
}
```

**Action:** For each function, uncomment the implementation and adapt it for runtime3.c's type system:
- `incRef(Value*, n)` → `incRef((Term)(Value *)Value*, n)`
- `dec_and_free(Value*, n)` → `dec_and_free(termVal((Term)(Value *)Value*), n)`
- `integerSha1(Value*)` → `integerSha1(termVal((Term)(Value *)Value*))`
- `new_num(new_i24(x))` → `(Value *)newI60(x)`
- Remove `HashedValue` caching logic (type doesn't exist)
- Remove `Integer` struct usage (integers are I60 terms)
- Remove `ListType` case (doesn't exist)

**Functions to uncomment (27 total):**
1. `clone_BitmapIndexedNode` ✅
2. `createNode` ✅
3. `bmiHashVec` ✅
4. `nakedSha1` ✅
5. `bmiCount` — pending (uses count protocol, Integer type)
6. `bmiCopyAssoc`
7. `bmiMutateAssoc`
8. `bmiGet`
9. `bmiDissoc`
10. `arrayNodeCopyAssoc`
11. `arrayNodeMutateAssoc`
12. `arrayNodeGet`
13. `arrayNodeCount`
14. `collisionAssoc`
15. `collisionCount`
16. `collisionVec`
17. `collisionDissoc`
18. `collisionGet`
19. `arrayNodeVec`
20. `arrayNodeDissoc`
21. `get` (polymorphic dispatch) — needs wiring
22. `baseDissoc` (polymorphic dispatch) — needs wiring
23. `hashVec` (polymorphic dispatch)
24. `copyAssoc` (polymorphic dispatch)
25. `mutateAssoc` (polymorphic dispatch)
26. `hashMapGet`
27. `hashMapAssoc`

### Phase 2: Port Unique Functions from core.c

Two functions exist in core.c but are **missing entirely** from runtime3.c:

| Function | What it does | Depends on |
|---|---|---|
| `bmiHashSeq` | Flattens BMI node (and sub-nodes) into `List` of key/value pairs | `hashSeq` (recursive), `listCons` |
| `equal` | Compares two values for equality — `integer_EQ` for Integers, `equalSTAR` for others | `integer_EQ`, `equalSTAR`, `maybe`, `nothing` |

These need to be added to runtime3.c.

### Phase 3: Fix `mutateAssoc` Missing Collision Case

In core.c, `mutateAssoc` has the `HashCollisionNodeType` case commented out:
```c
case HashCollisionNodeType:
    return(collisionAssoc(node, k, v, hash, shift));
// */
```
This means mutable assoc on collision nodes falls back to `copyAssoc`. Uncomment this case.

### Phase 4: Protocol Bindings

The C functions exist but are **never wired to Toccata protocol symbols**.

**Current state:** Extern declarations in `core.h` / `runtime3.h`:
```c
extern Value *(*dissoc)(FnArity *, Value *, Value *, Value *, Value *);
extern Value *(*hashSeq)(FnArity *, Value*, Value *s);
extern Value *(*count)(FnArity *, Value *);
extern Value *(*vals)(FnArity *, Value *);
```

**What's needed:** Assign these function pointers during initialization so that:
- `(assoc m k v)` dispatches to `hashMapAssoc` / `mutateAssoc` based on `m`'s type
- `(dissoc m k)` dispatches to `baseDissoc` based on `m`'s type
- `(hash-seq m s)` dispatches to `bmiHashSeq` / `arrayNodeSeq` / `collisionSeq` based on `m`'s type
- `(count m)` dispatches to `bmiCount` / `arrayNodeCount` / `collisionCount` based on `m`'s type
- `(vals m)` dispatches to `hashVec` based on `m`'s type

**Files to modify:** `core.c` initialization code, protocol dispatcher in `new.c` or `base.toc`.

### Phase 5: `get*` / `dissoc*` Variants

The test uses `get*` and `dissoc*` which take explicit hash and shift parameters:
```lisp
(get* m k default hash shift)
(dissoc* m k hash shift)
```

**What's needed:** New Toccata functions that bypass automatic hash computation and accept pre-computed hash + shift.

**Implementation:** C wrapper functions that call `get()` / `baseDissoc()` directly with the provided hash and shift, then define them as Toccata functions.

### Phase 6: `assoc-all`

The test uses `(assoc-all {} k1 v1 k2 v2 ...)` for batch insertion.

**What's needed:** A variadic function that repeatedly calls `assoc` for each key-value pair.

**Implementation:** C function that takes an alternating sequence of keys and values, iterates through them calling `hashMapAssoc`, then define as Toccata function.

### Phase 7: Empty Hash-Map (`{}`) Constructor

There's no `HashMapCtor` or equivalent for creating empty HashMaps.

**What's needed:**
- A Toccata function or literal that creates an empty HashMap
- Should return the `emptyBMI` singleton (it's already defined as a global)
- The compiler needs to know how to encode `{}` literals

**Files to modify:** Compiler literal handling in `new.c`, or add a `HashMapCtor` function.

### Phase 8: `empty?` Protocol for HashMap

The test calls `(empty? {})` and `(empty? one-hash-map)`.

**What's needed:** The `empty?` protocol implementation for HashMap types.

**Implementation:** Check if the node is `emptyBMI` or has `bitmap == 0` (for BMI) / all-zero array (for ArrayNode).

### Phase 9: `=` Protocol for HashMap

The test calls `(= {} {})` and `(= bmi (dissoc bmi ...))`.

**What's needed:** The `=` protocol implementation for HashMap types.

**Implementation:** Compare structure — same node type, same bitmap (for BMI), same key/value pairs recursively. For collision nodes, compare all pairs.

### Phase 10: `hash-seq` Return Type

The test expects `(hash-seq m [""])` to return something with `count` that works with Vector operations.

**Current state:** `bmiHashSeq` returns a `List`, but the test uses `count` and `Vector`-style operations on the result.

**What's needed:** Either:
- Make `hash-seq` return a Vector, or
- Ensure the `count` protocol works on the returned type

## Test Strategy: Build `test-hash-map.c` Step-by-Step

Similar to `test-hvm.c` which tests the HVM interaction rules, we build `test-hash-map.c` to test hash-map operations. Each phase adds more tests:

### Phase 1: Memory Management (DONE)
- `testEmptyBmiNode` — create/destroy empty BMI node
- `testBmiNodeOneItem` — create/destroy BMI node with 1 item
- `testArrayNode` — create/destroy ArrayNode
- `testCollisionNode` — create/destroy HashCollisionNode
- `testFreeBitmapNode` — verify freeBitmapNode works
- `testFreeHashCollisionNode` — verify freeHashCollisionNode works

### Phase 2: BMI Operations
- `testBmiCopyAssoc` — add key/value to empty BMI → single-item BMI
- `testBmiCopyAssocUpdate` — update existing key in BMI
- `testBmiCopyAssocCollision` — add key with same hash → creates collision node
- `testBmiCopyAssocBranch` — add key with different hash → creates branch node
- `testBmiCopyAssocWide` — add >16 entries → promotes to ArrayNode
- `testBmiGet` — lookup existing key
- `testBmiGetMiss` — lookup missing key
- `testBmiDissoc` — remove key from single-item BMI
- `testBmiDissocEmpty` — remove last key → returns emptyBMI
- `testBmiCount` — count entries in BMI

### Phase 3: ArrayNode Operations
- `testArrayNodeCopyAssoc` — add to empty ArrayNode
- `testArrayNodeGet` — lookup in ArrayNode
- `testArrayNodeDissoc` — remove from ArrayNode
- `testArrayNodeCount` — count entries

### Phase 4: CollisionNode Operations
- `testCollisionAssoc` — add to collision node
- `testCollisionGet` — lookup in collision node
- `testCollisionDissoc` — remove from collision node
- `testCollisionCount` — count entries

### Phase 5: Public API
- `testHashMapGet` — public get API
- `testHashMapAssoc` — public assoc API

### Phase 6: Hash Functions
- `testNakedSha1` — SHA1 hashing of various types
- `testNakedSha1Consistent` — same input → same hash
- `testNakedSha1Different` — different input → different hash

### Phase 7: Integration
- `testLargeMap` — build 2000-entry map, verify count
- `testMapRoundtrip` — assoc then get retrieves same value
- `testMapDissocRoundtrip` — assoc then dissoc removes entry
- `testMapEquality` — two maps with same entries are equal

## Implementation Order

> **Strategy:** Uncomment stubs incrementally as needed for each test, rather than all at once. This keeps the diff small and makes debugging easier.

1. **Phase 1: Add test-hash-map.c tests** — memory management, basic node creation
2. **Uncomment `freeBitmapNode` / `freeHashCollisionNode`** — fix memory leaks in existing tests
3. **Uncomment stubs needed for BMI operations** — `bmiCopyAssoc`, `bmiGet`, `bmiDissoc`, `bmiCount`, `clone_BitmapIndexedNode`, `createNode`
4. **Uncomment stubs needed for ArrayNode operations** — `arrayNodeCopyAssoc`, `arrayNodeGet`, `arrayNodeDissoc`, `arrayNodeCount`
5. **Uncomment stubs needed for CollisionNode operations** — `collisionAssoc`, `collisionGet`, `collisionDissoc`, `collisionCount`
6. **Uncomment `get` / `baseDissoc` / `hashVec` / `copyAssoc` / `mutateAssoc`** — polymorphic dispatch functions
7. **Uncomment `hashMapGet` / `hashMapAssoc`** — public API
8. **Port `bmiHashSeq`** from core.c — flatten hash-map to sequence
9. **Port `equal`** from core.c — value equality for key comparison
10. **Uncomment `collisionAssoc` case** in `mutateAssoc` — fix core.c
11. **Wire protocol bindings** — assign function pointers so Toccata functions dispatch to C implementations
12. **Add `{}` constructor** — enable empty HashMap creation
13. **Add `empty?` protocol** for HashMap
14. **Add `=` protocol** for HashMap
15. **Add `get*` / `dissoc*` variants** — explicit hash/shift parameters
16. **Add `assoc-all`** — batch insertion
17. **Fix `hash-seq` return type** if needed

## Test File

- **`regression-tests/hash-map-regressions.toc`** — 40+ assertions covering:
  - Empty map, singleton, multi-entry maps
  - `assoc`, `assoc-all`, `get`, `get*`, `dissoc`, `dissoc*`
  - `count`, `hash-seq`, `empty?`, `=`
  - Collision handling
  - Large maps (2000+ entries)
  - Array node promotion (>16 entries)
  - `instance?` checks
  - `foo` / `bar` helper functions testing type assertions

## Build Target

Add to Makefile:
```makefile
.PHONY: test-hash-map
test-hash-map:
	$(CC) $(CFLAGS) -o regression-tests/test-hash-map $(TOC_FLAGS) $(LDFLAGS) -DTESTING_HVM=1 new.c regression-tests/test-hash-map.c graph.c runtime3.c
	./regression-tests/test-hash-map
```

## Files to Read

| File | Purpose |
|---|---|
| `core.c` | Reference implementations (lines ~3673–4440) |
| `runtime3.c` | Stubs to uncomment (lines ~1978–2859) |
| `runtime3.h` | Type definitions, extern declarations |
| `core.h` | Type definitions, protocol externs |
| `regression-tests/test-hvm.c` | Reference test pattern |
| `regression-tests/hash-map-regressions.toc` | Test cases |
| `hvm-core.toc` | Protocol definitions (`assoc`, `dissoc`, `hash-seq`, `count`, etc.) |
| `new.c` | Compiler — may need modifications for `{}` literal |
