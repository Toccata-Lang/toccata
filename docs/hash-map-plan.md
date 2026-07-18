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
- `vals` — get all values

## Execution Order — DO NOT SKIP AHEAD

**All BMI tests must be written and passing before uncommenting any ArrayNode or CollisionNode stubs.**

The BMI stubs are already active. The remaining work is to write the missing BMI tests:
- `testBmiCopyAssocBranch` (A2d — branch node)
- `testBmiCopyAssocSubNodeNoChange` (A1a — nested no-change)
- `testBmiCopyAssocSubNodeChange` (A1b — nested clone)
- `testBmiMutateAssoc` (refs==1 in-place mutation)

Only after all four are passing should you uncomment the ArrayNode stubs. Never jump ahead to uncommenting stubs for node types you haven't fully tested yet.

## Progress

### Done
- [x] `clone_BitmapIndexedNode` — type casts for incRef
- [x] `createNode` — no incRef calls (caller handles ref counting)
- [x] `bmiHashVec` — type casts for incRef/hashVec/mutateVectConj/dec_and_free
- [x] `nakedSha1` — reworked to take Term, use termTag() dispatch
- [x] BMI operations active — `bmiMutateAssoc`, `bmiGet`, `bmiDissoc`
- [x] `freeBitmapNode` / `freeHashCollisionNode` — fixed to not call `termVal()` on I60/F60/REF terms
- [x] `equal()` — handles I60 terms via `integer_EQ`, uses `equalSTAR` for other types
- [x] `isNothing()` — helper to check if a Term is the nothing singleton
- [x] `testBmiCopyAssoc` — first BMI operation test

## Architecture Reference

### Node Types (defined in `runtime3.h` / `core.h`)

| Type | ID | Purpose |
|---|---|---|
| `BitmapIndexedNode` | 11 | Sparse bitmap trie — efficient for small-to-medium maps |
| `ArrayNode` | 12 | Fallback when BMI gets too wide (>16 entries) |
| `HashCollisionNode` | 13 | Stores entries that hash-collide |
| `HashMapType` | 14 | Abstract type number for `instance?` checks |

### `bmiCopyAssoc` Code Paths (9 total)

```
bmiCopyAssoc(node, key, val, hash, shift)
│
├─ bit = bitpos(hash, shift)
├─ idx = popcount(bitmap & (bit-1))
│
├─ IF bit IS set (entry exists at idx)
│  │
│  ├─ A1: keyOrNull == NULL (sub-node, not a KV pair)
│  │  ├─ A1a: copyAssoc returns SAME → no-op (return original)
│  │  └─ A1b: copyAssoc returns DIFF → clone with new sub-node
│  │
│  ├─ A2: keyOrNull != NULL (KV pair exists)
│  │  ├─ A2a: key==key AND val==val → no-op (return original)
│  │  ├─ A2b: key==key AND val≠val → clone, update value
│  │  ├─ A2c: key≠key AND hash==hash → collision node
│  │  └─ A2d: key≠key AND hash≠hash → branch node (createNode)
│  │
│  └─ IF bit NOT set (no entry here)
│     ├─ B1: n >= 16 → promote to ArrayNode
│     └─ B2: n < 16 → insert into BMI node
```

