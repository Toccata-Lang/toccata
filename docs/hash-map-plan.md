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

**IMPORTANT: Tests are converted in their order of appearance in `main()`** (lines 2368–2417 of `test-hash-map.c`). When checking progress, scan `main()` top-to-bottom and find the first unchecked test.

**Use skill `skills/convert-test-to-strings.md` for the conversion process.** It contains the step-by-step workflow, pool accounting reference, commit rules, and common pitfalls.

## Current Task: Convert Tests to Use Allocated Strings

**The hash-map tests originally used I60 integers for keys and values.** This works fine for testing the structural logic at the upper levels of the hash-map implementation — bit manipulation, node promotion, collision detection, sub-node branching — but it is insufficient because it completely ignores garbage collection.

I60 terms are leaf terms: no heap allocation, no ref counting, no `incRef`/`dec_and_free` calls. The tests verify structural correctness and memory accounting (malloc_count/free_count of nodes), but they **do not exercise garbage collection of keys and values**.

**The current task is to convert all tests to use heap-allocated String keys and String values**, and fix any memory leaks that surface during the conversion.

When converting a test:
1. Replace `newI60(x)` keys/values with `stringValue("key")` / `stringValue("value")`
2. Track the expected `malloc_count` and `free_count` for the String allocations
3. Watch for leaks: `incRef` on a String value that is never `dec_and_free`'d, old values not freed during mutation, etc.
4. Document any leak patterns found in `skills/memory-leak-hunting.md`

## Test Checklist (order of appearance in `main()`)

- [x] `testEmptyBmiNode` — BMI node allocation/free (no KV pairs, already GC-relevant)
- [x] `testBmiNodeOneItem` — BMI node allocation/free (no KV pairs, already GC-relevant)
- [x] `testArrayNode` — ArrayNode allocation/free (no KV pairs, already GC-relevant)
- [x] `testCollisionNode` — HashCollisionNode allocation/free (no KV pairs, already GC-relevant)
- [x] `testFreeBitmapNodeHighCount` — freeBitmapNode actual free path
- [x] `testFreeArrayNode` — freeArrayNode pool recycle
- [x] `testFreeHashCollisionNode` — freeHashCollisionNode actual free
- [x] `testBmiCopyAssoc` — add key/value to empty BMI (B2)
- [x] `testBmiMutateAssoc` — refs==1, empty BMI → insert single entry (2b)
- [x] `testBmiCopyAssocNoOp` — same key, same value → no-op (A2a)
- [x] `testBmiCopyAssocUpdate` — same key, different value → clone (A2b)
- [x] `testBmiGet` — lookup existing key
- [x] `testBmiGetMiss` — lookup missing key
- [x] `testBmiDissoc` — remove key from single-item BMI (uses String KV)
- [x] `testBmiDissocEmpty` — remove last key → returns emptyBMI
- [x] `testBmiCopyAssocBranch` — two keys at different bit positions → branch node (A2d)
- [x] `testBmiCopyAssocSubNodeNoChange` — nested tree, inner update no-change → no-op (A1a)
- [x] `testBmiCopyAssocSubNodeChange` — nested tree, inner update changes → clone (A1b)
- [x] `testBmiCopyAssocCollision` — identical SHA1 hash → collision node (A2c)
- [x] `testBmiCount` — N-entry map, verify count == N
- [x] `testBmiMutateAssocUpdateValue` — bit set, same key + different value (1c)
- [x] `testBmiMutateAssocInsert` — bit not set, n < 16, insert (2b)
- [x] `testBmiMutateAssocBranch` — bit set, different key + different hash (1e)
- [x] `testBmiMutateAssocCollision` — bit set, different key + same hash (1d)
- [x] `testBmiMutateAssocSubNodeRecurse` — bit set, sub-node case (1a)
- [x] `testBmiMutateAssocNoOp` — bit set, same key + same value (1b)
- [x] `testBmiMutateAssocPromote` — bit not set, n >= 16, promote to ArrayNode (2a)
- [x] `testArrayNodeCopyAssoc` — add key/value to empty ArrayNode
- [x] `testArrayNodeCopyAssocA2` — non-empty ArrayNode, add to empty slot
- [x] `testArrayNodeCopyAssocB1` — ArrayNode with sub-node, same key+value (no-op)
- [x] `testArrayNodeCopyAssocB2` — ArrayNode with sub-node, different value
- [x] `testArrayNodeCopyAssocB2Multi` — multiple entries, update one sub-node
- [x] `testArrayNodeGet` — lookup existing key in ArrayNode
- [x] `testArrayNodeGetMiss` — lookup missing key (empty slot)
- [x] `testArrayNodeGetB2Miss` — key not in BMI sub-node
- [x] `testArrayNodeCount` — count entries in ArrayNode
- [x] `testArrayNodeCountEmpty` — count empty ArrayNode (no KV pairs, already GC-relevant)
- [x] `testArrayNodeCountSingle` — count single-entry ArrayNode
- [x] `testArrayNodeDissocEmptySlot` — key not found (empty slot)
- [x] `testArrayNodeDissoc` — remove key from ArrayNode
- [x] `testArrayNodeMutateAssocInsert` — slot is empty → insert new entry
- [x] `testArrayNodeMutateAssocRecurse` — slot has BMI sub-node → recurse
- [x] `testCollisionAssocAdd` — add new key to collision node
- [ ] `testCollisionAssocUpdate` — update existing key value
- [ ] `testCollisionAssocPromote` — add key with different hash → promotes to BMI
- [ ] `testCollisionCount` — count entries
- [ ] `testCollisionVec` — flatten to vector
- [ ] `testCollisionDissoc` — remove from collision node
- [ ] `testCollisionGet` — lookup in collision node
- [ ] `testBmiHashVec` — flatten BMI to vector of pairs

## Files to Read

| File | Purpose |
|---|---|
| `runtime3.c` | Stubs to uncomment (lines ~1978–2859) |
| `runtime3.h` | Type definitions, extern declarations |
| `regression-tests/test-hash-map.c` | Tests for the C-level functions of the hash-map implementation |
| `regression-tests/test-hvm.c` | Reference test pattern |
| `new.c` | Compiler — may need modifications for `{}` literal |
