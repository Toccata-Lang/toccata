# Parser Generator Plan (grammar data → recursive-descent reader)

Status: v1 emitter built and verified through item 7c; item 8's
blocker (the inlined-cond codegen crash) resolved by the grouped-
literal site-(a) form (item-8 note, UPDATE 2). REWRITE DIRECTED
(2026-09-26): the owner judged the v1 emitter horrible (its
explicit-recursion / `-acc` threading exists largely for the
reduce-capture leak, now RESOLVED) and directed a fold-based
rewrite (emitter v2) on hvm-core's `fold` recursion scheme —
items 10–18; v1 items 3–7c are superseded. Settled items are final
until re-opened; open items are queued. Companion to
`docs/new-compiler-plan.md` — this is a side project (dev-time
tooling), not a phase of the compiler plan.

## Goal

A generator, written in Toccata and built by `new-toc`, that takes the
parser-combinator grammar (data) and emits a recursive-descent parser
module in Toccata source for that grammar. The grammar is the `67a1125`
version of `intrp-rdr.toc` (the combinator approach: `ParserState`,
`ParserResults`, `ParserCombinator` deftype + rules as data; commit
found via git history 2026-09-04, last combinator commit before the
recursive-descent rewrite `807e3e1`). The emitted module follows the
conventions of the hand-written `interpreter/intrp-rdr.toc`
(remaining-string `ParserState`, maximal-run reads via `read-run`,
`ParserResults`).

Why a generator rather than a combinator engine: the compiler plan's
performance objection to the combinator approach was that the *engine*
builds the whole grammar structure at every compiler startup. The
generator is a dev-time step: grammar-as-data lives only in this
project; the output is plain compiled parse functions with the same
startup property as the hand-written rdr. It is also a cleaner version
of the compiler plan's "EBNF / grammar tooling" item — that item reads
and re-parses parser *source* (`intrp-ebnf.toc`, broken); this generates
from grammar *data* directly.

Scope: the `67a1125` grammar covers expressions only — int/float/
string/symbol/call (the vector/hash rules are commented out in the
source; no top-level forms, `let`, `cond`, threading, defp/deftype
rules). The generated parser collects token **text** (strings / vectors
of strings) — no AST, no desugarings (those are a separate layer on
top, as in the hand-written rdr). Extending the grammar data to the
full language is out of scope; the emitter would change little, the
grammar data would grow.

Relationship to the compiler plan: the hand-written
`interpreter/intrp-rdr.toc` remains the phase-1 reader. If the grammar
were later extended to the full language, a generated parser could
replace it — speculative, not planned.

## Files

- `interpreter/intrp-grammar.toc` — the grammar data: the `67a1125`
  content, ported to compile clean under new-toc (library, no main).
  Source: the top-level `intrp-rdr.toc` fetched from `67a1125`
  (untracked). It is MOVED (renamed — it cannot share a name with the
  hand-written `interpreter/intrp-rdr.toc` in the same directory) and
  the top-level copy deleted. One spelling of the module in the graph.
- `interpreter/intrp-emit.toc` — the emitter (library, no main;
  rewritten IN PLACE for v2 by items 10–16 — the v1 source stays in
  git history).
- `interpreter/emit-*.toc` — acceptance drivers (committed, same
  convention as the `rdr-*` drivers).
- `interpreter/gen-rdr.toc` — the generated module. Written by the
  driver (inline-C file write), built by a Makefile target, a build
  artifact — NOT committed.
- Throwaway probes: `scratch/` (never committed from there).

## Settled design

### Emitter API — v2, fold-based (settled 2026-09-26)

The emitter `add-ns`es the grammar module (for the `ParserCombinator`
values) and the hand-written rdr module (for shared helpers such as
`vect-concat`; define locally instead if the dependency is unwanted —
implementer's choice, but keep module path spellings identical).

The v1 design (the `emit-body` / `emit-pred` / `emit-many` /
`emit-ref` / `emit-fn` defp protocols, the flipped-receiver
`emit-many`, the `walk-children` / `contains-recur?` /
`alt-char-level?` pre-passes, the `-acc` line-building recursions)
is RETIRED (2026-09-26, owner decision): its convoluted shape
existed largely to route around the reduce-capture leak, which is
RESOLVED (2026-09-26, probed — Inherited verified facts), and the
recursion scheme the rewrite needs already exists: `fold` /
`unfold` (hvm-core.toc:586–599) over the `recurse` container
protocol (hvm-core.toc:123), with `ParserCombinator` implementing
`recurse` for every ctor (item 2). v2 is two phases.

**Phase 1 — analyze (the catamorphism).** `analyze [pc]` =
`(fold pc h)` — the core `fold`, no new walker. `h` is ONE function
dispatching on `type-name`; after `recurse` reassembles a node, the
CONTAINER ctors' fields hold the children's `NodeIR` values while
the LEAF ctors hold their raw fields — `h` knows which by ctor.
`Recur` is a `recurse`-leaf, so the fold terminates structurally
even though the generated parser is recursive. The IR is
classification + structure only — NO names, NO source lines:

```toccata
;; kind        — the bare ctor name ("All", "Many", "CharRange",
;;               "NotChar", "Error", "AlwaysSucceed", "Ignore",
;;               "Rule", "Recur"), or "String" for a bare string
;;               literal
;; char-level? — Some None iff this node classifies a single char:
;;               CharRange / NotChar / one-char String; Any iff ALL
;;               alts char-level; Rule / Many iff the child is;
;;               All / Ignore never
;; pred        — [String] char-predicate source (char-level nodes
;;               only; [] otherwise)
;; data        — ctor-specific: CharRange [lo hi]; NotChar [ch];
;;               String [lit]; Error [msg]; AlwaysSucceed [v];
;;               All / Any [NodeIR ...]; Many / Ignore [NodeIR];
;;               Rule [name NodeIR]; Recur []
(deftype NodeIR [kind char-level? pred data])
```

The classification is BOTTOM-UP, so the v1 pre-passes disappear:
a `Many`'s fast/slow path IS its child IR's `char-level?`; an
`Any`'s combined predicate reads its alts' flags. (The v1
`needs-recur` pre-pass is dropped outright — the `(def <rule>)`
crutches are emitted for EVERY rule, so no flag is needed; see the
Generated module template.)

**Phase 2 — render (top-down over the IR).** A plain `defn` walks
the IR with the context (the enclosing rule name + the helper-name
prefix) and emits source per the Ctor table: the index-path helper
names are assigned here (names are top-down, so the bare fold stays
name-free — no placeholder rewriting), a `Recur` renders as a call
to the context's rule name; the `(def <rule>)` crutches are
module-level (every rule — see the Generated module template).
Line assembly is plain vector append — no
`-acc` threading, no flipped-receiver protocol. (An IR flag is
DATA the render inspects, not a tag-shaped protocol impl — the
Result-discrimination section's objection to tag protocols does not
apply.)

**Driver-facing API (unchanged from v1 — the driver and Makefile
take no edits):**

```toccata
(deftype EmitCtx [rule prefix])
;; The char-level predicate source for pc, as a String (the
;; driver's write-file contract). Aborts on a non-char-level pc.
(defn emit-pred [pc ctx])
;; The full generated module as [String] lines: header, (def)
;; crutches, one parser defn + helpers per Rule, main template.
(defn emit-module [rules entry])
```

**Incremental swap (settled; mechanics clarified 2026-09-26,
ralph review):** during items 12–14a the render dispatches per
ctor and falls back to the v1 emission for ctors not yet ported
(the v1 code stays in the file); item 16 deletes it. The render
walks the ORIGINAL combinator and its IR in LOCKSTEP (the entry
points have both — the IR is isomorphic to the combinator except
for the Recur `f` field, which the v1 emission ignores), because
the v1 functions take combinator values, not IRs: a ported ctor
renders from the IR; an unported one delegates its subtree to the
v1 defns `emit-rule-block` calls at that position, with the same
context. Where v1's `emit-rule-block` gives a Rule-over-X shape a
special treatment (the Many branch — loop defn + one-arg wrapper),
the whole rule block falls back to v1. Item 13 adds one shape-level
fallback inside the ported `Any`: an Any with ≥2 bare-String alts
delegates to the v1 grouped emission until item 13a. `make
emit-pred` (all 13 diffs) must pass at the end of EVERY item
10–16.

**Regression oracle (settled):** v2 must reproduce the committed
expected files BYTE-EXACTLY — the 13 files in
`interpreter/emit-want/` (six predicates, six synthetic modules,
module-real) and the item-8 corpus `-want` files. The generated-
CODE contract (the Ctor table, the naming, the templates, the
grouped-literal site-(a) form, the crutches) is unchanged by the
rewrite — only the emitter's internals move. A failed diff is a v2
contract bug by default (fix the emitter); a deliberate output
change requires the owner to update the expected file and record it
in the as-built note.

### Char-level vs parser-level

A combinator is **char-level** iff it classifies a single character:
`CharRange`, `NotChar`, bare `String` (one char), `Any` of char-level
alts, and `Rule`/`Many` whose child is char-level (they delegate).
In v2 the classification is computed bottom-up in `analyze` and
carried on the IR (`char-level?`); `emit-pred`'s abort on a
non-char-level node stays the emitter-bug signal. `Many` of a
char-level child is the **maximal-run fast path** (one generated
set-predicate defn + one `read-run`), not a loop.

### Ctor → generated code

Generated code assumes the helper layer (from
`interpreter/intrp-rdr.toc`): `ParserState`, `ParserResults`
(`ParserMatch`/`ParserIgnore`/`ParserError` — no `ParserFail`),
`Token`, `make-state`, `take-char`, `skip-whitespace`, `read-run`,
`str-prefix?`, `state-line`, and the `parse-then` / `parse-or` kit
from the Result-discrimination section.

In v2 this table drives the RENDER phase (the column names are the
v1 protocol names — the render computes the same source).

| Ctor | `emit-pred` (over char string `c`) | `emit-body` (given let-bound `state`) |
|---|---|---|
| `CharRange [lo hi]` | `(and (<= LO (char-code c)) (<= (char-code c) HI))` — LO/HI evaluated by the emitter at emit time (`char-code` + `str*` over the Integer) | one-char parser: `(let [input (.input state) c (subs input 0 1)] (cond (str= c "") (ParserError "unexpected end of input" state) <pred> (ParserMatch c (take-char state)) (ParserError "unexpected character" state)))` |
| `NotChar [ch]` | `(not (str= c CH))` — `not` + `str=` are Maybe-flavored (see the booleans fact in Inherited verified facts) | one-char parser as above with the NotChar pred |
| bare `String` (literal) | `(str= c S)` | one-char literal match, LET-FREE (inlinable in a cond clause): `(cond (str-prefix? S (.input state)) (ParserMatch S (take-char state)) (ParserError "expected S" state))` |
| `Any [ps]` | `(or <pred1> ... <predN>)` — `emit-pred` over each alt; a non-char-level alt hits the default abort | nested `parse-or` per the site-(a) template; anonymous alts lifted to helpers first |
| `All [ps]` | — (default abort) | nested `parse-then` per the site-(c) template: `(parse-then (<ref0> state) (fn [v0 s1] (parse-then (<ref1> s1) (fn [v1 s2] ... (ParserMatch [v0 ... vN] sN)))))` |
| `Many [p]` | — (default abort) | path chosen by `emit-many` (flipped receiver — see Emitter API). Char-level child → FAST PATH: emit a set-predicate defn `<name>-char` from the child's `emit-pred` + a run wrapper `(defn <name> [state] (let [t (read-run state <name>-char)] (ParserMatch (.text t) (.state t))))` — note the `Token` → `ParserMatch` wrap; body expr `(<name> state)`. General child → SLOW PATH: lift the child to a named fn if anonymous, then the site-(b) loop `(defn <name> [state acc] (parse-or (parse-then (<child> state) (fn [v s2] (<name> s2 (vect-conj acc v)))) (fn [e] (ParserMatch acc state))))`; body expr `(<name> state empty-vector)` |
| `Ignore [p]` | — (default abort) | site-(c) shape: `(parse-or (parse-then (<ref> state) (fn [v s2] (ParserIgnore s2))) identity)` — the fn captures nothing; no helper needed |
| `AlwaysSucceed [v]` | — (default abort) | `(ParserMatch <v> state)` — value rendered via `str-vect`; UNUSED in the 67a1125 grammar (impl kept for completeness) |
| `Error [msg]` | — (default abort) | `(ParserError MSG state)` — UNUSED in the 67a1125 grammar |
| `Rule [name p]` | delegates to `p`'s `emit-pred` (inlined at char-level use sites) | module-level `(defn <name> [state] <emit-body p (EmitCtx name name->)>)` for EVERY Rule (char-level body → the one-char parser defn); in sub-position, `emit-ref` → the bare name |
| `Recur [f]` | — (default abort) | a call to `(.rule ctx)` — self-recursion only; the `f` field (the `sub-expression` defn in the grammar file) is data the emitter ignores. Limitation noted: a `Recur` targeting a rule other than the enclosing one is unsupported (the 67a1125 grammar has only self-recursion) |

Every top-level `Rule` gets a parser defn, uniformly. Char-level rules
also get their predicate inlined/generated at each use site (a `Many`
fast path generates its own `<name>-char` predicate defn); a
char-level rule's one-char parser defn may end up unused (e.g.
`digits`) — harmless.

