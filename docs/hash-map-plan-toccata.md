# Hash-Map Plan — Toccata-Level Implementation and Testing

## Overview

The Toccata layer wraps the C-level bitmap-trie functions (see `hash-map-plan-c.md`) with inline-C wrappers in `hvm-core.toc` and implements the hash-map protocol methods in Toccata. Testing is exercised through `regression-tests/test-bmi.toc` (BMI node level) and, eventually, `regression-tests/hash-map-regressions.toc` (full map level).

**State (2026-08-16):** HEAD `56354fb`; working tree clean except this plan; both suites pass — `test-bmi` (4 Group A cases, malloc/free balanced, `remaining nodes: 0`) and `test-hash-map` (54 tests, all green).

## Big Picture: Node Types and the Toccata-Level Assoc Requirement

There are 3 hash-map node types, each with a copy-assoc and a mutate-assoc function in `runtime3.c`:

| Node type | C copy-assoc | C mutate-assoc |
|---|---|---|
| `BitmapIndexedNode` | `bmiCopyAssoc` (runtime3.c:2270) | `bmiMutateAssoc` (2297) |
| `ArrayNode` | `arrayNodeCopyAssoc` (2456) | `arrayNodeMutateAssoc` (2494) |
| `HashCollisionNode` | `collisionAssoc` (2514) | none — the `HashCollisionNodeType` case in `mutateAssoc` (2775) is commented out, so collision nodes fall back to `copyAssoc` |

The C-level dispatchers are `copyAssoc` (runtime3.c:2761) and `mutateAssoc` (2775), switching on `node->type`.

**Why the C versions can't be called from Toccata.** They perform equality checks internally via the C-level `equal` (runtime3.c:1875), which only handles I60 and String (`strCmp`, BOOM for anything else) and cannot dispatch to the `=` protocol function. Correct behavior for arbitrary key types (e.g. deftypes with custom `=`) requires the `=` protocol, which lives at the Toccata level — C cannot call up into it.

**Consequence.** The copy-assoc and mutate-assoc logic for all 3 node types must be implemented in Toccata, using the wrapped C functions for the mechanical parts (bitmap ops, node cloning, array manipulation) and the protocol `=`/`sha1` for the semantic parts. The Toccata-level `bmiCopyAssoc` (hvm-core.toc:1041) is the working example of this pattern: it does `(= currKey k)` in Toccata and delegates to `bitpos`, `bmiBitMap`, `bmiChild`, `bmiKey`, `bmiVal`, `bmiClone`, `bmiUpdate`, `addCopiedBMI`, `bmiReplaceCopied`.

**Status of the 5 functions at the Toccata level:** `bmiCopyAssoc` is implemented (partially tested — see below). `bmiMutateAssoc`, `arrayNodeCopyAssoc`, `arrayNodeMutateAssoc`, `collisionAssoc` (copy) are not yet implemented in Toccata. Collision nodes need no mutate variant (copy path only, matching the C dispatcher).

## Protocol Functions NOT wired up yet

- `count` — returns count of sequence
- `sha1` — computes SHA1 hash
- `get` — polymorphic get dispatch
- `baseDissoc` — polymorphic dissoc dispatch
- `vals` — get all values

## BMI Surface (what's testable from Toccata today)

**Exposed in `hvm-core.toc`:** `emptyBMI`, `bitpos`, `bmiBitMap`, `bmiKey`, `bmiVal`, `bmiChild`, `bmiClone`, `bmiUpdate`, `addCopiedBMI`, `bmiReplaceCopied`, `bmiCopyAssoc`, `bmiCount`/`count`, `bmiHashVec`, `bmiVec`/`vec` (Toccata-level recursion — the C `bmiHashVec` aborts on sub-node slots via the dead `hashVec` dispatcher), `empty?`

