# NodeIR redesign: positional `data` vector → multi-ctor deftype

Status: phase 1 (tasks 1-7) done 2026-09-27; phase 2:
tasks 8-9 done 2026-09-27, tasks 10-11 pending
Date: 2026-07-09 (phase 2 added 2026-09-27)

## Goal

Replace the `NodeIR` type's positional `data` vector — a `Vector` whose
element meaning is determined by index, selected by the `kind` string —
with a multi-ctor deftype whose ctors carry named fields, per
docs/toccata-style.md's grouping rule ("grouping related values whose
meaning is determined by position is done only with a `deftype` ctor
with appropriately named fields, accessed via getters").

The redesign is internal to the IR: the generated source (the render
output) must be **byte-identical** before and after. Only
`interpreter/intrp-emit.toc` changes (plus the docs). The grammar,
state, and raw-AST modules are untouched.

## Settled design

### The target types

A single-ctor wrapper carries the char-level classification (one field
on one type — getter-safe); a multi-ctor deftype carries the shape
(named fields per ctor).

```
(deftype NodeIR [node char-level?])

(deftype IRNode
  (CharRange [lo hi])
  (NotChar [ch])
  (Str [lit])
  (Any [any-alts])
  (All [all-parsers])
  (Many [many-child])
  (Rule [rule-name rule-child])
  (Ref [ref-name])
  (Node [node-name node-child])
  (Concat [concat-parsers])
  (Ignore [ignore-child])
  (AlwaysSucceed [as-value])
  (Error [error-msg]))
```

- `node` is the `IRNode` (the shape); `char-level?` is the
  classification (`Some` iff the node classifies a single char). The
  wrapper keeps `char-level?` on exactly one type, so the `char-level`
  helper stays a single getter (no per-ctor dispatch).
- The 13 `IRNode` ctors mirror the grammar's 12 ctors plus `Str` (the
  bare `String` literal, which is the core `String` type in the grammar,
  not a grammar ctor — and a ctor named `String` is unbuildable, core
  collision).
- `char-level?` is set by the builders exactly as today: `CharRange` /
  `NotChar` / `Str` (1 char) / `Any` (all alts char-level) / `Many`
  (child char-level) / `Rule` (child char-level) are `Some`; the rest
  are `None`.

### Field names are unique across every type in the build (settled)

new-toc resolves `.field` getters by name lookup over already-defined
types (docs/new-compiler-plan.md, 2026-09-01), and a `.field` on a
multi-ctor value is **unsafe when several ctors share the field name**.
The grammar is loaded in the same build (`add-ns`), so the `IRNode`
field names must be unique not just across the `IRNode` ctors but across
the grammar and state field names too. The taken set is
`{char, file, input, line, lower, lower-case, msg, name, parser,
parsers, pred, s, state, text, upper, upper-case, value, values}`.

The settled names above are unique against it. In particular:
`any-alts` (not `alts`), `all-parsers` / `concat-parsers` (not
`parsers`), `rule-name` / `ref-name` / `node-name` (not `name`),
`as-value` (not `value`), `error-msg` (not `msg`), `ch` (not `char`).
Because every field name is unique, the render phase can use **direct
`.field` getters** — no per-ctor accessor protocols are needed.

### Ctor names (settled by Task 1)

Primary: mirror the grammar's ctor names (`CharRange`, `NotChar`, `Str`,
`Any`, `All`, `Many`, `Rule`, `Ref`, `Node`, `Concat`, `Ignore`,
`AlwaysSucceed`, `Error`). These live in the emitter namespace; the
grammar's same-named ctors live in the grammar namespace (referenced as
`grammar/X`). Task 1 verifies new-toc accepts same-named ctors in
different namespaces (the C codegen does not emit colliding
identifiers). Fallback if it does not: prefix the `IRNode` ctors with
`Ir` (`IrCharRange`, `IrNotChar`, `IrStr`, …). The field names are
unaffected either way; only the ctor names and the dispatch keys change.

### Dispatch and the string-literal key

The render dispatch (`render-child`) and the value-shape helpers switch
from the `kind` string to `(type-name (.node ir))`. The bare string
literal's key changes from `"String"` to `"Str"` (the ctor name). The
`kind` string field is deleted (redundant — the node's type-name is the
kind).

### No `!` annotations on the multi-field ctors

Per the existing header note and docs/new-compiler-plan.md (2026-08-28),
the `IRNode` ctors carry no `!` annotations (new-toc mis-applies them on
multi-field ctors).

### Transition naming (why `NodeIR2` / `cl?`)

The migration keeps the file load-clean at every step, so the new types
coexist with the old `NodeIR [kind char-level? data]` for a few tasks.
Two hazards shape the transition names:

- The new wrapper cannot be named `NodeIR` while the old one exists
  (same-namespace type collision) → it is `NodeIR2` until the old one is
  deleted.
- The new wrapper's classification field cannot be named `char-level?`
  while the old `NodeIR` has a `char-level?` field (two types with the
  same field name make the `char-level` getter's name lookup
  ambiguous) → it is `cl?` until the old `NodeIR` is deleted.

Task 6 deletes the old `NodeIR` and renames `NodeIR2` → `NodeIR` and
`cl?` → `char-level?`.

## Tasks

Each task ends with the library loading clean (`*** Loaded
interpreter/intrp-emit.toc`, exit 134). Structural edits to the `.toc`
file go through `tools/toc-edit/toc_edit.py` (check → show → edit →
check). Probes sit in `interpreter/` (same module path) so the grammar
is not double-loaded; delete them after.

### Task 1 — Settle the ctor names (probe, no committed changes)

A temp probe in `interpreter/` that `add-ns`es `intrp-state` and
`intrp-grammar` (as `intrp-emit.toc` does), defines a multi-ctor deftype
with the **primary** ctor names (mirroring the grammar) and the settled
field names, and contains one `defn` that constructs each ctor and reads
each field back via a getter (to force the C codegen for both the ctors
and the unique getters). Build it with new-toc.

Resolves two questions:
1. **Ctor-name collision** — does new-toc accept emitter-namespace ctors
   with the same names as the grammar's (loaded via `add-ns`)? If yes,
   the primary names are settled. If the C codegen emits colliding
   identifiers, settle the `Ir`-prefixed set instead.
2. **Getter uniqueness** — do the settled (unique) field names resolve to
   the correct fields (no cross-type ambiguity with the grammar/state
   fields)?