Non-Rule top-level defs in the grammar file that no emitted rule
references (`linear-whitespace`, `whitespace`, `skip-ws`, the
keyword/delimiter string defs, the type-constraint rules,
`vector-expression`, `hash-map-expression`) are NOT emitted —
`emit-module` takes an explicit rule vector (task 8 fixes it to the set
reachable from `expression`).

### Generated-helper naming

Index paths, deterministic, no counter state: a helper for the
anonymous combinator at index `i` under a node named `N` is `N-i`;
deeper levels append (`N-i-j`). Top-level rules use their `Rule` name.
Kinds are not in the name (the shape is visible in the defn body).
Example: `double-quoted-string` = `All ["\"" (Many (Any [(NotChar
"\"") escaped-char])) "\""]` generates `double-quoted-string` (the
defn), `double-quoted-string-1` (the Many loop),
`double-quoted-string-1-0` (the lifted Any),
`double-quoted-string-1-0-0` (the lifted NotChar), plus
`double-quoted-string-1-char` (only on the fast path — this Many's
child is not char-level, so the slow path applies and no `-char` defn
is generated).

Lifting rule (reconciled with the byte-exact oracle, 2026-09-26
ralph review): an anonymous combinator is lifted to a named helper
UNLESS it is a bare `String`. The committed want files lift every
anonymous non-String child — e.g. `ig-all-2`, an anonymous `All`
child of an `All`, whose body contains no `let` — so the earlier
(a)/(b)/(c) formulation under-described the v1 output; the
committed want files are the spec. Bare `String` bodies are let-free
and inline everywhere. Consequence: the generated code contains NO
`let` in any non-else `cond` clause, by construction (the one-char
parser bodies — the only lifted bodies containing a `let` — never
inline).

### Value semantics (what the generated parsers produce)

- one-char parsers (`CharRange`/`NotChar`/`String`) → `ParserMatch` of
  the one-char string
- `Many` fast path → `ParserMatch` of the run as ONE string (the
  `read-run` `Token` text) — a deliberate tightening of combinator
  semantics (a vector of one-char strings), matching the settled
  maximal-run convention
- `Many` slow path → `ParserMatch` of a vector of child values
- `All` → `ParserMatch` of a vector of sub-values
- `Any` → the successful alternative's result
- `Ignore` → `ParserIgnore`; `Error` → `ParserError`

### Generated module template

`interpreter/gen-rdr.toc` (written by the driver):

```toccata
(add-ns rdr (module "intrp-rdr.toc"))   ;; helper layer — bare name,
                                         ;; same directory

;; the parse-then / parse-or kit (Result-discrimination section)

;; emitted rule defns + generated helpers

(main [argv]
  ;; slurp (inline C, same pattern as the interpreter drivers) the
  ;; file at argv element 1, make-state, parse a SEQUENCE of
  ;; expressions (skip-whitespace, entry rule, repeat until EOF),
  ;; print one result line per expression: the match value via
  ;; str-vect/str*, or `file:line: msg` for a ParserError (the
  ;; settled error convention). Exit non-zero on parse error.
  ...)
```

The emitter emits a bare `(def <rule>)` crutch after the header for
EVERY rule (the v1 `rule-decls` and all 13 committed want files do
this — the 2026-09-26 ralph review reconciled the earlier "only
`needs-recur` rules" wording with the byte-exact oracle and dropped
the now-unneeded `needs-recur` IR flag). Rationale (v1 comment,
item 8): the compiler is single-pass — a defn body referencing a
later top-level symbol fails `Undefined symbol`, and the defn order
(rule-vector order, entry first) leaves most rules forward-
referenced; the lifted Many-loop helper is emitted BEFORE the rule
defn and calls it (a cross-defn forward reference — the item-7c
crutch); declaring every rule is uniform and makes the module
robust to rule-vector order. The bare def must be separated from
the defn — immediately before does not register (see the Inherited
verified facts).

## Result discrimination (settled in task 1, 2026-09-04)

The generated parser discriminates `ParserMatch` / `ParserIgnore` /
`ParserError` with a two-op protocol kit, emitted once in the
generated-module header. The hand-written rdr's `result-kind`
integer-tag + `(cond (= k 2) ...)` pattern
(`interpreter/intrp-rdr.toc:282–285`) was flagged by the owner as
violating "no implementations that only return an integer; use
`int-cond` to select the code path". Of the options discussed
2026-09-04: **A** (integer tag + explicit `int-cond`) is rejected —
it keeps the integer-only impls and is codegen-identical to the
flagged status quo (`cond` over an I60 test already lowers to
`int-cond`, hvm-core.toc:546); **B** (three-continuation
`result-case`) is rejected — it passes three continuations per site
where one suffices. The core `map` / `flat-map` protocols are NOT
extended with monadic impls: their contract is element-wise
collection traversal (hvm-core.toc:993–1000), and a second monadic
contract under the same names would be ambiguous.

```toccata
;; Sequence: f is 2-arg (value, state) -> ParserResults. Match applies
;; f over its value and threaded state; Ignore/Error short-circuit to
;; r (early stop).
(defp parse-then [r f])
(extend-type ParserMatch (parse-then [r f] (f (.value r) (.state r))))
(extend-type ParserIgnore (parse-then [r f] r))
(extend-type ParserError (parse-then [r f] r))

;; Choose: Match/Ignore return r; Error forces else-f over the failing
;; result. The closure is only forced on the error path — laziness IS
;; the short-circuit.
(defp parse-or [r else-f])
(extend-type ParserMatch (parse-or [r e] r))
(extend-type ParserIgnore (parse-or [r e] r))
(extend-type ParserError (parse-or [r e] (e r)))
```

Site templates (the four shapes formerly open):

- **(a) `Any` ordered alternatives** — nested `parse-or`; each
  else-fn tries the next alternative; the innermost else-fn returns
  the final alternative BARE, so the last Error falls out for free:
  `(parse-or (<alt1> state) (fn [e1] (parse-or (<alt2> state) (fn [e2] (<altN> state)))))`
- **(b) `Many` slow-path loop exit** — `parse-or` over `parse-then`;
  error → return the accumulator:
  `(defn <name> [state acc] (parse-or (parse-then (<child> state) (fn [v s2] (<name> s2 (vect-conj acc v)))) (fn [e] (ParserMatch acc state))))`
- **(c) `All` / `Ignore` step checks** — nested `parse-then`, the
  innermost fn building the value vector:
  `(parse-then (<ref0> state) (fn [v0 s1] (parse-then (<ref1> s1) (fn [v1 s2] ... (ParserMatch [v0 ... vN] sN)))))`;
  `Ignore` of a child: `(parse-or (parse-then (<ref> state) (fn [v s2] (ParserIgnore s2))) identity)`
- **(d) the generated `main`** — `parse-or` over `parse-then`; the
  else-fn inspects the error (message + `state-line`) for the
  `file:line: msg` line and the non-zero exit:
  `(parse-or (parse-then (<entry> state) (fn [v s] <print the match line> (ParserMatch v s))) (fn [e] <print file:line: msg; exit non-zero>))`

Short-circuit note: an `Ignore` stops a `parse-then` sequence and
wins a `parse-or` choice. In the emitted grammar (the task-8 rule
set) no emitted rule's subtree contains an `Ignore`, so those paths
are unreachable; the templates are total regardless.

**Standing condition (task 2a):** the kit's continuations capture
free variables — (b)'s `fn [v s2]` captures `acc`, (a)'s else-fns
capture `state`, (c)'s inner fns capture prior `vN`s. The inherited
reduce-capture leak (1MB term buffer exhausted over ~194 nodes) is
the same shape; task 2a probes it before any kit template is used in
a committed driver. UPDATE (2026-09-26): the reduce-capture leak is
RESOLVED — capture-reducing probes run clean at small and 2000-
element scale (malloc diff 0, remaining nodes 0), so this standing
condition no longer blocks kit-template use; the explicit-recursion
(-acc) shape already in the emitter remains valid but is no longer
required.

**`Many` fast/slow classification (ruling, SUPERSEDED 2026-09-26):**
the v1 ruling — the flipped-receiver protocol `emit-many` (char-
level ctors carry the run-path impls, the default body the loop
path; a yes/no tag protocol rejected as tag-shaped-impl) — is
superseded by the v2 design: the classification is a bottom-up IR
flag (`char-level?` in `analyze`), and the render branches on it.
The invariant survives in the form that applies to v2: the
generated code's branches are real code paths, and the emitter's
classification is DATA, not a protocol-impl shape.

Out of scope for THIS project: retrofitting the hand-written
`interpreter/intrp-rdr.toc`'s `result-kind` to this pattern —
separate follow-up under the compiler plan, now unblocked.

## Inherited verified facts (from docs/new-compiler-plan.md)

Curated for this project; the source of truth is the compiler plan.

**Build and diagnostics**

- RUNTIME TOOLCHAIN BROKEN (2026-09-04, item 2): the git-HEAD
  runtime sources (`new.c` / `runtime3.c` / `graph.c`) cannot run ANY
  program generated by the current `new-toc` binary (2026-09-03
  07:32) — even `(main [_] 0)` crashes the SAFETY check `Error: LAZ
  pair requires positive term in port 2 / Port 2 term tag: SUB at
  new.c:516` (and segfaults without `-DSAFETY=1`). Rebuilding the OLD
  generated C (`rdr-exprs.c`) that produced the working prebuilt
  binary ALSO crashes against the current runtime sources — the
  prebuilt drivers (e.g. `./rdr-exprs`, built 2026-09-04 08:45) work,
  but were linked against an older runtime that no longer exists on
  disk. This blocks all driver-based verification (items 2a–8) until
  the owner restores a consistent runtime/new-toc pair. Also observed:
  the `new-toc` binary is non-deterministic (6 runs of the same input
  → 6 different generated-C outputs, plus occasional segfaults — the
  symbol-loss race in broader form), and `new-toc.c` is MISSING from
  disk (only `new-toc.c~`) — new-toc cannot be rebuilt from the
  Makefile.
