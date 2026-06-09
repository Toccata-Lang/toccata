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
| 16 | DUP/LAM | `dupLam` | DUP connects to LAM. After: two LAMs with SUP and LAZ/DUP chains. |
| 17 | DUP/SUP | `dupSup` | Same label: annihilation (aux ports rewired, both consumed). Different labels: commutation/expansion (new SUP + LAZ/DUP chains). |

**ERA/VAR implementation details:**
- `eraVar` calls `take(termLoc(var))` which follows VAR chains and frees locations
- For VAR (SUB/LAZ): `swap(termLoc(val), ERA)` places ERA at the SUB/LAZ location, then `eraseLazy` if LAZ
- For non-VAR (actual term): calls `interact(ERA, val)` which dispatches to the appropriate handler
- Note: `take()` frees the location, so for non-VAR values the location is freed (not replaced with ERA)
- Registered in `interactions[ERA][VAR] = &eraVar` in `hvmInit()`

**ERA/SUP implementation:**
- `eraSup` creates VAR terms pointing to SUP's ports and calls `interact` on them
- This dispatches to `eraVar`, which places ERA at each port location

**DUP/LAM implementation:**
- Three branches: ERA in port1 (take+move), ERA in port2 (take+move), or full expansion
- Full expansion: creates two new LAMs, a SUP, and a lazy DUP (or direct DUP if bod is I60)
- The `makeLazyDup` helper creates either a DUP with native values in both ports, or a LAZ→DUP self-referential chain
- Handler must check for ERA in DUP ports before moving new LAMs in (swap on ERA triggers interact)
- Tests: `testDupLam` (ERA/ERA ports), `testDupLamWithSub` (SUB/SUB ports, bod=I60), `testDupIdentity` (SUB/SUB ports, self-referential LAM → lazy DUP chain) — all pass

**DUP/SUP implementation:**
- Two label-dependent variants: same label = annihilation (SUP aux values wired into DUP ports, both consumed); different labels = commutation/expansion (creates new SUP + LAZ/DUP chains via `makeLazyDup`)
- ERA in DUP ports handled before expansion (take ERA, move SUP to other port)
- `makeLazyDup` with native values puts values directly in DUP ports; with other values creates LAZ/DUP chains
- Tests: `testDupSupAnnihilation` (matching labels), `testDupSupCommutation` (different labels, I60 SUP ports), `testDupSupCommutationLam` (different labels, identity LAM SUP ports) — all pass

**Tests added:**
- `testEraVarI60` — ERA/VAR→I60 ✅ (location contains ERA after interaction)
- `testEraVarSup` — ERA→SUP ✅ (both SUP ports contain ERA)
- `testEraVarChain` — VAR→VAR→I60 ✅ (location contains ERA after interaction)
- `testEraVarLam` — REMOVED (LAM pairs go to LAZ branch, not interact branch)
- `testAppNulLamArg` — APP/NUL with LAM arg ✅
- `testSubNulLamBody` — SUB/NUL with LAM body ✅
- `testSwapSub` — swap with SUB pair (label > 0) ✅
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
- [x] **DUP/LAM** — DUP connects to LAM. After: two LAMs with SUP and LAZ/DUP chains. Handler: `dupLam`. Tests: `testDupLam` (ERA/ERA), `testDupLamWithSub` (SUB/SUB, bod=I60), `testDupIdentity` (SUB/SUB, self-referential LAM → lazy DUP chain).
- [x] **DUP/SUP** — DUP connects to SUP. Same label: annihilation (aux ports rewired, both consumed). Different labels: commutation/expansion (new SUP + LAZ/DUP chains). Handler: `dupSup`. Tests: `testDupSupAnnihilation` (matching labels), `testDupSupCommutation` (different labels, I60 SUP ports), `testDupSupCommutationLam` (different labels, identity LAM SUP ports).

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
- **`freeLoc` double-free detection:** Now aborts on attempting to free an already-freed location (VOID check for odd locations, NUL+0xFF check for even locations). Tests must not call `take`/`freeLoc` on ports that were already freed by `freePair` coalescing during an interact.
- **SUB pairs require label > 0** for connected ports. `makePair(SUB, 0, ...)` produces a no-op SUB pair (ports not connected).

## Test Results

All tests pass. 1000+ shuffled order runs verified — no order-dependent bugs.

### Free/Alloc Tests

| Test | Result |
|------|--------|
| testAllocPairReturnsEven | ✅ |
| testFreeLocCoalesces | ✅ |
| testFreeLocSingleCellDoesNotFreePair | ✅ |
| testInterleavedFreeLoc | ✅ |
| testMakePairStoresTerms | ✅ |
| testStressAllocFree | ✅ 20 cycles × 100 pairs |
| testFreeListEntryFormat | ✅ |

### Interaction Tests

