# The `closure-capture-sup` Node Leak

A detailed, self-contained writeup of the 4-node leak in the
`closure-capture-sup` regression test, mapped to the graph rendered at
`scratch/sup/vect-reduce-args-toplevel.svg`.

The **general diagnosis technique** (how to find *any* lazy-machine leak) is
in [node-leaks.md](node-leaks.md). This document is the *specific* bug: what
the test computes, the two ingredients that collide, the step-by-step runtime
mechanism, why the machine permits it, and the fix direction.

## TL;DR

`vect-reduce` re-applies its closure `f`, so codegen dupes `f` → a `DUP(36)`
over the closure. When that `DUP` meets the closure LAM, `dupLam` shares the
closure **body** (via `makeLazyDup`) under one `SUP(36)`. The body contains a
`DUP(0)` over the accumulator `acc` (because `acc` is used in two branches).
When the `isdot=0` branch forces `acc`, that body `DUP(0)` meets the **live
closure `SUP(36)`** → `dupSup` **EXPAND** → 2 new `SUP(36)` with **no matching
`DUP(36)`** to annihilate them. They strand → 4 leaked node-pairs.

The machine's sharing bet ("evaluate the shared body once, distribute the
result") is only sound if the body never spawns an *unbalanced* `SUP(lb)` while
the closure `SUP(lb)` is live. A body `DUP(k≠lb)` meeting the closure `SUP(lb)`
does exactly that.

Note the leak is **label-gated**: `dupSup` EXPANDs only when `k ≠ lb`; if
codegen gives the closure dup and the body `DUP` the *same* label, the meeting
annihilates cleanly and nothing leaks (§9).

**Status (2026-09-26): fixed.** A `dupSup` shortcut for the self-referential
meeting shape eliminates the unbalanced `SUP(lb)` spawn; the regression test
runs clean and the result is semantically correct (§8).

## 1. What the test computes

`regression-tests/closure-capture-sup.toc` calls:

```
(vect-reduce (Some 1) [1 0] 0 (Box 0) f)
```

where `f` is a closure that **captures an outer value `V` (= 42)** and **uses
its accumulator `acc` in two places**:

```
f = fn [acc isdot] (int-cond isdot acc (Box (+ (.v V) (.v acc))))
```

- `isdot = 1` → return `acc` untouched (does not read `V`).
- `isdot = 0` → `Box(42 + acc.v)` — **forces `acc`** and reads `V`.

The reduce vector is `[1 0]` (mixed): one step takes the `isdot=1` branch, the
next takes the `isdot=0` branch (which forces `acc`). A uniform vector
(`[1 1]` or `[2 2]`) does **not** leak — see §4.

## 2. The two ingredients (both visible in the graph)

### Ingredient A — the closure: a `DUP` in its body + a capture

In the SVG the closure is the LAM chain:

- **`2:`** — outer `LAM`, binds `acc`
- **`0:`** — inner `LAM`, binds `isdot`
- **`3a:`** — `LAZ`, the **shared body**

Inside the body, `acc` appears in two places (the `int-cond` true-branch and
the `(.v acc)` term), so codegen emits a `DUP` over it:

- **`c:`** — the **`DUP(0)`** over `acc` (label `c:\n0`). This is the node that
  breaks the bet.
- **`e:`** — the lazy-dup `LAZ` that wires the `DUP` to the `acc` slot
  (`e:sw → c:`, `e:se → 2:sw`).

The **capture** is the outer `V`:

- **`42`** — the captured value, fed into the `+` (**`2c:`**) and `Box`
  (**`53:`** / **`55:`**). It lives at a *single outer location* that one
  branch erases and the other forces.
- **`intCond`** (**`56:`**) — the strict conditional that branches on `isdot`.

### Ingredient B — the closure duplication (`DUP 36`)

This is **not** drawn in the SVG (the SVG is the top-level *args* to
`vect-reduce`). It is created **inside `vect-reduce` at runtime**: `vect-reduce`
uses `f` twice (current step + recursive call), so its generated C emits

```c
f_20 = dupeArg(f_20, &f_20_1, 36);   // glblvect_reduce587
```

→ a **`DUP(36)`** over the closure. Label `36` (hex `24`) is the closure-dup
label. (The leak dump prints the label in **hex**: `SUP 24` = label 36 decimal.)