- RUNTIME TOOLCHAIN RESTORED (2026-09-04, item 2a): the RUNTIME
  TOOLCHAIN BROKEN fact above is SUPERSEDED — fresh `new-toc` builds
  compile and run clean against the current runtime sources (a
  trivial program and a 2000-iteration probe: exit 0, malloc diff 0,
  remaining nodes 0, deterministic across reruns and a rebuild).
  Driver-based verification (items 2a–8) is unblocked. The
  `new-toc.c`-missing / non-determinism caveats in that fact still
  stand.
- Always capture and read new-toc's stderr on a failed build (never
  `2>/dev/null`): `Undefined symbol: 'x' at file: N`, `Error at file:
  N; msg` point directly at the problem. `*** Loading/Loaded/declare`
  lines are normal progress. Blank lines in stderr are normal — when
  filtering for error lines, filter `^\*\*\* ` AND blank lines.
- If new-toc segfaults/aborts with NO error message, the crash is
  transient — retry up to 5 times total. If a run prints an error
  message before aborting, it is a real error — stop retrying, fix it.
- Compile check for library files: `./new-toc <file> > /dev/null` —
  exit code is unreliable (134 in one clean load, 0 in another —
  2026-09-04, item 3); pass = `*** Loaded <file>`
  in stderr with no other error lines. `'main' function is missing or
  malformed` + `Could not find implementation of 'Container/map' for
  type 'Agent' ... at core: 1453` are baseline noise in the abort path
  for any file. A program WITH `main` exits 0 on success.
- Driver build recipe (Makefile `rdr-top` pattern): `./new-toc
  <driver>.toc > <driver>.tmp`; awk `#line` step
  (`awk '/^#$$/ { printf "#line %d \"m.c\"\n", NR+1, "<driver>.c";
  next; } { print; }'`); `clang-format -i`; `$(CC) $(CFLAGS) -o
  <driver> $(TOC_FLAGS) $(LDFLAGS) <driver>.c new.c runtime3.c
  graph.c`. `TOC_FLAGS` carries `-DCHECK_MEM_LEAK=1 -DSAFETY=1
  -DSTATS=1` — WITHOUT `-DCHECK_MEM_LEAK=1`, lazy evaluation hits
  `BOOM("Make this threadsafe")` in `eraseLazy` (new.c).
- Non-deterministic module symbol loss: new-toc sometimes drops a
  module's last defn from the symbol table (~90% of builds in the
  observed case) — the importer fails `Undefined symbol`. Same file
  sometimes compiles clean → a race, not a source error. Workaround
  (verified): forward-declare the accumulator fn and order the two
  defns so the public entry is NOT last (the `parse-program` /
  `parse-program-acc` pattern). UPDATE (2026-09-09, item 6): the race
  became DETERMINISTIC (10/10) for `intrp-emit.toc` (last defn
  `emit-module`); a bare `(def emit-module)` at the TOP of the file
  (separated from the defn) prints `*** declare emit-module` and
  fixes it. A bare def IMMEDIATELY before the same-named defn prints
  no declare line and does not register. Runtime binding verified:
  probe `(def f)` + `(defn f [x] (+ x 1))` + `(f 41)` → 42 — declared
  global and defn binding are the same global (resolves the
  "runtime binding UNVERIFIED" caveat on the forward-decl fact
  above).
- let-wrapping-cond miscompiles (2026-09-09, item 6): a defn body
  shaped `(let [n ...] (cond t (let ...) (let ...)))` — a let
  wrapping a cond whose clauses are both lets — LOADS CLEAN but the
  program aborts at runtime with `bad incRef value: (nil)` even when
  the function is NEVER CALLED (crash at module load/registration).
  Fix: inline the let-bound value so the cond is the defn body
  directly (let INSIDE cond clauses is safe — `all_body-acc`).
  Related: a let directly in a NON-ELSE cond clause is REJECTED at
  load as a malformed cond — the branches must be plain calls (helper
  defns), as in `any-body-acc`.
- new-toc codegen drops a call arg (2026-09-09, item 6): in
  `any-body-step` (let of 3 bindings incl. a protocol call; body a
  4-arg call whose 1st arg is a nested 2-arg `append-acc`), the 2nd
  arg of the OUTER `append-acc` (the else-fn line) is DROPPED — the
  call is emitted 1-arg and the let binding erased to `ERA` in the
  generated C; deterministic (4/4+). The arg is dropped whether
  inline, let-bound, or a helper call in arg position. Workaround
  (verified): move the whole let + appends into a separate defn
  (`step-newacc`); `any-body-step` is then a single 4-arg call and
  builds clean. A scratch probe with a call-expression in that arg
  position CRASHES new-toc's codegen (abort, no message, truncated C
  output) — the same fragility. NOTE: many smaller probes with
  similar shapes build clean, so the trigger is not yet isolated;
  treat nested-append arg positions in large modules as suspect.
- `pr*` side-effect lines inside a defn BODY miscompile (2026-09-09,
  item 6): debug `pr*` calls added to a defn body (outside the
  result position) change the generated code's behavior — the driver
  miscompiled until every such line was removed. Debug by a separate
  driver file, not by sprinkling `pr*` into library defns.
- Any alts are tried IN ORDER over the defn's `state` (2026-09-09,
  item 6): a bare-String alt's literal must start with a char no
  earlier alt matches, or the earlier alts eat it char-by-char (the
  `"xy"` alt after `an-alpha` never fired — `x` and `y` each won via
  `an-alpha`); and since the String body's `take-char` consumes
  exactly one char, the literal must be a SINGLE char or the
  remainder is re-parsed as the next expression.
- first/rest iteration over a bare string walks it CHAR BY CHAR
  (2026-09-09, item 6): a helper iterating a collection arg via
  `count`/`first`/`rest` (e.g. `append-acc`) given a bare STRING arg
  yields one-char-string elements (`to-str` renders them space-
  joined). The collection arg must be a vector — `(conj [] s)`. Also:
  `(vect-conj [l] xs)` appends `xs` as ONE nested element; splice a
  recursive vector result with `append-acc` (a nested vector in an
  emitted-lines vector renders under `to-str` with stray `[`
  brackets).
- RUNTIME TOOLCHAIN BROKEN AGAIN (2026-09-09, item 6): SUPERSEDES the
  RUNTIME TOOLCHAIN RESTORED fact — after a machine reboot, the
  `new-toc` binary (2026-09-05 20:40) crashes (exit 134, NO error
  message) on ALL non-trivial inputs: 0/30+ on
  `interpreter/intrp-grammar.toc` (loaded clean earlier the same
  session), also `intrp-rdr.toc` and the emitter; trivial programs
  (`(main [_] 0)`, 50 flat defns, a single simple deftype) still
  compile. Rebuild BLOCKED: `make -B new-toc` triggers a `toccata`
  rebuild whose link fails (`undefined reference to emptyBMI`).
  Owner must restore/rebuild the toolchain. Earlier the same session
  the same binary built the drivers intermittently (retry loops of
  5–12 attempts). UPDATE (2026-09-09, item 6, second degradation):
  the restored binary (2026-09-09 19:10) worked for a window (full
  `emit-pred` 9/9 OK + `gen-rdr` build + sample run) then degraded
  MID-SESSION: all non-trivial files crash again (`intrp-grammar.toc`
  15/15, `emit-pred.toc` 15/15, no error message), while trivial and
  small files still build. Machine memory was healthy (43GB
  available). The degradation appears to be a machine-state issue,
  not a source issue (the pristine committed driver fails too).
  UPDATE (2026-09-09, item 6, third degradation): the same 19:10
  binary opened another window this evening (trivial programs, both
  libraries, and the pre-item-6 driver all clean — 5/5) and the
  crash set then EXPANDED over ~30 min: the `-` / `+` single-char
  literals in the driver's Any-alt position went from building to
  deterministic silent aborts (13/13, 8/8), and by the end even the
  known-good pre-item-6 driver segfaulted intermittently (1/8) while
  the new driver crashed 15/15. Memory healthy (43GB). The crash set
  drifts with machine state — a content combination that builds at
  one moment may crash ten minutes later; verify in a healthy window
  and do not chase content workarounds against a drifting crash set.
  UPDATE (2026-09-10, item 7a, fourth degradation): the same 19:10
  binary is now in a LEAKING state, not just a crashing one: the
  PRISTINE committed HEAD driver (9 checks, unmodified) deterministically
  leaks 19 malloc nodes (12/12 runs; it was diff 0 in the 2026-09-09
  healthy window), so `make emit-pred` fails at the leak check even
  without any item-7a code; probe drivers (HEAD + extra emission
  checks) crash at load (5/5). The item-7a driver itself loads and runs
  clean in this state (10/10 OK, deterministic 6/6) but leaks 30
  (the 19 baseline + 11 from the mn emission path), and the generated
  module builds and parses correctly (see the Item 7a note). The
  leak/remaining-nodes half of the 7a done-when is PENDING a healthy
  window; the functional half is verified. UPDATE (2026-09-10, item
  7b): the LEAK baseline has DRIFTED again within the same window —
  the pristine HEAD driver now leaks 30 (was 19 in the 7a run; the
  7b driver leaks 45 = the 30 baseline + 15 from the fn emission
  path), while everything functional still works (11/11 OK,
  deterministic; the generated module's success path is diff 0 /
  remaining 0). The baseline drifts with machine state; attribute
  driver-side leaks to the baseline by measuring HEAD in the same
  window, and hold the leak half of a done-when to the generated
  module's success path until a healthy window. UPDATE (2026-09-10,
  item-8 investigation): a healthy window opened that morning on the
  same 19:10 binary — trivial programs, both libraries, the HEAD
  driver (12/12 OK, ITRS 2.48M, remaining 0; it leaks 53 = the
  drifted baseline), the rc gen module, and several small probe
  modules all built 3/3. The item-8 content crashes (the full
  real-grammar module; the ≥2-inlined-cond parse-or shape) were
  DETERMINISTIC in that window and are content triggers, not drift —
  see the two new codegen-crash facts below.
- A 5/5 silent-abort is NOT always the toolchain (2026-09-10, item
  7b): an UNBALANCED-PAREN source error in `emit-pred.toc` (a
  fingerprint def one `)` short) crashed new-toc with a silent abort
  5/5 — no error message — while the SAME class of error elsewhere
  in the same session printed `Error at file: N; Missing ")"`. The
  retry rule (silent crash = transient) can send a run chasing a
  healthy window for a source bug. Before blaming the toolchain on a
  5/5 silent abort: check the file's paren balance (strings
  excluded) and compile a known-good file (e.g. `git stash` + HEAD
  driver) in the same window to confirm the toolchain itself is up.
- Single-char `-` / `+` string literals crash new-toc codegen
  (2026-09-09, item 6): in `emit-pred.toc`, the grammar-data alt
  `(grammar/Any [... "-"])` (and likewise `"+"`) crashes new-toc's
  codegen with a silent abort (no error message) — deterministic
  while observed (13/13, 8/8). Every other single char tested at the
  same position builds (`x 9 ! * ( ) = < > , : ? @ $ % # _`); `/` is
  flaky (1 segfault in 3). The SAME literals inside the item-5
  fingerprint strings build fine, so the trigger is
  position/context-specific, not the char in general. Workaround:
  the synthetic sample uses `!`.
