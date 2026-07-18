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

## Architecture Reference

### Node Types (defined in `runtime3.h` / `core.h`)

| Type | ID | Purpose |
|---|---|---|
| `BitmapIndexedNode` | 11 | Sparse bitmap trie — efficient for small-to-medium maps |
| `ArrayNode` | 12 | Fallback when BMI gets too wide (>16 entries) |
| `HashCollisionNode` | 13 | Stores entries that hash-collide |
| `HashMapType` | 14 | Abstract type number for `instance?` checks |

## What Needs to Be Done

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
- [x] `testBmiMutateAssocNoOp` — bit set, same key + same value (1b)
- [x] `testBmiMutateAssocUpdateValue` — bit set, same key + different value (1c)
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

## Files to Read

| File | Purpose |
|---|---|
| `runtime3.c` | Stubs to uncomment (lines ~1978–2859) |
| `runtime3.h` | Type definitions, extern declarations |
| `regression-tests/test-hash-map.c` | Tests for the C-level functions of the hash-map implementation |
| `regression-tests/test-hvm.c` | Reference test pattern |
| `new.c` | Compiler — may need modifications for `{}` literal |
