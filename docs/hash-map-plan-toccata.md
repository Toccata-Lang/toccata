# Hash-Map Plan — Toccata-Level Implementation and Testing

## Overview

The Toccata layer wraps the C-level bitmap-trie functions (see `hash-map-plan-c.md`) with inline-C wrappers in `hvm-core.toc` and implements the hash-map protocol methods in Toccata. Testing is exercised through `regression-tests/test-bmi.toc` (BMI node level) and, eventually, `regression-tests/hash-map-regressions.toc` (full map level).

## Protocol Functions NOT wired up yet

- `count` — returns count of sequence
- `sha1` — computes SHA1 hash
- `get` — polymorphic get dispatch
- `baseDissoc` — polymorphic dissoc dispatch
- `vals` — get all values

## BMI Surface (what's testable from Toccata today)

**Exposed in `hvm-core.toc`:** `emptyBMI`, `bitpos`, `bmiBitMap`, `bmiKey`, `bmiVal`, `bmiChild`, `bmiClone`, `bmiUpdate`, `addCopiedBMI`, `bmiReplaceCopied`, `bmiCopyAssoc`, `bmiCount`/`count`, `bmiHashVec`/`vec`, `empty?`

**Not exposed** (in `runtime3.c` only — can't test until wrapped): `bmiGet`/`mapGet`, `bmiDissoc`, the mutate family (`bmiMutateAssoc`, `bmiReplaceMutate`, `addMutateBMI`), `bmiSetKey`/`bmiSetVal`

**Currently covered in test-bmi.toc:** bitmap-of-empty, add-to-empty (count 1), same-key+same-value ×3 (no-op branch only). The same-key-different-value branch (`bmiClone`, fixed in `9f5c40c`) is not directly tested — the existing replace test uses identical values, so it hits the no-op path.

## BMI Tests (`regression-tests/test-bmi.toc`)

The C-level tests in `hash-map-plan-c.md` verify the raw functions. The cases below cover the Toccata protocol layer — the inline-C wrappers and the Toccata `bmiCopyAssoc` implementation.

### Prerequisite: controlled-hash key type

Add a `ControlledHash`-style deftype (same pattern as `hash-map-regressions.toc`) so tests can pin exact bit positions:

```
(deftype CHash [h eq]
  (= [x y] (and (= (type-num x) (type-num y)) eq (= h (.h y))))
  (sha1 [_] h))
```

Useful hashes: `3` (bit 3), `35` (bit 3 at shift 0, bit 1 at shift 5), `4` (bit 4), `67` (bit 3, then bit 2), `195` (bit 3, then bit 6 at shift 5), and `7` with `eq=None` vs `eq=Some` for full collisions. Note `=` is asymmetric by design (the `eq` gate is on the first arg): to make two same-hash keys unequal, the `eq=None` key must be the *new* key in the assoc.

### Test cases

**A. `bitpos` (pure, untested)**
- [ ] `(bitpos 3 0) == 8` — mask = `1 << (hash & 31)`
- [ ] `(bitpos 35 0) == 8` — same bit at shift 0
- [ ] `(bitpos 35 5) == 2`, `(bitpos 3 5) == 1` — shift behavior
- [ ] `(bitpos 64 0) == 1` — wraps to bit 0

**B. `bmiBitMap`**
- [x] *(existing)* emptyBMI → 0
- [ ] one key, hash 3 → bitmap `8`
- [ ] keys hash 3 + 4 → bitmap `24`
- [ ] keys hash 3 + 35 (sub-node created) → bitmap still `8` (sub-nodes don't add bits at this level)

**C. `bmiKey` / `bmiVal` (untested; exercise the owned-ref `incRef` wrappers)**
- [ ] after assoc `(CHash 3 (Some 3))`/`(Val 1)`: `(bmiKey m 8)` equals the key, `(bmiVal m 8)` equals `(Val 1)`
- [ ] two-entry node: key/val correct at both bits 8 and 16 (index ordering)
- ⚠️ **Landmine, do not test yet:** `bmiKey`/`bmiVal` on a sub-node slot aborts — the slot's key is `0` and the wrapper unconditionally `incRef`s it (`incRef` aborts on NULL, `runtime3.c:808`). Either the wrapper needs a `None` return for sub-node slots, or this stays untested.

**D. `bmiChild` (untested)**
- [ ] k/v slot → `None`
- [ ] sub-node slot (keys hash 3 + 35) → `Some(child)`; extracted child has `count == 2` and correct `vec`

**E. `bmiClone` (untested — covers the `9f5c40c` fix)**
- [ ] one-entry node, clone with same key, **different** value → count 1, `vec == [[k v2]]`
- [ ] two-entry node, clone value at bit 8 → count 2, other entry (bit 16) preserved, replaced value present

**F. `bmiUpdate` (untested)**
- [ ] node with sub-node at bit 8 + flat entry at bit 16: replace child with a fresh single-pair sub-node → count 2, `vec` shows new child's pair + preserved flat entry
- [ ] same-child no-op: extract child via `bmiChild`, `bmiUpdate` it back → count unchanged, no crash (pointer-equality path)

**G. `addCopiedBMI` direct (untested)**
- [ ] add to emptyBMI, hash 3 → bitmap 8, count 1, key/val at bit 8
- [ ] node with bit-16 entry, add key at bit 8 → **insert-before**: bitmap 24, old entry still at bit 16
- [ ] node with bit-8 entry, add key at bit 16 → **insert-after**: bitmap 24, old entry still at bit 8
- ⚠️ **Blocked:** 16-entry → ArrayNode conversion hits `BOOM("wtf")` in copied `addCopiedBMI` (`runtime3.c:2046,2060`). Untestable until implemented.

**H. `bmiReplaceCopied` direct (untested — both branches)**
- [ ] different hashes at this level (stored hash 3, new hash 35) → sub-node: count 2, `bmiChild` at bit 8 → `Some`, `vec` has both pairs
- [ ] identical hashes, unequal keys (both hash 7; new key `eq=None` added second so `=` fails) → **collision node**: count 2, `vec` has both pairs

**I. `bmiCopyAssoc` integration (5 branches; 2 of 5 covered)**
- [x] *(existing)* add to empty
- [x] *(existing)* same key + same value → no-op, count 1
- [ ] same key, **different** value → count 1, `vec == [[k v2]]` (bmiClone branch)
- [ ] different key, different bit → count 2 (add branch)
- [ ] different key, same bit, different hash at shift 5 (3 vs 35) → count 2, sub-node (replace→createNode branch)
- [ ] different key, identical hash (7/7 collision) → count 2, collision node (replace→collision branch)
- [ ] **recurse-into-child branch:** node with keys 3+35, then assoc key 67 (bit 3 again) → count 3
- [ ] **deep createNode:** keys 3, 35, 67, then 195 (bit 3 → bit 6 at shift 5) → count 4 (exercises `createNode`'s same-bit-at-next-level recursion)
- [ ] same key, different value, **key located inside a sub-node** (depth 2) → count unchanged, `vec` shows updated value (child-recurse + clone at depth)

**J. `count` / `vec` recursion (untested beyond count-1)**
- [ ] flat 3-entry node → count 3
- [ ] mixed node (2 flat + sub-node of 2) → count 4
- [ ] `vec` of flat node → `[[k1 v1] [k2 v2] ...]` in bit order
- [ ] `vec` of node with sub-node → sub-node's pairs **flattened in** (not nested)

**K. `empty?` (untested)**
- [ ] `(empty? emptyBMI) == (Some emptyBMI)`
- [ ] `(empty? one-entry-node) == None`

**L. Memory hygiene (implicit, per status.md failure conditions)**
Every case above must end with `malloc_count == free_count` and `glblAlloced == 0`. Highest-risk paths, all now exercised: owned refs from `bmiKey`/`bmiVal`/`bmiChild` (the `test-bmi` double-free area), the no-op branch's `dec_and_free` dance, `bmiUpdate` same-child path, collision-node `incRef`s.

### Out of scope until more is exposed

- `get` / `dissoc` tests — belong in `hash-map-regressions.toc` once `bmiGet`/`bmiDissoc` are wrapped
- mutate family — not exposed
- 16+ entry ArrayNode conversion — `BOOM` until implemented
- shift-exhaustion (`shift > 60` abort) — deliberate crash, don't test

## Files to Read

| File | Purpose |
|---|---|
| `hvm-core.toc` | Inline-C BMI wrappers + Toccata protocol implementations |
| `regression-tests/test-bmi.toc` | BMI-level regression tests |
| `regression-tests/hash-map-regressions.toc` | Full hash-map regression tests (ControlledHash pattern reference) |
| `regression-tests/regression-tester.toc` | `rt/test` assertion macro |
| `new.c` | Compiler — may need modifications for `{}` literal |
