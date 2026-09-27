# NodeIR redesign: positional `data` vector → multi-ctor deftype

Status: planned
Date: 2026-07-09

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