Delete the probe. **Done when:** the ctor-name set (primary or
`Ir`-prefixed) and the field-name set are settled and recorded in the
as-built notes; no committed file changed.

### Task 2 — Capture the pre-redesign render baseline (probe)

Recreate the 4b.1 (9 leaf shapes: CharRange, NotChar, String, Ref,
AlwaysSucceed-String, AlwaysSucceed-Vector, Error, Ignore, Concat) and
4b.2 (3 wrapper shapes: Node, Ignore, Concat) probes against the
**current** `intrp-emit.toc`, run them, and save the emitted source to
`scratch/` (one file per probe). This is the byte-compare target for
Task 5b. Delete the probes (keep the `scratch/` output). **Done when:**
the baseline files exist in `scratch/` and are non-empty.

### Task 3 — Add the new types (unused)

Insert into `intrp-emit.toc` (via `toc_edit`): the `IRNode` multi-ctor
deftype (Task-1 ctor names, settled field names, no `!` annotations) and
the `NodeIR2 [node cl?]` wrapper. The old `NodeIR` stays. Nothing
references the new types yet. **Done when:** `check` exits 0; the
library loads clean.

### Task 4a — Migrate the leaf builders

Rewrite the 6 leaf `ir-*` builders — `ir-char-range`, `ir-not-char`,
`ir-string`, `ir-ref`, `ir-always-succeed`, `ir-error` — to construct
`(NodeIR2 (<IRNode ctor> …) <cl?>)` instead of
`(NodeIR "<kind>" <cl?> [data…])`. These have no IR-node children, so
each edit is a flat ctor + fields rewrite. Migrate the `char-level`
helper to read `(.cl? ir)` (so it works on `NodeIR2` values);
`all-ir-char-level?` is unchanged in body (it calls `char-level`).
Everything else — the composite builders, the `analyze-node` protocol
impls, the `fold`/`recurse`, the `analyze` entry — is unchanged. The
old `NodeIR` deftype stays defined (the not-yet-migrated consumers
still reference `.kind` / `.data` on it). **Done when:** the library
loads clean; a temp probe runs `analyze` over the 6 leaf shapes and the
returned `NodeIR2` values are hand-verified (correct ctor per shape,
named fields correct, `cl?` correct). Delete the probe.

### Task 4b — Migrate the composite builders

Rewrite the 7 composite `ir-*` builders — `ir-any-dispatch`/`ir-any`,
`ir-all`, `ir-many`, `ir-rule`, `ir-node`, `ir-concat`, `ir-ignore` —
the same way. These recurse into child IR nodes, so verify each
builder's child threading against the probe output. Change the
`analyze-node` defp's `! -> NodeIR` annotation → `! -> NodeIR2`. **Done
when:** the library loads clean; a temp probe runs `analyze` over the
5-rule hand-written grammar plus the 13 standalone ctor shapes and the
returned `NodeIR2` values are hand-verified (correct ctor per shape,
named fields correct, `cl?` correct). Delete the probe.

### Task 5a — Migrate the value-shape helpers

Rewrite `ir-value-shape` and `ir-value-arity` to read `NodeIR2`'s named
fields: dispatch on `(type-name (.node ir))`; replace the `.data` reads
with the unique getters (`.any-alts`, `.rule-child`, `.many-child`,
`.as-value`, `.all-parsers`). Self-contained — no render involved, no
baseline needed. **Done when:** the library loads clean; a temp probe
prints shape/arity per shape and the values are hand-verified against
the pre-migration behaviour. Delete the probe.

### Task 5b — Migrate the render dispatch and the 9 render fns

Rewrite to read `NodeIR2`'s named fields instead of `.kind` / `.data`:
- `render-child` — dispatch on `(type-name (.node ir))`; pass
  `(.node ir)` to the per-ctor render fns; the string-literal key is
  `"Str"`.
- the 9 render fns (`render-char-range`, `render-not-char`,
  `render-string`, `render-ref`, `render-always`, `render-error`,
  `render-ignore`, `render-concat`, `render-node`) — replace
  `(extract (get (.data ir) i))` with the unique getters (`.lo`, `.hi`,
  `.ch`, `.lit`, `.ref-name`, `.as-value`, `.error-msg`,
  `.ignore-child`, `.concat-parsers`, `.node-name`, `.node-child`).

The vector-threading helpers (`concat-tail`, `node-value-args`,
`concat-fragment`, `concat-fragments`, `build-join`, `gen-vnames`,
`join-strings`, `vec-arg-exprs`) operate on vectors/strings, not on
`.data` directly — unchanged. Note: after Task 4 the render fns still
reading `.data` load clean but render nothing correct, so this task is
the one that restores the render path; the byte-identical check is its
done-criterion. **Done when:** the library loads clean; the 4b.1 +
4b.2 probes pass with emitted source **byte-identical** to the Task 2
baseline. Delete the probes.

### Task 6 — Delete the old `NodeIR`; rename to the final names

Delete the old `(deftype NodeIR [kind char-level? data])`. Rename the
wrapper `NodeIR2 [node cl?]` → `NodeIR [node char-level?]`. Update the
`char-level` helper's getter (`.cl?` → `.char-level?`) and the
`analyze-node` defp annotation (`! -> NodeIR2` → `! -> NodeIR`). The
`IRNode` ctors are bare names in the emitter namespace, so the type
rename does not touch them. **Done when:** `check` exits 0; the library
loads clean; a grep confirms no `.kind`, `.data`, `cl?`, or `NodeIR2`
references remain.

### Task 7 — Update the docs

Update the Settled section of `docs/parser-generator-plan.md` (the
`NodeIR` shape description), the header comment in
`interpreter/intrp-emit.toc` (the `NodeIR` shape note), and this plan's
as-built notes. **Done when:** the docs reflect the new shape; no stale
reference to the positional `data` vector remains.

## Phase 2 — complete elimination of the `NodeIR` wrapper

Added 2026-09-27, after phase 1 completed.

### Motivation

Phase 1 kept `NodeIR [node char-level?]` as a wrapper to carry the
char-level classification as a stored field. The classification is
fully derivable from the `IRNode` value: it is a per-ctor constant
for 10 of the 13 ctors, and for the rest a function of the
fields/children (`Str` — `lit` is one char; `Any` — every alt is
char-level; `Rule` / `Many` — the child is char-level). Making it a
protocol over `IRNode` removes the wrapper's only field, and with it
the wrapper itself: `analyze` returns a bare `IRNode`, and every
consumer drops a level of indirection.

