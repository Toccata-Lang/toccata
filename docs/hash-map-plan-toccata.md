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

**Why the C versions can't be called from Toccata.** They perform equality checks internally via the C-level `equal` (runtime3.c:1875), which only handles I60 and String (`strCmp`, BOOM for anything else) and cannot dispatch to the `=` protocol function. Correct behavior for arbitrary key types (e.g. deftypes with custom `=`) requires the `=` protocol, which lives at the Toccata level — C cannot call up into it. (The `sha1` global pointer has the same shape of problem; see the Blocker section below.)

**Consequence.** The copy-assoc and mutate-assoc logic for all 3 node types must be implemented in Toccata, using the wrapped C functions for the mechanical parts (bitmap ops, node cloning, array manipulation) and the protocol `=`/`sha1` for the semantic parts. The Toccata-level `bmiCopyAssoc` (hvm-core.toc:1041) is the working example of this pattern: it does `(= currKey k)` in Toccata and delegates to `bitpos`, `bmiBitMap`, `bmiChild`, `bmiKey`, `bmiVal`, `bmiClone`, `bmiUpdate`, `addCopiedBMI`, `bmiReplaceCopied`.

**Status of the 5 functions at the Toccata level:** `bmiCopyAssoc` is implemented (partially tested — see below). `bmiMutateAssoc`, `arrayNodeCopyAssoc`, `arrayNodeMutateAssoc`, `collisionAssoc` (copy) are not yet implemented in Toccata. Collision nodes need no mutate variant (copy path only, matching the C dispatcher).

## Protocol Functions NOT wired up yet

- `count` — returns count of sequence
- `sha1` — computes SHA1 hash
- `get` — polymorphic get dispatch
- `baseDissoc` — polymorphic dissoc dispatch
- `vals` — get all values

## BMI Surface (what's testable from Toccata today)

**Exposed in `hvm-core.toc`:** `emptyBMI`, `bitpos`, `bmiBitMap`, `bmiKey`, `bmiVal`, `bmiChild`, `bmiClone`, `bmiUpdate`, `addCopiedBMI`, `bmiReplaceCopied`, `bmiCopyAssoc`, `bmiCount`/`count`, `bmiHashVec`/`vec`, `empty?`