## 3. The runtime mechanism (step by step)

1. **`DUP(36)` meets the closure LAM → `dupLam`** (fires 4× — the reduce is
   recursive, each step re-dupes `f`). `dupLam` produces:
   - two LAM copies,
   - **one shared `SUP(36)`** over their param ports,
   - the **body shared** via `makeLazyDup(36, body)` — a single `LAZ`
     referenced from both `DUP(36)` ports.

   This is the core bet: *evaluate the body once, distribute the result to
   both copies.*

2. **Both copies get applied** — `acc`/`isdot` move into the two `SUP(36)`
   param ports. Inside the body: `acc = SUP(36)(acc_a, acc_b)`,
   `isdot = SUP(36)(1, 0)`.

3. **The shared body is forced once** (`forceLazy` swaps
   `SUB{DUP(36), VAR}` into the body slot).

4. **`int-cond` distributes over `isdot`'s `SUP(36)`** (`strictArgs`), giving
   two branches:
   - `isdot = 1`: result = `acc` (untouched → `V` erased via `eraVar`);
   - `isdot = 0`: `Box(42 + acc.v)` → **forces `acc`**.

5. **Forcing `acc` is the trigger.** The body's **`DUP(0)`** (`c:`) meets the
   **live closure `SUP(36)`** → `dupSup` **EXPAND** (labels differ: 0 ≠ 36) →
   **2 `DUP(0)` + 2 `SUP(36)`**. The instrumented log shows exactly this
   (`scratch/sup/graphs/sup-log.txt`):

   ```
   dupSup: EXPAND dup lab=0 loc=00c (p1Tag=SUB p2Tag=SUB)
                 + SUP lab=36 loc=018 (xTag=VAR yTag=SUP) -> 2 SUP(36)
   ```

   (3 such EXPANDs — one per reduction step.)

6. **The new `SUP(36)`s strand.** A `SUP(l)` is destroyed only by meeting a
   matching `DUP(l)`. The *only* `DUP(36)` was the closure dup, already
   consumed by `dupLam` in step 1. The `SUP(36)`s created by the EXPAND in step
   5 have **no matching `DUP(36)`** → never annihilated → they leak.

**SUP(36) accounting:** 10 created (4 from `dupLam` + 6 from 3 EXPANDs);
5 annihilated, 3 erased by `eraSup`, **2 stranded** → the 4 leaked node-pairs.

## 4. Why the machine allows it (the design defect)

`makeLazyDup(lb, arg)` is the heart of the lazy machine. For a non-leaf `arg`
it builds **one** `LAZ{DUP(lb), arg}` and puts that *same* `LAZ` in **both**
ports of a fresh `DUP(lb)`:

```c
Term lz = makePair(LAZ, 0, dp, arg);
swap(portLoc(1, loc), lz);
swap(portLoc(2, loc), lz);
```

