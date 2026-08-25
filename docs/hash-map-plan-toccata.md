# Hash-Map Plan — Toccata-Level Implementation and Testing

## Overview

The Toccata layer wraps the C-level bitmap-trie functions (see `hash-map-plan-c.md`) with inline-C wrappers in `hvm-core.toc` and implements the hash-map protocol methods in Toccata. Testing is exercised through `regression-tests/test-bmi.toc` (BMI node level), `regression-tests/test-array-node.toc` (ArrayNode level), `regression-tests/test-collision-node.toc` (HashCollisionNode level), and `regression-tests/hash-map-regressions.toc` (full map level — the owner's integration suite).

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

## ControlledHash — the integration suite's controlled key type

`hash-map-regressions.toc` (the owner's integration suite) defines its own controlled key type, distinct from the `CHash` used by the node-level suites (`test-bmi`/`test-array-node`/`test-collision-node`):

```
(deftype ControlledHash [y eq]
  (str-vect [_] (str-vect y))
  (= [_ v] (and (= eq (.eq v)) (Some _)))
  (sha1 [_] y))
```

**Purpose — two independent knobs for collision testing.** `y` is the hash: `sha1` returns it, so a test pins exactly which slot/bit a key lands in. `eq` is the equality discriminator: two keys may share a `y` (same hash → same slot) yet carry different `eq` values, so they fail `=` and form a genuine collision node instead of being treated as one key. This is what lets the suite force N keys into one hash bucket and decide, per key, "same key" vs "distinct colliding key."

**`=` semantics (revised 2026-08-24).** Equality is decided *solely* by the `eq` slot — structural equality via the `=` protocol, which dispatches on the eq value's own type (the suite uses strings `"a"`/`"b"` and integers `1`/`2`/`3` as discriminators) — independent of `y`:
- same `y`, different `eq` → not equal → collision
- same `y`, same `eq` → equal → same key
- **Invariant when writing tests:** `=` ignores `y` entirely, so two keys with the *same `eq` but different `y`* compare equal. Treat `eq` as the key's true identity (same `eq` ⇒ same key ⇒ same hash).

**Why the earlier `=` was wrong.** It was `(and eq (= y (.y v)) (Some _))` — an on/off *gate*, not a discriminator. A key with `eq=Some` passed the gate and then matched *any* same-`y` key (the other key's `eq` was never inspected), so two same-hash keys could not be made unequal unless one carried `eq=None`; and an `eq=None` key matched *nothing* — not even a structurally identical key. The `eq` slot did not discriminate; it only gated. The revision makes `eq` the actual equality value.

**Also (2026-08-24):** the suite's obsolete GC-probe expressions `(inc <literal>)` were replaced with the computed literal integers; only `(inc n)` over the `reduce` variable remains.

## BMI Surface (what's testable from Toccata today)

**Exposed in `hvm-core.toc`:** `emptyBMI`, `emptyArrayNode` (constructor — fresh empty ArrayNode per call, caller owns the ref), `emptyCollisionNode` (constructor — fresh empty HashCollisionNode per call, caller owns the ref; `count` field is dual-use so `collisionCount` reports 2, not 0, for a fresh node), `arrayNodeCopyChild` (owned — `incRef`s the sub-node, `None` for an empty slot), `arrayNodeUpdate` (clone with slot `idx` replaced — the C `bmiUpdate` same-child no-op path), `arrayNodeCopyAssoc` (Toccata-level — sub-node slots recurse through the `copyAssoc` protocol; an empty slot gets a fresh BMI child), `arrayNodeMutateChild` (borrowed — no `incRef`, `None` for an empty slot — the `bmiMutateChild` pattern), `arrayNodeSetSlot` (in-place slot set — the C `arrayNodeMutateAssoc` `node->array[idx] = ...`), `arrayNodeMutateAssoc` (Toccata-level — sub-node slots recurse through the `assoc*` protocol; an empty slot gets a fresh BMI child), `arrayNodeCount` (read-only — the C `arrayNodeCount` consumes its arg; sums `mapCount` over the non-empty slots), `arrayNodeGet` (Toccata-level — occupied slots recurse through the `get*` protocol), `arrayNodeDissoc` (Toccata-level — occupied slots recurse through the `dissoc*` protocol; an emptied child leaves its slot pointing at `emptyBMI`), `arrayNodeVec` (Toccata-level recursion — the C `arrayNodeVec` aborts via the dead `hashVec` dispatcher), `bitpos`, `bmiBitMap`, `bmiKey`, `bmiVal`, `bmiCopyChild` (owned — `incRef`s the child), `bmiMutateChild` (borrowed — no `incRef`), `bmiClone`, `bmiUpdate`, `addCopiedBMI`, `bmiReplaceCopied`, `bmiCopyAssoc`, the mutate family (`bmiMutateAssoc`, `bmiReplaceMutate`, `bmiSetKV`, `bmiSetChild`), `bmiGet` (Toccata-level — sub-node slots recurse through the `get*` protocol), `bmiDissoc` (Toccata-level — sub-node slots recurse through the `dissoc*` protocol, the Toccata counterpart of C's `baseDissoc` dispatcher; `=` protocol with the lookup key first, per the C `bmiDissoc` and the commented integration tests' eq-gate expectation), `bmiSlotCount` (read-only — popcount of the bitmap), `bmiShrink` (copy with one slot removed — the C `bmiDissoc` flat-removal path), `collisionCount` (read-only — the C `collisionCount` consumes its arg), `collisionKey`/`collisionVal` (owned refs — entries are always non-NULL), `collisionGet` (Toccata-level — linear scan, `=` protocol, entry key first), `collisionUpdate` (clone with entry `i` replaced by (k v) — the C `collisionAssoc` update outcome, count unchanged), `collisionAppend` (clone with (k v) appended — the C `collisionAssoc` append outcome, count +1), `collisionShrink` (clone with entry `i` removed — the C `collisionDissoc` multi-entry hit path, surviving entries compacted), `collisionVec` (wraps the C `collisionVec` directly — alive and purely mechanical, no key equality; conjs (k v) pairs in entry order, NULL-guarded; consumes the node and the initial vec), `collisionAssoc` (Toccata-level — hash match → `collisionUpdate`/`collisionAppend`; hash mismatch → promote to a BMI via `copyAssoc` on `emptyBMI` + an `assoc*` fold of the existing entries), `collisionDissoc` (Toccata-level — `=` protocol with the lookup key first, per the C `collisionDissoc`/`bmiDissoc`; last entry → `emptyBMI`, miss → node unchanged, multi-entry hit → `collisionShrink`), `refs-count`, `set-hash-val`, `bmiCount`/`count`, `bmiHashVec`, `bmiVec`/`vec` (Toccata-level recursion — the C `bmiHashVec` aborts on sub-node slots via the dead `hashVec` dispatcher), `bmiHashSeq`/`arrayNodeHashSeq` (Toccata-level `hash-seq` — the seed vector is threaded through the recursion, each slot's pair(s) conjs'd in slot order; the sub-node recurse goes through the `hash-seq` protocol, type dispatch — the C `bmiHashSeq`/`arrayNodeSeq` route through the dead `hashSeq` dispatcher), `pair-val` (the value of a [k v] pair — `extract (get pair 1)`; a destructuring lambda does not codegen), `empty?`, the `copyAssoc`/`assoc*`/`get*`/`dissoc*`/`hash-seq` protocols, the `get` method for `BitmapIndexedNode` (sentinel default → `Some`/`None`), the `dissoc`/`dissoc*` methods for `BitmapIndexedNode` (miss returns the map unchanged), the `hash-seq`/`vals` methods for all three node types (`hash-seq` → the Toccata `bmiHashSeq`/`arrayNodeHashSeq`, or `collisionVec` for a collision node; `vals` → `map` over `hash-seq m []` extracting the pair value via `pair-val`), the `copyAssoc`/`assoc*`/`get*`/`vec`/`dissoc*` methods for `HashCollisionNode` (`copyAssoc`/`assoc*` both route to the Toccata `collisionAssoc` — C's `mutateAssoc` has no collision case, so `assoc*` always copies; `vec` → `collisionVec` over the C `collisionVec`; `dissoc*` → the Toccata `collisionDissoc`), the `copyAssoc`/`assoc*`/`get*`/`count`/`vec`/`dissoc*` methods for `ArrayNode`, the `assoc` method for `HashMap` (the map-level `assoc` — `(assoc m k v)` → `(assoc* m k v (sha1 k) 0)`; `extend-type HashMap` registers it for all three node types — types 11/12/13 all map to the name "HashMap" in the codegen typeNameMap, the `empty?` pattern; the `refs-count` dispatch inside `assoc*` handles mutate-vs-copy — verified the mutate path fires when the map is used exactly once)

**Not exposed** (in `runtime3.c` only — can't test until wrapped): `mapGet` (the type dispatcher — protocol dispatch replaces it), `addMutateBMI` (the Toccata `bmiMutateAssoc` reuses `addCopiedBMI` for the empty-slot case), raw `bmiSetKey`/`bmiSetVal`

## Testing

The C-level tests in `hash-map-plan-c.md` verify the raw functions; the regression suites cover the Toccata protocol layer — the inline-C wrappers and the Toccata-level protocol implementations. Low-level functions (`bitpos`, `bmiBitMap`, `bmiKey`, `bmiVal`, `bmiCopyChild`, `bmiMutateChild`, `bmiClone`, `bmiUpdate`, `addCopiedBMI`, `bmiReplaceCopied`) are not tested standalone — they are covered via the higher-level tests.

**Running the tests:** `make test-bmi`, `make test-array-node`, `make test-collision-node`, and `make hash-map-regressions` are the correct way to run the suites. `make` can be used for any Makefile target without thinking further — no need to run the underlying `./new-toc` + `clang` steps by hand. The full suite can be run in parallel with `make -j tests` — much faster than serial `make tests`.

### Verifying a change

- Every Toccata suite you touched or added: the suite's success line is printed (every `rt/test` passed), `diff: 0` and `remaining nodes: 0`, and the suite's `.rslt` diff shows only execution-stat changes.
- Touched `hvm-core.toc` → `make -j tests` (the full suite, in parallel — much faster; no other suite may regress).
- Touched `runtime3.c` → additionally `make test-hash-map` (the 55 C tests) and the shuffle-stability check below.

**Shuffle-stability (run the binary 1000×, don't recompile):** `test-hash-map` runs its tests in a random order every run (Fisher-Yates seeded by `time ^ pid`), and its `check_counts` expectations are pool-sensitive (e.g. `testBmiCopyAssocPromote` `(0, 0)`, `testBmiMutateAssocPromote` `(120, 0)`), so a single run only validates one order. After any change that affects this suite (`runtime3.c` or `regression-tests/test-hash-map.c`), re-run the already-built binary 1000 times to confirm the counts stay order-independent:
`for i in $(seq 1 1000); do ./regression-tests/test-hash-map >/dev/null 2>&1 || { echo "FAIL at $i"; break; }; done`
Completes with no `FAIL` line = 1000/1000 green; a `FAIL at N` means iteration N aborted — re-run `./regression-tests/test-hash-map` once to see the message.

**Known pre-existing failure (deferred, do not chase):** `test-fusing` leaks — `remaining nodes: 239 (656)` on stdout and `Leaked pairs!! 239` on stderr. This is present in the committed baseline, is unrelated to the hash-map work, and is deferred until later. When running `make -j tests`, ignore the `test-fusing` leak; every other suite must still be clean (`diff: 0`, `remaining nodes: 0`).

**Known flaky compiler:** `new-toc` has intermittently segfaulted during codegen (~1/10 runs, reproduced on the unmodified baseline; see implementation-notes.md §10) — retry the `.c` generation until it succeeds.

### Memory hygiene

Every test case must end with `diff: 0` and `remaining nodes: 0` (see Lessons for what the counters actually measure).

**When a memory leak or double-free is found** (non-zero `diff:`, `refs too small` abort, or end-of-test leak), follow the `skills/memory-leak-hunting.md` skill — it covers the shrink-to-minimal-expression workflow, refcount trajectory logging, the lldb workflow, and the known leak/double-free patterns. "Memory leak" includes double frees: a leak shows as non-zero `diff:` or `remaining nodes`; a double free shows as an abort from `dec_and_free` ("failure in decRefs, refs too small").

### Controlled-hash key type for the node-level suites

The node-level suites (`test-bmi`/`test-array-node`/`test-collision-node`) use a `ControlledHash`-style deftype (same pattern as `hash-map-regressions.toc`) so tests can pin exact bit positions:

```
(deftype CHash [h eq]
  (= [x y] (and (= (type-num x) (type-num y)) eq (= h (.h y))))
  (sha1 [_] h))
```

Useful hashes: `3` (bit 3), `35` (bit 3 at shift 0, bit 1 at shift 5), `4` (bit 4), `67` (bit 3, then bit 2), `195` (bit 3, then bit 6 at shift 5), and `7` with `eq=None` vs `eq=Some` for full collisions. Note `=` is asymmetric by design (the `eq` gate is on the first arg): to make two same-hash keys unequal, the `eq=None` key must be the *new* key in the assoc.

### Known limitations

- `bmiKey`/`bmiVal` on a sub-node slot — aborts: the slot's key is `0` and the wrapper unconditionally `incRef`s it (`incRef` aborts on NULL, `runtime3.c:808`). Needs a `None` return for sub-node slots before it can be tested.
- shift-exhaustion (`shift > 60` abort) — deliberate crash, don't test.

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
| `new.c` | Compiler |
| `docs/implementation-notes.md` | Working notes on Toccata/HVM internals — term layout, arg passing, protocol dispatch codegen, BMI C API hazards, build/test pipeline |
| `skills/memory-leak-hunting.md` | Skill to follow when a memory leak or double-free is found |
