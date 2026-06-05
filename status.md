# HVM Interaction Rules — Implementation Status

## Before Implementing a New Rule

Read these files for context before starting any new interaction:

1. `skills/interaction-recipe.md` — The full recipe: how to implement & test interaction rules, with lessons learned and patterns
2. `docs/calculus.md` — Formal calculus: node types, polarities, all 15 interaction rules with before/after descriptions
3. `docs/interactions.dot` — Visual diagrams for each rule
4. `docs/implementation.md` — Architecture reference: term layout, memory, reduction engine
5. `new.h` — Type definitions, tag constants, function declarations
6. `new.c` — Core implementation: `take`, `swap`, `move`, `interact`, existing rules
7. `test-hvm.c` — Test suite: patterns for building terms and verifying results
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

## Next: ERA/LAZ and LAZ branch of ERA/VAR

- [ ] **ERA/LAZ** — ERA connects to LAZ. Follow the LAZ chain and erase it.
- [ ] **ERA/VAR LAZ branch** — When `take` returns a VAR (term is LAZ/SUB), need `eraseLazy` to handle LAZ chains.
- [ ] **ERA/VAL** — ERA connects to VAL. Special handling for native value erasure.

## Remaining — Ordered by Implementation Priority

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

- [ ] **DUP/LAM** — DUP connects to LAM. After: two LAMs with SUP and LAZ/DUP chains.
- [ ] **DUP/SUP** — DUP connects to SUP. After: complex rewiring with SUP chains.

### Tier 5: LAZ interactions (most complex)

- [ ] **ERA/LAZ DUP** — ERA connects to LAZ with DUP. After: rewiring with DUP chain.
- [ ] **ERA/LAZ (APP/OP)** — ERA connects to LAZ with APP/OP wildcard. After: two ERAs + NUL.
- [ ] **(APP/OPX/OPY/DUP)/LAZ DUP** — Empty triangle connects to LAZ with DUP. After: DUP rewiring.
- [ ] **(APP/OP)/LAZ (APP/OP)** — Empty triangle connects to LAZ with APP/OP wildcard. After: complex rewiring.

## Notes

- `eraLeaf` handles all three leaf cases (NUL, I60, F60) — it's a no-op since both sides are already freed by `swap`/`interact`.
- ERA/VAR is needed by `eraSup` (and potentially other rules) — follows the VAR to the actual term and erases it.
- ERA/VAL needs special handling for native value erasure.
- Rules are registered in `hvmInit()` via the `interactions[16][16]` jump table.
- Each rule needs a corresponding test in `test-hvm.c`.
- LAZ rules are the most complex due to lazy evaluation semantics and self-referential structures.