| Test | Result |
|------|--------|
| testAppLam | ✅ |
| testMoveEra | ✅ |
| testTakeVarChain | ✅ |
| testTakeLaz | ✅ |
| testTakeSub | ✅ |
| testCascading | ✅ |
| testMoveNul | ✅ |
| testEraLam | ✅ |
| testEraLamNulBody | ✅ |
| testEraLamLamBody | ✅ |
| testEraBoth | ✅ |
| testAppNul | ✅ |
| testOpxNul | ✅ |
| testOpYNul | ✅ |
| testSubNul | ✅ |
| testEraSup | ✅ |
| testDupNul | ✅ |
| testDupNum | ✅ |
| testOpxNum | ✅ |
| testOpYNum | ✅ |
| testEraSupLam | ✅ |
| testNegSupXNul | ✅ |
| testNegSupYNul | ✅ |
| testNegSupGeneral | ✅ |
| testNegSupOpxXNul | ✅ |
| testNegSupOpYYNul | ✅ |
| testDupNulSub | ✅ |
| testOpxNumSub | ✅ |
| testOpxNumMul | ✅ |
| testOpxNumEq | ✅ |
| testOpxNulExplicit | ✅ |
| testEraLamTriple | ✅ |
| testDupNumDifferent | ✅ |
| testSubNulLiteral | ✅ |
| testMultiRedex | ✅ |
| testEraVarI60 | ✅ |
| testEraVarSup | ✅ |
| testEraVarChain | ✅ |
| testCascadingRedex | ✅ |
| testAppNulLamArg | ✅ |
| testSubNulLamBody | ✅ |
| testSwapSub | ✅ |
| testIsCycleLazyDup | ✅ |
| testIsCycleNoCycle | ✅ |
| testIsCycleVarToLaz | ✅ |
| testIsCycleVarToI60 | ✅ |
| testIsCycleVarThruSup | ✅ |
| testIsCycleVarThruSupI60 | ✅ |
| testIsCycleVarThruLamI60 | ✅ |
| testIsCycleVarThruSupNul | ✅ |
| testIsCycleVarThruLamNulPort2 | ✅ |
| testIsCycleVarThruSupDupPort2 | ✅ |
| testEraseLazyCycle1 | ✅ |
| testEraseLazyCycle2 | ✅ |
| testEraseLazyNoCycleDup1 | ✅ |
| testEraseLazyNoCycleDup2 | ✅ |
| testEraVarThruSupI60 | ✅ |
| testEraVarThruLamI60 | ✅ |
| testEraVarThruSupNul | ✅ |
| testEraVarThruLamNulPort2 | ✅ |
| testEraVarThruSupVarToDupPort1 | ✅ |
| testEraVarThruSupDupPort2 | ✅ |
| testEraVarAppThunkI60 | ✅ |
| testEraVarAppThunkNul | ✅ |
| testEraVarAppThunkSupI60 | ✅ |
| testEraVarAppThunkSupNul | ✅ |
| testEraVarAppThunkLamI60 | ✅ |
| testEraVarAppThunkLamNul | ✅ |
| testEraVarAppThunkSupLamI60 | ✅ |
| testEraVarAppThunkSupLamNul | ✅ |
| testDupLam | ✅ (ERA/ERA ports, full erasure) |
| testDupLamWithSub | ✅ (SUB/SUB ports, bod=I60) |
| testDupIdentity | ✅ (SUB/SUB ports, self-referential LAM → lazy DUP chain) |
| testDupSupAnnihilation | ✅ (matching labels, SUB aux ports, I60 aux ports) |
| testDupSupCommutation | ✅ (different labels, I60 SUP ports) |
| testDupSupCommutationLam | ✅ (different labels, identity LAM SUP ports) |

### Test Infrastructure

- `freeLoc()` now aborts on double-free (VOID check for odd locations, free-list-entry check for even locations)
- Location 0 is a valid allocation target — `buffEnd` starts at 0, `freePair(0)` works normally
- Removed `testAllocPairNeverZero` and `testFreePairZeroIsNoop` (no longer applicable)
- `testSwapSub` uses `makePair(SUB, 1, ...)` (label > 0 for connected ports)
- `graph.c` `downBranch`: fixed duplicate edge drawing when VAR chain leads to SUB (while loop draws edge, SUB case was also drawing it)
- `testDupIdentity` cleanup order matters: `take` on VAR→SUP follows the chain and frees the target location, so original LAM port1 must be verified with `get()` before taking du2 port2

## Bugs Fixed

### testMultiRedex double-free (fixed 2026-06-08)

**Symptom:** `freeLoc` aborts with "double free of odd location" after `testMultiRedex`.

**Root cause:** During the outer `interact(outerApp, outerLam)`, `appLam` calls `move(portLoc(2, outerApp), innerLam)`, which calls `freeLoc(portLoc(2, outerApp))`. Since port 1 was already VOID, `freePair` coalesces and frees the entire `outerApp` pair. The test cleanup then called `take(portLoc(2, outerApp))` on an already-freed port.

**Fix:** Removed the `take(portLoc(2, outerApp))` call from the test cleanup. The pair was already freed by `freePair` during the interact.
