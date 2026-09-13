# Finding Node Leaks in the Lazy Machine

A field guide to diagnosing "remaining nodes: N" leaks in new-toc generated
programs, distilled from the `closure-capture-sup` regression investigation.

## The symptom

Built with `-DCHECK_MEM_LEAK=1 -DSAFETY=1 -DSTATS=1`, a clean program prints:

```
<value>
- Threads: 1
- ITRS: <n>
remaining nodes: 0
```

A leak shows `remaining nodes: N (M)` plus `Leaked pairs!! N` and a dump of the
surviving pairs. The printed value can still be **correct** — the defect is
purely the leftover graph, so a wrong-looking output is NOT a prerequisite.

The leak dump prints each surviving **pair** (2 cells) with its location, e.g.:

```
00c  SUP 24 042    FREE
042  VAR 0 02e  VAR 0 08c
```

Read it as: pair at `0x00c` is a `SUP` (label `0x24` = **36**) whose port 1
points to `0x042` and port 2 is `FREE`. A `FREE`/`NUL` port on a `SUP` or `DUP`
is the classic "half-annihilated" signature: one side resolved, the other
stranded.

> **The label is printed in HEX** (`printRawTerm` uses `%x` for the label, and
> `%.3x` for the location). So a dump line `SUP 24 ...` is label `0x24` = 36
decimal, NOT label 24. Convert to decimal before grepping the generated C —
otherwise you chase a `dupeArg(..., 24)` that was never the culprit.

> **Note (2026-07-17):** `closure-capture-sup.toc` has been simplified so it
> leaks only **4 nodes** (a `Box`-capturing `f` running `vect-reduce` over
> `[1 0]` — one branch returns `acc` without touching `V`, the other forces
> `(.v V)`). Use this minimal repro instead of any earlier, larger variant.
> Its leak dump:
>
> ```
> 00a  SUP 24 024    FREE      <- SUP(0x24=36), half-annihilated (port2 FREE)
> 024  VAR 0 03e  VAR 0 08c
> 03e  VAL 7f...010  FREE      <- the captured Box value
> 08c  VAR 0 00a  FREE         <- VAR pointing back at the SUP (cycle)
> ```
>
> 4 leaked pairs = 2 stranded SUP(36) pairs + 2 dangling VAR pairs. The real
> `checkBuff` only prints the COUNT — the per-pair loop is commented out — so
> enable it in the scratch copy (or use the scratch build in `scratch/sup/`,
> which already has it plus lifecycle logging) to see the pairs at all.

## The core technique: instrument new.c, rebuild in scratch, mine the logs

new.c is the lazy-machine runtime. All leaks are ultimately caused by an
interaction rule leaving a node behind, so the highest-leverage move is to
**add logging to the interaction functions** and watch the specific node kind
you suspect (usually `SUP` or `DUP`) live and die.

### Setup (do not touch the real new.c)

```sh
mkdir -p scratch/sup && cd scratch/sup
cp ../new.c ../runtime3.c ../graph.c .
cp ../regression-tests/<test>.c .
# edit scratch/sup/new.c freely
clang -g -o sup -march=native -I. -I.. \
  -DCHECK_MEM_LEAK=1 -DSAFETY=1 -DSTATS=1 \
  -lm -lpthread -latomic new.c runtime3.c graph.c <test>.c
./sup <arg> 2>&1 | tail
```

Keep `-g` — the lldb breakpoint trick below needs symbols. The
`scratch/sup/` build from the 2026-07-17 session already contains the
leak-pair dump and the SUP lifecycle logging; reuse it.

> **Instrumentation completeness:** `strictArgs` (the `case SUP:` in
> `strictArgs`, ~line 799) is a FOURTH SUP site beyond `dupLam`/`dupSup`/
> `negSup`: it consumes a SUP and recreates it with the same label (net-zero,
> but it moves the location). If you don't log it, consume/create counts look
> wrong. (`newTerm` only makes leaves — VAR/NUL/ERA — so `makePair` and
> `strictArgs` are the only SUP creation paths; generated C never creates SUPs.)

Keep a `FILE *supLog = fopen("graphs/sup-log.txt","w")` and `fprintf` from the
interaction functions. Revert with `cp ../new.c new.c` when done.

### 1. Log the lifecycle of the leaking node kind

For a SUP leak, log every create/annihilate/expand in `dupLam`, `dupSup`,
`negSup`, `eraSup`:

```c
fprintf(supLog, "dupLam: create SUP lab=%d (lam loc=%.3x)\n", dupLab, termLoc(lam));
fprintf(supLog, "dupSup: ANNIHILATE SUP lab=%d loc=%.3x\n", supLab, termLoc(sup));
fprintf(supLog, "dupSup: EXPAND 2 SUPs lab=%d (dup lab=%d)\n", supLab, dupLab);
```

Then **count**: created vs. annihilated. An imbalance pinpoints which rule is
producing unbalanced nodes. In the simplified `closure-capture-sup` (4-node
leak): 4 `dupLam` creates + 3 `EXPAND`s (each consumes 1, creates 2 → net +1)
= 7 SUP(36); 5 `ANNIHILATE`s; `negSup`/`strictArgs` consume+recreate are
net-zero → **2 SUP pairs stranded** (the `(13)` variant had the same shape at
larger scale: 6 made, 2 annihilated, 4 stranded). The 3 EXPANDs were all
`dup lab=0 + SUP lab=36` — the `DUP(0)` over `acc` meeting the live closure
SUP.

### 2. Correlate the leak label with the generated C

The leaked SUP's **label** is the key. **Convert the hex label from the dump to
decimal first** (see the hex warning above), then grep the generated C for that
decimal label to find which `dupeArg(..., <label>)` created the matching DUP:

```sh
# e.g. dump shows 'SUP 24' -> 0x24 = 36 decimal
grep -n "dupeArg.*36\|, 36)" regression-tests/<test>.c
```

The label ties the leak back to a specific source-level duplication (a closure
dup, a vector dup, an accumulator dup). In `closure-capture-sup` the leaked SUPs
were lab 36 (dumped as `24`) = the `vect-reduce` closure duplication
(`f_20 = dupeArg(f_20, &f_20_1, 36)`), **not** the `last` closure dup (lab 24
decimal = dumped as `18`).

### 3. Dump per-interaction graphs

Drop a `.dot` of the (neg, pos) pair at every non-trivial interaction in
`interact()`, numbered by a global counter. ~230 graphs for this test. The
**last** graph before the leak and a dedicated `RESULT-SUP.dot` (dump the result
root) show the stranded node's exact wiring. Grep the result graph for the node
kind:

```sh
grep -A1 'label="S"' graphs/RESULT-SUP.dot   # S = superposition
```

### 4. Trace the specific location end-to-end

Once you know the label and the responsible rule, log every interaction that
touches one specific location (the captured var, the acc, the body). Print
`loc=%.3x` and `valTag`/`lzTag` in `negVar`/`eraVar`/`forceLazy`/`eraseLazy`.
This shows exactly where a location is **forced in one branch and erased in
another** — the signature of a shared-location leak.

### 5. Conditional breakpoint on creation (lldb)

To find where a specific node is *created*, breakpoint `makePair` with a
tag+label condition and read the backtrace:

```sh
# lldb has no 'condition' command; the condition flag on 'breakpoint set' is
# lowercase -c (uppercase -C is commands-on-hit and silently drops the
# condition). C enums are NOT visible to lldb's C++ expression evaluator —
# use numeric tag values from new.h (SUP = 0x09).
printf 'b -n makePair -c "tag == 9 && lab == 36"\nrun <arg>\nc\nbt 15\nquit\n' \
  > /tmp/lldb-cmds.txt
lldb --source /tmp/lldb-cmds.txt ./sup
```

(gdb is usually not good enough here: it fails to resolve the enum in the
condition, "No symbol SUP in current context".)

Two uses:
* **Hit** → `bt` shows the creating call site directly.
* **Never hits** (program runs to completion) → the node was never created
  with that tag+label. That is itself the answer: in this investigation the
  condition `tag == 9 && lab == 24` (SUP with decimal label 24) never fired,
  which proved the dump's "24" was a hex misread of label 36 (see the warning
  in The symptom) — no in-place label mutation, no mystery creation path.

## The lazy machine's sharing bet (root-cause model)

`makeLazyDup(lb, arg)` is the heart of the lazy machine. For a non-leaf `arg`
it builds **one** `LAZ{DUP(lb), arg}` and puts that *same* LAZ in **both** ports
of a fresh `DUP(lb)`:

```c
Term lz = makePair(LAZ, 0, dp, arg);
swap(portLoc(1, loc), lz);
swap(portLoc(2, loc), lz);
```

