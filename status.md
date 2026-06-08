# HVM Interaction Rules — Implementation Status

## Before Implementing a New Rule

Read these files for context before starting any new interaction:

1. `skills/interaction-recipe.md` — The full recipe: how to implement & test interaction rules, with lessons learned and patterns
2. `docs/calculus.md` — Formal calculus: node types, polarities, all 15 interaction rules with before/after descriptions
3. `docs/interactions.dot` — Visual diagrams for each rule
4. `docs/implementation.md` — Architecture reference: term layout, memory, reduction engine
5. `new.h` — Type definitions, tag constants, function declarations
6. `new.c` — Core implementation: `take`, `swap`, `move`, `interact`, existing rules
7. `regression-tests/test-hvm.c` — Test suite: patterns for building terms and verifying results
8. `graph.c` — DOT graph generation for debugging
9. `Makefile` — Build command for `test-hvm`

Read in order: the calculus defines the rules, the implementation shows how they work, the tests show how to exercise them, and the graph/debug files help diagnose issues.

## Implemented

| # | Rule | Handler | Notes |
|---|------|---------|-------|
| 1 | APP/LAM | `appLam` | Beta reduction — passthrough wiring |
| 2 | ERA/LAM | `eraLam` | Erase body, rewire ports to ERA/NUL |
| 3 | ERA/NUL | `eraLeaf` | No-op — both leaves, already freed |
| 4 | ERA/I60 | `eraLeaf` | No-op — both leaves, already freed |
| 5 | ERA/F60 | `eraLeaf` | No-op — both leaves, already freed |
| 6 | APP/NUL | `appNul` | Redirect: port1→NUL, port2→ERA |
| 7 | OPX/NUL | `opxNul` | Same as APP/NUL |
| 8 | OPY/NUL | `opyNul` | Same as APP/NUL |
| 9 | SUB/NUL | `subNul` | Same pattern |
| 10 | ERA/SUP | `eraSup` | Two ERAs to SUP's ports |
| 11 | DUP/NUL | `dupNul` | Both DUP ports→NUL |
| 12 | DUP/NUM | `dupNum` | Both DUP ports→# |
| 13 | OPX/NUM | `opxNum` | #→OPY, OPY ports wired |
| 14 | OPY/NUM | `opyNum` | b→result (#1 op #2) |
| 15 | ERA/VAR | `eraVar` | Follow VAR chain, erase actual term |

**ERA/VAR implementation details:**
- `eraVar` calls `take(termLoc(var))` which follows VAR chains and frees locations
- For VAR (SUB/LAZ): `swap(termLoc(val), ERA)` places ERA at the SUB/LAZ location, then `eraseLazy` if LAZ
- For non-VAR (actual term): calls `interact(ERA, val)` which dispatches to the appropriate handler
- Note: `take()` frees the location, so for non-VAR values the location is freed (not replaced with ERA)
- Registered in `interactions[ERA][VAR] = &eraVar` in `hvmInit()`

**ERA/SUP implementation:**
- `eraSup` creates VAR terms pointing to SUP's ports and calls `interact` on them
- This dispatches to `eraVar`, which places ERA at each port location

**Tests added:**
- `testEraVarI60` — ERA/VAR→I60 ✅ (location contains ERA after interaction)
- `testEraVarSup` — ERA→SUP ✅ (both SUP ports contain ERA)
- `testEraVarChain` — VAR→VAR→I60 ✅ (location contains ERA after interaction)
- `testEraVarLam` — REMOVED (LAM pairs go to LAZ branch, not interact branch)
- `testAppNulLamArg` — APP/NUL with LAM arg ✅
- `testSubNulLamBody` — SUB/NUL with LAM body ✅
- `testSwapSub` — swap with SUB literal ✅
- `testEraVarThruSupI60` — ERA/VAR→DUP port1, context SUP→DUP port2 ✅
- `testEraVarThruLamI60` — ERA/VAR→DUP port1, context LAM→VAR→DUP port2 ✅
- `testEraVarThruSupNul` — ERA/VAR→DUP port1, context SUP(NUL,VAR→DUP port2) ✅
- `testEraVarThruLamNulPort2` — ERA/VAR→DUP port1, context LAM(APP,VAR→DUP port2) ✅
- `testEraVarThruSupVarToDupPort1` — ERA/VAR→DUP port1, context SUP(VAR→DUP port2) ✅
- `testEraVarThruSupDupPort2` — ERA/VAR→DUP port1, context SUP→LAM→APP→VAR→DUP port2 ✅
- `testEraVarAppThunkI60` — LAZ thunk=APP, context=I60 ✅
- `testEraVarAppThunkNul` — LAZ thunk=APP, context=NUL ✅
- `testEraVarAppThunkSupI60` — LAZ thunk=APP, context=SUP(I60,NUL) ✅
- `testEraVarAppThunkSupNul` — LAZ thunk=APP, context=SUP(NUL,I60) ✅
- `testEraVarAppThunkLamI60` — LAZ thunk=APP, context=LAM(APP,I60) ✅
- `testEraVarAppThunkLamNul` — LAZ thunk=APP, context=LAM(APP,NUL) ✅
- `testEraVarAppThunkSupLamI60` — LAZ thunk=APP, context=SUP(I60,LAM) ✅
- `testEraVarAppThunkSupLamNul` — LAZ thunk=APP, context=SUP(NUL,LAM) ✅
- `testEraseLazyCycle1` — LAZ DUP cycle, ERA→port1, context→port2 ✅
- `testEraseLazyCycle2` — LAZ DUP cycle, ERA→port1, context→port2 ✅
- `testEraseLazyNoCycleDup1` — LAZ DUP no-cycle, ERA→port1, context=I60 ✅
- `testEraseLazyNoCycleDup2` — LAZ DUP no-cycle, ERA→port1, context=I60 ✅

## Next: Tier 4 constructor interactions

- [x] **`isCycle` / `findCycle`** — Cycle detection infrastructure for lazy DUP erasure. Traverses from LAZ context, checks VAR chains and direct LAZ terms in ports.

**Note: ERA/LAZ is not a separate implementation gap.** An ERA can only reach a LAZ node through a VAR chain (LAZ is positive-polarity but stored only in negative ports, so it's always pointed to by VAR). Therefore `eraVar` covers that case — when `take` returns LAZ, `eraVar` calls `eraseLazy` which handles all LAZ sub-cases (DUP with cycle detection, APP/OP wildcard erasure).

**Cycle detection details:**
- `isCycle(context, lazLoc)` starts from LAZ's positive port (context) and looks for a path back to the LAZ
- `findCycle` traverses VAR chains and pair node ports, checking for both VAR→lazLoc and direct LAZ term matches
- The cycle closes when a port directly contains the LAZ term (DUP's ports contain LAZ directly, not via VAR)
- Linear scan visited set — O(n²) worst case, hash set TODO for optimization
- Tests: `testIsCycleLazyDup`, `testIsCycleNoCycle`, `testIsCycleVarToLaz`, `testIsCycleVarToI60`, `testIsCycleVarThruSup`, `testIsCycleVarThruSupI60`, `testIsCycleVarThruLamI60`, `testIsCycleVarThruSupNul`, `testIsCycleVarThruLamNulPort2`, `testIsCycleVarThruSupDupPort2` — all pass, good coverage of VAR chains through SUP/LAM/APP to DUP

**eraVar tests:** `testEraVarI60`, `testEraVarChain`, `testEraVarThruSupI60`, `testEraVarThruLamI60`, `testEraVarThruSupNul`, `testEraVarThruLamNulPort2`, `testEraVarThruSupVarToDupPort1`, `testEraVarThruSupDupPort2`, `testEraVarAppThunkI60`, `testEraVarAppThunkNul`, `testEraVarAppThunkSupI60`, `testEraVarAppThunkSupNul`, `testEraVarAppThunkLamI60`, `testEraVarAppThunkLamNul`, `testEraVarAppThunkSupLamI60`, `testEraVarAppThunkSupLamNul` — all pass, covers cycle and non-cycle cases with varied context trees

**eraseLazy tests:** `testEraseLazyCycle1`, `testEraseLazyCycle2`, `testEraseLazyNoCycleDup1`, `testEraseLazyNoCycleDup2` — all pass

**Lazy DUP port convention:** When testing eraVar with a LAZ whose thunk is a DUP, the context (LAZ port 2) loops back to one DUP port and the VAR given to `interact(ERA, var)` targets the other. All tests use context→port2, interact→port1.

## Remaining — Ordered by Implementation Priority

### Deferred — Leaf redirections (done after all constructor rules)

- [ ] **ERA/VAL** — ERA connects to VAL. Handler: `eraVal` (not `eraLeaf` — special handling needed for native value erasure).
- [ ] **DUP/VAL** — DUP connects to VAL. Handler: `dupVal`.
- [ ] **APP/VAL** — APP connects to VAL. Handler: `appVal`.

### Tier 1: Simple leaf redirections (same pattern as APP/LAM)

- [x] **APP/NUL** — APP principal connects to NUL. Redirect: port1 → NUL, port2 → ERA.
- [x] **OPX/NUL, OPY/NUL** — Identical to APP/NUL. OPX or OPY principal connects to NUL. Redirect: port1 → NUL, port2 → ERA.
- [x] **SUB/NUL** — Same pattern. SUB circle node connects to NUL. Redirect: port1 → NUL, port2 → ERA.

### Tier 2: ERA on constructors

- [x] **ERA/SUP** — ERA connects to SUP. After: two ERAs to SUP's ports.
- [x] **DUP/NUL** — DUP connects to NUL. After: NUL to both DUP ports.

### Tier 3: NUM (#) interactions

- [x] **DUP/NUM** — DUP connects to #. After: # to both DUP ports.
- [x] **OPX/NUM** — OPX connects to #. After: # connects to OPY, OPY ports wired.
- [x] **OPY/NUM** — OPY connects to #. After: b connects to result (#1 op #2).

### Tier 4: Constructor interactions

- [ ] **APP/SUP, OPX/SUP, OPY/SUP** — `*/SUP` wildcard. Negative constructor connects to SUP. After: SUP principal connects to `b`, aux ports connect to two `*` wildcards, LAZ/DUP chains to `x`/`y`. Handler: `negSup`.
- [ ] **DUP/LAM** — DUP connects to LAM. After: two LAMs with SUP and LAZ/DUP chains. Handler: `dupLam`.
- [ ] **DUP/SUP** — DUP connects to SUP. After: complex rewiring with SUP chains. Handler: `dupSup`.

### Tier 5: LAZ interactions (most complex)

- [ ] **(APP/OPX/OPY/DUP)/LAZ DUP** — Empty triangle connects to LAZ with DUP thunk. After: new DUP node created, LAZ unwrapped. Handler: `lazDup`.
- [ ] **(APP/OP)/LAZ (APP/OP)** — Empty triangle connects to LAZ with APP/OP thunk. After: new empty triangle created, LAZ unwrapped. Handler: `lazAppOp`.

**Note: ERA/LAZ DUP and ERA/LAZ (APP/OP) are handled by `eraVar` → `eraseLazy`.** These are not separate interaction rules — ERA reaches LAZ only through VAR chains, so `eraVar` covers both cases.

## Notes

- `eraLeaf` handles all three leaf cases (NUL, I60, F60) — it's a no-op since both sides are already freed by `swap`/`interact`.
- ERA/VAR is needed by `eraSup` (and potentially other rules) — follows the VAR to the actual term and erases it.
- ERA/VAL needs special handling for native value erasure.
- Rules are registered in `hvmInit()` via the `interactions[16][16]` jump table.
- Each rule needs a corresponding test in `test-hvm.c`.
- LAZ rules are the most complex due to lazy evaluation semantics and self-referential structures.
- **LAZ structural constraint:** LAZ is positive-polarity but stored only in negative ports (APP port 2, DUP). LAZ's context (port 2) is positive, so it can't directly contain DUP — must go through VAR chain. Cycle detection follows this VAR chain from context to find the DUP, then checks if DUP's ports directly contain the LAZ.

## Test Results (2026-06-07)

All 10 free/alloc tests pass. Interaction tests run sequentially after free/alloc tests:

| Test | Result | Notes |
|------|--------|-------|
| testAllocPairReturnsEven | ✅ | |
| testAllocPairNeverZero | ✅ | |
| testFreePairZeroIsNoop | ✅ | |
| testFreeLocCoalesces | ✅ | |
| testFreeLocSingleCellDoesNotFreePair | ✅ | |
| testInterleavedFreeLoc | ✅ | |
| testMakePairStoresTerms | ✅ | |
| testFreeLocVoidIsNoop | ✅ | |
| testStressAllocFree | ✅ | 20 cycles × 100 pairs |
| testFreeListEntryFormat | ✅ | |
| testAppLam | ✅ | appLam called once |
| testMoveEra | ✅ | appLam called #2 (redex) |
| testTakeVarChain | ✅ | appLam called #3 (redex) |
| testTakeLaz | ✅ | appLam called #4 (redex) |
| **testTakeSub** | ❌ | appLam called #5 — **FAILS** |

**testTakeSub** fails with `trying to move a negative to location 007`.
Debug shows `appLam #5: neg=APP loc=006 pos=LAM loc=006` — both APP and LAM have the same location. Free list is corrupted.

## Bugs Fixed

### Location 0 collision with leaf terms (fixed 2026-06-07)

**Symptom:** `testSwapSub` would crash with `term has no location: SUB` when run after certain preceding tests in randomized order.

**Root cause:** `allocPair()` could return location 0 when `buffEnd` was 0 or when location 0 was on the free list. A pair allocated at location 0 would produce `newTerm(tag, lab, 0)`, which for SUB tag equals the SUB literal (`0x2`). This made `makePair(SUB, ...)` return the SUB literal instead of a valid SUB pair, causing `termLoc(SUB_literal)` to crash.

**Fix:** Skip location 0 in both `allocPair()` (don't return it from free list or extend past it) and `freePair()` (never free location 0). This ensures location 0 is never allocated or freed, eliminating the collision with leaf term values.

## Next: Fix free list corruption (order-dependent state leakage)

**Symptom:** Tests fail with order-dependent bugs:

1. **`testTakeSub` fails**: `appLam` called 5 times. 5th call has APP and LAM both at location 006 (same location). Fails with `trying to move a negative to location 007`.
2. **`move` finds positive term**: `move()` encounters NUL (positive) where it expects a negative term, triggering SAFETY abort.

**Root cause:** The free list management in `allocPair`/`freePair` is corrupted by a chain of bugs:

1. **`allocPair` EMPTY_FREE_LIST returns wrong value**: `fetch_add(&buffEnd, 2)` returns the OLD `buffEnd` value. When `buffEnd=2`, it returns 2 and sets `buffEnd=4`. The code was returning 2 (the old value) instead of 4 (the actual new location). This causes pairs to be allocated at wrong locations.

2. **`freeList` gets corrupted to 0**: When `freePair` is called with `freeList=0` (corrupted from step 1), it writes `newTerm(NUL, 0xFF, 0)` to the buffer — a self-referential entry. Subsequent `allocPair` reads this and sets `freeList = 0xFF3 >> 36 = 0`, perpetuating the corruption.

3. **Cascading allocation corruption**: Once the free list is corrupted, all subsequent allocations go to wrong locations, causing pairs to overwrite each other and tests to fail with "trying to move a negative" or "found positive at move target" errors.

**Fix needed:** The core fix is in `allocPair` EMPTY_FREE_LIST case. When `fetch_add` returns a non-zero value, return `old + 2` (the actual new location). Also ensure `freePair` never writes invalid entries (location 0) to the buffer. The `case 0:` handler in `allocPair` and `currTop=0` guard in `freePair` are temporary mitigations — the real fix is ensuring the free list stays consistent.

**Priority:** High — prevents all tests from passing in sequence.

**Note:** The error is "trying to move a negative" (SAFETY abort), not `glblAlloced != 0`. The free list corruption causes `allocPair` to return the same location for both APP and LAM in `testTakeSub`, so `take(portLoc(2, pos))` gets the wrong term (ERA instead of a positive value).