So `arg` is **shared**: the first port to be forced evaluates it once
(`forceLazy` swaps a `SUB{DUP, VAR-to-arg}` into `arg`'s slot); the second port
finds that `SUB` and reuses the result. The machine *bets* the body can be
evaluated once and the result distributed.

`dupLam` uses exactly this to duplicate a closure: two LAM copies, a `SUP(lb)`
over their param ports, and `makeLazyDup(lb, body)` over the **shared** body.

**The bet holds** when the body only *reads* shared state the same way in every
branch (e.g. reduce over `[1 1]` or `[2 2]` — the captured var is resolved or
erased identically each iteration, so any stray `SUP(lb)` gets `eraSup`-erased).

**The bet breaks** when, under a live closure `SUP(lb)`, the body contains a
`DUP(k≠lb)` that meets the closure `SUP(lb)`: `dupSup`'s EXPAND rule
*multiplies* the `SUP(lb)` (`DUP(k)+SUP(lb) → 2 DUP(k) + 2 SUP(lb)`) with no
corresponding source of `DUP(lb)`. The mixed vector `[1 0]` is what makes one
branch force `acc` (triggering the EXPAND) while the other does not — so the
spawned `SUP(lb)` has no counterpart to erase or annihilate it.

All three trigger conditions hold here:

1. the body contains a `DUP` (the `acc` dup, `c:`);
2. the closure is dup'd (`vect-reduce` re-applies `f` → `DUP(36)`);
3. the shared body is actually forced while the closure `SUP(36)` is live.
4. the closure-dup label and the body-`DUP` label **differ** (implicit —
   `dupSup` EXPANDs only on `k ≠ lb`). Codegen's global, generation-order
   label assignment decides this; see §9.

## 5. The 4 leaked nodes (from the leak dump)

```
00a  SUP 24  024  FREE      <- stranded SUP(36)   (24 hex = 36)
024  VAR 0   03e            <- dangling VAR
03e  VAL 7f...010 FREE      <- a heap value (the captured Box)
08c  VAR 0   00a            <- VAR pointing BACK at the SUP (cycle 08c -> 00a)
```

A `SUP(36)` that never met its `DUP(36)`, plus dangling `VAR`s, one of which
loops back to the `SUP` — hence the cycle the leak checker reports. `Leaked
pairs!! 4` = these 4 node-pairs.

## 6. Reading it in the SVG

Open `scratch/sup/vect-reduce-args-toplevel.svg` (the top-level `vect-reduce`
args, 1 subgraph). Follow:

- The **closure**: `2:` → `0:` → `3a:` (the LAM chain into the shared body).
- In the body, **`c:`** is the `DUP(0)` over `acc` (the trigger); **`e:`** is
  the lazy-dup `LAZ` tying it to the `acc` slot.
- **`42`** is the captured `V`; **`intCond`** (`56:`) is the branch point;
  **`1`** / **`0`** via **`vect-conj`** (`4b:` / `4d:`) are the `[1 0]` vector.
- The **`DUP(36)`** that actually causes the leak is *not drawn* (it is created
  inside `vect-reduce` at runtime) — the SVG shows the closure and its body
  (ingredient A); ingredient B is the runtime `dupeArg` in
  `glblvect_reduce587`.

## 7. Fix direction (superseded — see §8)

The fix that was actually implemented took a different form (a `dupSup`
shortcut, not a `dupLam` body-scan). Kept for the record:

In `dupLam`, if the body contains any `DUP`, **do not share it**: give each LAM
copy its own body *and* its own `SUP(lb)` over its own param ports, so a body
`DUP(k)` only ever meets its own copy's `SUP(lb)` and annihilation stays
balanced. Concretely this needs:

1. A **body-scan** for `DUP` tags (walk the term graph, stop at leaves / `VAL`
   / `VAR` — do not follow `VAR`s, since the relevant `DUP`s are structural
   pairs inside the body, not behind references).
2. A **non-shared duplication path** in `dupLam`: deep-copy the body per LAM
   copy, with per-copy params and per-copy `SUP(lb)`, handling the shared
   capture `V`.

The detection is understood; the **replacement strategy** (what `dupLam` does
*instead* of sharing) is the unsolved part. A conservative rule — "any `DUP` in
body ⇒ don't share" — is always correct (labels are reused in codegen, so label
equality is unreliable) but occasionally forgoes the sharing optimization.

Compile-time detection (codegen already computes per-var refcounts in
`.refs-map`) is the better long-term home: it can do precise parameter-linearity
analysis and restructure codegen (inline / pre-dup the body) rather than carry a
runtime fallback — but it gates the same unsolved replacement problem.

## 8. The fix (implemented in `new.c`/`dupSup`, 2026-09-26)

The implemented fix is **not** the §7 `dupLam` body-scan. It is a shortcut in
`dupSup`'s label-mismatch branch: before the standard EXPAND, check whether
either of the SUP's aux ports is a `VAR` pointing **back at the DUP's own
port** (the self-referential cycle this leak produces when the body `DUP(0)`
meets the live closure `SUP(36)`):

```c
Tag xVar = termTag(x) == VAR;
Tag yVar = termTag(y) == VAR;
if (xVar && termLoc(x) == portLoc(1, dup)) {
  freeLoc(portLoc(1, dup));
  move(portLoc(2, dup), y);
} else if (xVar && termLoc(x) == portLoc(2, dup)) {
  freeLoc(portLoc(2, dup));
  move(portLoc(1, dup), y);
} else if (yVar && termLoc(y) == portLoc(1, dup)) {
  freeLoc(portLoc(1, dup));
  move(portLoc(2, dup), x);
} else if (yVar && termLoc(y) == portLoc(2, dup)) {
  freeLoc(portLoc(2, dup));
  move(portLoc(1, dup), x);
} else {
  // original EXPAND, unchanged
}
```

