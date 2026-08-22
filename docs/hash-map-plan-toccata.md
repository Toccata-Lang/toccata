# Hash-Map Plan — Toccata-Level Implementation and Testing

## Overview

The Toccata layer wraps the C-level bitmap-trie functions (see `hash-map-plan-c.md`) with inline-C wrappers in `hvm-core.toc` and implements the hash-map protocol methods in Toccata. Testing is exercised through `regression-tests/test-bmi.toc` (BMI node level) and, eventually, `regression-tests/hash-map-regressions.toc` (full map level).

**State (2026-08-22):** `bmiCopyAssoc` and `bmiMutateAssoc` are implemented at the Toccata level and fully branch-tested; both suites pass — `test-bmi` (27 cases: Group A copy-assoc + Group B mutate-assoc + `get`, `diff: 0`, `remaining nodes: 0`, `SAFETY=1`) and `test-hash-map` (55 tests, all green). Phase 1 (`get`) complete: `bmiGet` is wrapped (task 1.1), the `get*` protocol + `get` method for `BitmapIndexedNode` are in place (task 1.2), and `get` is exercised by tests 1.3–1.7 (flat hit, flat miss, sub-node hit, sub-node miss, collision-node hit). `HashCollisionNode` gained a `get*` method (task 1.7) so `bmiGet`'s sub-node recursion reaches collision children: Toccata `collisionGet` — a linear scan over `collisionCount`/`collisionKey`/`collisionVal` wrappers, `=` protocol with the entry key first (the `bmiCopyAssoc` convention; matches the commented integration tests' expectation that an `eq=Some` lookup key finds its own entry). Phase 2 (`dissoc`) in progress: `bmiDissoc` is wrapped (task 2.1) and the `dissoc*` protocol + `dissoc` method for `BitmapIndexedNode` are in place (task 2.2) — a miss returns the map unchanged, and `bmiDissoc`'s sub-node recursion now goes through `dissoc*` (type dispatch, the Toccata counterpart of C's `baseDissoc`); the `dissoc` method is exercised by tests 2.3–2.7 (remove from a flat node → `bmiShrink`; remove the last entry → `emptyBMI`; missing key — bit-not-set and key-mismatch — returns the map unchanged; remove a key inside a sub-node → child shrinks, parent cloned via `bmiUpdate`; remove the last key inside a sub-node that is the parent's only entry → the whole map collapses to `emptyBMI`). Phase 3 (`ArrayNode`) in progress: the Toccata-level `arrayNodeCopyAssoc` (task 3.5) and `arrayNodeMutateAssoc` (task 3.6) are implemented — both route the sub-node recurse through a protocol (`copyAssoc`/`assoc*` respectively, type dispatch) and install a fresh BMI child into an empty slot; `arrayNodeMutateAssoc` borrows the child (`arrayNodeMutateChild`, no `incRef`) and swaps the slot in place (`arrayNodeSetSlot`), so an exclusively-owned child is mutated in place and a shared one is copied. The `ArrayNode` protocol methods (`copyAssoc`/`assoc*`/`get*`/`count`/`vec`/`dissoc*`) land in task 3.7, and the ArrayNode test suite (`test-array-node`) is seeded (task 3.1) with its cases in 3.9–3.15.

## Big Picture: Node Types and the Toccata-Level Assoc Requirement

There are 3 hash-map node types, each with a copy-assoc and a mutate-assoc function in `runtime3.c`:

| Node type | C copy-assoc | C mutate-assoc |
|---|---|---|
| `BitmapIndexedNode` | `bmiCopyAssoc` (runtime3.c:2270) | `bmiMutateAssoc` (2297) |
| `ArrayNode` | `arrayNodeCopyAssoc` (2456) | `arrayNodeMutateAssoc` (2494) |
| `HashCollisionNode` | `collisionAssoc` (2514) | none — the `HashCollisionNodeType` case in `mutateAssoc` (2775) is commented out, so collision nodes fall back to `copyAssoc` |

The C-level dispatchers are `copyAssoc` (runtime3.c:2761) and `mutateAssoc` (2775), switching on `node->type`.

**Why the C versions can't be called from Toccata.** They perform equality checks internally via the C-level `equal` (runtime3.c:1875), which only handles I60 and String (`strCmp`, BOOM for anything else) and cannot dispatch to the `=` protocol function. Correct behavior for arbitrary key types (e.g. deftypes with custom `=`) requires the `=` protocol, which lives at the Toccata level — C cannot call up into it.

**Consequence.** The copy-assoc and mutate-assoc logic for all 3 node types must be implemented in Toccata, using the wrapped C functions for the mechanical parts (bitmap ops, node cloning, array manipulation) and the protocol `=`/`sha1` for the semantic parts. The Toccata-level `bmiCopyAssoc` (hvm-core.toc:1186) is the working example of this pattern: it does `(= currKey k)` in Toccata and delegates to `bitpos`, `bmiBitMap`, `bmiCopyChild`, `bmiKey`, `bmiVal`, `bmiClone`, `bmiUpdate`, `addCopiedBMI`, `bmiReplaceCopied`.

**Status of the 5 functions at the Toccata level:** `bmiCopyAssoc`, `bmiMutateAssoc`, `arrayNodeCopyAssoc`, and `arrayNodeMutateAssoc` are implemented (the BMI pair fully tested; the ArrayNode pair scratch-verified, with the `ArrayNode` protocol methods landing in task 3.7). `collisionAssoc` (copy) is not yet implemented in Toccata. Collision nodes need no mutate variant (copy path only, matching the C dispatcher).

## Protocol Functions NOT wired up yet

- `count` — returns count of sequence
- `sha1` — computes SHA1 hash
- `get` — polymorphic get dispatch
- `baseDissoc` — polymorphic dissoc dispatch
- `vals` — get all values

## BMI Surface (what's testable from Toccata today)

**Exposed in `hvm-core.toc`:** `emptyBMI`, `emptyArrayNode` (constructor — fresh empty ArrayNode per call, caller owns the ref), `arrayNodeCopyChild` (owned — `incRef`s the sub-node, `None` for an empty slot), `arrayNodeUpdate` (clone with slot `idx` replaced — the C `bmiUpdate` same-child no-op path), `arrayNodeCopyAssoc` (Toccata-level — sub-node slots recurse through the `copyAssoc` protocol; an empty slot gets a fresh BMI child), `arrayNodeMutateChild` (borrowed — no `incRef`, `None` for an empty slot — the `bmiMutateChild` pattern), `arrayNodeSetSlot` (in-place slot set — the C `arrayNodeMutateAssoc` `node->array[idx] = ...`), `arrayNodeMutateAssoc` (Toccata-level — sub-node slots recurse through the `assoc*` protocol; an empty slot gets a fresh BMI child), `bitpos`, `bmiBitMap`, `bmiKey`, `bmiVal`, `bmiCopyChild` (owned — `incRef`s the child), `bmiMutateChild` (borrowed — no `incRef`), `bmiClone`, `bmiUpdate`, `addCopiedBMI`, `bmiReplaceCopied`, `bmiCopyAssoc`, the mutate family (`bmiMutateAssoc`, `bmiReplaceMutate`, `bmiSetKV`, `bmiSetChild`), `bmiGet` (Toccata-level — sub-node slots recurse through the `get*` protocol), `bmiDissoc` (Toccata-level — sub-node slots recurse through the `dissoc*` protocol, the Toccata counterpart of C's `baseDissoc` dispatcher; `=` protocol with the lookup key first, per the C `bmiDissoc` and the commented integration tests' eq-gate expectation), `bmiSlotCount` (read-only — popcount of the bitmap), `bmiShrink` (copy with one slot removed — the C `bmiDissoc` flat-removal path), `collisionCount` (read-only — the C `collisionCount` consumes its arg), `collisionKey`/`collisionVal` (owned refs — entries are always non-NULL), `collisionGet` (Toccata-level — linear scan, `=` protocol, entry key first), `refs-count`, `set-hash-val`, `bmiCount`/`count`, `bmiHashVec`, `bmiVec`/`vec` (Toccata-level recursion — the C `bmiHashVec` aborts on sub-node slots via the dead `hashVec` dispatcher), `empty?`, the `copyAssoc`/`assoc*`/`get*`/`dissoc*` protocols, the `get` method for `BitmapIndexedNode` (sentinel default → `Some`/`None`), the `dissoc`/`dissoc*` methods for `BitmapIndexedNode` (miss returns the map unchanged), the `get*` method for `HashCollisionNode`

**Not exposed** (in `runtime3.c` only — can't test until wrapped): `mapGet` (the type dispatcher — protocol dispatch replaces it), `bmiDissoc`, `addMutateBMI` (the Toccata `bmiMutateAssoc` reuses `addCopiedBMI` for the empty-slot case), raw `bmiSetKey`/`bmiSetVal`

**Currently covered in test-bmi.toc:** 33 cases — Group A (`bmiCopyAssoc` integration, 10), Group B (`bmiMutateAssoc` integration, 9), `get` (7: flat hit, flat miss bit-not-set, flat miss key-mismatch, sub-node hit, sub-node miss bit-not-set, sub-node miss key-mismatch, collision-node hit), `dissoc` (6: flat-node remove, last-entry → `emptyBMI`, missing-key no-op bit-not-set, missing-key no-op key-mismatch, sub-node remove → child shrink + parent clone, sub-node emptied as the parent's only entry → collapse to `emptyBMI`), plus bitmap-of-empty. See the checklists below.

## BMI Tests (`regression-tests/test-bmi.toc`)

The C-level tests in `hash-map-plan-c.md` verify the raw functions. The cases below cover the Toccata protocol layer — the inline-C wrappers and the Toccata `bmiCopyAssoc`/`bmiMutateAssoc` implementations.

**Running the tests:** `make test-bmi` is the correct way to run this suite. `make` can be used for any Makefile target without thinking further — no need to run the underlying `./new-toc` + `clang` steps by hand. The full suite can be run in parallel with `make -j tests` — much faster than serial `make tests`.

### Prerequisite: controlled-hash key type

Add a `ControlledHash`-style deftype (same pattern as `hash-map-regressions.toc`) so tests can pin exact bit positions:

```
(deftype CHash [h eq]
  (= [x y] (and (= (type-num x) (type-num y)) eq (= h (.h y))))
  (sha1 [_] h))
```

Useful hashes: `3` (bit 3), `35` (bit 3 at shift 0, bit 1 at shift 5), `4` (bit 4), `67` (bit 3, then bit 2), `195` (bit 3, then bit 6 at shift 5), and `7` with `eq=None` vs `eq=Some` for full collisions. Note `=` is asymmetric by design (the `eq` gate is on the first arg): to make two same-hash keys unequal, the `eq=None` key must be the *new* key in the assoc.

### Test cases

Low-level functions (`bitpos`, `bmiBitMap`, `bmiKey`, `bmiVal`, `bmiCopyChild`, `bmiMutateChild`, `bmiClone`, `bmiUpdate`, `addCopiedBMI`, `bmiReplaceCopied`) are not tested standalone right now — they are covered via the higher-level tests below.

**A. `bmiCopyAssoc` integration**
- [x] *(existing)* add to empty
- [x] *(existing)* same key + same value → no-op, count 1
- [x] same key, **different** value → count 1, `vec == [[k v2]]` (bmiClone branch)
- [x] different key, different bit → count 2 (add branch)
- [x] different key, same bit, different hash at shift 5 (3 vs 35) → count 2, sub-node (replace→createNode branch)
- [x] different key, identical hash (7/7 collision) → count 2, collision node (replace→collision branch)
- [x] **recurse-into-child branch:** node with keys 3+35, then assoc key 67 (bit 3 again) → count 3
- [x] **child-recurse + add at depth 1:** keys 3, 35, 67, then 195 (bit 3 again → bit 6 at shift 5, empty slot in the child) → count 4
- [x] same key, different value, **key located inside a sub-node** (depth 2) → count unchanged, `vec` shows updated value (child-recurse + clone at depth)
- [x] same key, same value, **key located inside a sub-node** (depth 2) → count and `vec` unchanged (child-recurse no-op → `bmiUpdate` same-child path)

Note: `createNode`'s same-bit-at-next-level recursion is covered only via the mutate path (Group B, 3 vs 1027) — it is shared C code, but the copy path (`bmiReplaceCopied`) never drives it.

**B. `bmiMutateAssoc` integration** (reached via `assoc*`, which routes here when `refs-count m == 3`)
- [x] same key, different value → `bmiSetKV` in place, count 1, `vec` updated
- [x] same key, **same** value → `bmiSetKV` still runs (the mutate path has no no-op branch), count 1, `vec` unchanged
- [x] different key, different bit → `addCopiedBMI`, count 2
- [x] different key, same bit, different hash at shift 5 (3 vs 35) → `bmiReplaceMutate` (createNode), count 2, `vec` flattened
- [x] different key, same bit at shifts 0 and 5, different at 10 (3 vs 1027) → `bmiReplaceMutate` (createNode recursion — nested sub-node), count 2, `vec` flattened
- [x] different key, identical hash (7/7 collision) → `bmiReplaceMutate` (collision node), count 2
- [x] **recurse-into-child, exclusive child** → child rebuilt with the new key, count 3, `vec` flattened
- [x] **recurse-into-child, same key inside sub-node** → `bmiSetKV` at depth, count unchanged, `vec` updated
- [x] **shared child** (sub-node also held by a second map) → child's refs-count ≠ 3, so `assoc*` copies instead of mutating; the mutated map gains the key, the other map is untouched (copy fallback — see Lessons)

**C. `count` / `vec` recursion (untested beyond count-1)**
- [ ] flat 3-entry node → count 3
- [ ] mixed node (2 flat + sub-node of 2) → count 4
- [ ] `vec` of flat node → `[[k1 v1] [k2 v2] ...]` in bit order
- [ ] `vec` of node with sub-node → sub-node's pairs **flattened in** (not nested)

**D. `empty?` (untested)**
- [ ] `(empty? emptyBMI) == (Some emptyBMI)`
- [ ] `(empty? one-entry-node) == None`

**E. Memory hygiene (implicit, per status.md failure conditions)**
Every case above must end with `diff: 0` and `remaining nodes: 0` (see Lessons for what the counters actually measure). Highest-risk paths, all now exercised: owned refs from `bmiKey`/`bmiVal`/`bmiCopyChild` (the `test-bmi` double-free area), the no-op branch's `dec_and_free` dance, `bmiUpdate` same-child path, collision-node `incRef`s, `bmiSetKV`'s double-release of the old slot's key/val, and `bmiSetChild`'s slot swap in both the in-place and copy-fallback child cases.

**When a memory leak or double-free is found** (non-zero `diff:`, `refs too small` abort, or end-of-test leak), follow the `skills/memory-leak-hunting.md` skill — it covers the shrink-to-minimal-expression workflow, refcount trajectory logging, the lldb workflow, and the known leak/double-free patterns.

### Out of scope until more is exposed

- `get` / `dissoc` tests — belong in `hash-map-regressions.toc` once `bmiGet`/`bmiDissoc` are wrapped
- 16+ entry ArrayNode conversion — the `BOOM("wtf")` guard in copied `addCopiedBMI` has been removed; the C-level branch is exercised and fixed (task 3.2, `testBmiCopyAssocPromote`); the Toccata-level exercise is Roadmap 3.8
- `bmiKey`/`bmiVal` on a sub-node slot — aborts: the slot's key is `0` and the wrapper unconditionally `incRef`s it (`incRef` aborts on NULL, `runtime3.c:808`). Needs a `None` return for sub-node slots before it can be tested
- shift-exhaustion (`shift > 60` abort) — deliberate crash, don't test

## Roadmap: Remaining Toccata-Level Work (Ralph loop tasks)

The BMI assoc side is done (Groups A/B, fully branch-tested). What remains is the rest of the Toccata-level hash-map functionality. This section is the task list for the Ralph loop: **each iteration implements exactly one unchecked task, in list order, and nothing else.** Tasks are dependency-ordered — do not skip ahead. The commented-out tests in `regression-tests/hash-map-regressions.toc` are a reference for what the full-map functionality should do — but do not run or modify that suite: it is the final integration test, run by the owner after this roadmap is done. This roadmap supersedes the "Out of scope until more is exposed" list as those items get unblocked.

### Conventions (apply to every task)

1. First check `git status`: if a previously interrupted iteration left uncommitted changes, either finish that task properly or revert them before starting.
2. Before wrapping anything, read the C reference function in `runtime3.c` and the existing wrappers in `hvm-core.toc`. The pattern to mirror is the `bmiCopyAssoc`/`bmiMutateAssoc` pair: C for the mechanical parts, protocol `=`/`sha1` for the semantic parts, owned refs from wrappers (see Lessons).
3. Verify after each change — every Toccata suite you touched or added (`make test-bmi`, `make test-array-node`, `make test-collision-node`, as applicable):
   - the suite's success line is printed (every `rt/test` passed)
   - `diff: 0` and `remaining nodes: 0`
   - the suite's `.rslt` diff shows only execution-stat changes
   - touched `hvm-core.toc` → `make -j tests` (the full suite, in parallel — much faster; no other suite may regress)
   - touched `runtime3.c` → additionally `make test-hash-map` (the 55 C tests)
     - **Shuffle-stability (run the binary 1000×, don't recompile):** `test-hash-map` runs its tests in a random order every run (Fisher-Yates seeded by `time ^ pid`), and its `check_counts` expectations are pool-sensitive (e.g. `testBmiCopyAssocPromote` `(0, 0)`, `testBmiMutateAssocPromote` `(120, 0)`), so a single run only validates one order. After any change that affects this suite (`runtime3.c` or `regression-tests/test-hash-map.c`), re-run the already-built binary 1000 times to confirm the counts stay order-independent:
       `for i in $(seq 1 1000); do ./regression-tests/test-hash-map >/dev/null 2>&1 || { echo "FAIL at $i"; break; }; done`
       Completes with no `FAIL` line = 1000/1000 green (verified 2026-08-22); a `FAIL at N` means iteration N aborted — re-run `./regression-tests/test-hash-map` once to see the message.

   **Known pre-existing failure (deferred, do not chase):** `test-fusing` leaks — `remaining nodes: 239 (656)` on stdout and `Leaked pairs!! 239` on stderr. This is present in the committed baseline, is unrelated to the hash-map work, and is deferred until later. When running `make -j tests`, ignore the `test-fusing` leak; every other suite must still be clean (`diff: 0`, `remaining nodes: 0`).
4. Memory error → follow `skills/memory-leak-hunting.md`. "Memory leak" includes double frees: a leak shows as non-zero `diff:` or `remaining nodes`; a double free shows as an abort from `dec_and_free` ("failure in decRefs, refs too small").
5. Compiler changes (`new.c`, codegen, rebuilding `toccata`/`new-toc`) are out of scope — respond STUCK. `runtime3.c` changes are allowed only when a task explicitly says so (or for a leak fix).
6. Mark the task `[x]`, update the BMI Surface / State lines if the exposed surface changed, and commit staging only the files you changed. Message: `task N: <short name> — <what/why>`.

### Phase 1 — `get`

- [x] **1.1** Wrap `bmiGet` (C ref `runtime3.c:2368` — `(node, key, def, hash, shift)`, returns `def` on miss) in `hvm-core.toc`. C's `mapGet` (2401) is the type dispatcher; at the Toccata level protocol dispatch replaces it, so wrap only the BMI variant for now.
- [x] **1.2** Add a `get*` protocol and a `get` method for `BitmapIndexedNode`, mirroring the `assoc*`/`copyAssoc` pattern: `get* [m k def hash shift]` → the wrapper; `get [m k]` → computes `(sha1 k)`, calls `get*` with a sentinel default, and maps the result to `Some`/`None` per the commented tests' usage (`(= (Some "a") (get ...))`).
- [x] **1.3** `test-bmi.toc`: get test — hit (flat).
- [x] **1.4** `test-bmi.toc`: get test — miss (returns `def`).
- [x] **1.5** `test-bmi.toc`: get test — hit inside a sub-node (depth 2).
- [x] **1.6** `test-bmi.toc`: get test — miss inside a sub-node.
- [x] **1.7** `test-bmi.toc`: get test — hit inside a collision node.

### Phase 2 — `dissoc`

- [x] **2.1** Wrap `bmiDissoc` (C ref `runtime3.c:2431` — `(node, key, hash, shift)`; returns `emptyBMI` when the last entry is removed; watch the sub-node shrink/clone paths).
- [x] **2.2** Add a `dissoc*` protocol and a `dissoc` method for `BitmapIndexedNode` (same pattern as 1.2; no default value — a miss returns the map unchanged).
- [x] **2.3** `test-bmi.toc`: dissoc test — remove from a flat node.
- [x] **2.4** `test-bmi.toc`: dissoc test — remove the last entry (→ `emptyBMI`).
- [x] **2.5** `test-bmi.toc`: dissoc test — remove a missing key (no-op).
- [x] **2.6** `test-bmi.toc`: dissoc test — remove a key inside a sub-node (shrink).
- [x] **2.7** `test-bmi.toc`: dissoc test — remove so a sub-node becomes empty and is the parent's only entry.

### Phase 3 — ArrayNode (16+ entries)

ArrayNode tests live in their own suite — `test-bmi.toc` is for BitmapIndexedNodes only. The suite covers both directly-built (3.4) and promoted (3.8) ArrayNodes.

- [x] **3.1** Create the ArrayNode test suite: `regression-tests/test-array-node.toc` (copy the scaffolding from `test-bmi.toc` — `rt` import, `CHash`/`Key1`/`Val` deftypes, main + success-println shape) and add `test-array-node` to `REG_TESTS` in the Makefile (the existing pattern rules build and run it exactly like `test-bmi`). Seed it with one trivially-green test and commit the baseline `.rslt`.
- [x] **3.2** C: 16+ promotion branch of `addCopiedBMI` — the `BOOM("wtf")` calls have been removed by the owner; the conversion code is now live and `make test-hash-map` passes, but the branch has never been exercised (the removed TODO flagged doubt about the `cloneBitmapIndexedNode(&emptyBMI, ...)` calls, and the C suite only tests mutate-side promotion via `addMutateBMI`). The first real exercise is 3.8 — if it exposes a bug in this branch, fixing it is in scope for that task. Done: `testBmiCopyAssocPromote` (C suite, 55 tests) drives `bmiCopyAssoc`'s empty-slot path on a full 16-slot node; it exposed the doubt for real — `cloneBitmapIndexedNode(&emptyBMI, ...)` sees `emptyBMI.bitmap == 0`, builds an empty sub-node, and silently drops the key/val (data loss + ref leaks). Fixed by building the sub-nodes like `addMutateBMI` does (`malloc_bmiNode(1)` + `bitpos(hash, shift+5)`), with copy-path refcounts (the old node is freed without clearing its slots, so each moved pair takes fresh refs). The size-1/size-16 BMI pools are prewarmed in the C suite's `main` so both promotion tests' `check_counts` stay order-independent under the shuffle (`testBmiMutateAssocPromote` 140→120).
- [x] **3.3** Make the Toccata mutate path promotable — decided: **reuse the fixed `addCopiedBMI`** in `bmiMutateAssoc`'s empty-slot case (no `addMutateBMI` wrapper). Reading the C: `addCopiedBMI` and `addMutateBMI` are identical except in the 16+ promotion branch, where `addCopiedBMI` incRefs existing entries (copy) and `addMutateBMI` moves them (no incRef, old slots cleared); both yield the same final ArrayNode (one ref per entry, old node freed), so `addCopiedBMI` is correct for an exclusively-owned node — its extra incRefs balance against the old node's release. The empty-slot case already called `addCopiedBMI` (now fixed by 3.2), so no code change was needed beyond documenting the decision in `bmiMutateAssoc`'s comment. Verified with a scratch build: a 16-entry BMI built via `assoc*` (the mutate path) promotes to an ArrayNode (type 12) on the 17th entry, `diff: 0`, `remaining nodes: 0`.
- [x] **3.4** Wrap a constructor for an empty `ArrayNode` (see how C's `testArrayNode` in `test-hash-map.c` allocates one) so tests can build ArrayNodes directly, not only via promotion. Done: `emptyArrayNode` in `hvm-core.toc` — a zero-arg defn whose inline C is `malloc_arrayNode()` (runtime3.c:650, declared in runtime3.h:376), the exact allocation C's `testArrayNode` uses. Returns a fresh node per call (all 32 slots 0, `refs = refsInit` — caller owns the ref; a `def`-inline would be a single shared global, so it must be a `defn`). Verified with a scratch build: type 12 (`ArrayNodeType`), two calls yield distinct pointers, `diff: 0`, `remaining nodes: 0`.
- [x] **3.5** Toccata `arrayNodeCopyAssoc` mirroring C `runtime3.c:2488` (protocol `=`/`sha1` for the semantic parts). Done: `arrayNodeCopyAssoc` in `hvm-core.toc` — the slot index is `mask(hash, shift) = (hash >> shift) & 31` (computed in Toccata via `bit-shift-right`/`bit-and`); the mechanical slot ops are two new wrappers, `arrayNodeCopyChild` (read slot `idx`, owned ref via `incRef`, `None` for an empty slot — the `bmiCopyChild` pattern) and `arrayNodeUpdate` (clone with slot `idx` replaced, the C `bmiUpdate` same-child no-op path). The semantic part is the sub-node recurse through the `copyAssoc` protocol (type dispatch — C's internal `equal` can't reach the `=` protocol); an empty slot gets a fresh BMI child (`copyAssoc` on `emptyBMI` at `shift+5`), mirroring C. No `=`/`sha1` at this level — the C `arrayNodeCopyAssoc` does no key equality (it routes by index; equality happens in the child's own `copyAssoc`); `set-hash-val` caches the hash on the key, per the `bmiCopyAssoc` convention. Verified with a scratch build (17-entry promoted ArrayNode): empty-slot assoc, occupied-slot assoc, no-op, and update all give the right counts with `diff: 0` and `remaining nodes: 0`; `make -j tests` clean (all `.rslt` byte-identical to baseline).
- [x] **3.6** Toccata `arrayNodeMutateAssoc` mirroring C `runtime3.c:2526`. Done: `arrayNodeMutateAssoc` in `hvm-core.toc` — the slot index is `mask(hash, shift) = (hash >> shift) & 31` (computed in Toccata via `bit-shift-right`/`bit-and`, the `arrayNodeCopyAssoc` convention). The mechanical slot ops are two new wrappers: `arrayNodeMutateChild` (read slot `idx`, **borrowed** — no `incRef`, `None` for an empty slot — the `bmiMutateChild` pattern; the ref the caller receives IS the slot's ref) and `arrayNodeSetSlot` (point slot `idx` at `child` in place — the C `node->array[idx] = ...`, no release of the old slot ref; transfers `m`'s ref and takes `child`'s). The semantic part is the sub-node recurse through the `assoc*` protocol (type dispatch — the Toccata counterpart of C's `mutateAssoc` dispatcher): the borrowed child is mutated in place when exclusively owned (`refs-count == 3`) and copied otherwise; either way `arrayNodeSetSlot` points the slot at the (re)built child — in the copy case the swap balances, since the copy path consumed the old child's slot ref. An empty slot gets a fresh BMI child (`copyAssoc` on `emptyBMI` at `shift+5`), mirroring C. The C-level `refs != 1` fallback is handled by the `assoc*` dispatch instead (task 3.7), per the `bmiMutateAssoc` convention. `set-hash-val` caches the hash on the key. Verified with a scratch build: empty-slot install, occupied-slot in-place child mutation, same-key update inside the child, and the shared-child copy fallback (a second map sharing the child is left untouched) all give the right counts/vecs with `diff: 0` and `remaining nodes: 0`; `make test-bmi`/`make test-array-node`/`make -j tests` clean (all `.rslt` byte-identical to baseline). Note: the shared-child scratch case needed a dataflow thread (a `seq2` helper whose result depends on `refs-count` of the copy) to force the copy before the mutation — independent let bindings are forced in no guaranteed order, and the unused-binding form of the thread does not reliably force.
- [ ] **3.7** Wire the `ArrayNode` protocol methods: `copyAssoc`/`assoc*` (→ 3.5/3.6), plus wrappers + methods for `arrayNodeGet` (2589), `arrayNodeCount` (2607), `arrayNodeVec` (2709), `arrayNodeDissoc` (2727).
- [ ] **3.8** `test-bmi.toc`: the promotion test — assoc a 17th entry into a BMI map (BMI→ArrayNode), count 17.
- [ ] **3.9** `test-array-node.toc`: count test.
- [ ] **3.10** `test-array-node.toc`: get test — hit.
- [ ] **3.11** `test-array-node.toc`: get test — miss.
- [ ] **3.12** `test-array-node.toc`: vec test.
- [ ] **3.13** `test-array-node.toc`: assoc test — into an empty slot.
- [ ] **3.14** `test-array-node.toc`: assoc test — into a sub-node slot.
- [ ] **3.15** `test-array-node.toc`: dissoc test.

### Phase 4 — `collisionAssoc` + collision children

CollisionNode tests live in their own suite — `test-bmi.toc` is for BitmapIndexedNodes only.

- [ ] **4.1** Create the CollisionNode test suite: `regression-tests/test-collision-node.toc` (same scaffolding as 3.1) and add `test-collision-node` to `REG_TESTS` in the Makefile. Seed with one trivially-green test and commit the baseline `.rslt`.
- [ ] **4.2** Wrap a constructor for an empty `HashCollisionNode` (see how C's `testCollisionNode` in `test-hash-map.c` allocates one) so tests can build collision nodes directly.
- [ ] **4.3** Toccata `collisionAssoc` mirroring C `runtime3.c:2546` (add, update, promote to BMI on hash mismatch).
- [ ] **4.4** Wire `copyAssoc`/`assoc*` for `HashCollisionNode` — this also fixes the known BOOM when `assoc*` recurses into a collision-node child.
- [ ] **4.5** `test-collision-node.toc`: add test — add a key to a collision node.
- [ ] **4.6** `test-collision-node.toc`: update test — update an existing key.
- [ ] **4.7** `test-collision-node.toc`: promote test — promote to BMI on hash mismatch.
- [ ] **4.8** `test-collision-node.toc`: count test.
- [ ] **4.9** `test-collision-node.toc`: vec test.
- [ ] **4.10** `test-collision-node.toc`: get test.
- [ ] **4.11** `test-collision-node.toc`: dissoc test.
- [ ] **4.12** `test-bmi.toc`: the BMI-side integration test — `assoc*` into a map whose child is a collision node.

### Phase 5 — `hash-seq`/`vals` + stress

- [ ] **5.1** Wire `vals`/`hash-seq` per the commented tests' usage in `hash-map-regressions.toc` (`(hash-seq full-bmi [""])` — read them to pin the exact protocol shape; the Toccata `bmiVec-bm` recursion is the working pattern for descending into sub-nodes; the C `hashVec` dispatcher is dead). No tests in this step — those are 5.2–5.4.
- [ ] **5.2** `test-bmi.toc`: `hash-seq`/`vals` test — a 3-entry BMI.
- [ ] **5.3** `test-array-node.toc`: `hash-seq`/`vals` test — an ArrayNode.
- [ ] **5.4** `test-collision-node.toc`: `hash-seq`/`vals` test — a collision node.
- [ ] **5.5** `test-array-node.toc`: stress test — 27-entry map built by `reduce` over a range (per the commented tests' shape), assert count (promotes to ArrayNode).
- [ ] **5.6** `test-array-node.toc`: stress test — 2001-entry map built by `reduce` over a range (per the commented tests' shape), assert count (promotes to ArrayNode).

### Phase 6 — map-level `assoc` (last)

- [ ] **6.1** Add an `assoc` protocol for `HashMap`: `(assoc m k v)` → `(assoc* m k v (sha1 k) 0)` (the `refs-count` dispatch inside `assoc*` handles mutate-vs-copy). Do not add tests for this — the owner will handle them later.
- [ ] **6.2** Final: `make tests` fully green (including `test-bmi`, `test-array-node`, `test-collision-node`); update the State line; mark this roadmap complete. `hash-map-regressions.toc` (the integration suite) is intentionally left untouched for the owner.

## Lessons (from the mutate-assoc work, Aug 2026)

- **Owned vs borrowed child wrappers.** `bmiChild` was split into `bmiCopyChild` (incRefs — the caller owns a ref) and `bmiMutateChild` (borrowed — no incRef). The original mutate-path leak (`diff: 7`) came from the child-recurse always copying the child while `bmiSetChild` swapped the slot without releasing the old child's slot ref. The fix routes the recurse through `assoc*` on the borrowed child: an exclusively-owned child is mutated in place (no slot swap), a shared child is copied (the swap balances — verified by the shared-child test).
- **Don't hand-trace refcounts; let the counters decide.** The compiler gives every source-level appearance of a value its own ref, scheduled by code position (implementation-notes §12). A static trace predicted a leak in the shared-child copy fallback that the test disproved (`remaining nodes: 0`). Write the test; `CHECK_MEM_LEAK` + `SAFETY=1` are the oracle.
- **What the counters measure.** `malloc_count` counts `my_malloc` pool refills (50/300-struct batches with free lists) and saturates at the high-water mark — it stays ~constant across workloads and is not a workload signal. The leak signals are `diff` (checked-out values never returned) and `remaining nodes: 0 (N)`.
- **`assoc*`'s magic number 3.** `refs-count == 3` means the caller holds exactly one ref to the node (1 external + 2 dupe-refs for the node's other two appearances in `assoc*`'s body) — i.e. safe to mutate in place.
- **The mutate path has no no-op branch.** Unlike `bmiCopyAssoc`, `bmiMutateAssoc` runs `bmiSetKV` even for same-key-same-value (slot reset, old contents freed).
- **`vec` element order is implementation-dependent** — noted at the top of `test-bmi.toc`; an order change is not an error, just update the tests.

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