Accepted trade-off: the stored flag was computed once, bottom-up,
during the fold; the protocol re-derives it on every consultation
(the `Any` impl re-traverses the alts). The IR is grammar-sized and
the classification is consulted a handful of times in the render
(`ir-value-shape`'s `Many` case; the `Many` fast/slow split, item
4b.5) — negligible.

The render output must be **byte-identical** before and after (the
phase-1 baselines are the net).

### Settled design (phase 2)

- `char-level` becomes a **protocol** over `IRNode`, returning a
  boolean exactly like the current `char-level` defn: `(defp
  char-level [n])` + one `extend-type` impl per ctor:
  - `CharRange` / `NotChar` → `true`
  - `Str` → `true` iff `(count .lit)` is 1
  - `Any` → `true` iff every alt in `.any-alts` is char-level (a
    reduce calling `char-level`)
  - `Rule` / `Many` → `(char-level <child>)`
  - `All` / `Ignore` / `Node` / `Concat` / `Ref` /
    `AlwaysSucceed` / `Error` → `false`
  The impls are recursive through the defp (declared before the
  impls — single-pass safe); the recursion is plain calls over
  already-built IR values, no fold, so the defn-handler miscompile
  hazard does not apply. This is the first `extend-type` over the
  `IRNode` ctors — task 8's probe forces the codegen and verifies it.
- The `ir-*` builders return bare `IRNode`s (the wrapper
  construction goes away); `ir-string`'s count branch and
  `ir-any-dispatch`'s flag logic move into the protocol impls
  (`ir-any` becomes a plain `(Any (.parsers v))`).
- `analyze-node`'s annotation → `! -> IRNode`; the `IRNode` child
  fields hold bare `IRNode`s.
- `ir-value-shape` / `ir-value-arity` / `render-child` take bare
  `IRNode`s: the `n (.node ir)` binds and `(.node ir)` unwraps are
  dropped; dispatch on `(type-name ir)`. The 9 per-ctor render fns
  are unchanged (they already take the `IRNode`).