- `exit` path skips the harness epilogue; ASan is not a leak oracle
  (2026-09-09, item 6): the Toccata `exit` builtin calls C `exit()`,
  so the malloc/remaining-node stats (printed only in the epilogue's
  `freeAll`/`freeGlobals`) are structurally unavailable on an exit
  path — the canonical zero-leak check needs a clean return. Under
  `-fsanitize=address` even a trivial `(main [_] 0)` "leaks" 16–32
  bytes of harness baseline and loses its stdout buffer (the epilogue
  dies mid-`freeGlobals`). Verified pattern for exit-path binaries:
  success path → assert malloc diff 0 + remaining nodes 0; error
  path → assert output + non-zero exit, and use an ASan build only
  to confirm no heap errors and LSan = harness baseline alone (a
  trivial `(main [_] (exit 1 "x"))` control leaks 16 bytes / 2
  allocations from the same normalize frames).
- A `scratch/` probe cannot `add-ns` a module in `interpreter/` —
  add-ns paths resolve against the importing file's directory and
  `../` is forbidden, so `scratch/` probes are limited to
  self-contained sources. A throwaway probe that must import an
  interpreter module is created in `interpreter/` temporarily and
  deleted before the commit (2026-09-04, item 3).
- A program ending with a SUB or SUP error probably means a function
  called with the wrong number of arguments.
- A wrong-type field access / mis-threaded call can make a driver a
  SILENT NO-OP (2026-09-10, item 7c): a `->` threading bug in
  `emit-module` — threading `(rule-decls rules)` between
  `(emit-header)` and `(emit-rules-acc rules)` passed the header's
  LINES vector as `rule-decls`' `rules` argument (its signature is
  `[rules]`, not `[acc rules]` — the `->` into `emit-rules-acc` was
  fine because its first param IS the acc) — made `rule-decls-acc`
  run `(.parser ...)` over plain
  String lines. No load error, no crash: the compiled driver ran,
  exited 0, printed NOTHING (not even the first check line), ITRS
  ~10.5k (vs ~2.5M for a working run), malloc diff 0, deterministic
  3/3. The wrong-type `.parser` access produced an error value that
  silently killed the enclosing `let` result chain, and the lazy
  machine never forced the `pr*` side effects. Debug hint: a driver
  that suddenly prints nothing with exit 0, diff 0, and ITRS dropped
  to ~10k has a silent error value in its `let` chain (wrong-type
  field access, mis-threaded call) — not a toolchain crash; bisect
  with a small probe that calls the suspect function directly.

**Language / core API (new core, as used by generated + emitter code)**

- Bare `(def name)` is a forward declaration (2026-09-04, item 2):
  a bare `(def name)` with no value declares the symbol — new-toc
  prints a `*** declare <name> <global>` line — and makes it usable
  before the defining `def`/`defn` appears later in the file. All
  other ordering is strict use-after-definition: a `def` initializer,
  `defn` body, or `fn` body referencing a top-level symbol defined
  LATER in the file fails `Undefined symbol: '<name>'` (100%
  reproducible — not the symbol-loss race). This is what makes the
  `expression` (def) ↔ `sub-expression` (defn) cycle in
  `intrp-grammar.toc` expressible: declare `sub-expression` first,
  define `expression`, then define the `defn`. Runtime binding of the
  declared global is UNVERIFIED (no driver can run — see the
  toolchain fact below).
- Self-recursion through a LIFTED HELPER needs the `(def name)`
  crutch after all (2026-09-10, item 7c): the generated-module
  template note "self-recursion needs none under new-toc" holds only
  when the recursive call sits in the rule's OWN defn (a defn calling
  itself). When the `Recur` lands in a lifted helper defn (the `Many`
  loop defn `<rule>-1-1`, emitted BEFORE the rule defn per use-after-
  definition), the helper's call to the rule name is a CROSS-DEFN
  forward reference and fails `Undefined symbol: '<rule>'` at load
  (deterministic). The emitter now emits a bare `(def <rule>)` after
  the module header for every rule whose subtree contains a `Recur`
  (`contains-recur?` / `rule-decls`); the bare def is separated from
  the rule defn by the other rule blocks (immediately-before does not
  register — the fact above). Verified: the generated rc module
  loads, builds, and parses nested input with the crutch.
- A zero-length-capable parser in a LOOP POSITION spins the term
  buffer (2026-09-10, item 7c): the site-(b) `Many` loop and the
  generated `parse-seq` terminate on `ParserError`; if the loop CHILD
  (or the entry rule) can `ParserMatch` WITHOUT consuming input (a
  `Many` fast-path `read-run` over a non-matching prefix returns a
  zero-length run), the loop retries on the same state forever and
  dies with `Error: Not enough space to allocate pair.
  buffEnd=1048576, buffSize=1048576 at new.c:264` (the 1MB term
  buffer). The hand-written rdr's `parse-expr` is TOTAL for exactly
  this reason (its last clause takes a char and retries; it never
  matches empty). The item-7c synthetic grammar's first `symbol-ish`
  (a `Many` of a `CharRange`) hit this; the fix was a non-empty
  `symbol-ish` (a single-char rule — the real grammar's `symbol` is
  non-empty too: `symbol-start` + rest). WARNING for item 8: the
  real grammar's `expression` has `int-literal` (`Many digits` —
  zero-length-capable) as its FIRST Any alt; as generated,
  `expression` would match empty on non-digit input and `parse-seq`
  would spin. Item 8 must address this (non-empty number shape in the
  grammar data, or a consumption guard) — the plan does not settle
  it; treat as an item-8 design question, not a 7c gap.