So `arg` is **shared**: the first port to be forced evaluates it once
(`forceLazy` swaps a `SUB{DUP, VAR-to-arg}` into `arg`'s slot); the second port
finds that SUB and reuses the result. The machine *bets* the body can be
evaluated once and the result distributed.

`dupLam` uses exactly this to duplicate a closure: two LAM copies, a `SUP(lb)`
over their param ports, and `makeLazyDup(lb, body)` over the **shared** body.

**The bet holds** when the body only *reads* shared state (e.g. reduce over
`[1 1]` or `[2 2]` — the captured var is resolved or erased the same way every
iteration).

**The bet breaks** when, under a live closure SUP, one branch **resolves** a
shared captured location and another **erases** it (reduce over `[1 0]` — one
iteration returns `acc` without touching `V`, the next forces `(.v V)`). A
single location cannot be both forced and erased. Concretely, with the closure
SUP(lb) live the params are themselves SUP(lb); the shared body's `DUP(0)` over
`acc` meets `SUP(lb)` → `dupSup` **EXPAND** → `2 DUP(0) + 2 SUP(lb)`, and those
new SUP(lb) are never met by matching DUP(lb) → they strand in the result.

## Interaction rules that matter for leaks

| Rule | File fn | Leak-relevant behavior |
|------|---------|------------------------|
| `DUP+LAM` | `dupLam` | Splits a DUP into `SUP(lb)` (params) + lazy `DUP(lb)` (shared body). Creates SUPs. |
| `DUP+SUP` same lab | `dupSup` ANNIHILATE | Consumes one DUP + one SUP. The only thing that removes a SUP. |
| `DUP+SUP` diff lab | `dupSup` EXPAND | `DUP(a)+SUP(b) → 2 DUP(a) + 2 SUP(b)`. **Multiplies** SUPs; the usual leak source when `a≠b` is "wrong". |
| `ERA+SUP` | `eraSup` | Erases both SUP ports; does **not** create the balancing DUP. |
| strict arg meets SUP | `strictArgs` `case SUP` | Consumes a SUP, recreates it same-label (net-zero, moves location). Must be logged or create/annihilate counts look wrong. |
| `neg+VAR`→LAZ | `negVar`→`forceLazy` | Forces a thunk; swaps SUB into the shared slot. |
| `ERA+VAR`→LAZ | `eraVar`→`eraseLazy` | Erases a thunk. |

A SUP is balanced only if, for every SUP(lb) created, an equal number of
DUP(lb) later annihilate it. EXPAND with a mismatched label is how the count
runs away.

## new.c vs reference-new.c (delta that does NOT fix this bug)

`reference-new.c` is the same lazy machine (identical `makeLazyDup`, `dupLam`,
`dupSup`, `forceLazy`) with older snake_case naming — it does **not** build
against `new.h`, so it is not a drop-in oracle. The only interaction handler it
has that new.c lacks is `eralaz` (`ERA/LAZ`: force the lazy if the ERA is
labeled, else erase). new.c has no `ERA/LAZ` row (falls to `badrdx`). Also,
reference `eravar` swaps the *actual* `era` term (label preserved) and passes it
to `eraseLazy(lz, era)`; new.c swaps the generic `ERA`. Neither delta changes
this test: in the leak every `eraVar` fires with `eraLab=0`, and `eralaz` is
never invoked (ERA meets a VAR, not a bare LAZ).

## Fixes tried (and why they failed)

| Attempt | Result |
|---------|--------|
| `forceLazy`: `freePair` the LAZ after the two `take()`s (the in-code TODO) | SAFETY crash — "found positive at move target": the LAZ pair is still referenced, so it must **not** be freed there. |
| `eraseLazy`: early-return on `termTag(laz)==VOID` | No effect (still 13). |
| `eraVar`: swap `era` instead of `ERA` (preserve label) | No effect (eraLab is 0 throughout). |
| Add `interactions[ERA][LAZ]=&eraLaz` (mirror reference) | Handler never called (0 invocations). |

## Confirmed root cause (2026-09-16, instrumented scratch build)

Traced end-to-end in `scratch/sup/` (extra logging in `dupSup`/`forceLazy`/
`negVar`/`eraVar`; full sequence in `scratch/sup/NOTES.md` and
`graphs/sup-log.txt`). The mechanism in the sharing-bet section above is
confirmed, with the exact trigger pinned down:

1. `vect-reduce`'s generated C dupes the closure: `f_20 = dupeArg(f_20, &f_20_1, 36)`.
2. `dupLam` splits it (4× — the reduce is recursive, each step re-dupes): two
   LAM copies, ONE shared `SUP(36)` over their param ports, body SHARED via
   `makeLazyDup(36, bod)`.
3. Both copies applied → inside the body: `acc = SUP(36)(acc₁, acc₂)`,
   `isdot = SUP(36)(1, 0)`; the shared body is forced ONCE.
4. The body is `int-cond isdot acc (Box(+ (.v V) (.v acc)))` — `acc` appears in
   TWO places (true-branch + `.v acc`), so the body contains a **`DUP(0)` over
   `acc`** (`acc_9 = dupeArg(acc_9, &acc_9_1, 0)` in the generated C). THIS is
   the node that breaks the bet — not the captured `V` itself.
5. `intCond` (strictArgs) distributes over the `isdot` SUP: the `isdot=1`
   branch returns `acc` untouched (V erased); the `isdot=0` branch forces `acc`.
6. Forcing `acc`: the body's `DUP(0)` meets the live closure `SUP(36)` →
   `dupSup` EXPAND (0 ≠ 36) → `2 DUP(0) + 2 SUP(36)`. Log:
   `dupSup: EXPAND dup lab=0 loc=00c (p1Tag=SUB p2Tag=SUB) + SUP lab=36 loc=018 (xTag=VAR yTag=SUP) -> 2 SUP(36)`
   — 3 such EXPANDs, one per reduction step.
7. The new `SUP(36)`s need `DUP(36)`s to annihilate, but the only `DUP(36)` was
   already consumed by `dupLam`. Nothing ever meets them → they strand.

Counting closes exactly: 4 (dupLam) + 6 (3 EXPANDs × 2) = 10 SUP(36) created;
5 annihilated + 3 eraSup-erased = **2 stranded** = the 4 leaked pairs (2 SUP +
2 dangling VAR; the `0x08c → 0x00a` VAR points back at the stranded SUP — the
cycle in the dump).

The bet (evaluate the shared body once, distribute the result) is only sound if
the body, evaluated under the live closure `SUP(lb)`, never creates an
**unbalanced** `SUP(lb)`. A body containing a `DUP(k≠lb)` that meets the
closure `SUP(lb)` does exactly that. Trigger conditions (all present here):
body contains a `DUP` (acc used in two branches) ∧ closure is dup'd
(vect-reduce) ∧ the shared body is forced while the closure SUP is still live.
`[1 1]`/`[2 2]` don't leak because both branches treat `acc`/`V` identically,
so the stranded SUPs get erased; `[1 0]` forces `acc` in only one branch.

Also confirmed: `reference-new.c` has the **identical** `duplam`/
`makeLazyDup` — it is not an oracle that avoids this; the bug is fundamental to
the lazy machine as written.

## Open question / likely real fix direction

The leak is that the **shared body** (and thus the captured `V` inside it) is
evaluated once while it needs per-branch independence when one branch resolves
`V` and another erases it. The probable fix is to make the captured location
get **duplicated** (not shared) when a closure is dup'd — i.e. change how
`dupLam`/`makeLazyDup` treat a body that captures a location which can be
resolved in one superposed branch and erased in another. No minimal correct
edit has been found yet; the next concrete step is to log the exact `V`
location through the reduction and confirm the force-then-erase on the same
slot, then scope a duplication fix to that case.

**Confirmed on the simplified 4-node repro (2026-07-17):** the stranded nodes
are 2 SUP(36) pairs, and all 3 `dupSup` EXPANDs are `dup lab=0 + SUP lab=36`
— i.e. the `DUP(0)` over `acc` in the shared body meeting the live closure
SUP(36). The label-correlation and lifecycle-count steps both point at the
`vect-reduce` closure dup (`f_20 = dupeArg(f_20, &f_20_1, 36)`) as the source
of the unbalanced SUPs. The fix direction (duplicate, don't share, the
captured location under a live closure SUP) is unchanged.

## Checklist for a new leak

1. Reproduce: `make <test>`, read the `.rslt` + leak dump. Note the **label(s)**
   of the stranded `SUP`/`DUP` and any `FREE` ports.
2. Convert the dump's **hex** label to decimal, then grep generated C for it →
   find the responsible `dupeArg`.
3. Instrument that node kind's create/annihilate/expand in scratch new.c; count
   the imbalance.
4. Dump the result graph; confirm the stranded node's wiring.
5. If the label/call site is unclear, conditional-break `makePair` in lldb
   (numeric tag, lowercase `-c`); "never hits" rules out that label entirely.
6. Trace the one shared location end-to-end (force vs. erase per branch).
7. Form a hypothesis about *which rule* leaves the node; test the smallest edit
   in scratch; only port to `new.c` once `remaining nodes: 0`.