- Deleted: `(deftype NodeIR [node char-level?])`, the `char-level`
  defn (replaced by the protocol), `all-ir-char-level?` (replaced by
  the `Any` impl's reduce).
- Transition name: while the old defn `char-level` exists, the new
  protocol is `ir-char-level?` (same-namespace collision); the
  rename happens in the task that deletes the defn.

### Tasks (phase 2)

Tasks continue the phase-1 numbering. Each ends with the library
loading clean; structural edits go through `toc_edit.py`; probes sit
in `interpreter/` (same module path) and are deleted after.

#### Task 8 — Add the char-level protocol (coexisting, unused)

Add `defp ir-char-level?` + the 13 impls after `all-ir-char-level?`
(single-pass: before the first use in task 9). During the transition
the children are still wrapped, so the `Rule` / `Many` impls unwrap
their child (`(ir-char-level? (.node <child>))`) and the `Any` impl
reuses `all-ir-char-level?` (it reads the wrapped children's stored
flags — correct while the builders still wrap). Nothing on the main
path references the protocol yet.

**Done when:** `check` exits 0; the library loads clean; a temp
probe (the task-4b 5-rule grammar + 13 standalone shapes, with the
recursive printer) verifies `(ir-char-level? (.node ir))` equals
`(char-level ir)` on every node of the folded IR. Delete the probe.

#### Task 9 — Migrate to bare `IRNode`; rename the protocol

- The 13 `ir-*` builders return bare `IRNode`s (no wrapper;
  `ir-string` → `(Str v)`, count branch gone; `ir-any-dispatch`
  gone, `ir-any` → `(Any (.parsers v))`).
- `analyze-node`'s annotation → `! -> IRNode`.
- The protocol impls drop the `.node` unwraps; the `Any` impl gets
  its own reduce over `.any-alts` calling `ir-char-level?`; delete
  `all-ir-char-level?`.
- `ir-value-shape` / `ir-value-arity`: take the bare `IRNode`, drop
  the `n (.node ir)` binds, dispatch on `(type-name ir)`; the
  `Many` case calls `ir-char-level?`; the `All` arity case tests
  `(type-name c)` (no `.node`).
- `render-child`: dispatch on `(type-name ir)`, pass `ir` to the
  per-ctor fns.
- Delete the `char-level` defn (point its one call site —
  `ir-value-shape`'s `Many` case — at `ir-char-level?` first), then
  rename the protocol `ir-char-level?` → `char-level` (defp + impls
  + call sites).
- The `NodeIR` deftype still exists but is unreferenced; it goes in
  task 10.

**Done when:** `check` exits 0; the library loads clean; the 4b.1
(9 shapes) + 4b.2 (3 shapes) probes, recreated to construct bare
`IRNode`s directly, render **byte-identical** to
`scratch/nodeir-baseline-4b1.txt` / `-4b2.txt` (task-5b method:
`pr*` the whole expected output as one unit, strip the runtime-stats
tail and the `typeName:` noise, `cmp`). Delete the probes.

#### Task 10 — Delete the `NodeIR` deftype

Delete `(deftype NodeIR [node char-level?])` + its header comment;
update the migration-history comments that describe the wrapper as
current (the IRNode comment's "multi-ctor replacement for the old
NodeIR's positional data vector" line becomes the full-history
statement: positional `data` vector → wrapper + `IRNode` → bare
`IRNode` with the `char-level` protocol).

**Done when:** `check` exits 0; the library loads clean; a grep
confirms no `NodeIR` reference remains outside historical comments.

#### Task 11 — Update the docs

- `docs/parser-generator-plan.md`, Settled section, the phase-1
  paragraph: the IR is the bare `IRNode`; the char-level
  classification is a protocol over `IRNode` (the classification
  rule itself is unchanged).
- The header comment of `interpreter/intrp-emit.toc`.

**Done when:** no stale reference to the `NodeIR` wrapper as a
current type remains in either doc.

## As-built notes

### Task 1 (ctor names settled) — 2026-09-27

Probe `interpreter/irnode-probe.toc` (deleted after): `add-ns`ed
`intrp-state` and `intrp-grammar`, defined the `IRNode` multi-ctor
deftype with the **primary** ctor names and the settled field names,
plus one `defn` constructing every ctor and reading every field back
via a getter. Built clean under new-toc (`*** Loaded
interpreter/irnode-probe.toc`, exit 134; the first attempt was a
transient segfault with a clean-load stderr, the retry was clean —
the standard missing-main abort path, identical to loading the
existing `intrp-emit.toc`).

1. **Ctor-name collision: no collision.** new-toc accepts
   emitter-namespace ctors with the same names as the grammar's
   ctors (loaded via `add-ns` in the same build). The **primary names
   are settled**: `CharRange`, `NotChar`, `Str`, `Any`, `All`, `Many`,
   `Rule`, `Ref`, `Node`, `Concat`, `Ignore`, `AlwaysSucceed`, `Error`
   (no `Ir` prefix needed).
2. **Getter uniqueness: clean.** All 16 settled field names (`lo`,
   `hi`, `ch`, `lit`, `any-alts`, `all-parsers`, `many-child`,
   `rule-name`, `rule-child`, `ref-name`, `node-name`, `node-child`,
   `concat-parsers`, `ignore-child`, `as-value`, `error-msg`) compiled
   as `.field` getters with no ambiguity against the grammar and
   state field names. The field-name set is settled.

No committed file changed.

### Task 2 (render baseline captured) — 2026-09-27

Baseline files (persistent artifacts on disk, never committed, never
deleted):
- `scratch/nodeir-baseline-4b1.txt` (2375 bytes) — the 9 shapes:
  CharRange [a z], NotChar [x], String ["abc"] (3 chars — exercises
  the N-take-char nesting), Ref [foo], AlwaysSucceed-String ["abc"],
  AlwaysSucceed-Vector [empty vector], Error ["expected a token"],
  Ignore over CharRange, Concat of String ["ab"] + CharRange.
- `scratch/nodeir-baseline-4b2.txt` (2405 bytes) — the 3 wrapper
  shapes: Node ["Symbol", Concat of CharRange a-z + CharRange 0-9],
  Ignore over CharRange, Concat of the two CharRanges.

The probes (in `interpreter/`, deleted after capture) constructed the
IR nodes directly as `(emit/NodeIR <kind> <cl?> <data-vector>)` and
rendered each with `(emit/render-child ir "sv")`, printing a
`SHAPE <name>` label plus each emitted line with an `L<i>` index via
`pr*` (a `reduce` over the line vector). File content: the probe's
whole stdout with the runtime-stats tail (from `- Threads:` onward)
stripped — the ITRS / remaining-node / malloc counts are
run-variable and would false-diff a byte compare. `pr*` adds no
newline, so each file is a single line; lines appear in reverse
creation order (the recorded 4b.1 fact), so each `SHAPE` label sits
AFTER its content line. Every shape renders exactly ONE line (the
wrappers inline their children's lines into the parse-then line), so
only `L0` occurs.

Verified: the library loads clean (`*** Loaded
interpreter/intrp-emit.toc`, exit 134, captured stderr non-empty);
both probes built and ran clean (exit 0, malloc diff 0, remaining
nodes 0); every emitted line hand-verified (skip-at-entry, range /
not-equal preds, the 3-deep take-char nesting, `(foo sv)` ref call,
`empty-vector` constant, the discard fn, the `(str v0 v1)` join in
child order, the `(raw/Symbol v (state/state-line (state/skip-
whitespace sv)))` ctor call with the auto-loc — matching the 4b.1 /
4b.2 as-built notes). Method check: re-running both probes and
diffing against the saved baselines is byte-identical (`cmp` clean
both), so the Task 5b byte-compare target is stable. No deviation
from the plan.

### Task 3 (new types added, unused) — 2026-09-27

Inserted after the old `NodeIR` deftype in `interpreter/intrp-emit.toc`
(29 added lines, nothing else touched — `git diff --stat` confirms): the
`IRNode` multi-ctor deftype (13 ctors, primary names per the Task 1
note, the 16 settled field names, no `!` annotations) and the `NodeIR2
[node cl?]` single-ctor wrapper, each with a header comment noting the
transition names and why. The old `NodeIR` stays; nothing references
the new types yet.

Method: `toc_edit insert` on the `NodeIR` deftype node (path 3,
`--after`) — accepted on the first attempt (exit 0, new-toc validated
the candidate); the splice left one blank-line gap off (no blank after
the old deftype, double blank after the new one), fixed with a
byte-exact Python whitespace-only edit (unowned inter-node whitespace
is not toc_edit-reachable — the known limitation). Verified: `check`
exit 0; library loads clean (`*** Loaded
interpreter/intrp-emit.toc`, exit 134, captured stderr 21 lines,
non-empty). No deviation from the plan.

### Task 4a (leaf builders migrated) — 2026-09-27

Rewrote the 6 leaf builders in `interpreter/intrp-emit.toc` to
construct `(NodeIR2 (<IRNode ctor> ...) <cl?>)`: `ir-char-range` →
`(NodeIR2 (CharRange (.lower v) (.upper v)) (Some None))`;
`ir-not-char` → `(NodeIR2 (NotChar (.char v)) (Some None))`;
`ir-string` → `(NodeIR2 (Str v) (Some None))` for the 1-char case,
`(NodeIR2 (Str v) None)` otherwise; `ir-ref` → `(NodeIR2 (Ref
(.name v)) None)`; `ir-always-succeed` → `(NodeIR2 (AlwaysSucceed
(.value v)) None)`; `ir-error` → `(NodeIR2 (Error (.msg v)) None)`.
The `char-level` helper now reads `(.cl? ir)` (its header comment
updated to say so). `all-ir-char-level?` unchanged (it calls
`char-level`). All 7 node edits (6 defns + the comment) via
`toc_edit replace`, each new-toc-validated (exit 0, first attempt).

Expected mid-migration state (per the plan, confirmed): the composite
builders still construct the old `NodeIR` (their data vectors now
hold `NodeIR2` children), and the render path (still `.kind` /
`.data`) is runtime-broken until tasks 4b/5b — the library still
loads clean, and no committed consumer calls the render path.

Verified: `check` exit 0; library loads clean (`*** Loaded
interpreter/intrp-emit.toc`, exit 134, captured stderr 21 lines,
non-empty). Temp probe `interpreter/ir4a-probe.toc` (deleted after):
`add-ns`ed `emit` + `grammar`, ran `emit/analyze` over the 6 leaf
shapes (CharRange "a" "z", NotChar "x", String "a" and "abc", Ref
"foo", AlwaysSucceed "abc" and `(vector)`, Error "expected a
token"), printing per shape the node type-name, each named field via
its getter, and the `cl?` type-name. Probe build/run recipe
(reusable for 4b/5a/5b): `./new-toc <probe> > tmp` emits the C to
stdout (exit 0) and does NOT run it — then the Makefile's awk `#line`
step, then `clang -g -march=native -I. -DCHECK_MEM_LEAK=1 -DSAFETY=1
-DSTATS=1 -lm -lpthread -latomic probe.c new.c runtime3.c graph.c`
(from the repo root), then run the binary. The probe ran clean:
exit 0, malloc diff 0, remaining nodes 0. All 8 printed lines
hand-verified (output in reverse creation order, read content-wise):
correct `IRNode` ctor per shape, named fields correct (lo=a hi=z;
ch=x; lit=a / lit=abc; ref-name=foo; as-value String "abc" /
Vector []; error-msg="expected a token"), `cl?` `Some` exactly for
CharRange / NotChar / the 1-char Str and `None` for the rest.
No deviation from the plan.

### Task 4b (composite builders migrated) — 2026-09-27

Rewrote the 7 composite builders in `interpreter/intrp-emit.toc` to
construct `(NodeIR2 (<IRNode ctor> ...) <cl?>)`: `ir-any-dispatch` →
`(NodeIR2 (Any cs) (Some None))` / `(NodeIR2 (Any cs) None)` (the
`ir-any` wrapper unchanged); `ir-all` → `(NodeIR2 (All (.parsers v))
None)`; `ir-many` → `(NodeIR2 (Many c) (.cl? c))`; `ir-rule` →
`(NodeIR2 (Rule (.name v) c) (.cl? c))`; `ir-node` → `(NodeIR2 (Node
(.name v) (.parser v)) None)`; `ir-concat` → `(NodeIR2 (Concat
(.parsers v)) None)`; `ir-ignore` → `(NodeIR2 (Ignore (.parser v))
None)`. The `analyze-node` defp annotation is now `! -> NodeIR2`. All
9 node edits (7 defns + the defp + the container-ctors comment) via
`toc_edit replace`, each new-toc-validated.

DEVIATION from the plan's prediction (one line, forced by the actual
code): the plan said `all-ir-char-level?` is "unchanged in body (it
calls `char-level`)" — it does NOT call the `char-level` helper (that
helper is defined later in the file; new-toc is single-pass, so the
call would not compile); it read `(.char-level? ir)` directly. After
Task 4a its inputs are `NodeIR2` values, whose field is `cl?` (and
`.char-level?` resolves to the OLD `NodeIR`'s field — a mis-read on a
`NodeIR2`), so the body had to change to `(.cl? ir)` for `ir-any` to
work at all. Everything else matches the plan.

Verified fact (settles the child-field typing the plan left implicit):
the `IRNode` child fields (`any-alts`, `all-parsers`, `many-child`,
`rule-child`, `node-child`, `concat-parsers`, `ignore-child`) hold
**NodeIR2 wrappers** (the child IRs), not bare `IRNode`s — the same
child threading as the old `.data` vectors. The plan's Task 5a/5b
readings already implied this (`ir-value-shape (extract (get
(.any-alts ir) 0))` then dispatches on `(.node …)`), but a probe
printer that assumed bare `IRNode` children aborts with
`print-node: unknown ctor: NodeIR2` — the first 4b probe did exactly
that and had to be fixed (children printed via a `print-child` helper
that reads `(.node c)` / `(.cl? c)`).

Expected mid-migration state (per the plan, confirmed): the render
path (still `.kind` / `.data`) is runtime-broken — it now mis-reads
`NodeIR2` values — but the library loads clean and no committed
consumer calls it; tasks 5a/5b restore it.

Verified: `check` exit 0; library loads clean (`*** Loaded
interpreter/intrp-emit.toc`, exit 134, captured stderr 21 lines,
non-empty). Temp probe `interpreter/ir4b-probe.toc` (deleted after):
`add-ns`ed `emit` + `grammar`, ran `emit/analyze` over a 5-rule
grammar covering all 13 ctors (r1 Rule+CharRange; r2 Rule+Any of
CharRange+NotChar; r3 Rule+Node+Concat+Ref+Many; r4 Rule+All+Ignore+
Ref; r5 Rule+Any of AlwaysSucceed+Error) plus the 13 standalone ctor
shapes (CharRange, NotChar, 3-char String, Ref, AlwaysSucceed-String,
AlwaysSucceed-empty-Vector, Error, Any, All, Many, Node, Concat,
Ignore), printing per shape the `IRNode` ctor, every named field via
its getter, and the `cl?` type-name, recursing into child IRs. The
printer's mutually recursive defns needed crutch declarations (`(def
print-node)` etc. — single-pass; the established `render-child`
pattern). The probe's first build was a transient segfault (clean
load, no error message); the retry was clean — the standard policy.
Ran clean: exit 0, malloc diff 0, remaining nodes 0. All 18 printed
lines hand-verified (output in reverse creation order): correct ctor
per shape; named fields correct (lo/hi, ch, lit, ref-name, rule-name,
node-name, error-msg, as-value type+count); `cl?` `Some` exactly for
CharRange / NotChar / the char-level Any (r2, s8) / Many-over-
CharRange (s10) and their delegating Rules (r1, r2), `None` for all
the rest — including the 3-char Str, the non-char-level Any (r5), and
Node / Concat / All / Ignore / Ref / AlwaysSucceed / Error. No other
deviation from the plan.

### Task 5a (value-shape helpers migrated) — 2026-09-27

Rewrote `ir-value-shape` and `ir-value-arity` in
`interpreter/intrp-emit.toc` to read `NodeIR2`'s named fields: both
bind `n (.node ir)` once and dispatch on `(type-name n)`; the
string-literal key is `"Str"` (was `"String"`); the `.data` reads are
the unique getters — shape: `.any-alts` (Any, `extract (get … 0)`),
`.rule-child` (Rule), `.many-child` (Many, via the `char-level`
helper), `.as-value` (AlwaysSucceed); arity: `.all-parsers` (All,
reducing over the non-Ignore children, detected by
`(str= (type-name (.node c)) "Ignore")`), `.any-alts` (Any),
`.rule-child` (Rule). The `ir-value-shape` header comment's leaf list
updated (`String` → `Str`). Both defns via `toc_edit replace`
(new-toc-validated); each splice left one extra blank line (unowned
inter-node whitespace — the known limitation), fixed with byte-exact
Python whitespace-only edits. `check` exit 0.

DEVIATION from the plan's wording (one structural point, forced by the
actual types): the task text (and the Task 4b note quoting it) shows
the getters applied to the wrapper — `(.any-alts ir)`. That is a
RUNTIME dispatch failure: the 16 named fields live on the `IRNode`
ctors, not on the `NodeIR2` wrapper, so `(.any-alts ir)` on a `NodeIR2`
aborts with `No implementation of '.any-alts' found for type NodeIR2`
(verified with a minimal probe before the fix). The getters are
applied to `(.node ir)` — bound once as `n` at the top of each fn.
Same correction applies to Task 5b's render fns: the unique getters
(`.lo`, `.hi`, …) take `(.node ir)`, not `ir`.

Verified: `check` exit 0; library loads clean (`*** Loaded
interpreter/intrp-emit.toc`, exit 134, captured stderr 21 lines,
non-empty). Temp probe `interpreter/ir5a-probe.toc` (deleted after):
`add-ns`ed `emit`, constructed 12 `NodeIR2` shapes directly
(CharRange, NotChar, 3-char Str, Concat, All of 3 incl. an Ignore,
Any, Rule-over-All, Any-over-Rule-over-All, Many-over-CharRange,
Many-over-Any, AlwaysSucceed-String, AlwaysSucceed-Vector) and printed
`ir-value-shape` for all 12 plus `ir-value-arity` for the three
vector-shaped ones. First two builds were transient segfaults (clean
load, no error message); the third was clean — the standard policy.
Ran clean: exit 0, malloc diff 0, remaining nodes 0. All 15 lines
hand-verified against the pre-migration behaviour: `string` for
CharRange / NotChar / Str / Concat / Any-over-Str / Many-over-
CharRange (char-level fast path) / AlwaysSucceed-String; `vector` for
All / Rule-over-All / Any-over-Rule / Many-over-Any (slow) /
AlwaysSucceed-Vector; arities 2 (All with one Ignore excluded), 2
(Rule→All), 1 (Any→Rule→All with one Ignore excluded). Note:
`type-name` prints a `typeName: <name>` debug line per call under
`-DSTATS=1` — expected noise in probe output, not a result. No other
deviation from the plan.

### Task 5b (render dispatch + 9 render fns migrated) — 2026-09-27

Rewrote `render-child` and the 9 render fns in
`interpreter/intrp-emit.toc` to read `NodeIR2`'s named fields:
`render-child` binds `k (type-name (.node ir))` and passes `(.node ir)`
(the bare `IRNode`) to each per-ctor fn; the string-literal key is
`"Str"` (was `"String"`). The render fns now take the `IRNode` and read
the unique getters directly: `.lo`/`.hi` (char-range), `.ch`
(not-char), `.lit` (string), `.ref-name` (ref), `.as-value` (always),
`.error-msg` (error), `.ignore-child` (ignore — passed to
`render-child`), `.concat-parsers` (concat), `.node-name`/`.node-child`
(node). The vector-threading helpers (`concat-tail`, `node-value-args`,
`concat-fragment(s)`, `build-join`, `gen-vnames`, `join-strings`,
`vec-arg-exprs`) are unchanged. All 10 defns via `toc_edit replace`
(each new-toc-validated); two first attempts (render-ignore,
render-node) were rejected with `Missing ")"` — my snippets were each
missing one closing paren (the paren-heavy-line hazard, AGENTS.md);
fixed the snippets and re-ran. The splices left extra blank lines
(unowned inter-node whitespace — the known limitation), collapsed with
a byte-exact whitespace-only edit (this also removed double blanks
left by the task 4a/4b/5a splices).

No deviation from the plan: the Task 5a correction (getters apply to
`(.node ir)`, not the wrapper) was applied as predicted — the render
fns take the `IRNode` that `render-child` passes them, and the getters
apply to that parameter.

Verified: `check` exit 0; library loads clean (`*** Loaded
interpreter/intrp-emit.toc`, exit 134, captured stderr 21 lines,
non-empty). Recreated the 4b.1 (9 shapes) and 4b.2 (3 shapes) probes
(in `interpreter/`, deleted after) against the migrated path,
constructing `NodeIR2` values directly and rendering via
`emit/render-child`. Each probe builds the WHOLE expected baseline
output (shapes in reverse creation order; each shape's single emitted
line + `L0` + `SHAPE <name>`; trailing newline) as ONE string and
`pr*`s it as a single unit — see the new fact below: multi-unit probe
output is no longer byte-comparable now that the render path's
type-name dispatch interleaves newline-bearing debug noise. Both ran
clean: exit 0, malloc diff 0, remaining nodes 0. With the
runtime-stats tail (from `- Threads:`) and the `typeName:` noise
stripped, the output is byte-identical to
`scratch/nodeir-baseline-4b1.txt` (2375 bytes) and
`scratch/nodeir-baseline-4b2.txt` (2405 bytes) — `cmp` clean on both.

New durable facts (also appended to new-compiler-plan.md Verified
facts):
- `type-name` prints a `typeName: <name>\n` debug line to stdout
  UNCONDITIONALLY — confirmed present in a probe build with only
  `-DCHECK_MEM_LEAK=1 -DSAFETY=1` (no `-DSTATS`); the Task 5a note
  attributed it to `-DSTATS=1`, it is not gated by that flag.
- The `pr*` unit reversal (the recorded 4b.1 fact) holds only for a
  clean unit stream: once newline-bearing debug printing is
  interleaved (the `typeName:` noise), the multi-unit output order is
  no longer a plain unit reversal (labels and content lines come out
  separated) and the noise lines themselves are duplicated/dropped.
  Byte-compare probes must `pr*` the whole expected output as a
  single unit (a single unit prints atomically and in order).

### Task 6 (old NodeIR deleted, final names applied) — 2026-09-27

Deleted `(deftype NodeIR [kind char-level? data])` and its header
comment; renamed the wrapper `NodeIR2 [node cl?]` → `NodeIR [node
char-level?]` (its comment updated to the final form, reusing the old
NodeIR comment's classification description); updated every reference
— `all-ir-char-level?` (`.cl?` → `.char-level?`), the 13 `ir-*`
builders (`NodeIR2` → `NodeIR`; `ir-many` / `ir-rule` also `.cl? c` →
`.char-level? c`), the `analyze-node` defp annotation (`! -> NodeIR`),
the `char-level` helper + comment (`.cl?` → `.char-level?`), and the
stale comments (the IRNode comment's "Unused until the builders
migrate (tasks 4a/4b)" line dropped; the container-ctors comment's
"NodeIR2s" → "NodeIRs").

DEVIATION from the plan's implied order (forced by new-toc's
single-pass symbol resolution): the rename cannot be "delete the old
dtype, rename the wrapper" — the direct rename edit was REJECTED
(`*** Undefined symbol: 'NodeIR2' at <file>: 76`): the builders still
reference `NodeIR2`, and updating the references before the deftype
exists fails identically. What was done instead: (1) delete the old
`NodeIR` + its comment (nothing references it — no `.kind` / `.data`
/ `.char-level?` uses remained after 5b), (2) `insert` the new
`(deftype NodeIR [node char-level?])` before the `NodeIR2` deftype
(both coexist, loads clean), (3) replace all 16 reference nodes, (4)
`delete` the `NodeIR2` deftype. All 21 node edits via toc_edit
(insert / replace / delete), each new-toc-validated; the insert splice
left the two deftypes on one line and the deletes left double blank
lines — both fixed with byte-exact whitespace-only edits (unowned
inter-node whitespace, the known limitation).

Verified: `check` exit 0; library loads clean (`*** Loaded
interpreter/intrp-emit.toc`, exit 134, captured stderr 21 lines,
non-empty); grep confirms no `.kind`, `.data`, `cl?`, or `NodeIR2`
reference remains in the file. Temp probe `interpreter/ir6-probe.toc`
(deleted after): `add-ns`ed `emit` + `grammar`, ran `emit/analyze`
over `grammar/CharRange "a" "z"` and `grammar/Rule "r" (grammar/Many
(grammar/CharRange "0" "9"))`, printed the `IRNode` ctor + the
`char-level?` type-name per shape, and rendered the CharRange via
`emit/render-child`. Ran clean: exit 0, malloc diff 0, remaining
nodes 0. Output hand-verified: s1 ctor=CharRange cl=Some; s2
ctor=Rule cl=Some (delegation through Many-over-CharRange); the
rendered line matches the 4b.1 CharRange baseline shape (skip-at-
entry, one-char test, range pred, take-char, the error message).
New durable fact (mirrored to new-compiler-plan.md Verified facts):
the deftype-rename hazard above. No other deviation from the plan.

### Task 7 (docs updated) — 2026-09-27

Updated the two docs the task names. (1) The Settled section of
`docs/parser-generator-plan.md`, the Phase 1 (analyze) paragraph: the
final sentence ("The IR carries classification … plus the structure
the render needs") now states the final shape explicitly — `NodeIR
[node char-level?]`, `node` the `IRNode` multi-ctor deftype (13 ctors
mirroring the grammar's, bare `String` → `Str` for the core
collision), named fields unique across every type in the build (the
render phase uses direct `.field` getters), the `char-level?`
classification rule. (2) The header comment of
`interpreter/intrp-emit.toc` (node 0, a comment node — `toc_edit
replace` with the full new comment text, new-toc-validated): the
"IR carries the classification … raw fields on leaves, child IRs on
containers" sentence now describes `NodeIR [node char-level?]` and
the `IRNode` multi-ctor deftype, cross-referencing this plan. The
historical DEVIATION paragraph in the header and the IRNode
deftype's "multi-ctor replacement for the old NodeIR's positional
data vector" note are migration-history, not shape descriptions —
left as-is. No other file touched; the Phase 2 ctor table's "bare
`String`" row refers to the grammar's ctor (unchanged) and was
left.

Verified: `check` exit 0; library loads clean (`*** Loaded
interpreter/intrp-emit.toc`, exit 134, captured stderr 21 lines,
non-empty); grep confirms no stale reference to the positional
`data` vector (no `[kind`, `data vector`, `positional`, `NodeIR2`,
or `` `data` `` in the Settled section of parser-generator-plan.md
or the intrp-emit.toc header — the only "positional data vector"
mention left in the .toc file is the IRNode comment's
migration-history line). No deviation from the plan. This completes
Tasks 1–7.

### Task 8 (char-level protocol added, coexisting, unused) - 2026-09-27

The working tree already contained the protocol (uncommitted, from an
interrupted task-8 start): `defp ir-char-level?` + impls after
`all-ir-char-level?`, exactly per the settled design (CharRange /
NotChar -> `(Some None)`; Str -> `(Some None)` iff `(count (.lit n))`
is 1; Any -> `(all-ir-char-level? (.any-alts n))` - reuses the stored
flags of the still-wrapped children; Many / Rule ->
`(ir-char-level? (.node <child>))`; All / Ref / Node / Concat /
Ignore / AlwaysSucceed / Error -> `None`). One fix: the tree carried
an extra 14th impl for the `NodeIR` wrapper itself - deleted it
(`toc_edit delete`, new-toc-validated; the splice's blank-line residue
collapsed with a byte-exact whitespace-only edit - unowned inter-node
whitespace, the known limitation). The settled design is one impl per
`IRNode` ctor (13); the wrapper impl is not in it and would have had
to be deleted in task 10 with the deftype.

Verified: `check` exit 0; library loads clean (`*** Loaded
interpreter/intrp-emit.toc`, exit 134, captured stderr 21 lines,
non-empty). Temp probe `interpreter/ir8-probe.toc` (deleted after):
`add-ns`ed `emit` + `grammar`, ran `emit/analyze` over the task-4b
5-rule grammar (all 13 ctors) plus the 13 standalone ctor shapes, and
a recursive checker walked each folded IR as a PURE computation -
`check-node` returns `(vector total bad)`: total counts every node
visited, bad counts the nodes whose stored `.char-level?` flag
disagrees with `(emit/ir-char-level? (.node ir))` - and `walk` prints
ONE summary line per shape. All 18 lines hand-verified: the totals
match the hand count exactly (r1=2, r2=4, r3=6, r4=5, r5=4, s1..s7=1
each, s8=3, s9=4, s10=2, s11=2, s12=3, s13=2 - 44 nodes overall) and
`bad=0` on every line - the protocol equals the stored flag on every
node of the folded IR. Ran clean: exit 0, malloc diff 0, remaining
nodes 0. Build recipe as in the Task 4a note (new-toc to stdout,
awk `#line`, clang; first attempt clean).

New fact: the first probe version printed one `pr*` line per node from
the recursive walker, and the stream SILENTLY DROPPED UNITS -
deterministically (stable md5 across runs) the FIRST child's line in
every multi-element vector walk was missing (8 of 44 lines), while the
lines that did print all showed correct stored/proto values. The
recorded multi-unit output-ordering hazard (Task 5b note) extends to
DATA units, not just the `typeName:` noise: a per-node `pr*` stream is
not a reliable verification channel; a pure computation (count/sum
returned as the value, one summary `pr*` per shape) is. No other
deviation from the plan.

### Task 9 (migrated to bare IRNode; protocol renamed) — 2026-09-27

Migrated the whole file to the bare `IRNode` and renamed the protocol.
The 13 `ir-*` builders return bare `IRNode`s (no wrapper): `ir-string`
→ `(Str v)` (the 1-char count branch gone), `ir-any-dispatch` deleted
and `ir-any` → `(Any (.parsers v))`, `ir-many` → `(Many (.parser v))`,
`ir-rule` → `(Rule (.name v) (.parser v))` (both drop the `let [c …]` /
`.char-level? c` delegation — the flag is the protocol's now); the rest
are flat `(Ctor <fields>)`. `analyze-node`'s annotation → `! -> IRNode`
(and the `analyze` comment's `-> NodeIR` → `-> IRNode`). The protocol
impls drop the `.node` unwraps (Many / Rule read `(.many-child n)` /
`(.rule-child n)` directly); the `Any` impl gets its own reduce over
`.any-alts` calling the protocol; `all-ir-char-level?` deleted (with its
comment). `ir-value-shape` / `ir-value-arity` take the bare `IRNode`
(the `let [n (.node ir)]` binds dropped, dispatch on `(type-name ir)`);
the `Many` case calls the protocol; the `All` arity case tests
`(type-name c)` (no `.node`). `render-child` dispatches on
`(type-name ir)` and passes `ir` to the per-ctor fns (the 9 fns are
unchanged — they already take the `IRNode`). The `char-level` defn is
deleted (its one call site — `ir-value-shape`'s `Many` case — pointed at
the protocol first), then the protocol is renamed `ir-char-level?` →
`char-level`. The `NodeIR` deftype still exists but is now
unreferenced; it goes in task 10.

DEVIATION from the settled design (one point, forced by the language):
the design says the protocol returns a **boolean** (`true` / `false`)
"exactly like the current `char-level` defn". But `true` and `false`
are NOT defined symbols in the new-toc build — a probe returning them
is rejected with `*** Undefined symbol: 'true'` (verified before the
edit). The codebase's boolean idiom is a comparison result (the old
`char-level` defn returned `(str= … "Some")`), not a literal. So the
protocol keeps the **Maybe** representation task 8 built (`(Some None)`
iff char-level, else `None`) — which is also exactly what task 9's
instructions describe (drop the `.node` unwraps; the `Any` reduce;
delete `all-ir-char-level?`). Verified with a probe that `cond` treats
`(Some None)` as truthy and `None` as falsy (and `and` likewise), so
`ir-value-shape`'s `Many` case works unchanged. The task 8 as-built
note's "exactly per the settled design" claim was about the shape, not
the return type; the return type is the one place the settled-design
text and the buildable language disagree.

Method: all structural edits via `toc_edit` (replace / delete), each
new-toc-validated. The protocol rename `ir-char-level?` → `char-level`
(19 occurrences: defp + 13 impls + recursive calls + the `ir-value-
shape` call site + the comment) was a **global byte-exact token
replace** (a Python `str.replace`), done AFTER the old `char-level`
defn was deleted so the name was free. A one-node-at-a-time rename is
not `toc_edit`-doable: the defp and its `extend-type` impls must agree
on the protocol name, so there is NO load-clean intermediate state
(rename the defp first → impls reference an undefined protocol; rename
the impls first → they `extend-type` the old `char-level` defn, a
non-protocol). The rename is a pure symbol substitution (no span
surgery), so the byte-exact replace is the right tool; `check` was run
after.

New facts: (1) `true` / `false` are not defined symbols (see the
deviation). (2) A **comment node's span includes its trailing
newline** — replacing a comment node with a snippet that has no trailing
newline merges the comment onto the next line (`*** Error … Invalid
expression`); the snippet must end in `\n`. (Defn-node spans do NOT
include the following blank line, so defn snippets need no trailing
newline.) (3) The protocol-rename hazard above (no valid intermediate
state).

Verified: `check` exit 0; library loads clean (`*** Loaded
interpreter/intrp-emit.toc`, exit 134, captured stderr non-empty);
grep confirms no `.node` reads remain (only the `.node-child` /
`.node-name` field getters in `render-node`), no `.char-level?` reads
outside the `NodeIR` deftype field + header comment, no
`all-ir-char-level?` / `ir-any-dispatch` / `NodeIR2` / `cl?` / `.kind`
/ `.data`. Recreated the 4b.1 (9 shapes) and 4b.2 (3 shapes) probes
(in `interpreter/`, deleted after) to construct **bare `IRNode`s
directly** (`emit/CharRange "a" "z"`, …) and render each via
`emit/render-child ir "sv"`; each builds the whole expected baseline
output as ONE string (shapes in reverse creation order; each shape's
single `L0` line + `SHAPE <name>`) and `pr*`s it as a single unit
(task-5b method). Both ran clean: exit 0, malloc diff 0, remaining
nodes 0. With the `typeName:` noise and the runtime-stats tail (from
`- Threads:`) stripped, the output is byte-identical to
`scratch/nodeir-baseline-4b1.txt` and `-4b2.txt` — `cmp` clean on
both. (The baseline files end with a single trailing `\n` — a
file-creation artifact, since `pr*` adds no newline; the strip
re-appends it for the compare. The rendered content itself is
newline-free and matches byte-for-byte.) No other deviation from the
plan.