- No `instance?` — dispatch is via protocols (defp + extend-type). A
  defp WITH a body registers the body under `UnknownType` (the
  dispatcher's default case); a defp without a body aborts on an
  unimplemented receiver.
- Booleans: `not` exists (hvm-core.toc:101, owner-added 2026-09-04):
  `(defn not [x] (cond x None (Some nothing)))` — `None` when `x` is
  truthy, `(Some nothing)` when falsy. `str=` returns Maybe
  (`strCmp` STR_EQ). `cond` impls: `None` → else, `Some` → then,
  `Integer` → `int-cond` (hvm-core.toc:115, 543); `and`/`or` have
  `None`/`Some` impls. I60 also works as a `cond` test (the Integer
  impl); `int-=`/`int-<`/`<=` are I60-flavored (the hand-written rdr's
  `char-code` predicates use `and`/`or`/`<=` over them and run clean).
  `int-cond` itself is a C-level REF (`(def int-cond (inline
  "newRef(intCond);"))`) called as `(int-cond n non-z z)`.
- `+` is exactly 2-arg (nest to combine more). `first` on a Vector
  returns `Some element` — extract with `(extract (first v))`; `rest`
  returns a Vector. `subs` is 3-arg: `(subs s start len)`; the rest of
  a string is `(subs s 1 (count s))`. `count` is O(1) for SubString
  and StringBuffer. `char-code` = first char's integer code (0–255);
  `char` is the inverse. `str-prefix?` exists (STR_PREFIX).
- Named / namespace-qualified functions are first-class values —
  passable as arguments, callable as `(f x)`.
- `reduce` is a left fold `(reduce coll init f)`. A `reduce` whose
  closure captures a FREE VARIABLE used to leak term pairs on the lazy
  machine (over 194 nodes it exhausted the 1MB term buffer) — the
  historical workaround was explicit recursion passing the value as a
  plain parameter (the `threading-acc` / `count-class-acc` pattern).
  RESOLVED (2026-09-26): the leak no longer reproduces — capture-
  reducing probes run clean at small and 2000-element scale (malloc
  diff 0, remaining nodes 0; `scratch/reduce-capture-probe.toc` /
  `-probe2.toc`). The explicit-recursion workaround is no longer
  required; existing `-acc` code remains valid.
- `str*` over a vector of `str-vect` implementors concatenates to one
  String; `(str* [n])` renders an Integer (via `number-str`). `pr*`
  takes ONE string — `pr*` on a vector aborts silently; build the
  string with `(str* [...])` first.
- `escape-chars` (hvm-core.toc:652) escapes `\` `"` `\n` `\r` `\f`
  `\b` `\t` to two-char sequences — exactly a Toccata string-literal
  body; the emitter uses it (`render-literal`) to splice grammar
  strings into generated source (2026-09-04, item 3).
- `type-name` over a deftype ctor value returns the BARE ctor name
  (e.g. `"All"` — no namespace prefix); the emitter's default abort
  message uses it (2026-09-04, item 3).
- A defp default body runs at runtime for a VAL receiver with no
  `extend-type` impl — verified with an `abort` default body: called
  through `Rule` delegation on a `Rule(All [...])` value, the `All`
  hit the default, printed the message, exited 134 (2026-09-04,
  item 3).
- `str-append` (hvm-core.toc:608) appends IN PLACE into the dest's
  buffer — the dest must be a pre-allocated StringBuffer with enough
  capacity (the hand-written rdr pre-allocates its acc). Appending to
  a static string literal (e.g. `(str-append "" "x")`) overflows the
  global buffer — segfault (ASan: global-buffer-overflow in strncat)
  (2026-09-04, item 2a).
- `vect-concat` is user-defined (in `interpreter/intrp-rdr.toc`,
  defined BEFORE the grouping deftypes), not core.

**new-toc codegen crashes (item-8 investigation, 2026-09-10)**

- Two or more INLINED bare-String conds in a `parse-or` (Any /
  site-(a)) position crash new-toc codegen (2026-09-10, item-8
  investigation): a defn shaped like the generated Any emission —
  nested `(parse-or <alt> (fn [eN] ...))` — builds clean with at most
  ONE inlined bare-String cond (a `(cond (str-prefix? L ...) ...)`
  body, in any position: first, middle, or last/bare) plus defn-call
  alts, but crashes (silent abort exit 134, truncated C output with
  NO `mainFn`, sometimes exit 0 with the same truncated C) with TWO
  or more inlined conds — deterministic 3/3 for every ordering
  tested (2 inlined + 1 call in all three orders; 3 inlined; 4/6/8/
  10/11 inlined). The SAME inlined conds in a `parse-then` (All /
  site-(c)) position build clean (the rc module's `rc-entry-1` has
  two). The real grammar's `symbol-start` (1 call + 10 inlined) and
  `rest-of-symbol` (2 calls + 11 inlined) therefore CANNOT build as
  emitted — the settled Lifting rule ("Bare `String` bodies are
  let-free and inline everywhere") conflicts with this. A 13-alt
  flat `(or ...)` char PREDICATE defn builds fine (the crash is the
  parse-or position, not the alt count or the predicate size). The
  full emitted real-grammar module and a symbol-rules-only
  sub-module both crash 5/5+ for this reason. Owner decision needed:
  lift bare-String alts to helper defns in the Any emission
  (template change), or a toolchain fix.
- A defn referencing an UNDEFINED top-level symbol crashes new-toc
  codegen silently (2026-09-10, item-8 investigation): a parse-seq
  whose entry call named a symbol with no defn anywhere in the file
  crashed new-toc (silent abort, truncated C, 3/3+) with NO
  `Undefined symbol` error — the documented `Undefined symbol`
  failure is for symbols defined LATER in the file; a symbol defined
  NOWHERE crashes instead. When bisecting a silent codegen crash,
  check that every referenced top-level symbol has a defn (my own
  probe modules hit this twice while extracting sub-modules).

**deftype / annotations**

- A deftype has two forms: single-ctor `(deftype CtorName [fields]
  <impls>)` and multi-ctor `(deftype Type (Ctor1 [fields] <impls>)
  ...)`. In the multi-ctor form the type name AND each ctor name are
  types. A bare name in the ctor list references an existing ctor.
- A constructor declared with no fields is a singleton VALUE, referred
  to by name — never called as `(Ctor [])` (cross-namespace calls of
  that shape generate crashing C).
- `!` annotations are parsed/validated by new-toc even though they
  don't affect codegen; they are a hazard on multi-field ctors
  ("Conflicting assertions"). Grouping deftypes: no `!` annotations.
- Constructor names are globally unique per namespace; a bare name
  cannot redefine an existing ctor. A bare reference to a user-file
  ctor fails when that ctor carries `!` annotations over
  String/Location-kind types.

**Source-writing hazards (apply to the emitter's own source AND to
generated code)**

- NO local symbol may shadow a core-namespace symbol — new-toc emits
  colliding C identifiers (the mangled local name collides with an
  unrelated core entity); the Toccata-level load check does NOT catch
  it, only a clang build of the generated C does.
- `cond` takes flat (test value) pairs + a trailing default. What
  new-toc limits is NESTING: 8+ levels of nested `cond` are rejected
  (7 is the limit). Flat clauses are unbounded (`symbol-start`'s 11
  alternatives generate a flat cond — fine).
- malformed-cond quirks: a `let` in a NON-ELSE clause is rejected
  ("malformed 'cond' expression") even with no nested cond; a let
  containing a nested cond whose clauses are both lets; a field getter
  inside a vector arg to a deftype ctor inside a let inside a cond
  clause. A `let` in the ELSE clause is fine. Fix pattern: extract the
  let into a helper defn so the clause is a plain call. (The generator
  avoids all of these by construction — see Lifting rule.)
- A call whose target is an Integer or String literal is invalid;
  new-toc's typer misreports it as `Conflicting assertions (571)`
  (latent typer bug). Avoidance: never put an Integer or String in
  operator position.
- Hash-map keys must be a single type — `strSha1` includes the value's
  TYPE, so a SubString key and a same-content String key hash
  differently and `get` misses.
- Lazy machine: a continuation that drops a threaded value lets the
  machine skip earlier side effects — thread side-effect results
  through a strict combination (e.g. `+`) into the result chain.
  `fn` literals with underscore params miscompile — use named params.
- `inline` C bodies must set `result = <Term>` — codegen emits the
  body inside a `void` fn and appends the `move(portLoc(2, args),
  result)` itself; a `return` in the body is a clang error ("void
  function should not return a value"). The `(inline TypeExpr "...")`
  annotation does NOT change the generated C fn signature. `inline` is
  not allowed in a `cond` CLAUSE ("'inline' expressions not allowed
  here" — defn-body position is fine). Fn args are visible in the C as
  `<arg>_1`, `<arg>_2`, ... (1-based) (2026-09-04, item 2a).

**Modules**

- `add-ns` paths are bare relative paths resolved against the
  importing file's directory — no `../`, no absolute, no leading `./`.
- NEVER use `../` or double-spell a module path: new-toc's module cache
  keys on the RAW path string; the same file under two spellings is
  compiled twice with fresh type numbers → cross-module protocol
  dispatch fails at runtime (`No implementation of 'X' found for type
  Y (N)`) even though the value prints the right type name. Keep every
  reference to a module spelled identically across the graph.
- A nested `append-acc` as a DIRECT call argument is miscompiled by
  new-toc codegen (2026-09-10, item 7a): in `intrp-emit.toc`, a defn
  whose body passed a nested `(append-acc ... (append-acc ...))`
  expression as a direct argument to another call (`wrap-defn-acc`'s
  second argument) loaded clean but MISCOMPILED — the argument
  arrived at runtime as a raw Term (type tag 6) instead of a Vector,
  and `count` over it crashed with `No implementation of 'count'
  found for type <unknown> (6)` (in `wrap-defn-acc`, deterministic
  6/6 in the full driver; the SAME defn shape loaded and ran clean in
  a smaller probe file — the miscompile is C-layout-dependent, like
  the item-6 arg-drop). The proven fix (the `step-newacc` / `any-
  body-step` pattern, now `many-loop-step` / `many-loop-lines-acc`):
  build the nested `append-acc` inside its OWN helper defn and pass
  the helper's RESULT (a plain call) as the argument; never inline
  the nested `append-acc` at the call site. A let-bound value built
  by such a helper is equally safe to pass (the `many-loop-defn` `body`).
- A vector LITERAL containing a `to-str` over a LET-BOUND value
  crashes new-toc codegen at load (2026-09-10, item 7a): in
  `intrp-emit.toc`, `[(to-str [(extract (first ref))])]` where `ref`
  is a let-bound parameter crashed new-toc at load (silent abort,
  deterministic); the same content built with `conj` —
  `(conj [] (to-str [...]))` — loads and runs clean. The pre-7a
  emitter already used `conj` for 1-element vectors in exactly these
  positions (`step-newacc`, `any-body-last`); keep that convention for
  any vector literal whose element references a let-bound value.

**Conventions**

- Temporary files live in `scratch/`; a scratch file that proves
  useful long term is moved to `interpreter/` and committed from there
  — never committed from `scratch/`. Interpreter work lives in
  `interpreter/`.
- Before writing or editing any `.toc` file, read
  `docs/toccata-style.md` and follow it (cond spacing, let-binding
  rules, naming, "write the fold not the -acc function" — note the
  generated Many loop is NOT a vector walk, so acc-recursion is the
  correct shape there, as in the hand-written rdr's `collect-until-acc`
  / `threading-acc`).
- Never touch the `toccata` Makefile target. Never `sudo`.

**As-built notes**

- Rewrite decision (2026-09-26): the owner judged the v1 emitter
  horrible — its explicit-recursion / `-acc` threading shape exists
  largely for the reduce-capture leak, which is RESOLVED (probed
  2026-09-26: capture-reducing probes clean at small and 2000-
  element scale, malloc diff 0, remaining nodes 0). hvm-core.toc
  carries the recursion schemes (`fold` / `unfold`, hvm-core.toc:
  586–599, over the `recurse` container protocol, hvm-core.toc:123),
  and the `ParserCombinator` deftype already implements `recurse`
  for every ctor (item 2) — the owner directed a fold-based rewrite
  (emitter v2, items 10–18). The committed `interpreter/emit-want/`
  expected files and the item-8 corpus `-want` files are the byte-
  exact regression oracle for v2. The v1 as-built notes below are
  retained as history; the v1 emitter source remains in git
  history (the rewrite is in place at `interpreter/intrp-emit.toc`).

- Item 2 (2026-09-04): the 67a1125 source's `recurse`/`str-vect`
  impls carried free-variable bugs (`parser` / `name` / `f` used bare
  instead of `.field` getters — they could never have compiled) and
  used old-core `first` (now returns `Some`) / `to-str` shapes. They
  were ported, not kept verbatim: `recurse` impls fixed to use
  getters; `str-vect` impls rewritten in the `intrp-ast.toc`
  `(flat-map [...] str-vect)` convention; `Recur`'s `str-vect` emits
  the constant `["(Recur)"]` (it must not call the stored `f` —
  calling an arbitrary function REF is unsupported in this new-toc
  build). The bare `(def sub-expression)` forward declaration from the
  original was kept (it is live new-toc syntax — see the fact above),
  with the `defn` after `expression` as in the original.

- Item 3 (2026-09-04): `emit-pred`'s `Rule` impl delegates to the
  child verbatim, so an `Any` containing a Rule alt emits a NESTED
  `(or (or ...) ...)` — `symbol-start` / `rest-of-symbol` embed
  `alpha`'s `(or <lower> <upper>)` as their first alt. The driver
  builds the expected strings from parts (`want-alpha` spliced into
  `want-symbol-start` / `want-rest-of-symbol`) so the nesting stays
  consistent by construction. All six predicate fingerprints + the
  `emit-module` v1 fingerprint matched exactly; the driver is
  deterministic across a rerun and a full rebuild (exit 0, malloc
  diff 0, remaining nodes 0). `emit-module` v1 emits the add-ns
  header line + one `(defn <name>-char [c] <pred>)` line per rule;
  `entry` is accepted but unused until the item-4 main template.

- Item 6 (2026-09-09, VERIFIED 2026-09-09 evening): the Any
  (parser-level) emission is
  CODE-COMPLETE in `interpreter/intrp-emit.toc` — `any-body-acc`
  (site-(a) nested parse-or; last alt bare; `(count ps)` inlined at
  both use sites per the let-wrapping-cond hazard; the two branches
  are helper defns `any-body-last` / `any-body-step` — a let directly
  in a non-else cond clause is rejected as a malformed cond),
  `step-newacc` (the step's new-acc lines in their own defn — see the
  codegen arg-drop fact below), `any-body`, the `Any` `emit-body` /
  `walk-children` impls, the `child-ref` `sv` parameter (All/Ignore
  thread `s<i>`; Any calls every alt over the defn's `state`),
  `append-to-last` (flattened with `append-acc`), the `emit-module`
  + `any-body-acc` forward declarations (symbol-loss race fix), and
  the main-template fix: `parse-seq` accumulates the OUTPUT STRING in
  `acc` (pure dataflow — the per-line `pr*` counts threaded through
  `+` were skipped by the lazy machine on the error path: `exit` cut
  the result chain before the `+` was forced, dropping earlier
  prints); `main` prints the accumulated lines on success, and
  `parse-error-line` splices them into the exit message on error.
  The driver (`emit-pred.toc`) carries the `an-any` synthetic grammar
  (Any of a named Rule `an-alpha`, an anonymous All (lifted to
  `an-any-1`), and a bare String alt `"!"` — the literal is a SINGLE
  char outside a-z0-9, and NOT `-` or `+` (those literals in this
  position crash new-toc's codegen — see the fact above): alts are
  tried in order over the defn's `state`, so a literal starting with
  an a-z/digit char is eaten
  char-by-char by the earlier alts — the original `"xy"` sample
  printed `x` and `y` (an-alpha wins) and the String alt NEVER fired,
  violating the done-when's "each alternative wins at least once" —
  and `take-char` consumes exactly one char, so a longer literal
  would leave a re-parsed remainder), the `emit-module-an`
  exact-fingerprint check (9 checks total), and the `gen-sample.toc`
  write (`a / 42 / ! / 5`). Verification state: with the toolchain
  UP earlier this session (new-toc 2026-09-09 19:10), `make
  emit-pred` showed 9/9 OK (zero malloc diff, 0 remaining nodes) and
  `make gen-rdr` built; `./gen-rdr interpreter/gen-sample.toc` on the
  OLD `"xy"` sample printed `a` / `[4 2]` / `x` / `y` /
  `interpreter/gen-sample.toc:4:expected "xy"`, exit 1, zero leaks
  (consistent with the generated code — and the exposure that led to
  the `"-"` sample fix above). The `want-an-any-defn` fingerprint
  also gained a 7th closing paren on the last line (the old 6 was
  wrong: ParserError + cond + fn + parse-or + fn + parse-or + defn).
  The single-char sample change is UNVERIFIED — the toolchain
  degraded again mid-session (see the BROKEN AGAIN fact's third
  update): the driver crashed 15/15 (silent abort, no error
  message). Also found and fixed this run: the committed driver had
  a MISSING `)` on the `an-any` def (the unverified `"-"` edit
  dropped it — `Error at emit-pred.toc: 253; Missing ")"`); the fix
  is verified (the parse error is gone; the subsequent crashes are
  the toolchain, not the source). The sample char is now `!` (was
  `-`): `-` and `+` in the Any-alt position crash new-toc's codegen
  deterministically (see the fact above), `!` builds. FINAL
  VERIFICATION (2026-09-09 ~20:35, healthy window of the 19:10
  binary): the previous fix was INCOMPLETE — the `an-any` def was
  still missing the `Any` vector's closing `]` (new-toc: `Error at
  interpreter/emit-pred.toc: 255; Missing "]"` — a real parse error,
  fixed by adding the bracket; the def now balances 6/6 parens, 2/2
  brackets). With the fix: `make emit-pred` built on the 2nd attempt
  (1st attempt after the fix segfaulted post-load with no message —
  the usual transient) and printed 9/9 OK (digits, upper-case,
  lower-case, alpha, symbol-start, rest-of-symbol, emit-module,
  emit-module-ig, emit-module-an), malloc diff 0, remaining nodes 0,
  exit 0. `make gen-rdr` built clean; `./gen-rdr
  interpreter/gen-sample.toc` (a / 42 / ! / 5) printed `a` /
  `[4 2]` / `!` / `interpreter/gen-sample.toc:4:expected "!"`, exit
  1 — each alternative won once (an-alpha, the lifted anonymous All
  an-any-1, the bare String alt) and the failure case yielded the
  expected error. Zero leaks: the success path (a / 42 / !) exits 0
  with malloc diff 0 and remaining nodes 0; the error path exits via
  C `exit(1)`, which skips the harness epilogue, so the canonical
  stats are unavailable there — an ASan build shows no heap errors
  and only the harness-baseline LSan leaks (see the new fact above).

- Item 7a (2026-09-10): the `Many` slow path is CODE-COMPLETE in
  `interpreter/intrp-emit.toc` — `defp emit-many [child ctx]` (the
  flipped-receiver protocol: the receiver is the CHILD combinator; the
  DEFAULT body is the loop path, the char-level ctors get the run-path
  impls in item 7b), the site-(b) acc-recursion loop defn emission
  (split per the codegen hazards below: `many-loop-step` [acc name ref
  i] builds the body lines one step per i = 0..4 in its own defn — a
  nested `append-acc` as a direct call argument is miscompiled, see
  the fact below; `many-loop-lines-acc` recurses passing the step
  helper's RESULT, never an inline `append-acc`; `many-loop-lines`
  seeds it; `many-loop-defn` let-binds the body and passes the LET-
  BOUND value to `wrap-defn-acc` — the inline nested `append-acc` as
  `wrap-defn-acc`'s second argument was miscompiled: it arrived as a
  raw Term (type 6) and `count` crashed with `No implementation of
  'count' found for type <unknown> (6)`), `many-child-defns` (the
  lifted child helper when the child is anonymous), the `Many`
  `emit-body` impl (the call expression only — the defn comes from
  `emit-many`), the `Many` `child-ref` impl (`(<prefix><i> <sv>
  empty-vector)`), `Many` in `walk-children`, the `lifted-child-block`
  dispatch (a lifted Many child is the loop defn via `but-last`
  `emit-many`; every other lifted child the sub-helpers + `emit-fn`),
  `emit-rule-block`'s Many branch (a Rule whose parser is a Many gets
  the loop defn named `<rule>-0` and the rule defn is the one-arg
  wrapper), and the `prefix-name` / `but-last` / `last-line` helpers.
  The driver (`emit-pred.toc`) gains the synthetic mn grammar
  (`mn-alpha` = Rule over CharRange a-z; `mn-run` = Rule over Many of
  the named Rule — the loop defn `mn-run-0`, the rule defn the
  wrapper, not called by the entry; `mn-pair` = anonymous All of two
  CharRanges; `mn-entry` = Rule over All of [Many of mn-alpha,
  Many of mn-pair, bare String "!"]), the `want-mn-*` exact-
  fingerprint checks (10 checks total), and the `gen-sample-ok.toc`
  write (the success-case sample; `gen-sample.toc` is now the failure
  case). Every `want-*` fingerprint line that ends a defn was audited
  for the missing-bracket/missing-paren class of error (the item-6
  `Missing "]"` shape) — the mn fingerprints were built from the
  verified generated output, not by hand.
  Verification (2026-09-10, the fourth-degradation window — see the
  BROKEN AGAIN fact): FUNCTIONAL half of the done-when VERIFIED and
  deterministic (6/6 reruns): `make emit-pred` prints 10/10 OK
  (digits, upper-case, lower-case, alpha, symbol-start, rest-of-
  symbol, emit-module, emit-module-ig, emit-module-an, emit-module-
  mn); the generated module (`interpreter/gen-rdr.toc`) loads, builds,
  and compiles; `./gen-rdr interpreter/gen-sample.toc` (abc0a1b! / x)
  prints `[[a b c] [[0 a] [1 b]] !]` then
  `interpreter/gen-sample.toc:2:expected "!"`, exit 1 — the loops
  terminate on non-matching input (mn-entry-0 stops at "0", mn-entry-1
  at "!") and the failure case yields the expected error; `./gen-rdr
  interpreter/gen-sample-ok.toc` (abc0a1b! / !) prints `[[a b c]
  [[0 a] [1 b]] !]` then `[[] [] !]`, exit 0 — the zero-length Many
  match works (both loops return empty vectors, then "!"). LEAK half
  PENDING a healthy window: in the current window the pristine HEAD
  driver leaks 19 (was 0 on 2026-09-09) and this driver leaks 30
  (the 19 + 11 from the mn emission path — deterministic 6/6); the
  +11 is attributed to the current toolchain state, not a source
  bug (the emitter code is pure — no globals, no mutation — and the
  same acc-recursion shapes are used throughout the pre-7a emitter).
  New codegen hazards found (now facts below): the nested-`append-`
  `acc`-as-direct-call-argument miscompile, and the vector-literal-
  containing-`to-str`-over-a-let-bound load crash (use `conj` for the
  1-element vector instead).

- Item 7b (2026-09-10): the `Many` fast path is CODE-COMPLETE and
  VERIFIED in `interpreter/intrp-emit.toc` — `many-run-defn` (the
  `<name>-char` set-predicate defn from the child's `emit-pred` +
  the `read-run` wrapper with the `Token` → `ParserMatch` wrap; the
  wrapper's last line ends with FOUR closes after the final `t`
  — `.state`, `ParserMatch`, `let`, defn — the ctor-table shape),
  `many-run-path` (the defns + the call expression `(<name> state)`
  as the last line), `many-loop-path` (the item-7a default body
  extracted so the `Any` impl can fall through to it), the `emit-`
  `many` impls (CharRange / NotChar / bare String → run path; `Any`
  → run path over the combined `(or ...)` pred when ALL alts are
  char-level, loop path when mixed — no abort at a legitimate mixed
  site; Rule / Many delegate per the ctor table; the default body
  stays the loop path), and `alt-char-level?` / `all-alts-
  char-level?` (the alt-level char queries — a plain `defn` over
  `type-name`, the same dispatch style as `lifted-child-block`'s
  Many check; the flipped-receiver protocol stays the Many fast/slow
  classification; `all-alts-char-level?` is forward-declared at the
  top of the file — it and `alt-char-level?` are mutually
  recursive). The `Many` `child-ref` impl moved to the item-7b
  section and is now PATH-AWARE: fast path → `(<name> <sv>)`, loop
  path → `(<name> <sv> empty-vector)` (it needs `alt-char-level?`,
  defined later than the item-5 section). Consequence: a `Many` of
  a char-level Rule is now FAST (the Rule delegates) — the item-7a
  mn fingerprints for `mn-run-0` / `mn-entry-0` changed from loop
  defns to the fast-path shape (the `-char` defn is named after the
  Many's helper name: `mn-run-0-char`, not `mn-run-char`), and the
  mn module is now the fingerprint check only. The driver gains the
  fn synthetic grammar (`fn-alpha` = Rule over CharRange a-z;
  `fn-digits` = Rule over Many of a CharRange — fast; `fn-word` =
  Rule over Many of the char-level Rule — fast via delegation;
  `fn-entry` = All of [fn-digits, fn-word, Many of the mixed Any
  fn-mix (fn-alpha alt + anonymous All alt — SLOW: lifted to
  fn-entry-2-0, the All to fn-entry-2-0-1, its CharRange to
  fn-entry-2-0-1-0, loop defn fn-entry-2), bare String "!"]), the
  `want-fn-*` exact-fingerprint checks (11 total), and the fn
  module + samples are what get written to `gen-rdr.toc` /
  `gen-sample.toc` (failure: `123abc45!` / `*`) / `gen-sample-
  ok.toc` (success: `123abc45!` / `!`). Verification (2026-09-10,
  the fourth-degradation window — see the BROKEN AGAIN fact's fifth
  update): `make emit-pred` prints 11/11 OK, deterministic 3/3
  reruns, remaining nodes 0; the driver leaks 45 = the 30 HEAD
  baseline (drifted from 19) + 15 from the fn emission path — the
  leak half is PENDING a healthy window as in 7a. The generated
  module loads (`*** Loaded`), builds, and compiles; the SUCCESS
  path (`123abc45!` / `!`) prints `[123 abc [[4] [5]] !]` then
  `[  [] !]` (the two zero-length fast runs render as empty
  strings), exit 0, malloc diff 0, remaining nodes 0; the FAILURE
  path (`123abc45!` / `*`) prints `[123 abc [[4] [5]] !]` then
  `interpreter/gen-sample.toc:2:expected "!"`, exit 1 — each fast-
  path run is ONE string (`123`, `abc`), the slow loop yields a
  vector of child values (`[[4] [5]]` — each child value is the
  All's one-element vector), and the loop terminates on the
  non-matching `*`. New fact recorded: a 5/5 silent-abort is NOT
  always the toolchain — an unbalanced-paren source error crashed
  new-toc silently 5/5 this run (see the fact above).

- Item 7c (2026-09-10): `Recur` + self-recursion is CODE-COMPLETE
  and VERIFIED in `interpreter/intrp-emit.toc` — the `Recur`
  `emit-body` impl (a call to `(.rule ctx)` over `state`; the `f`
  field is data the emitter ignores), the child-position treatment
  (`Recur` `child-ref` impl → `(rule-name sv)` — referenced by the
  enclosing rule name like `Rule`; `Recur` `child-lifted?` impl →
  `None`, never lifted — no trivial wrapper defn), `contains-recur?`
  / `contains-recur-any` / `contains-recur-any-acc` (a plain defn
  over `type-name`, the `alt-char-level?` dispatch style;
  `contains-recur?` forward-declared at the top — mutually recursive
  with `contains-recur-any-acc`), `rule-decls-acc` / `rule-decls`
  (a `(def <name>)` line for every rule whose subtree contains a
  `Recur`), and the `emit-module` final form (header + rule-decls +
  one parser defn per Rule + main template; the header + decls are
  joined in a LET — a nested `append-acc` as a direct call argument
  is the codegen hazard; the pre-7c `->` threading into
  `emit-rules-acc` was fine because its first param IS the acc, but
  threading into `rule-decls [rules]` was the silent-no-op bug — see
  the fact above). Consequence: the generated module carries a
  `(def <rule>)` forward declaration after the header for every
  self-recursive rule — the template note "self-recursion needs
  none" is REFUTED for the lifted-helper shape (see the fact above).
  The driver gains the rc mini S-expression grammar (`rc-symbol-ish`
  = Rule over CharRange a-z — a SINGLE char, non-empty: the entry
  and the Many-loop child must never match zero-length or the loop
  spins the term buffer — the first version, a `Many` of the
  CharRange, matched empty and spun to `new.c:264`; the real
  grammar's `symbol` is non-empty too; WARNING for item 8: the real
  `expression`'s first alt `int-literal` = `Many digits` is
  zero-length-capable — see the fact), `rc-entry` = Rule over Any
  [rc-symbol-ish, All ["(" (Many (Recur grammar/sub-expression))
  ")"]] (the `f` field is the grammar's `sub-expression` defn —
  data the emitter ignores), the `want-rc-*` exact-fingerprint
  checks (12 total), and the rc module + samples are what get written
  to `gen-rdr.toc` / `gen-sample.toc` (failure: `a` / `(def(ghi))` /
  `(abc`) / `gen-sample-ok.toc` (success: `a` / `(def(ghi))` /
  `((x))`). Verification (2026-09-10): `make emit-pred` 12/12 OK,
  deterministic 3/3, remaining nodes 0 (the driver leaks 53 = the 45
  HEAD baseline measured in the same window + 8 from the rc path —
  the leak baseline drifts with machine state; the leak half is held
  to the generated module's success path, as in 7a/7b); the generated
  module loads (with the `(def rc-entry)` crutch), builds, and
  compiles; the SUCCESS path prints `a` / `[( [d e f [( [g h i] )]]
  )]` / `[( [[( [x] )]] )]`, exit 0, malloc diff 0, remaining 0,
  deterministic 2/2 (nested input to the correct vector-of-text
  values — `to-str` renders strings bare and vectors bracketed, space-
  joined); the FAILURE path prints `a` / `[( [d e f [( [g h i] )]]
  )]` / `interpreter/gen-sample.toc:3:expected ")"`, exit 1 (the
  unterminated `(abc` line: the Many loop stops at EOF, then the
  missing `)` errors — the loop terminates on non-matching input). New
  facts recorded: the self-recursion crutch (refutes the template
  note), the silent-no-op wrong-type-field-access hazard, the
  zero-length-match term-buffer spin (with the item-8 warning).

- Item 8 (2026-09-10, INVESTIGATION ONLY — box left unchecked,
  STUCK): the real grammar cannot produce the item-8 corpus as
  written; two design gaps + one toolchain blocker, all verified
  this run. (1) ZERO-LENGTH COMMIT: `expression`'s first alt
  `int-literal` (Rule name `integer`, a `Many digits` fast path) can
  match EMPTY — `read-run` (intrp-rdr.toc:195) returns a zero-length
  `Token` on a non-matching prefix, so `integer` is
  `(ParserMatch "" state)` on any non-digit input — and `parse-or`
  commits a Match without trying the next alt. Runtime-proven with a
  throwaway module carrying the generated `digits`/`integer` defns
  verbatim + the generated parse-seq (entry `integer`): input `abc`
  and `123.45` both die with `Error: Not enough space to allocate
  pair. buffEnd=1048576, buffSize=1048576 at new.c:264` (the 7c
  spin); input `123` prints `123`, exit 0, diff 0, remaining 0. In
  the real module `expression` therefore matches empty on every
  non-digit-leading input and `parse-seq` spins — every corpus case
  (symbols, strings, calls, malformed lines) is unparseable. The
  plan anticipated this ("item 8 must address it (non-empty number
  shape in the grammar data, or a consumption guard) — the plan does
  not settle it"). (2) DEAD FLOAT: `float-literal` is unreachable in
  the real `expression` — `integer` is tried FIRST: digit-leading
  input commits non-empty (`123.45` splits into `123` + `.45`, and
  the `.45` remainder spins per (1)); dot-leading input commits the
  EMPTY integer match. The float rule itself works when reachable:
  the same generated `float` defns with entry `float` parse `.45` to
  `[ . 45]`, exit 0, diff 0. The corpus's "floats (incl. multi-digit
  both sides)" is impossible with the current grammar data; the fix
  is a grammar-DATA reorder (float before int in the `expression`
  Any) that the plan does not cover anywhere — and even float-first
  leaves gap (1) breaking all non-numeric input, so BOTH fixes are
  needed. (3) CODEGEN CRASH: the full emitted real-grammar module
  (18 KB, 10 rules) and a symbol-rules-only sub-module both crash
  new-toc codegen deterministically (truncated C, silent abort or
  exit 0 — 11+ attempts across several windows); the trigger is
  isolated in the two new facts above (≥2 inlined bare-String conds
  in a parse-or position — `symbol-start` / `rest-of-symbol`).
  Owner decisions needed: the zero-length fix (data vs guard, and
  the value-shape consequence — a non-empty `All [digit (Many
  digits)]` int renders `[4 [2]]` for `42`), the float reorder (data
  edit to the owner's actively-edited `intrp-grammar.toc`, which has
  uncommitted owner changes), and the inlined-cond codegen crash
  (lift bare-String alts to helper defns — a site-(a) template
  change — or a toolchain fix). UPDATE (2026-09-10): the owner
  resolved (1) and (2) as grammar-DATA edits in `intrp-grammar.toc`.
  (1) `int-literal` is now `(Rule "integer" (All [digits (Many
  digits)]))` — non-empty, so the zero-length commit is gone; the
  value-shape consequence stands (an int renders as
  `[<first-digit> [<rest...>]]` — `42` → `[4 [2]]`, `7` → `[7 []]`;
  corpus expectations must use this shape). (2) `float-literal` was
  REMOVED from `expression`'s alts — the rule def remains in the
  file but is unreferenced and will not be emitted; floats are out
  of scope for the item-8 corpus for now (the reorder option was
  rejected in favor of dropping floats). Also in the same edit:
  `whitespace` is now `(Many linear-whitespace)` (`Any ["," " "
  "\t"]`) — a comma is whitespace (owner-intentional). Blocker (3)
  (the inlined-cond codegen crash) remains open. Load verification
  PASSED (2026-09-10, re-run after a bogus 'blocked' call): 10/10
  runs print `*** Loaded interpreter/intrp-grammar.toc` followed
  only by the two baseline-noise lines (the missing-main abort
  path; exit 134 is a normal clean-load exit for library files).
  The earlier 'degraded toolchain window' claim was a BOGUS TEST —
  a shell redirection bug (`out=$(cmd > /dev/null 2>&1)` sends
  stderr to /dev/null too, so the captured output was always empty
  and every run 'failed' the grep); the file was loading clean all
  along. The capture rule is now recorded in AGENTS.md's
  diagnostics rules. UPDATE 2 (2026-09-10, implementation): blocker
  (3) is RESOLVED by the grouped-literal site-(a) form (owner
  decision 2026-09-10): ALL bare-String alts of an Any collapse into
  ONE inlined cond (a flat (or ...) of str-prefix? tests) at the
  FIRST String alt's slot — one inlined cond in parse-or position is
  the verified-building shape (the crash was ≥2 inlined bare-String
  conds). Implementation found and fixed two emitter bugs the
  driver's an-any check (1 String alt) could not see: (a)
  any-body-refs-acc passed bare `Some` (a fielded ctor, not a value)
  as the group-done marker — it arrived as a Function at runtime
  (No implementation of 'cond' found for Function); fixed to
  `(Some None)`. (b) group-result-lines left the grouped cond
  UNCLOSED in the ≥2-literal case (the three result lines netted +1:
  the subs value leaves ParserMatch open across the take-char line,
  which carries one close only) — the ParserError line now ends with
  three closes (itself + ParserMatch + cond), making the group's
  lines self-balanced in any parse-or slot. Driver extended to 13
  checks (12 + emit-module-real: the full real-grammar module
  fingerprint, 196 lines); on a match it writes
  interpreter/gen-rdr.toc (the real module) plus the item-8 corpus
  files: gen-corpus.toc / -want.toc (the success case — 17 lines:
  ints, strings with each escape, symbols incl. operator names,
  nested calls), gen-corpus-bad{1,2,3}.toc / -want.toc (the failure
  cases: unterminated string, bare ")", trailing garbage after a
  complete expression — each a separate input because the generated
  main exits at the first error), and gen-corpus-empty.toc (the
  empty-input case; must print nothing). `make gen-corpus` runs the
  built gen-rdr over the corpus and diffs. VALUE-SHAPE DEVIATION:
  the plan predicted `[4 [2]]` for `42` (the slow-path Many shape),
  but the fast-path Many (read-run) returns a SINGLE string for the
  whole run, so `42` renders `[4 2]` and `7` renders `[7 ]` — the
  corpus expectations use the actual shape. STATUS: driver 13/13
  OK, remaining 0; the real-module build + corpus run are pending a
  healthy window (the crash-set drift: in the current window even a
  minimal 26-line grouped-cond probe fails 3/3 with the PEG reader's
  'malformed cond' misparse, while the same full module built 3/3
  earlier in the session — verify in a healthy window, per the
  blocker-3 note).

- Item 2a (2026-09-04): the closure-capture probe PASSED — the
  site-(b) shape is clean. The scratch driver (`scratch/probe-2a.toc`,
  never committed from there) carries a local 3-ctor result deftype
  (`PMatch [value state]` / `PIgnore [state]` / `PErr [msg state]`),
  the settled `parse-then` / `parse-or` kit verbatim, and a site-(b)
  acc-recursion (`many-digit` over a one-char `digit` parser) that
  rebuilds BOTH capturing continuations on every iteration — the
  `parse-then` fn captures `acc`, the `parse-or` else-fn captures
  `acc` and the state — with `acc` growing one element per iteration.
  Run over 2000 chars of input (2000 iterations — ~10x the ~194-node
  mark where the reduce-capture leak exhausted the 1MB term buffer):
  exit 0, malloc diff 0, remaining nodes 0, ~598k ITRS; deterministic
  across reruns and a full rebuild. The reduce-capture leak does NOT
  apply to this shape: the continuations are plain `fn` literals
  rebuilt per iteration, and the accumulator is threaded as a plain
  `defn` parameter — the same distinction as the documented
  reduce-vs-acc-recursion fix. The site-(a)/(b)/(c) kit templates
  stand; task 4 is unblocked. Probe hazards hit (now facts above):
  `inline` not allowed in a `cond` clause; the inline body must set
  `result` (a `return` breaks the void fn); `str-append` to a static
  string literal overflows the global buffer.

## Ralph loop — task list

Protocol: work top to bottom, one item per session; check an item off
only when its "done when" holds, then commit. Context for every item:
this file + AGENTS.md. Generated code follows `docs/toccata-style.md`.
Note: NEVER generate inline code. When needed, ask the user to provide
a solution.

Toolchain health gate (the crash set drifts with machine state — see
the BROKEN-AGAIN facts in Inherited verified facts): at the START of
each iteration, before any edit, `make emit-pred` must pass on the
committed state (every item commits, so the tree is clean at
iteration start). If it fails with silent crashes that survive the
5-retry rule, the window is degraded — do NOT edit source to chase a
degraded window; record the window state (which files crash, retry
counts) in the as-built note and stop the iteration. Leak half of a
done-when: if the committed state's driver already leaks in this
window (baseline drift), the requirement is leak == the baseline
measured on the committed state in the same window, recorded in the
as-built note (the item-7a/7b precedent).

- [x] **1. Owner decision: result-discrimination pattern (+ emitter
    classification ruling)**
  Settle the open items above: (a)–(d) site shapes get an exact
  generated-code template (which of options A/B/C, or a variant); the
  emitter's `Many` fast/slow classification gets a ruling (tag
  protocol vs flipped-receiver `emit-many`).
  - Done when: the owner writes the decision into this file's Settled
    section, replacing the Open items section, with the concrete
    generated-code template for each of (a)–(d).

- [x] **2. Grammar data file: `interpreter/intrp-grammar.toc`**
  Move the top-level `intrp-rdr.toc` (the fetched `67a1125` content,
  183 lines) to `interpreter/intrp-grammar.toc`; delete the top-level
  copy. Port to new-toc: drop the `main`; drop the `ParserState` and
  `ParserResults` deftypes (the generated module gets them from the
  helper layer — keeping them here would be a second, divergent
  definition); drop the `!` annotations on the multi-field
  `ParserCombinator` ctors (new-toc hazard); keep the `str-vect` impls
  if they compile clean (debug printing of the grammar); keep the
  `sub-expression` defn (the `Recur` `f` field is data; the emitter
  ignores it).
  - Done when: `./new-toc interpreter/intrp-grammar.toc > /dev/null`
    shows `*** Loaded interpreter/intrp-grammar.toc` with no other
    error lines (transient-crash retry rule applies), and the
    top-level `intrp-rdr.toc` is deleted.

- [x] **2a. Closure-capture probe (gates the kit templates)**
  Scratch driver (never committed from `scratch/`; built manually
  with the `rdr-top` recipe — no Makefile target): a
  self-contained mini-module mirroring the generated `Many` slow
  path — a local 3-ctor result deftype, the `parse-then` /
  `parse-or` kit extended over its ctors, and a site-(b)
  acc-recursion that rebuilds its capturing continuations (capturing
  `acc` and the threaded state) on every iteration, run over input
  long enough to have exhausted the 1MB term buffer under the
  documented reduce-capture leak (that leak fired over ~194 nodes —
  use ≥ 1000 iterations).
  - Done when: the probe builds and exits 0 with zero leaks and 0
    remaining nodes, and the outcome is recorded as an as-built note
    in this file. If it leaks, the site-(a)/(b)/(c) templates are
    re-opened before task 4.

- Items 3–7c (the v1 emitter: skeleton + char-level, leaf bodies +
  pipeline, `All` + `Ignore`, `Any`, `Many` slow, `Many` fast,
  `Recur`) are SUPERSEDED (2026-09-26) by items 10–16 below — the
  fold-based emitter v2 rebuilds the same generated-code contract,
  verified byte-exactly against the committed expected files. Their
  as-built notes are retained above; the v1 source is in git
  history. Old item 8 re-lands as 17; old item 9 as 18.

- [ ] **10. Emitter v2: `NodeIR` + `analyze` (the fold)**
  Rewrite `interpreter/intrp-emit.toc` IN PLACE: the `NodeIR`
  deftype (no `!` annotations), `h` — ONE function dispatching on
  `type-name` (after `recurse`, container ctors' fields hold the
  children's NodeIRs; leaves hold raw fields — see the Settled v2
  API) — and `analyze [pc]` = `(fold pc h)` (the hvm-core
  recursion scheme, hvm-core.toc:587 — no new walker). The v1
  `EmitCtx` / `emit-pred` / `emit-module` entry points stay callable
  on their v1 bodies until items 11–15 replace them — the driver is
  one binary and must build and pass at every step.
  - Done when: the library loads clean (`*** Loaded
    interpreter/intrp-emit.toc`, retry rule applies); `make
    emit-pred` still passes all 13 diffs (the v1 paths are
    untouched); a temporary interpreter-side probe (deleted before
    the commit — scratch probes cannot add-ns interpreter modules)
    folds `grammar/digits`, `grammar/alpha`, `grammar/symbol-start`,
    `grammar/expression` and prints the hand-verified top-node
    classifications (digits: char-level; alpha: char-level — Any of
    two char-level Rules; symbol-start: char-level — Any of a Rule
    + bare Strings; expression: NOT char-level); zero leaks, 0
    remaining nodes.

- [ ] **11. Emitter v2: `emit-pred` via analyze + render**
  The render phase for char-level IRs; `emit-pred [pc ctx]`
  switches to the v2 path: `analyze`, then render the predicate per
  the Ctor table's emit-pred column (the abort on a non-char-level
  IR stays the emitter-bug signal). Note: the v1 `Many` fast path
  builds the `<name>-char` set-predicate defn by calling
  `emit-pred` — after this item it calls the v2 defn; the module
  diffs are the drift catcher, so the v2 predicate output must stay
  byte-identical.
  - Done when: `make emit-pred` — the six predicate diffs pass
    byte-identical (scratch/emit-got/{digits,upper-case,lower-case,
    alpha,symbol-start,rest-of-symbol}.txt vs
    interpreter/emit-want/), and the seven module diffs still pass
    (v1 `emit-module` untouched); zero leaks, 0 remaining nodes.

- [ ] **12. Emitter v2: module assembly + leaf bodies**
  The render's module assembly: the header, the `(def <rule>)`
  crutches (EVERY rule — see the Generated module template), one
  parser defn + helpers per Rule, and the v1 main template verbatim.
  The render for the one-char parsers (`CharRange` / `NotChar` /
  bare `String` let-free), `AlwaysSucceed`, and `Error`. `All` /
  `Ignore` / `Any` / `Many` / `Recur` still fall back to the v1
  emission (the incremental swap — lockstep, see the Settled v2
  API).
  - Done when: `make emit-pred` — all 13 diffs pass byte-identical,
    with module.txt generated fully via the v2 path (the one-rule
    char-level grammar — assembly + crutches + one-char body + main
    template exercised end-to-end) and the `ig-alpha` block of
    module-ig.txt via the v2 path; zero leaks, 0 remaining nodes.

- [ ] **12a. Emitter v2: `All` + `Ignore` + the lifting rule**
  The render for `All` (site-(c) nested `parse-then`), `Ignore`
  (site-(c) shape — the fn captures nothing; no helper needed), and
  the lifting rule (anonymous non-String combinators to index-path
  helpers, emitted BEFORE the Rule defn that calls them — use-
  after-definition; bare `String` inlines everywhere).
  - Done when: `make emit-pred` — all 13 diffs pass byte-identical,
    with the `ig-all` and `ig-ignore` blocks of module-ig.txt now
    via the v2 path (module-ig is now fully v2 — it contains no
    Any/Many/Recur); zero leaks, 0 remaining nodes.

- [ ] **13. Emitter v2: `Any` (basic site-(a))**
  The render for parser-level `Any`: nested `parse-or` per the
  site-(a) template, anonymous alts lifted to index-path helpers,
  bare-String alts inlined as a single cond at their slot (with 0
  or 1 String alt the grouped form coincides with the plain form).
  An Any with ≥2 bare-String alts still falls back to the v1
  grouped emission (item 13a). `Many` / `Recur` still fall back to
  v1.
  - Done when: `make emit-pred` — all 13 diffs pass byte-identical,
    with module-an.txt now via the v2 path and the `expression` Any
    of module-real.txt (three named Rule alts + one anonymous All,
    no bare Strings) via the v2 path, while `symbol-start` /
    `rest-of-symbol` / `escaped-char` (≥2 bare-String alts) remain
    v1 fallbacks; zero leaks, 0 remaining nodes.

- [ ] **13a. Emitter v2: `Any` grouped-literal form (≥2 bare-String
    alts)**
  Collapse ALL bare-String alts of an Any into ONE inlined cond (a
  flat `(or ...)` of `str-prefix?` tests) at the first String alt's
  slot — the item-8 grouped-literal form (the inlined-cond codegen-
  crash resolution; ≥2 inlined bare-String conds in a parse-or
  position crash new-toc's codegen). The v1 fallback for ≥2-String-
  alt Any goes away. The an grammar has only ONE String alt, so
  this shape is exercised ONLY by module-real (symbol-start, rest-
  of-symbol, escaped-char) — watch the two bug shapes the item-8
  note recorded: the group-done marker must be `(Some None)`, not
  bare `Some`; the group's lines must be self-balanced (in the
  ≥2-literal case the ParserError line ends with three closes).
  - Done when: `make emit-pred` — all 13 diffs pass byte-identical,
    with every `Any` of module-real.txt (symbol-start, rest-of-
    symbol, escaped-char, expression) now via the v2 path; zero
    leaks, 0 remaining nodes.

- [ ] **14. Emitter v2: `Many` slow path (loop)**
  The render for a `Many` whose child IR is NOT char-level: the
  lifted child (if anonymous) + the site-(b) acc-recursion loop
  defn (acc-recursion, NOT a reduce — the loop is not a vector
  walk) + the child-ref call `(<name> <sv> empty-vector)`. A
  char-level child still falls back to the v1 fast path (item
  14a). `Recur` still falls back to v1.
  - Done when: `make emit-pred` — all 13 diffs pass byte-identical,
    with the slow loop defns now via the v2 path (`mn-entry-1` +
    lifted child in module-mn.txt; `fn-entry-2` + lifted child in
    module-fn.txt), the fast-path Many defns still v1 fallbacks;
    zero leaks, 0 remaining nodes.

- [ ] **14a. Emitter v2: `Many` fast path (maximal run)**
  The render for a `Many` whose child IR IS char-level: the
  `<name>-char` set-predicate defn from the child's predicate + the
  `read-run` wrapper with the `Token` → `ParserMatch` wrap + the
  child-ref call `(<name> <sv>)`. The v1 fallback for `Many` goes
  away.
  - Done when: `make emit-pred` — all 13 diffs pass byte-identical,
    with every `Many` of module-mn.txt / module-fn.txt now via the
    v2 path (incl. the fast paths via Rule delegation: `mn-run-0` /
    `mn-entry-0` / `fn-digits-0` / `fn-word-0`); zero leaks, 0
    remaining nodes.

- [ ] **15. Emitter v2: `Recur`**
  The render for `Recur` (a call to the context's rule name over
  the threaded state; the `f` field is data the emitter ignores; in
  child position referenced by the rule name, never lifted). The
  `(def <rule>)` crutches already come from the item-12 module
  assembly (every rule). The v1 fallback is now unreachable for the
  driver's grammars.
  - Done when: `make emit-pred` — all 13 diffs pass byte-identical,
    with module-rc.txt now generated via the v2 path; zero leaks, 0
    remaining nodes.

- [ ] **16. Emitter v2: delete the v1 code; full driver regression**
  Remove the v1 emission paths from `interpreter/intrp-emit.toc`:
  the v1 protocols (`emit-body` / `emit-ref` / `emit-many` /
  `child-ref` / `child-lifted?` / `walk-children`) and the defns
  only they call. The split is mechanical — once the render is
  total, the v1 defps are dead code; delete them and whatever
  becomes unreachable. KEEP everything the v2 render and the driver
  entry points use (`EmitCtx`, the `emit-pred` / `emit-module`
  public signatures, `render-literal` / `escape-str`, `append-acc`,
  `emit-header` / `emit-main`, and any line helper the v2 render
  shares with v1). No new emission code expected.
  - Done when: the library loads clean (`*** Loaded
    interpreter/intrp-emit.toc`, retry rule applies); `make
    emit-pred` — all 13 diffs pass byte-identical, including
    module-real.txt vs interpreter/gen-rdr.toc; `make gen-rdr`
    builds the generated real-grammar module; zero leaks, 0
    remaining nodes.

- [ ] **17. The real grammar + corpus** (old item 8, re-landed on
  v2)
  `make gen-corpus` over the committed corpus
  (`interpreter/gen-corpus*.toc`): ints (as-built value shape
  `[<first-digit> <rest-as-one-string>]` — the fast-path Many
  returns one string per run: `42` → `[4 2]`, `7` → `[7 ]`; floats
  OUT), strings with each escape (`\\` `\"` `\n` `\r` `\t`), symbols
  incl. operator names (`+`, `*`, `->`, `!x`), nested calls, empty
  input (prints nothing), and the malformed lines (unterminated
  string, bare `)`, trailing garbage after a complete expression —
  each a separate input file because the generated main exits at the
  first error). The item-8 investigation content stands (zero-
  length commit fixed by the owner's non-empty `int-literal`;
  floats dropped from `expression`; the inlined-cond codegen crash
  resolved by the grouped-literal site-(a) form — see the item-8
  note, UPDATE 2).
  - Done when: every corpus case matches its `-want` file, zero
    leaks, 0 remaining nodes.

- [ ] **18. Final verification** (old item 9, re-landed)
  Zero leaks (malloc/free diff 0, remaining nodes 0) across: the
  grammar library load, the emitter library load, the driver, and
  the generated module over the full corpus. Every item above
  checked. This file updated with the v2 as-built notes (the v1 →
  v2 emitter delta, any output deviations — expected none — and new
  hazards hit).
  - Done when: all items checked.


