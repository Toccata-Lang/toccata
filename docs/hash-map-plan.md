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

**Status: complete.** All 50 tests are converted to String keys/values (see checklist below).

## Next Task: Cover Remaining BOOM("test") Tripwires

Five `BOOM("test")` tripwires remain in `runtime3.c`: four on **nested (multi-level) BMI sub-node** paths in `bmiGet`/`bmiDissoc`, and one in `bmiDissoc` for a different-key no-op. The tests below exercise those paths. Per the conversion skill, each tripwire is removed in the commit of the test that first hits it.

| BOOM | Function | Untested path |
|---|---|---|
| runtime3.c:2346 | `bmiGet` | get recurses into a nested sub-node (`keyOrNull == 0`) |
| runtime3.c:2412 | `bmiDissoc` | sub-node unchanged after recurse (`n == valOrNode`) |
| runtime3.c:2416 | `bmiDissoc` | sub-node becomes `emptyBMI` and is the parent's only entry |
| runtime3.c:2420 | `bmiDissoc` | sub-node shrinks (changed) → clone parent |
| runtime3.c:2452 | `bmiDissoc` | bit set with a direct key, but a different key → no-op |

**Grouping:** a sub-node is created with 2 entries, so the "empty as only entry" case (2416) is only reachable after shrinking it to 1 entry (2420). Those two are one coupled test. The "unchanged" case (2412) and the different-key no-op (2452, a flat BMI) are each independent.

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
- [x] `testCollisionAssocUpdate` — update existing key value
- [x] `testCollisionAssocPromote` — add key with different hash → promotes to BMI
- [x] `testCollisionCount` — count entries
- [x] `testCollisionVec` — flatten to vector
- [x] `testCollisionDissoc` — remove from collision node
- [x] `testCollisionGet` — lookup in collision node
- [x] `testBmiHashVec` — flatten BMI to vector of pairs
- [x] `testBmiGetNested` — get a key stored in a nested sub-node (removes BOOM runtime3.c:2346)
- [x] `testBmiDissocSubNodeUnchanged` — dissoc a key not in the sub-node → sub-node unchanged (removes BOOM runtime3.c:2412)
- [x] `testBmiDissocSubNodeShrinkEmpty` — shrink the sub-node, then empty it as the parent's only entry (removes BOOMs runtime3.c:2420/2416)
- [ ] `testBmiDissocDifferentKey` — dissoc a different key at an occupied bit → no-op (removes BOOM runtime3.c:2452)

## Files to Read

| File | Purpose |
|---|---|
| `runtime3.c` | Stubs to uncomment (lines ~1978–2859) |
| `runtime3.h` | Type definitions, extern declarations |
| `regression-tests/test-hash-map.c` | Tests for the C-level functions of the hash-map implementation |
| `regression-tests/test-hvm.c` | Reference test pattern |
| `new.c` | Compiler — may need modifications for `{}` literal |
