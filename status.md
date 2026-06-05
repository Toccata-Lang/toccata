# HVM Interaction Rules — Implementation Status

## Implemented

| # | Rule | Handler | Notes |
|---|------|---------|-------|
| 1 | APP/LAM | `appLam` | Beta reduction — passthrough wiring |
| 2 | ERA/LAM | `eraLam` | Erase body, rewire ports to ERA/NUL |
| 3 | ERA/NUL | `eraLeaf` | No-op — both leaves, already freed |
| 4 | ERA/I60 | `eraLeaf` | No-op — both leaves, already freed |
| 5 | ERA/F60 | `eraLeaf` | No-op — both leaves, already freed |

## Remaining — Ordered by Implementation Priority

### Tier 1: Simple leaf redirections (same pattern as APP/LAM)

- [ ] **APP/NUL** — APP principal connects to NUL. Redirect: port1 → NUL, port2 → ERA.
- [ ] **SUB/NUL** — Same as APP/NUL. SUB circle node connects to NUL.
- [ ] **OP/NUL** — OPX or OPY principal connects to NUL. Same redirection pattern.

### Tier 2: ERA on constructors

- [ ] **ERA/SUP** — ERA connects to SUP. After: two ERAs to SUP's ports.
- [ ] **DUP/NUL** — DUP connects to NUL. After: NUL to both DUP ports.

### Tier 3: NUM (#) interactions

- [ ] **DUP/NUM** — DUP connects to #. After: # to both DUP ports.
- [ ] **OPX/NUM** — OPX connects to #. After: # connects to OPY, OPY ports wired.
- [ ] **OPY/NUM** — OPY connects to #. After: b connects to result (#1 op #2).

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
- Rules are registered in `hvmInit()` via the `interactions[16][16]` jump table.
- Each rule needs a corresponding test in `test-hvm.c`.
- LAZ rules are the most complex due to lazy evaluation semantics and self-referential structures.