**Not exposed** (in `runtime3.c` only — can't test until wrapped): `bmiGet`/`mapGet`, `bmiDissoc`, the mutate family (`bmiMutateAssoc`, `bmiReplaceMutate`, `addMutateBMI`), `bmiSetKey`/`bmiSetVal`

**Currently covered in test-bmi.toc:** bitmap-of-empty, add-to-empty (count 1), same-key+same-value ×3 (no-op branch), same-key-different-value (bmiClone branch — C-side fix `9f5c40c`, test added in `944748c`), different-key-different-bit (add branch, `56354fb`).

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

Low-level functions (`bitpos`, `bmiBitMap`, `bmiKey`, `bmiVal`, `bmiChild`, `bmiClone`, `bmiUpdate`, `addCopiedBMI`, `bmiReplaceCopied`) are not tested standalone right now — they are covered via the higher-level tests below.

**A. `bmiCopyAssoc` integration (5 branches; 3 of 5 covered)**
- [x] *(existing)* add to empty
- [x] *(existing)* same key + same value → no-op, count 1
- [x] same key, **different** value → count 1, `vec == [[k v2]]` (bmiClone branch)
- [x] different key, different bit → count 2 (add branch)
- [x] different key, same bit, different hash at shift 5 (3 vs 35) → count 2, sub-node (replace→createNode branch)
- [x] different key, identical hash (7/7 collision) → count 2, collision node (replace→collision branch)
- [x] **recurse-into-child branch:** node with keys 3+35, then assoc key 67 (bit 3 again) → count 3
- [x] **deep createNode:** keys 3, 35, 67, then 195 (bit 3 → bit 6 at shift 5) → count 4 (exercises `createNode`'s same-bit-at-next-level recursion)
- [ ] same key, different value, **key located inside a sub-node** (depth 2) → count unchanged, `vec` shows updated value (child-recurse + clone at depth)

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

### Out of scope until more is exposed

- `get` / `dissoc` tests — belong in `hash-map-regressions.toc` once `bmiGet`/`bmiDissoc` are wrapped
- mutate family — not exposed
- 16+ entry ArrayNode conversion — `BOOM("wtf")` in copied `addCopiedBMI` (`runtime3.c:2046,2060`) until implemented
- `bmiKey`/`bmiVal` on a sub-node slot — aborts: the slot's key is `0` and the wrapper unconditionally `incRef`s it (`incRef` aborts on NULL, `runtime3.c:808`). Needs a `None` return for sub-node slots before it can be tested
- shift-exhaustion (`shift > 60` abort) — deliberate crash, don't test

## C-level `sha1` global pointer — agreed fix

The remaining Group A cases (replace→createNode, collision, recurse-into-child, deep createNode, child-recurse clone at depth) all route through `bmiReplaceCopied`, which **segfaulted** in the test binary. **Fixed in `f6a65e0` (2026-08-17):** `getHashVal` in `runtime3.c` (I60 → `integerSha1`, String/SubString → `strSha1`, user deftype → cached `hashVal`, else BOOM) is assigned to the global pointer in `main`, and `set-hash-val` in `hvm-core.toc` is called at the top of `bmiCopyAssoc` so stored keys' `hashVal` is populated before they are re-hashed. The remaining Group A cases now run.

**Root cause.** `bmiReplaceCopied` (runtime3.c:2186) — and the other C BMI functions — re-hash the *existing* key via the global C function pointer `Term (*sha1)(FnArity *, Term)` (runtime3.c:3225). That pointer is **never initialized in the test binary**:
- runtime3.c defines it NULL
- the codegen that would emit an initialized dispatcher is commented out (`term-core-dispatchers`, codegen.toc:1136, commented block 1152-1155)
- the compiler binary works only because its own (older) generated code defines it initialized (new-toc.c:65443)
- the C-level test works around it with `sha1 = testingSha1;`

The Toccata-level code is **correct and never touches the pointer**: it hashes the *new* key via the protocol `sha1` and passes the result in as the `hash` param. The crash is entirely inside the C functions it delegates to, which re-hash the *existing* key themselves.

**Why the `hashVal` cache isn't populated today (the case for the setter).** `hashVal` is a per-term hash cache, but:
- Only some types have the field (runtime3.h): `String` (56), `Vector` (69), `BitmapIndexedNode` (87), `ArrayNode` (95), `HashCollisionNode` (101), `ReifiedVal` (108). The base `Value` (Integer) and `TermVal` do **not**.
- It is written non-zero in exactly **one** place: `strSha1` (runtime3.c:1796), for String terms. Every other occurrence is a `= 0` init at allocation.
- "A key in the node was hashed when added" is true, but "therefore `hashVal` is populated" is **not** a current invariant. Hashing happens at the Toccata level, and only String's impl writes the result back into the term. `CHash` (a `ReifiedVal`, the test's key type) has the field, but its impl `(sha1 [_] h)` returns `h` without storing it — so its `hashVal` stays 0.
- The only concrete C hashers are `strSha1` (runtime3.c:1765) and `integerSha1` (runtime3.c:1829); everything else goes through the global pointer. The generated dispatcher (`m0_disp_sha1_580` in new-toc.c) is a Toccata-level `switch` on type — not reproducible in C.

**Agreed premise (take as given for now).** A `TermVal` can never be hashed, so it can never be a hash-map key. The C side therefore never has to hash one.

**Agreed fix.** Two new C functions, plus rewiring the BMI call sites off the global pointer:

1. **A simple `sha1` function** (`runtime3.c`, same signature as the pointer, modeled on `testingSha1` in `regression-tests/test-hash-map.c:101`):
   - I60 → `integerSha1` (concrete)
   - VAL of a type with a `hashVal` field → return the cached `hashVal`
   - anything else (e.g. `TermVal`) → BOOM (see the agreed premise above)

   Unlike `testingSha1`, String is *not* special-cased via `strSha1` — String keys rely on the cache too, which the setter (item 2) maintains. Sound only under the invariant: **a key sitting in a BMI has its `hashVal` populated.**

2. **A `hashVal` setter** — a small C function (`runtime3.c`) wrapped for Toccata (`hvm-core.toc`): writes a computed `hash` into the key term's `hashVal` field (type-conditional — only the six types that have the field; a pure field write, safe even if the key term is shared, since the value written is the deterministic hash of that term). Called from the Toccata-level assoc entry on **every k/v pair insertion** — today `bmiCopyAssoc` (hvm-core.toc:1041), as the first line before any branching; the same call goes into the ArrayNode/CollisionNode/mutate entries when they are implemented (see Big Picture section). One write at the entry covers every storage branch (add, clone, replace→collision/createNode), and the invariant then holds by induction: any key in a BMI was once an insertion key.

**Work items (both in scope; land together).**
- **(a) `runtime3.c`:** add the simple `sha1` function and the setter; rewire the C BMI call sites that hash *stored* keys to call the simple function instead of the global pointer: `addMutateBMI` (2123), `bmiReplaceCopied` (2186), `bmiReplaceMutate` (2217), `collisionAssoc` (2520, 2547). Only 2186 is reachable from the test binary today; the rest matter once the mutate/collision/array-node paths are wrapped.
- **(b) `hvm-core.toc`:** wrap the setter and call it at the top of `bmiCopyAssoc`, so the new key's `hashVal` is set before it is stored or later becomes `currKey`.

Call sites that hash *lookup/new* keys are a different category — the invariant doesn't cover them:
- `arrayNodeDissoc` (2706) re-hashes the lookup key even though its `hash` is already a parameter — use the parameter.
- `hashMapAssoc` (2809) hashes a new key at the C entry point, where the setter is never called — keep the pointer (legacy entry anyway; see Big Picture section).
- `hashMapGet` (2796) is commented out (the function aborts) — not an active site.

(Alternative considered: assign the simple function to the global pointer, as the C test does with `sha1 = testingSha1;` (test-hash-map.c:2657). That patches only the test binary; the real system's pointer is the generated Toccata-level dispatcher, so the BMI call sites must call the simple function directly.)

**The C-level test remains a valid regression guard for the rewire.** Its collision cases use String keys hashed via `strSha1`, which populates `hashVal` (it is itself a cache reader/writer, runtime3.c:1765), and I60 keys go through deterministic `integerSha1` — so the simple function returns identical values at the rewired sites (2186, 2217, 2520, 2547). (`testingSha1Collision*` and the `COLLIDE_KEY_*` I60 macros in `test-hash-map.c` are dead code — never assigned or used; the real tests use string keys.)

**Note (2026-08-17): drop `hashVal` from the node types.** Remove the `hashVal` field from `BitmapIndexedNode`, `ArrayNode`, and `HashCollisionNode` (runtime3.h:87, 95, 101). Hash-maps — whose root is one of these nodes — should not be usable as keys; this extends the agreed premise (above) from `TermVal` to the node types. Consequences for the agreed fix:
- The "six types with a `hashVal` field" shrink to three: `String`, `Vector`, `ReifiedVal`.
- Simple `sha1` (item 1): a node-type VAL falls into the BOOM case — a map/node key is rejected, not served from a cache.
- `set-hash-val` (item 2, hvm-core.toc:1041): drop the `BitmapIndexedType`/`ArrayNodeType`/`HashCollisionNodeType` cases; node-type keys are a no-op there and BOOM in `sha1` if they ever reach a map.
- `strSha1`'s `((String *)arg0)->hashVal` write for `SubStringType` (runtime3.c:1796) is unaffected — it relies on the `String`/`ReifiedVal` offset match, both of which keep the field.

**Next.** Do (a) + (b) together, rebuild, then re-run the remaining Group A cases (createNode first), then B/C. The C-level tests in `hash-map-plan-c.md` remain runnable via their `testingSha1` workaround.

## Files to Read

| File | Purpose |
|---|---|
| `hvm-core.toc` | Inline-C BMI wrappers + Toccata protocol implementations |
| `regression-tests/test-bmi.toc` | BMI-level regression tests |
| `regression-tests/hash-map-regressions.toc` | Full hash-map regression tests (ControlledHash pattern reference) |
| `regression-tests/regression-tester.toc` | `rt/test` assertion macro |
| `new.c` | Compiler — may need modifications for `{}` literal |