| Path | What happens | Test idea |
|------|-------------|-----------|
| A1a | Sub-node unchanged → no-op | Update nested map where value doesn't change |
| A1b | Sub-node changes → clone | Update nested map, value changes |
| A2a | Same key, same value → no-op | `assoc` identical key/value |
| A2b | Same key, different value → clone | Update existing key |
| A2c | Collision → new HashCollisionNode | Two keys with same SHA1 hash |
| A2d | Branch → new sub-node | Two keys at different bit positions |
| B1 | **Promote to ArrayNode** | Add 17th entry to BMI (≥16 entries) |
| B2 | Insert into BMI node | Add first key to empty, or key at free bit |

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
16. `collisionVec` ✅
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
extern Value *(*count)(FnArity *, Value *);
extern Value *(*vals)(FnArity *, Value *);
```

**What's needed:** Assign these function pointers during initialization so that:
- `(assoc m k v)` dispatches to `hashMapAssoc` / `mutateAssoc` based on `m`'s type
- `(dissoc m k)` dispatches to `baseDissoc` based on `m`'s type
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

## Test Strategy: Build `test-hash-map.c` Step-by-Step

Similar to `test-hvm.c` which tests the HVM interaction rules, we build `test-hash-map.c` to test hash-map operations. Each phase adds more tests:

### Phase 1: Memory Management (DONE)
- `testEmptyBmiNode` — create/destroy empty BMI node
- `testBmiNodeOneItem` — create/destroy BMI node with 1 item
- `testArrayNode` — create/destroy ArrayNode
- `testCollisionNode` — create/destroy HashCollisionNode
- `testFreeBitmapNode` — verify freeBitmapNode works
- `testFreeHashCollisionNode` — verify freeHashCollisionNode works

### Phase 2: BMI Operations (BMI code is active — tests not yet written)

> **All BMI tests must be completed before moving on to Phase 3 (ArrayNode) or Phase 4 (CollisionNode).**
> The only exceptions are tests that inherently require ArrayNode or HashCollisionNode functionality — e.g. `testBmiCopyAssocPromote` (promotes to ArrayNode) and `testBmiCopyAssocCollision` (creates collision node). Those can be done in Phase 2 since they test BMI behavior that *produces* other node types.

Tests ordered by complexity. Start with trivial (2-3 assertions), work up to hard.

#### Trivial — 2-3 assertions, single key/value
- [x] `testBmiCopyAssoc` — add key/value to empty BMI → single-item BMI (B2)
- [x] `testBmiCopyAssocNoOp` — same key, same value → no-op (A2a)
- [x] `testBmiCopyAssocUpdate` — same key, different value → clone (A2b)
- [x] `testBmiGet` — lookup existing key
- [x] `testBmiGetMiss` — lookup missing key
- [x] `testBmiDissoc` — remove key from single-item BMI
- [x] `testBmiDissocEmpty` — remove last key → returns emptyBMI
- [x] `testBmiCount` — N-entry map, verify count == N

#### Medium — need nested structures or specific conditions
- [x] `testBmiCopyAssocBranch` — two keys at different bit positions → branch node (A2d)
- [x] `testBmiCopyAssocSubNodeNoChange` — nested tree, inner update no-change → no-op (A1a)
- [x] `testBmiCopyAssocSubNodeChange` — nested tree, inner update changes → clone (A1b)
- [x] `testBmiCopyAssocPromote` — 16+ entry map, add 17th → promote to ArrayNode (B1)

#### Hard — engineering constraints
- [x] `testBmiCopyAssocCollision` — two keys with identical SHA1 hash → collision node (A2c)

#### bmiMutateAssoc tests (7 paths) — **must complete before any other work**

These 7 tests cover every code path in `bmiMutateAssoc` (path 0 — refs!=1 delegation to copyAssoc — is already covered by copyAssoc tests).

> **Priority: These must be completed before moving on to any other work.**

Ordered by complexity, simplest first.

- [x] `testBmiMutateAssocNoOp` — bit set, same key + same value (1b)
  - Create BMI with one key/value pair
  - Set refs==1
  - Mutate with the same key and same value
  - Verify the same node pointer is returned
  - Verify no new allocations (glblAlloced unchanged)
- [x] `testBmiMutateAssocUpdateValue` — bit set, same key + different value (1c)
  - Create BMI with one key/value pair
  - Set refs==1
  - Mutate with same key, different value
  - Verify the same node pointer is returned (in-place)
  - Verify the value changed
  - Verify old value was freed (no leak)
- [x] `testBmiMutateAssocSubNodeRecurse` — bit set, sub-node case (1a)
  - Create BMI with a sub-node (two keys at different bit positions)
  - Set refs==1 on the node
  - Mutate the inner key/value
  - Verify the same node pointer is returned (in-place mutation)
  - Verify the inner value changed
- [x] `testBmiMutateAssocBranch` — bit set, different key + different hash (1e)
  - Create BMI with one key/value
  - Set refs==1
  - Mutate with a key at a different bit position
  - Verify the same node pointer is returned
  - Verify a new sub-node (via createNode) was created
  - Verify both entries are accessible
- [x] `testBmiMutateAssocCollision` — bit set, different key + same hash (1d)
  - Find two keys with identical SHA1 hash
  - Create BMI with one of them
  - Set refs==1
  - Mutate with the second key
  - Verify the same node pointer is returned
  - Verify a HashCollisionNode was created at the collision slot
  - Verify both entries are present
- [x] `testBmiMutateAssocInsert` — bit not set, n < 16, insert into new BMI (2b)
  - Create BMI with one key/value pair
  - Set refs==1
  - Mutate with a new key at a free bit position
  - Verify a new BMI node is returned (n+1 items)
  - Verify both entries are present
  - Verify old node was freed (no leak)
- [x] `testBmiMutateAssocPromote` — bit not set, n >= 16, promote to ArrayNode (2a)
  - Create BMI with 16 entries (full)
  - Set refs==1
  - Mutate with a new key (bit not set)
  - Verify an ArrayNode is returned (not the original node)
  - Verify all 16 original entries are present
  - Verify the new entry is present

### Phase 3: ArrayNode Operations

> **Prerequisite:** All BMI tests must pass first (they create the sub-nodes used by ArrayNode tests).

`arrayNodeCopyAssoc` has two top-level paths: `subNode == 0` (slot empty) vs `subNode != 0` (slot has a sub-node). Each has sub-paths.

#### `testArrayNodeCopyAssoc` — 5 paths

Decision tree: `subNode == 0` (slot empty) vs `subNode != 0` (slot has a sub-node)

**Path A: Slot is empty (`subNode == 0`)**
- **A1:** Empty ArrayNode + empty target slot → create new ArrayNode, copy nothing, store new entry
- **A2:** Non-empty ArrayNode + empty target slot → create new ArrayNode, copy existing entries via loop, store new entry at idx

**Path B: Slot has a sub-node (`subNode != 0`)**
`copyAssoc` is called on the sub-node, returning either the same pointer or a new one.
- **B1:** `n == subNode` (no-op, same key+value) → free `n`, return original node. No new allocations.
- **B2:** `n != subNode` (changed) → create new ArrayNode, copy all entries except idx, store `n` at idx, free original node.

Tests:
- [x] `testArrayNodeCopyAssocA1` — Empty ArrayNode, add first entry
  - Create empty ArrayNode
  - Add key/value → should create new ArrayNode with 1 entry
  - Verify type is ArrayNodeType
  - Verify the entry is in the correct slot (contains a BMI sub-node with the key)
  - Verify original node was freed (new pointer returned)
  - Create empty ArrayNode
  - Add key/value → should create new ArrayNode with 1 entry
  - Verify type is ArrayNodeType
  - Verify the entry is in the correct slot (contains a BMI sub-node with the key)
  - Verify original node was freed (new pointer returned)
- [x] `testArrayNodeCopyAssocA2` — Non-empty ArrayNode, add to empty slot
  - Create ArrayNode with one entry at slot X
  - Add key/value at slot Y (Y ≠ X)
  - Verify result has 2 entries at slots X and Y
  - Verify the entry at slot X was copied (same BMI sub-node pointer)
  - Verify the entry at slot Y is a new BMI sub-node
  - Create ArrayNode with one entry at slot X
  - Add key/value at slot Y (Y ≠ X)
  - Verify result has 2 entries at slots X and Y
  - Verify the entry at slot X was copied (same BMI sub-node pointer)
  - Verify the entry at slot Y is a new BMI sub-node
- [x] `testArrayNodeCopyAssocB1` — ArrayNode with sub-node, same key+value (no-op)
  - Create ArrayNode with a BMI sub-node containing key K
  - Call copyAssoc with the same key K and same value
  - Verify the original ArrayNode pointer is returned (no new allocation)
  - Verify the sub-node was freed (dec_and_free called on it)
  - Verify the slot still contains the key/value
  - Create ArrayNode with a BMI sub-node containing key K
  - Call copyAssoc with the same key K and same value
  - Verify the original ArrayNode pointer is returned (no new allocation)
  - Verify the sub-node was freed (dec_and_free called on it)
- [x] `testArrayNodeCopyAssocB2` — ArrayNode with sub-node, different value
  - Create ArrayNode with a BMI sub-node containing key K with value V1
  - Call copyAssoc with key K and different value V2
  - Verify a new ArrayNode is returned
  - Verify the old ArrayNode was freed
  - Verify the sub-node at the target slot contains the updated value V2
  - Verify other slots were copied correctly
  - Create ArrayNode with a BMI sub-node containing key K with value V1
  - Call copyAssoc with key K and different value V2
  - Verify a new ArrayNode is returned
  - Verify the old ArrayNode was freed
  - Verify the sub-node at the target slot contains the updated value V2
  - Verify other slots were copied correctly
- [x] `testArrayNodeCopyAssocB2-multi` — Multiple entries, update one sub-node
  - Create ArrayNode with 2 entries at different slots
  - Update the sub-node at slot X with a new value
  - Verify the entry at slot X was replaced with a new sub-node
  - Verify the entry at slot Y was copied (same pointer)
  - Verify the original ArrayNode was freed
  - Create ArrayNode with 2 entries at different slots
  - Update the sub-node at slot X with a new value
  - Verify the entry at slot X was replaced with a new sub-node
  - Verify the entry at slot Y was copied (same pointer)
  - Verify the original ArrayNode was freed

#### `testArrayNodeGet` — lookup in ArrayNode
- [x] `testArrayNodeGet` — Key found in BMI sub-node (Path B1)
  - Create ArrayNode with a BMI sub-node
  - Lookup the key stored in the sub-node
  - Verify the correct value is returned
  - Verify the ArrayNode was freed, BMI sub-node was incremented
  - Verify the returned value is the BMI's value (not a copy)
- [x] `testArrayNodeGetMiss` — Empty slot (Path A)
  - Create ArrayNode with entries at some slots
  - Lookup a key whose hash maps to an empty slot
  - Verify the default value is returned
  - Verify both the ArrayNode and key were freed
- [x] `testArrayNodeGetB2Miss` — Key not in BMI sub-node (Path B2)
  - Create ArrayNode with a BMI sub-node containing key K1
  - Lookup key K2 that hashes to the same slot but isn't in the BMI
  - Verify the default value is returned
  - Verify the ArrayNode was freed, BMI sub-node was freed by bmiGet

#### `testArrayNodeCount` — count entries
- [x] `testArrayNodeCount` — Count 2 entries in ArrayNode
  - Create ArrayNode with 2 entries at different slots
  - Call arrayNodeCount
  - Verify count returns 2
  - Verify the ArrayNode was freed
  - Verify each sub-node was counted via count_fn
- [x] `testArrayNodeCountEmpty` — Empty ArrayNode (Path A)
  - Create empty ArrayNode
  - Call arrayNodeCount
  - Verify count returns 0
- [x] `testArrayNodeCountSingle` — Single entry (Path B)
  - Create ArrayNode with one entry
  - Call arrayNodeCount
  - Verify count returns 1
  - Create ArrayNode with 2 entries at different slots
  - Call arrayNodeCount
  - Verify count returns 2
  - Verify the ArrayNode was freed
  - Verify each sub-node was counted via count_fn

#### `testArrayNodeDissoc` — remove from ArrayNode
- [x] `testArrayNodeDissocEmptySlot` — Key not found (empty slot, Path A)
  - Create ArrayNode with one entry at slot X
  - Dissoc key that hashes to empty slot Y
  - Verify original node pointer returned (no new allocation)
  - Verify original entry still exists
- [x] `testArrayNodeDissoc` — Key found in sub-node (Path B1)
  - Create ArrayNode with 2 entries
  - Dissoc one key
  - Verify result is a new ArrayNode with 1 entry
  - Verify the removed key is gone
  - Verify the remaining key is still accessible
  - Verify the remaining key is still accessible
  - Verify the original ArrayNode was freed

#### `testArrayNodeMutateAssoc` — mutate in-place when refs==1
`arrayNodeMutateAssoc` has two top-level paths: `subNode == 0` (slot empty) vs `subNode != 0` (slot has a sub-node).

- [x] `testArrayNodeMutateAssocRecurse` — Slot has BMI sub-node → recurse into BMI
  - Create ArrayNode with one entry at slot X (BMI sub-node)
  - Set refs==1 on the node
  - Mutate with a key that hashes to slot X
  - Verify the same ArrayNode pointer is returned (in-place mutation)
  - Verify the inner BMI entry was updated
  - Verify no new allocations (glblAlloced unchanged)
- [x] `testArrayNodeMutateAssocInsert` — Slot is empty → insert new entry
  - Create ArrayNode with one entry at slot X
  - Set refs==1 on the node
  - Mutate with a key that hashes to empty slot Y
  - Verify a new ArrayNode is returned (n+1 entries)
  - Verify both entries are present
  - Verify old node was freed (no leak)

### Phase 4: CollisionNode Operations
- [x] `testCollisionAssocAdd` — add new key to collision node (same hash, different key)
- [x] `testCollisionAssocUpdate` — update existing key value in collision node
- [x] `testCollisionAssocPromote` — add key with different hash → promotes to BMI
- [x] `testCollisionGet` — lookup in collision node
- [x] `testCollisionDissoc` — remove from collision node
- [x] `testCollisionCount` — count entries
- [x] `testCollisionVec` — flatten to vector


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

1. **Phase 1: Add test-hash-map.c tests** — memory management, basic node creation ✅ (DONE)
2. **Phase 2: Add and run BMI operation tests** — BMI code active, `testBmiCopyAssoc` written ✅
3. **Fix `freeBitmapNode` / `freeHashCollisionNode`** — remove `termVal()` call before `dec_and_free` ✅
4. **Implement `equal()`** — I60 via `integer_EQ`, other types via `equalSTAR` ✅
5. **Uncomment stubs needed for ArrayNode operations** — `arrayNodeCopyAssoc`, `arrayNodeGet`, `arrayNodeDissoc`, `arrayNodeCount`
6. **Uncomment stubs needed for CollisionNode operations** — `collisionAssoc`, `collisionGet`, `collisionDissoc`, `collisionCount`
8. **Add collisionAssoc tests** — add, update, promote ✅
9. **Uncomment `get` / `baseDissoc` / `hashVec` / `copyAssoc` / `mutateAssoc`** — polymorphic dispatch functions
10. **Uncomment `hashMapGet` / `hashMapAssoc`** — public API
11. **Uncomment `collisionAssoc` case** in `mutateAssoc` — fix core.c
12. **Wire protocol bindings** — assign function pointers so Toccata functions dispatch to C implementations
13. **Add `{}` constructor** — enable empty HashMap creation
14. **Add `empty?` protocol** for HashMap
15. **Add `=` protocol** for HashMap
16. **Add `get*` / `dissoc*` variants** — explicit hash/shift parameters
17. **Add `assoc-all`** — batch insertion

## Known Test Issue: No GC Exercise

**`regression-tests/test-hash-map.c` uses I60 integers for all keys and values.**

I60 terms are leaf terms — they have no heap allocations, no ref counting, no `incRef`/`dec_and_free` calls. The tests verify structural correctness and memory accounting (malloc_count/free_count), but they **do not exercise garbage collection**.

Specifically, the tests never:
- Create a VAL term that points to a heap-allocated struct (String, ReifiedVal, etc.)
- Exercise `incRef`/`dec_and_free` on heap-allocated values
- Test what happens when a key or value is a complex type (String, nested map, etc.)
- Test ref counting edge cases (shared references, partial updates that leave old values dangling)

**What's needed:** A separate test file (or extended tests) that use heap-allocated values as keys and values — e.g., String keys, nested HashMap values, ReifiedVal structs — to verify that `incRef`/`dec_and_free` paths work correctly through the hash-map operations.

## Test Function Checklist (I60 → GC Exercise)

Each test below currently uses only I60 integers for keys and values. The goal is to convert them to use heap-allocated VAL terms (String keys, ReifiedVal values) so that `incRef`/`dec_and_free` paths are exercised.

### Phase 1: Allocator Tests (no keys/values — already GC-relevant)
- [x] `testEmptyBmiNode` — BMI node allocation/free (no KV pairs, already GC-relevant)
- [x] `testBmiNodeOneItem` — BMI node allocation/free (no KV pairs, already GC-relevant)
- [x] `testArrayNode` — ArrayNode allocation/free (no KV pairs, already GC-relevant)
- [x] `testCollisionNode` — HashCollisionNode allocation/free (no KV pairs, already GC-relevant)
- [x] `testFreeBitmapNodeHighCount` — freeBitmapNode actual free path
- [x] `testFreeArrayNode` — freeArrayNode pool recycle
- [x] `testFreeHashCollisionNode` — freeHashCollisionNode actual free

### Phase 2: BMI mutateAssoc Tests (7 paths)
- [x] `testBmiMutateAssoc` — refs==1, empty BMI → insert single entry (2b)
- [ ] `testBmiMutateAssocNoOp` — bit set, same key + same value (1b)
- [ ] `testBmiMutateAssocUpdateValue` — bit set, same key + different value (1c)
- [ ] `testBmiMutateAssocSubNodeRecurse` — bit set, sub-node case (1a)
- [ ] `testBmiMutateAssocBranch` — bit set, different key + different hash (1e)
- [ ] `testBmiMutateAssocCollision` — bit set, different key + same hash (1d)
- [ ] `testBmiMutateAssocInsert` — bit not set, n < 16, insert (2b)
- [ ] `testBmiMutateAssocPromote` — bit not set, n >= 16, promote to ArrayNode (2a)

### Phase 3: BMI copyAssoc Tests
- [x] `testBmiCopyAssoc` — add key/value to empty BMI (B2)
- [x] `testBmiCopyAssocNoOp` — same key, same value → no-op (A2a)
- [ ] `testBmiCopyAssocUpdate` — same key, different value → clone (A2b)
- [ ] `testBmiCopyAssocBranch` — two keys at different bit positions → branch node (A2d)
- [ ] `testBmiCopyAssocSubNodeNoChange` — nested tree, inner update no-change → no-op (A1a)
- [ ] `testBmiCopyAssocSubNodeChange` — nested tree, inner update changes → clone (A1b)
- [ ] `testBmiCopyAssocCollision` — identical SHA1 hash → collision node (A2c)

### Phase 4: BMI get/dissoc/count Tests
- [ ] `testBmiGet` — lookup existing key
- [ ] `testBmiGetMiss` — lookup missing key
- [ ] `testBmiDissoc` — remove key from single-item BMI
- [ ] `testBmiDissocEmpty` — remove last key → returns emptyBMI
- [ ] `testBmiCount` — N-entry map, verify count == N
- [ ] `testBmiHashVec` — flatten BMI to vector of pairs

### Phase 5: ArrayNode Tests
- [ ] `testArrayNodeCopyAssoc` — add key/value to empty ArrayNode
- [ ] `testArrayNodeCopyAssocA2` — non-empty ArrayNode, add to empty slot
- [ ] `testArrayNodeCopyAssocB1` — ArrayNode with sub-node, same key+value (no-op)
- [ ] `testArrayNodeCopyAssocB2` — ArrayNode with sub-node, different value
- [ ] `testArrayNodeCopyAssocB2Multi` — multiple entries, update one sub-node
- [ ] `testArrayNodeGet` — lookup existing key in ArrayNode
- [ ] `testArrayNodeGetMiss` — lookup missing key (empty slot)
- [ ] `testArrayNodeGetB2Miss` — key not in BMI sub-node
- [ ] `testArrayNodeCount` — count entries in ArrayNode
- [ ] `testArrayNodeCountEmpty` — count empty ArrayNode
- [ ] `testArrayNodeCountSingle` — count single-entry ArrayNode
- [ ] `testArrayNodeDissocEmptySlot` — key not found (empty slot)
- [ ] `testArrayNodeDissoc` — remove key from ArrayNode
- [ ] `testArrayNodeMutateAssocRecurse` — slot has BMI sub-node → recurse
- [ ] `testArrayNodeMutateAssocInsert` — slot is empty → insert new entry

### Phase 6: CollisionNode Tests
- [ ] `testCollisionAssocAdd` — add new key to collision node
- [ ] `testCollisionAssocUpdate` — update existing key value
- [ ] `testCollisionAssocPromote` — add key with different hash → promotes to BMI
- [ ] `testCollisionCount` — count entries
- [ ] `testCollisionVec` — flatten to vector
- [ ] `testCollisionDissoc` — remove from collision node
- [ ] `testCollisionGet` — lookup in collision node

## Test File

- **`regression-tests/hash-map-regressions.toc`** — 40+ assertions covering:
  - Empty map, singleton, multi-entry maps
  - `assoc`, `assoc-all`, `get`, `get*`, `dissoc`, `dissoc*`
  - `count`, `empty?`, `=`
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