In the matching case the DUP is consumed **without spawning any new
`SUP(lb)`**: the DUP's self-referencing port location is freed and the *other*
aux term is moved into the surviving port. No unbalanced `SUP(lb)` is created,
so nothing can strand.

### Test results (fixed `new.c`, `-DCHECK_MEM_LEAK=1 -DSAFETY=1 -DSTATS=1`)

| Run | Pre-fix | Post-fix |
|---|---|---|
| `make closure-capture-sup` | `Leaked pairs!! 4`, `bad result SUP pair`, ITRS 202 | `remaining nodes: 0`, malloc/free 10302/10302, ITRS 196 |
| Probe: same test with `main` returning `(.v (f (Box 42)))` | — | `bashResult: 42`, `remaining nodes: 0` — the result is `Box 42` (42 + 0), semantically correct |
| `closure-dup-minimal` (equal-label annihilation, §9) | clean | still clean (`remaining nodes: 0`, ITRS 71) |

Reading the result: when the final answer is a VAL (a heap term such as
`Box 42`), `runtime3.c`'s main frees it and reports `I60 0`
(`case VAL: dec_and_free(result, 1); result = newI60(0);`), so the plain
test's `bashResult: 0` is the harness's representation of the Box result —
not evidence of an empty result (the `NUL`/`ERA` case prints the same thing;
the `.v` probe above is how to tell them apart).

`regression-tests/closure-capture-sup.rslt` was regenerated by
`make closure-capture-sup` and now records the fixed output.

### Scope / caveats

* The shortcut fires only on the self-referential shape (a SUP aux port that
  is a `VAR` into the DUP's own port). Every other label-mismatch meeting
  still takes the original EXPAND path — the general sharing-bet defect (§4)
  is patched for the shape this leak takes, not refuted in general.
* The fixed `new.c` still carries debug leftovers: a commented-out
  per-interaction dot dump and a dead `if (rdxCount == 1484)` block in
  `interact()`, and a hardcoded `get(0x8c)` leak dump in the CHECK_MEM_LEAK
  section.

## 9. Label sensitivity (minimal repro attempt, 2026-09-16)

The minimal repro **without** `vect-reduce` does not leak — and the reason is
codegen's label assignment, not the absence of any trigger condition.

`regression-tests/closure-dup-minimal.toc`: a closure `g` that captures `V`
with the same body as the original (the `DUP` over `acc` from the two
`int-cond` branches), used twice with mixed args — `(g (Box 0) 1)` as a side
expression, then `(g (Box 0) 0)` as the main expression. All three trigger
conditions of §4 hold (body `DUP`, closure dup'd by codegen, shared body
forced under the live `SUP` with one branch forcing `acc`). But the generated
C gave **both** dups label 0:

```c
acc_13 = dupeArg(acc_13, &acc_13_1, 0);  // body DUP over acc  -> label 0
g_5    = dupeArg(g_5,    &g_5_1,    0);  // closure DUP        -> label 0
```

Run with the instrumented scratch build (`CHECK_MEM_LEAK=1`): **no leak**
(`remaining nodes: 0`). SUP log: 2× `dupLam CREATE SUP lab=0`, 2×
`dupSup ANNIHILATE lab=0`, **0× EXPAND** — the body `DUP(0)` met the closure
`SUP(0)` with equal labels and annihilated, so the accounting stayed balanced.

In the original test the mismatch comes from *where* the closure dup is
generated: inside `vect-reduce`'s own function (`f_20 = dupeArg(f_20, &f_20_1, 36)`
— late in the global label sequence) versus the body's acc dup (label 0).

Consequences:

* Manifestation is gated on codegen's global, generation-order label
  assignment. The same source construct leaks or not depending on what labels
  the two dups happen to get — regression coverage for this bug is fragile.
* The §7 fix rule ("any `DUP` in body ⇒ don't share") must not rely on
  runtime label inequality: equal labels make the bug *silently not manifest*,
  so a label-comparison-based fix would pass tests while the defect remains.

### 9.1 Forcing the mismatch: the trigger repros, the leak still does not

Hacking the generated C (`scratch/sup/closure-dup-minimal.c`) to give the
closure dup a different label — `g_5 = dupeArg(g_5, &g_5_1, 500)` (was `0`) —
makes the trigger fire exactly per §3 step 5:

```
dupSup: EXPAND dup lab=0 loc=00c (p1Tag=2 p2Tag=2) + SUP lab=500 loc=046 (xTag=1 yTag=1) -> 2 SUP(500)
```

But the run still ends `remaining nodes: 0` — **no leak**. Instead it prints
`bad result SUP pair` (`runtime3.c`): one of the spawned unbalanced `SUP(500)`
flowed straight into the program's **final result**. The result-normalization
loop handles a SUP result by splitting it (`take` both branches, keep one as
the result, push the other onto `alts[]`), and every alt is then `ERA`'d — so
the stray SUP is cleaned up by the result path instead of stranding.

So the minimal test repros the *trigger* (the unbalanced EXPAND) but not the
*leak*. The leak additionally requires the spawned `SUP(lb)` to be **stranded
away from the final result** — unreachable, in a dangling cycle — which is what
`vect-reduce`'s recursive structure provides (multiple reduction steps,
intermediate results consumed along the way). Duping the closure alone, even
with a forced label mismatch, cannot repro the leak; the original test is
already close to minimal for it.

### 9.2 Keeping the SUP out of the result: it still does not leak

Changing the `.toc` so `main` returns `0` with `(f (Box 42))` demoted to a
side expression — so the stray SUP *cannot* flow into the final result — and
re-applying the label hack: the EXPAND fires again, but the run ends
`remaining nodes: 0` with no `bad result SUP pair`. The log shows why — the
EXPAND-spawned `SUP(500)` is **annihilated by the `DUP(500)` that
`makeLazyDup` created** inside the shared-body `LAZ` (tag 10 = DUP, tag 9 =
SUP):

```
dupSup: EXPAND dup lab=0 loc=018 (p1Tag=2 p2Tag=2) + SUP lab=500 loc=050 (xTag=1 yTag=1) -> 2 SUP(500)
strictArgs: CONSUME SUP lab=500 loc=004 ; CREATE SUP lab=500 (argsCount=0)
negSup: CONSUME SUP lab=500 loc=03a ; CREATE SUP lab=500
strictArgs: CONSUME SUP lab=500 loc=030 ; CREATE SUP lab=500 (argsCount=2)
negVar: neg lab=500 tag=10 loc=060 var loc=045 valTag=9
dupSup: ANNIHILATE SUP lab=500 loc=048
```

Full `SUP(500)` accounting balances (2 `dupLam` + 2 EXPAND created; all
consumed). The three runs line up:

| Run | `f`'s result | EXPAND | Outcome |
|---|---|---|---|
| no hack (labels 0/0) | main expr | no | clean annihilation (equal labels) |
| hack, `f` result is main expr | main expr | yes | stray SUP reached result → split + ERA'd |
| hack, `f` result is side expr, `main` returns `0` | side expr | yes | stray SUP annihilated by `makeLazyDup`'s `DUP(500)` |

The trigger is reliably reproducible in the minimal test with the label hack,
but the spawned SUP keeps finding a cleanup path (result-splitting, or
annihilation by the lazy-dup's own `DUP`) instead of stranding. In
`vect-reduce`, the recursive multi-step structure is what exhausts those
cleanup paths and strands the SUPs in a dangling cycle.

## 10. How to reproduce / inspect

```sh
cd scratch/sup
clang -g -o sup -march=native -I. -I.. \
  -DCHECK_MEM_LEAK=1 -DSAFETY=1 -DSTATS=1 \
  -lm -lpthread -latomic new.c runtime3.c graph.c closure-capture-sup.c
./sup 2>&1 | tail          # -> "remaining nodes: 4 (162)", "Leaked pairs!! 4"
dot -Tsvg graphs.dot -o vect-reduce-args-toplevel.svg
```

- `scratch/sup/new.c` — instrumented copy with SUP lifecycle logging in
  `dupLam` / `dupSup` / `negSup` / `eraSup` / `strictArgs` / `forceLazy` /
  `negVar` / `eraVar`.
- `scratch/sup/graphs/sup-log.txt` — the full SUP create/annihilate/expand
  lifecycle log.
- `scratch/sup/NOTES.md` — grounded notes + workflow preferences.
- `reference-new.c` has **identical** `dupLam` / `makeLazyDup` — this is a
  fundamental defect in the sharing bet, not a regression.