**Not exposed** (in `runtime3.c` only — can't test until wrapped): `bmiGet`/`mapGet`, `bmiDissoc`, the mutate family (`bmiMutateAssoc`, `bmiReplaceMutate`, `addMutateBMI`), `bmiSetKey`/`bmiSetVal`

**Currently covered in test-bmi.toc:** bitmap-of-empty, add-to-empty (count 1), same-key+same-value ×3 (no-op branch), same-key-different-value (bmiClone branch — C-side fix `9f5c40c`, test added in `944748c`), different-key-different-bit (add branch, `56354fb`).

## BMI Tests (`regression-tests/test-bmi.toc`)

The C-level tests in `hash-map-plan-c.md` verify the raw functions. The cases below cover the Toccata protocol layer — the inline-C wrappers and the Toccata `bmiCopyAssoc` implementation.

**Running the tests:** `make test-bmi` is the correct way to run this suite. `make` can be used for any Makefile target without thinking further — no need to run the underlying `./new-toc` + `clang` steps by hand.

### Prerequisite: controlled-hash key type

Add a `ControlledHash`-style deftype (same pattern as `hash-map-regressions.toc`) so tests can pin exact bit positions:

```
(deftype CHash [h eq]
  (= [x y] (and (= (type-num x) (type-num y)) eq (= h (.h y))))
  (sha1 [_] h))
```

Useful hashes: `3` (bit 3), `35` (bit 3 at shift 0, bit 1 at shift 5), `4` (bit 4), `67` (bit 3, then bit 2), `195` (bit 3, then bit 6 at shift 5), and `7` with `eq=None` vs `eq=Some` for full collisions. Note `=` is asymmetric by design (the `eq` gate is on the first arg): to make two same-hash keys unequal, the `eq=None` key must be the *new* key in the assoc.

### Test cases

Low-level functions (`bitpos`, `bmiBitMap`, `bmiKey`, `bmiVal`, `bmiChild`, `bmiClone`, `bmiUpdate`, `addCopiedBMI`, `bmiReplaceCopied`) are not tested standalone right now — they are covered via the higher-level tests below.

**A. `bmiCopyAssoc` integration**
- [x] *(existing)* add to empty
- [x] *(existing)* same key + same value → no-op, count 1
- [x] same key, **different** value → count 1, `vec == [[k v2]]` (bmiClone branch)
- [x] different key, different bit → count 2 (add branch)
- [x] different key, same bit, different hash at shift 5 (3 vs 35) → count 2, sub-node (replace→createNode branch)
- [x] different key, identical hash (7/7 collision) → count 2, collision node (replace→collision branch)
- [x] **recurse-into-child branch:** node with keys 3+35, then assoc key 67 (bit 3 again) → count 3
- [x] **deep createNode:** keys 3, 35, 67, then 195 (bit 3 → bit 6 at shift 5) → count 4 (exercises `createNode`'s same-bit-at-next-level recursion)
- [x] same key, different value, **key located inside a sub-node** (depth 2) → count unchanged, `vec` shows updated value (child-recurse + clone at depth)

**B. `count` / `vec` recursion (untested beyond count-1)**
- [ ] flat 3-entry node → count 3
- [ ] mixed node (2 flat + sub-node of 2) → count 4
- [ ] `vec` of flat node → `[[k1 v1] [k2 v2] ...]` in bit order
- [ ] `vec` of node with sub-node → sub-node's pairs **flattened in** (not nested)

**C. `empty?` (untested)**
- [ ] `(empty? emptyBMI) == (Some emptyBMI)`
- [ ] `(empty? one-entry-node) == None`

**D. Memory hygiene (implicit, per status.md failure conditions)**
Every case above must end with `malloc_count == free_count` and `glblAlloced == 0`. Highest-risk paths, all now exercised: owned refs from `bmiKey`/`bmiVal`/`bmiChild` (the `test-bmi` double-free area), the no-op branch's `dec_and_free` dance, `bmiUpdate` same-child path, collision-node `incRef`s.

**When a memory leak or double-free is found** (non-zero `diff:`, `refs too small` abort, or end-of-test leak), follow the `skills/memory-leak-hunting.md` skill — it covers the shrink-to-minimal-expression workflow, refcount trajectory logging, the lldb workflow, and the known leak/double-free patterns.

### Out of scope until more is exposed

- `get` / `dissoc` tests — belong in `hash-map-regressions.toc` once `bmiGet`/`bmiDissoc` are wrapped
- mutate family — not exposed
- 16+ entry ArrayNode conversion — `BOOM("wtf")` in copied `addCopiedBMI` (`runtime3.c:2046,2060`) until implemented
- `bmiKey`/`bmiVal` on a sub-node slot — aborts: the slot's key is `0` and the wrapper unconditionally `incRef`s it (`incRef` aborts on NULL, `runtime3.c:808`). Needs a `None` return for sub-node slots before it can be tested
- shift-exhaustion (`shift > 60` abort) — deliberate crash, don't test

## Files to Read

| File | Purpose |
|---|---|
| `hvm-core.toc` | Inline-C BMI wrappers + Toccata protocol implementations |
| `regression-tests/test-bmi.toc` | BMI-level regression tests |
| `regression-tests/hash-map-regressions.toc` | Full hash-map regression tests (ControlledHash pattern reference) |
| `regression-tests/regression-tester.toc` | `rt/test` assertion macro |
| `new.c` | Compiler — may need modifications for `{}` literal |
| `docs/implementation-notes.md` | Working notes on Toccata/HVM internals — term layout, arg passing, protocol dispatch codegen, BMI C API hazards, build/test pipeline |
| `skills/memory-leak-hunting.md` | Skill to follow when a memory leak or double-free is found |
