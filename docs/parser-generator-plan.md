# Parser Generator Plan (full grammar → generated Toccata reader)

Status: clean slate (2026-09-26). Supersedes
`docs/parser-generator-plan-bad.md` in its entirety — the v1 and v2
emitters are garbage (owner ruling); their source lives in git
history. Carried forward from the old plan: the `emit-want` byte-
exact test targets (re-blessed once, see items 5a/5b) and the
`parse-then` / `parse-or` result-discrimination kit (settled,
unchanged). The old plan file stays in the tree as-is for now; it is
deleted later.

## Goal

A generator, written in Toccata and built by `new-toc`, on the `fold`
recursion scheme (hvm-core.toc:586–599 over the `recurse` container
protocol), that takes the Toccata grammar as data
(`interpreter/intrp-grammar.toc`, grown from the expression-only
67a1125 port to the full scope-(a) language) and emits a **reader**
module in Toccata source that converts raw `.toc` files into a **raw
AST** — one AST node per top-level form, no desugaring.

The generated artifact is a reader, not a token printer: it exposes a
library `parse-program` (file → `ParserResults` over a `[TopLevel]`
vector) plus a thin CLI `main`.

**Acceptance (the final test):** the generated reader reads
`hvm-core.toc` (2101 lines; 194 top-level forms: 108 defn / 58 defp /
16 extend-type / 5 deftype / 5 def / 2 inline) and
`regression-tests/test11.toc` (7 forms: 2 def / 4 defn / 1 main) with
zero parse errors, verified by **structural assertions on file facts**
in an acceptance driver (item 7a) — not want-files (want-files test the
generator's output, not the generated reader), and no differential
against the hand-written reader (disowned: never finished or
verified).

Scope (a) — the grammar covers: top-level `def` / `defn` / `defp` /
`deftype` / `extend-type` / `inline` / `main`; expressions: calls,
`let`, `cond`, `fn`, `->` threading, `.field` getters, vectors, hash
maps, strings, ints, symbols; `!` type constraints as body elements
(`! sym Type`, `!returns Type`). Out of scope (the final task, item 8,
plans them): `match`, `do`, `defmacro`, `add-ns`, floats, `|`
(superposition is a sub-expression form only — it cannot appear at
the top level), and desugaring (future: composed recursion schemes,
TBD).

## Files

- `interpreter/intrp-grammar.toc` — the grammar data, extended IN
  PLACE: the `ParserCombinator` deftype (v3 ctor set) + the full
  scope-(a) grammar as purely acyclic data. Library, no main.
- `interpreter/intrp-emit.toc` — the emitter, rewritten IN PLACE for
  v3 (clean slate; v1/v2 source stays in git history). Library, no
  main.
- `interpreter/intrp-state.toc` — NEW: the helper kit (state /
  results / token) extracted from `interpreter/intrp-rdr.toc`.
  Library, no main.
- `interpreter/intrp-raw-ast.toc` — NEW: the raw AST deftypes.
  Library, no main. `interpreter/intrp-ast.toc` (the compiler plan's
  desugared AST) is UNTOUCHED.
- `interpreter/emit-pred.toc` — the acceptance driver for the
  generator, rewritten for the v3 contract (same synthetic-grammar
  families; the `rc` grammar rewritten `Recur` → `Ref`; a new
  `Node`-exercising grammar added).
- `interpreter/emit-want/*` — the expected files (14 after item 5a
  adds the `nd` grammar), RE-BLESSED once from the v3 emitter
  (regenerated in 5a, blessed in 5b); byte-exact net thereafter.
- `interpreter/gen-rdr.toc` — the generated reader module. Written by
  the driver, built by a Makefile target, a build artifact — NOT
  committed.
- `interpreter/rdr-accept.toc` — NEW: the acceptance driver
  (structural assertions over the two acceptance files).
- `interpreter/gen-bad*.toc` — NEW: the malformed-input corpus
  (replaces the retired `gen-corpus*` files).
- `Makefile` — `emit-pred` and `gen-rdr` targets kept; `gen-rdr`'s
  dep list swaps `interpreter/intrp-rdr.toc` for
  `interpreter/intrp-state.toc` + `interpreter/intrp-raw-ast.toc`;
  `gen-corpus` target dropped; new `gen-accept` target.
- `docs/parser-generator-plan-bad.md` — kept as-is for now; deleted
  later.

## Settled design

### Grammar data (v3)

**Ctor set** — `ParserCombinator` carries: `CharRange [lower upper]`,
`NotChar [char]`, `All [parsers]`, `Any [parsers]`, `Many [parser]`,
`Rule [name parser]`, `Ref [name]`, `Node [name]`, `Concat [parsers]`,
`Ignore [parser]`, `AlwaysSucceed [value]`, `Error [msg]` (+ bare
`String` literals). `Recur` is DROPPED. `Ignore` / `AlwaysSucceed` /
`Error` are kept — they will be used before this project is done
(owner ruling); two of them are used in scope (a) itself (see Value-
shape discipline).

**`Ignore` — redefined: parse-and-discard.** The render is
`(parse-then (<ref> state) (fn [_ s2] <rest>))` — parse the child,
discard the value, thread the state; inside an `All` it contributes
NO element to the value vector. It does NOT produce `ParserIgnore`
and does NOT short-circuit: the old plan's short-circuit semantics
(`Ignore` stops a `parse-then` sequence, wins a `parse-or` choice)
is SUPERSEDED — it was settled for the `skip-ws` use, which no longer
exists (whitespace is helper-layer). `ParserIgnore` remains a
`ParserResults` ctor in `intrp-state.toc` (the hand-written reader
uses it; the kit stays total over it), but generated code never
produces it. This is what lets form rules discard their keywords and
brackets: `(All [(Ignore "defn") symbol ...])`.

**`Concat [parsers]` — join to one string.** Flatten-join its
char-level children's values into a single `String` (a `Many` fast-
path run is already one string; `All`/vector fragments are spliced).
This is how the char-run value shapes become the raw AST's single
text fields: `symbol` → `"text"`, `int-literal` → `"42"`, string
content → the raw text with backslash escapes INTACT (unescaping is
a later-phase concern).

**`Ref [name]` — the only rule reference.** Every reference from one
rule to another (recursive or not, self or mutual) is `(Ref "name")`.
No `Rule` value ever appears inside another rule. Consequences:

- the grammar file is purely acyclic DATA — rule defs have no
  ordering constraints, no forward declarations, no defn-cycle
  tricks;
- self-recursion is `(Ref "expression")` inside `expression`'s own
  def;
- the fold is purely structural: ctor nodes recurse, `Ref` /
  `String` / leaf ctors are leaves — no force-time global resolution,
  no unnamed `Fn` values in the data.

**`Node [name]` — the AST tag.** `(Node "Defn" <parser>)` declares
that a successful parse at that position yields the raw-AST node
`(raw/Defn <sub-values...> <loc>)` instead of a plain value vector.
The emitter renders the ctor call; the name is DATA, so the emitter
stays fully generic (it knows nothing about Toccata). `Node` composes
at any depth (rule-level or sub-parse-level).

**Auto-loc.** A `Node`-tagged position appends
`(state/state-line <state at the Node position's entry>)` as the
FINAL field of the ctor call — the line where the form started. No
rule carries loc noise; no rule can forget it.

**Whitespace and comments are NOT grammar.** The helper layer owns
them: every generated parser entry (rule defns, lifted helpers,
inlined literal / one-char bodies, fast-path run wrappers) skips
whitespace+comments first (fixes the v1 gap — v1 generated code
cannot parse `( def x )`). The vestigial `linear-whitespace` /
`whitespace` / `skip-ws` rules come out of the grammar file.

**Rule inventory (scope a).** Token level: `digits`, `alpha`,
`symbol-start`, `rest-of-symbol`; `symbol` = `Node "Symbol"` over
`(Concat [symbol-start (Many rest-of-symbol)])` (start chars: alpha
`.` `_` `<` `>` `=` `+` `-` `*` `/`; continue adds digits `?` `!`;
`!` is NOT a start char — a leading `!` is a constraint, handled at
the body-element level); `field-symbol` = `Node "FieldGetter"` over
`(Ignore ".")` + a `Concat`ted name — tried BEFORE `symbol` in
`expression`'s `Any` (the only `FieldGetter` producer — the grammar
cannot inspect parsed values, so the dispatch is alt order).
Literals: `int-literal` = `Node "IntegerLit"` over
`(Concat [digits (Many digits)])` — non-empty (the owner's item-8
zero-length fix stands; `Concat` does not change the match shape);
`escaped-char`, `string` = `Node "StringLit"` over `(Ignore "\"")`
+ `Concat`ted content + `(Ignore "\"")`. Constraints (body
elements): `returns-constraint` = `Node "TypeConstraint"` over
`(All [(AlwaysSucceed "returns") <symbol>])` — listed BEFORE
`type-constraint` in `body-elt` (`"!"` prefix-matches `"!returns"`);
`type-constraint` = `Node "TypeConstraint"` over
`(All [(Ignore "!") <symbol> <symbol>])`. Expressions:
`expression` = Any of `int-literal`, `string`, `field-symbol`,
`symbol`, `group`, `vector`, `hash`; `group` = `(All ["(" group-head
")"])` with `group-head` = Any of the keyword-head forms (`let` /
`cond` / `fn` / `->`, each a `Node`-tagged rule that consumes its
full remainder) BEFORE the `call` catch-all (`Node "Call"` over
`(All [<expression-as-operator> (Many expression)])`) — keyword-
head alts fail fast on their literal and must precede the
catch-all or `(let ...)` is swallowed as a call. `vector` /
`hash` are `Node`-tagged (`VectorLit` / `HashLit`), brackets
`Ignore`d, pairs as `Node "HashPair"` / `Node "LetBinding"` rules.
`body-elt` = Any of the two constraints, `expression`. Top level:
`top-level-form` = Any of `defn-form`, `defp-form`, `deftype-form`,
`extend-type-form`, `inline-form`, `main-form`, `def-form` — `def`
LAST in its prefix family (`"def"` prefix-matches `defn` / `defp` /
`deftype`); each `Node`-tagged (`Defn` / `Defp` / `Def` / `DefType`
/ `ExtendType` / `Inline` / `Main`). Form rules inline their param
lists — `(Ignore "[") (Many <symbol>) (Ignore "]")` — so the value
shapes match the ctor fields exactly (no `param-list` rule: a rule
whose body is an `All` yields the `All`'s vector, which would nest
the param vector one level too deep). `deftype-ctor` = name +
field-list-OR-EMPTY (`Any [field-list (AlwaysSucceed empty-vector)]`
— the second `AlwaysSucceed` use) + `(Many method-form)`, so
`(None)`, `(Some [x])` and `(EndOfList (recurse [l f] l) ...)` all
parse to the same arity; fields are symbols or `!`-annotated.
`method-form` = name + inlined param list + body (the shape
`extend-type` methods and deftype impls share).

**Value-shape discipline.** A `Node`-tagged position's value vector
must match its raw-AST ctor's field list EXACTLY (the render spreads
the vector as ctor args, then appends the auto-loc). The tools:
`All` → vector of its non-`Ignore` sub-values; `Ignore` →
contributes nothing; `Many` (slow path) → vector (this is how
`[params]` and body elements become the ctor's vector fields);
`Concat` → one string; `AlwaysSucceed` → a constant sub-value
(the `"returns"` tag, the empty field list). CONSEQUENCE: every alt
of an `Any` whose result feeds a `Node` field or a shared position
must yield the SAME arity and shape — a mismatch is a wrong-arity
ctor call, caught at build time.

**Multi-char literals + `Any` ordering.** Bare `String` literals are
multi-char: the render is `(str-prefix? S input)` + N `take-char`
calls (the one-char case is N=1). Prefix-collision discipline: a more
specific alt must precede any alt whose literal it extends —
`defn` / `defp` / `deftype` before `def`; `!returns` before `!`; the
keyword-head forms before the `call` catch-all. (No word-bounding —
ordering is the mechanism.)

### Raw AST (`interpreter/intrp-raw-ast.toc`)

One ctor per source form; no desugared shapes; every ctor gets a
`str-vect` impl (printing + acceptance output).

- `Location [file line]`
- `TopLevel`: `Defn [name param-list body loc]`, `Def [name value
  loc]`, `Defp [name param-list body loc]`, `DefType [type-name
  ctors loc]`, `ExtendType [type-name methods loc]`, `Inline` (bare
  reference to `Expression`'s `Inline [type-expr c-code loc]` — a
  top-level inline form parses to that same ctor; the ctor-name-
  globality hazard forbids a second fielded `Inline`, owner ruling
  2026-09-26), `Main [param-list body loc]`
- `Expression`: `Symbol [text loc]`, `IntegerLit [value loc]`,
  `StringLit [value loc]`, `Call [operator operands loc]`,
  `Let [bindings body loc]`, `Cond [clauses loc]` (flat clause
  vector — pairing is a later concern), `Fn [param-list body loc]`,
  `Thread [expr steps loc]`, `FieldGetter [field-name loc]`,
  `VectorLit [exprs loc]`, `HashLit [pairs loc]`, `Inline [type-expr
  c-code loc]`
- auxiliaries: `TypeConstraint [symbol type-expr loc]`, `LetBinding
  [name expr loc]`, `Clause [test value loc]`, `HashPair [key value
  loc]`, `DefTypeCtor [name fields impls loc]`, `Method [name
  param-list body loc]`

**Every ctor's final field is `loc`** — `Node` always appends the
auto-loc, so any ctor it targets must end in `loc` or the call is
wrong-arity (this is why the auxiliaries carry it too). Bodies are
vectors that may mix `TypeConstraint` nodes and `Expression` nodes
(the `!` body elements). No `!` annotations on multi-field ctors
(new-toc hazard, inherited fact).

### Helper kit (`interpreter/intrp-state.toc`)

Extracted verbatim from `interpreter/intrp-rdr.toc` (which stays
untouched): `ParserState [input values]`, `ParserResults`
(`ParserMatch` / `ParserIgnore` / `ParserError`), `Token [text
state]`, `make-state`, `state-line`, `take-char`, `skip-comment`,
`skip-whitespace`, `run-length`, `read-run`. `str-prefix?` is CORE
(hvm-core.toc:663) — no extraction needed. The generated module
depends on `intrp-state.toc` + `intrp-raw-ast.toc` and on NOTHING
else — the dependency on the disowned hand-written reader is severed.

### Emitter (v3, clean slate, fold-based)

Two phases over the grammar data:

**Phase 1 — analyze.** `analyze [pc]` = `(fold pc analyze-node)` —
the core `fold`, no new walker. The handler is a PROTOCOL —
`defp analyze-node` + one `extend-type` impl per ctor type (the v1
EBNF emitter's pattern, `intrp-ebnf.toc`), NOT a defn dispatching
on `type-name`: a defn handler (even the parameter-split shape
`h` / `h-dispatch`) deterministically miscompiles as soon as the
fold reaches a map-recurse ctor (Any / All / Concat) — "Compiler
screwed up. Incomplete result" (Tag SUB), 5/5, even with a plain
no-cond handler (Inherited verified facts, 2026-09-27 item 4a;
deviation recorded in the item-4a as-built note). After `recurse`
reassembles a node, container ctors' fields hold
children's IR values; leaves hold raw fields. `Ref` and `String`
are leaves, so the fold terminates structurally even though the
generated reader is recursive. The IR is the bare `IRNode`
multi-ctor deftype — 13 ctors mirroring the grammar's (bare `String`
→ `Str`, since a ctor named `String` collides with the core `String`
type), with named fields unique across every type in the build (so
the render phase uses direct `.field` getters); the char-level
classification is the `char-level` protocol over the `IRNode` ctors
(`Some None` iff the node classifies a single char — CharRange /
NotChar / one-char Str; Any iff all alts; Rule / Many iff child).

**Phase 2 — render.** A plain `defn` walks the IR with context
(enclosing rule name + helper-name prefix) and emits source per ctor:

| Ctor | Render |
|---|---|
| `CharRange` / `NotChar` | one-char parser (skip first, then pred); bodies are let-free and inline at use sites |
| bare `String` | literal match, multi-char: `(str-prefix? S input)` + N `take-char`s (N=1 for one char); let-free, inlines at use sites |
| `Any` | nested `parse-or` (site-(a)); anonymous alts lifted to index-path helpers; ≥2 bare-String alts collapse into ONE inlined grouped cond (the item-8 grouped-literal form — the inlined-cond codegen crash resolution stands); ALT ORDER IS SEMANTICS — the grammar's ordering (longest-literal-first, keyword-heads before catch-alls) is preserved verbatim |
| `All` | nested `parse-then` (site-(c)); value = vector of its non-`Ignore` sub-values, UNLESS `Node`-wrapped |
| `Many` | char-level child → fast path (`<name>-char` set-predicate defn + `read-run` wrapper, run as ONE string); otherwise slow path (lifted child + site-(b) acc-recursion loop, value = vector) |
| `Rule` | module-level `(defn <name> [state] ...)`; in sub-position, a bare-name call |
| `Ref` | a call to the named rule over the threaded state — self or mutual, uniformly |
| `Concat` | parse the children in sequence, flatten-join their string values into ONE string (the value of the position) |
| `Node` | the child's value vector becomes `(raw/<name> v0 ... vN (state/state-line s-entry))` — the vector is spread as ctor args; arity is the grammar's responsibility (Value-shape discipline) |
| `Ignore` | `(parse-then (<ref> state) (fn [_ s2] <rest>))` — parse, discard the value, thread the state; contributes nothing to an enclosing `All`'s vector; produces no `ParserIgnore` (see the ctor-set section) |
| `AlwaysSucceed` / `Error` | `(ParserMatch <v> state)` / `(ParserError MSG state)` |

**Module assembly.** Header (`add-ns` state + raw; the
`parse-then` / `parse-or` kit; the parse-error kit), then
`(def <rule>)` crutches — **only for rules in a cycle** (self-`Ref`
or mutual; computed over the `Ref` name graph), then rule defns in
**topological order** (dependencies first, so every non-recursive
reference is a backward one and needs no crutch), then lifted
helpers, then `parse-program`, then `main`.

**Driver-facing API** (the driver and Makefile keep their shape):

```toccata
(deftype EmitCtx [rule prefix])
;; The char-level predicate source for pc, as a String. Aborts on a
;; non-char-level pc (emitter-bug signal).
(defn emit-pred [pc ctx])
;; The full generated module as [String] lines. `rules` = the
;; driver-supplied vector of the grammar's top-level Rules; the
;; emitter emits exactly the set reachable from `entry` over the
;; Ref name graph.
(defn emit-module [rules entry])
```

**Value semantics.** one-char parsers → the one-char string;
`Many` fast → the run as ONE string; `Many` slow → vector of child
values; `All` → vector of its non-`Ignore` sub-values; `Any` → the
winning alt's value; `Concat` → the joined string; `Node` → the
raw-AST ctor call (with auto-loc); `Ignore` → contributes nothing
to the parent value (state is threaded); `Error` → `ParserError`.

### Generated module template

`interpreter/gen-rdr.toc` (written by the driver):

```toccata
(add-ns state (module "intrp-state.toc"))
(add-ns raw (module "intrp-raw-ast.toc"))

;; the parse-then / parse-or kit (Result discrimination, below)
;; the parse-error kit (parse-error-msg / parse-error-state /
;;   parse-error-line — file:line: msg + exit 1)

;; (def <rule>) crutches — cycle rules only
;; rule defns (topological order) + lifted helpers

(defn parse-program [file]
  ;; make-state (slurp file), parse top-level forms until EOF,
  ;; return ParserResults over the [TopLevel] vector
  ...)

(main [argv]
  ;; thin CLI: parse-program over argv element 1; success → pr* the
  ;; AST (str-vect); error → file:line: msg, exit non-zero
  ...)
```

### Result discrimination (inherited; one semantics change)

The generated parser discriminates `ParserMatch` / `ParserIgnore` /
`ParserError` with the two-op protocol kit emitted once in the
module header — `parse-then` (Match applies the 2-arg continuation;
Ignore/Error short-circuit) and `parse-or` (Match/Ignore return r;
Error forces the else-fn — laziness IS the short-circuit) — with the
site templates (a) Any nested parse-or, (b) Many loop exit, (c) All
nested parse-then, (d) the main's error line. Full text in
`docs/parser-generator-plan-bad.md`, section "Result
discrimination". CHANGE: the old plan's `Ignore` short-circuit
semantics (its site template, the "`Ignore` stops a `parse-then`
sequence" note) is SUPERSEDED — the redefined `Ignore` (ctor-set
section) never produces `ParserIgnore`, so the kit's Ignore arms are
unreachable from generated code but stay for totality. Whitespace
remains helper-layer, never grammar.

### Test strategy

- **Generator output (want files):** the `emit-want` files (14 after
  item 5a) are regenerated from the v3 emitter over the same
  synthetic grammars (the `rc` grammar rewritten to `Ref`; a new `nd`
  grammar exercising `Node` + auto-loc + a mutual-Ref cycle added
  to the driver) and re-blessed ONCE after owner review (items
  5a/5b). The
  six predicate files are expected to survive byte-identical; the
  module files change (skip-at-entry, crutch policy, defn order,
  multi-char literals, the redefined `Ignore`). The `ig` grammar now
  tests the discard-and-continue `Ignore` semantics. Thereafter
  `make emit-pred` is the byte-exact net at every item.
- **The generated reader (acceptance files):** `make gen-accept`
  builds the generated module and runs `rdr-accept.toc`, which
  `add-ns`es it and asserts FILE FACTS (the generated module defines
  its own `main` — that is fine: `main` is just a global, and the
  driver's `main` is the one that runs; do not "fix" it): `hvm-core.toc` → 194
  `TopLevel` nodes, per-form counts 108 defn / 58 defp / 16
  extend-type / 5 deftype / 5 def / 2 inline, spot-checks (one
  defp's param list + `!returns`, one deftype's ctor list, one
  top-level inline's C-code prefix); `test11.toc` → 7 forms (2 def /
  4 defn / 1 main), spot-checks (the `->` threading form, the
  inline with escaped C). Zero leaks, 0 remaining nodes on the
  success path.
- **Error path:** a small malformed-input corpus (`gen-bad*.toc`,
  separate files because the CLI exits at the first error):
  unterminated string, bare `)`, trailing garbage after a complete
  form, empty file — each with expected `file:line: msg` output and
  non-zero exit.
- The old `gen-corpus*.toc` / `-want` files are RETIRED (they encode
  the old value shapes and the expression-only entry rule).

### Toolchain policy

Assume `new-toc` is perfect until proven otherwise (owner ruling,
2026-09-26): no window-chasing, no degradation-note ritual, no
baseline-drift bookkeeping. The standing diagnostic rules still
apply: always capture and read new-toc's stderr on a failed build;
a silent crash retries up to 5 times, then a stack-based
paren/nesting check and a known-good control file before the
toolchain is blamed; a 5/5 silent abort with balanced parens and a
healthy control IS evidence about the toolchain — record it and stop.

## Ralph loop — task list

Protocol: work top to bottom, one item per session; check an item
off only when its "done when" holds, then commit. Context for every
item: this file + AGENTS.md. Generated code follows
`docs/toccata-style.md`.

- [x] **1. Helper kit: `interpreter/intrp-state.toc`**
  Extract from `interpreter/intrp-rdr.toc` (which stays untouched):
  `ParserState`, `ParserResults`, `Token`, `make-state`,
  `state-line`, `take-char`, `skip-comment`, `skip-whitespace`,
  `run-length`, `read-run` — verbatim, plus the header comment
  adjusted (library, no main; namespaced by the generated module's
  `add-ns`, not by a reader).
  - Done when: `./new-toc interpreter/intrp-state.toc > /dev/null`
    shows `*** Loaded interpreter/intrp-state.toc` with no other
    error lines; `interpreter/intrp-rdr.toc` unmodified.

- [x] **2. Raw AST: `interpreter/intrp-raw-ast.toc`**
  The deftypes per the Raw AST section: `Location`, `TopLevel`
  (7 ctors), `Expression` (12 ctors), the auxiliaries
  (`TypeConstraint`, `LetBinding`, `Clause`, `HashPair`,
  `DefTypeCtor`, `Method`); `str-vect` impl on every ctor; no `!`
  annotations on multi-field ctors.
  - Done when: the library loads clean (same check as item 1);
    `interpreter/intrp-ast.toc` unmodified.

- [x] **3. Grammar v3 deftype + token/literal port**
  Rewrite the `ParserCombinator` deftype in
  `interpreter/intrp-grammar.toc`: drop `Recur`; add `Ref [name]`,
  `Node [name]`, `Concat [parsers]`; keep `Ignore` / `AlwaysSucceed`
  / `Error` (`Ignore` redefined — discard-and-continue, per the
  ctor-set section). Port the existing token and literal rules to
  the all-`Ref` convention (`expression`'s recursive positions
  become `(Ref "expression")`; the `sub-expression` defn trick is
  gone); delete the vestigial `linear-whitespace` / `whitespace` /
  `skip-ws` rules and the unused keyword/delimiter string defs.
  - Done when: the library loads clean; no `Recur`, no bare rule
    values in rule bodies (grep: every rule reference is a `Ref`).

- [x] **4a. Emitter v3: analyze (the fold + IR)**
  Rewrite `interpreter/intrp-emit.toc` in place (clean slate — the
  v1/v2 source is git history) for the analyze phase only: `NodeIR`
  (or equivalent) carrying the classification (`char-level?` —
  CharRange / NotChar / one-char String; Any iff all alts; Rule /
  Many iff child) plus the structure the render needs; `h` /
  `h-dispatch` (the parameter split, per the proven let-wrapping-
  cond miscompile shape); `analyze [pc]` = `(fold pc h)`. `Ref` and
  `String` are leaves, so the fold terminates structurally even
  though the generated reader is recursive.
  - Done when: the library loads clean; a temporary interpreter-
    side probe (deleted before the commit — scratch probes cannot
    `add-ns` interpreter modules) runs `analyze` over a small
    hand-written grammar and the IR is hand-verified (classification
    correct on every node, `Ref` / `String` / leaf ctors are leaves,
    container fields hold their children's IR).

- [x] **4b.1. Emitter v3: render — leaf ctors**
  Extend `interpreter/intrp-emit.toc` with render functions for the
  leaf ctors, exactly per the Ctor table: `CharRange` / `NotChar`
  (one-char parser — skip first, then pred; let-free, inlined at use
  sites); bare `String` (multi-char literal: `(str-prefix? S input)`
  + N `take-char` calls; N=1 for one char; let-free); `Ref` (a call
  to the named rule over the threaded state); `AlwaysSucceed` /
  `Error` (`(ParserMatch <v> state)` / `(ParserError MSG state)`).
  No context needed yet.
  - Done when: the library loads clean; a temporary probe (deleted
    before the commit) renders each leaf shape and each emitted
    source is hand-verified against the Ctor table.

- [x] **4b.2. Emitter v3: render — wrapper ctors**
  Extend the render with the wrappers: `Ignore` (parse-and-discard:
  `(parse-then (<ref> state) (fn [_ s2] <rest>))`); `Concat` (parse
  the children in sequence, flatten-join their string values into
  ONE string); `Node` (the child's value vector spread as
  `(raw/<name> v0 ... vN (state/state-line s-entry))` — the auto-
  loc). Render-child-then-wrap; no lifting yet.
  - Done when: the library loads clean; a temporary probe (deleted
    before the commit) renders a small grammar exercising all three
    wrappers and each emitted source is hand-verified (the discard-
    fn shape, the join, the ctor call with the auto-loc field).

- [ ] **4b.3. Emitter v3: render — `All`**
  Nested `parse-then` (site-(c)); value = vector of its non-`Ignore`
  sub-values, UNLESS `Node`-wrapped.
  - Done when: the library loads clean; a temporary probe (deleted
    before the commit) renders an `All` containing an `Ignore`
    member and the emitted source is hand-verified (nested
    parse-then, the `Ignore` contributing nothing to the value
    vector).

- [ ] **4b.4. Emitter v3: render — `Any`**
  Nested `parse-or` (site-(a)); ≥2 bare-String alts collapse into
  ONE inlined grouped cond (the item-8 grouped-literal form); anon-
  ymous alts lifted to index-path helpers (this is where the helper-
  name-prefix context first becomes real); ALT ORDER IS SEMANTICS —
  the grammar's ordering is preserved verbatim.
  - Done when: the library loads clean; a temporary probe (deleted
    before the commit) renders an `Any` with (a) ≥2 bare-String
    alts, (b) one char-level alt, and (c) one anonymous non-char-
    level alt, and the emitted source is hand-verified (the grouped
    cond, the lifted helper name, the alt order).

- [ ] **4b.5. Emitter v3: render — `Many`**
  Char-level child → fast path (`<name>-char` set-predicate defn +
  `read-run` wrapper, the run as ONE string); otherwise slow path
  (lifted child + site-(b) acc-recursion loop, value = vector).
  - Done when: the library loads clean; a temporary probe (deleted
    before the commit) renders a char-level `Many` and a non-char-
    level `Many`, and the emitted source is hand-verified (the
    fast/slow split, the read-run wrapper, the acc loop).

- [ ] **4b.6. Emitter v3: render — context plumbing**
  Thread the context (enclosing rule name + helper-name prefix)
  through all the render functions written so far (4b.1-4b.5); audit
  skip-at-entry in every parser entry (rule defns, lifted helpers,
  inlined literal / one-char bodies, fast-path run wrappers). A pure
  refactor of the existing renders — no new ctors.
  - Done when: the library loads clean; a temporary probe (deleted
    before the commit) re-renders the 4b.1-4b.5 shapes with the
    context threaded and each emitted source is hand-verified
    (output unchanged except lifted-helper names carrying the
    prefix; skip-at-entry present at every entry).

- [ ] **4b.7. Emitter v3: render — rule entry + integration probe**
  `Rule` → module-level `(defn <name> [state] ...)` source lines;
  `Ref` as a named call into it. No module assembly yet — the
  render functions return source lines for their node.
  - Done when: the library loads clean; a temporary probe (deleted
    before the commit) renders a hand-written 3-rule grammar (one
    char-level, one `Node`-tagged with a self-`Ref`, one mutual-`Ref`
    pair) and each node's emitted source is hand-verified against
    the Ctor table (skip-at-entry, the auto-loc field, the
    grouped-literal Any, the fast/slow Many split, `Ref` as a named
    call).

- [ ] **4c.1. Emitter v3: name graph (cycles + topological order)**
  Pure functions over the rule set (rule names + `Ref` edges read
  from the IR): which rules are in a cycle (self-`Ref` or mutual —
  the crutch set) and a dependency-first topological order (so
  every non-recursive reference is a backward one and needs no
  crutch).
  - Done when: the library loads clean; a temporary probe (deleted
    before the commit) runs both over a hand-built rule set with a
    self-`Ref`, a mutual-`Ref` pair, and an acyclic chain, and the
    cycle set and the order are hand-verified.

- [ ] **4c.2. Emitter v3: module assembly + driver API**
  Module assembly per the Settled section: the module header
  (add-ns state + raw; the `parse-then` / `parse-or` kit; the
  parse-error kit); `(def <rule>)` crutches exactly on the 4c.1
  cycle rules; rule defns in the 4c.1 topological order; lifted
  helpers; `parse-program` + thin `main` per the template; and the
  `emit-pred` / `emit-module` API per the Settled section.
  - Done when: the library loads clean; a temporary probe (deleted
    before the commit) runs `emit-module` over the same 3-rule
    grammar as 4b.7 and the emitted module builds clean under
    `new-toc` (loads with no error lines), with crutches exactly on
    the cycle rules, topological defn order, skip-at-entry, and the
    auto-loc field present.

- [ ] **5a. Driver rewrite + want-file regeneration**
  Rewrite `interpreter/emit-pred.toc` for the v3 contract: the
  same synthetic-grammar families (`ig` / `an` / `mn` / `fn` — `ig`
  now exercising the discard-and-continue `Ignore`), the `rc`
  grammar rewritten `Recur` → `Ref`, plus a new `nd` grammar
  exercising `Node` + auto-loc + `Concat` + a mutual-Ref cycle;
  regenerate all `emit-want` files from the v3 emitter and commit
  the regenerated module files as candidates (unblessed).
  - Done when: the driver loads clean and runs over all grammars;
    the six predicate outputs are byte-identical to the old blessed
    files; the regenerated module want files are committed as
    candidates; the generated `rc` module builds and its
    success/failure sample paths print the expected lines with the
    expected exits; zero leaks, 0 remaining nodes.

- [ ] **5b. Want-file blessing (one-time)**
  **Owner reviews and blesses the candidate `emit-want` files**
  (item 5a) against the old blessed ones — the one-time re-blessing;
  record the review in the as-built note.
  - Done when: `make emit-pred` passes every diff byte-identical
    against the blessed files.

- [ ] **6a. Grow the grammar to scope (a) — expression level**
  Add to `interpreter/intrp-grammar.toc` per the Rule inventory, the
  expression-level rules: `field-symbol`, the two constraint rules
  (`returns-constraint` / `type-constraint`), `body-elt`, and the
  expression forms (`group` / `group-head` with the keyword-head-
  before-catch-all ordering, `let` / `cond` / `fn` / `->` as
  `Node`-tagged rules, `vector` / `hash`), updating `expression` to
  include them. All rule references as `Ref`, every `Node` value
  vector matching its ctor's field list per the Value-shape
  discipline. The driver's real-grammar emission keeps `expression`
  as its entry.
  - Done when: the grammar library loads clean; the driver's
    real-grammar emission runs and the regenerated `module-real`
    is committed as a candidate (unblessed); all other `emit-pred`
    diffs byte-identical against the blessed files.

- [ ] **6a.2. `module-real` blessing (expression rules)**
  Owner reviews the candidate `module-real` (item 6a) and blesses
  it to reflect the new expression rules.
  - Done when: `make emit-pred` passes every diff byte-identical
    against the blessed files.

- [ ] **6b. Grow the grammar to scope (a) — top-level forms**
  Add to `interpreter/intrp-grammar.toc` per the Rule inventory, the
  top-level rules: `deftype-ctor` (with the `AlwaysSucceed`
  empty-field-list alt) / `method-form`, and the seven top-level
  form rules + `top-level-form` (`def-form` last in its prefix
  family). All `Ref`, every form rule `Node`-tagged with its raw-AST
  ctor name, every `Node` value vector matching its ctor's field
  list per the Value-shape discipline. Update the driver's
  real-grammar emission to the full rule set with `top-level-form`
  as the entry; regenerate `module-real` as a candidate (unblessed).
  - Done when: the grammar library loads clean; the regenerated
    `module-real` is committed as a candidate; all other `emit-pred`
    diffs byte-identical against the blessed files; `make gen-rdr`
    builds the generated full-grammar reader module.

- [ ] **6b.2. `module-real` blessing (full rule set)**
  Owner reviews the candidate `module-real` (item 6b) and blesses
  it (this item's diff review).
  - Done when: `make emit-pred` passes every diff byte-identical
    against the blessed files.

- [ ] **7a. Acceptance: `rdr-accept.toc` + `gen-accept` (success path)**
  Write `interpreter/rdr-accept.toc` (add-ns the generated module;
  the file-fact assertions per the Test strategy — counts,
  per-form tallies, spot-checks; parse-error branch and success
  path as separate defns, lets out of non-else cond clauses); add
  the `gen-accept` Makefile target (build the generated module, run
  the acceptance driver).
  - Done when: `make gen-accept` passes — both acceptance files read
    with zero parse errors and every assertion OK, zero leaks / 0
    remaining nodes on the success path.

- [ ] **7b. Error corpus + retire the old**
  Write the `gen-bad*.toc` corpus + expected outputs and wire it
  into `gen-accept` (each case: expected `file:line: msg` + non-zero
  exit); delete the retired `gen-corpus*.toc` / `-want` files and
  the `gen-corpus` target.
  - Done when: `make gen-accept` passes end to end — every corpus
    case matches its expected output + exit.

- [ ] **8. Final task: write the plan for the remaining forms**
  A new plan document covering: scope (b) grammar growth —
  `match`, `do`, `defmacro`, `add-ns`, floats, and `|`
  (sub-expression position only) — including any ctor or emitter
  extensions they force (e.g. does `match` pattern syntax fit the
  combinator set?), and the desugaring phase — raw AST →
  desugared `intrp-ast.toc` shapes via composed recursion schemes
  (the desugaring list in
  `docs/new-compiler-plan.md`, "Parser-owned desugarings", is the
  spec), with its own test surface.
  - Done when: the owner approves the new plan document.

## Inherited verified facts

Durable memory for the loop: `new-toc` coding techniques and hazards,
build/test quirks, generated-code shapes. Append-only, date-stamped,
one bullet per fact. A fact recorded here is settled — trust it rather
than re-checking, unless there is reason to doubt it. (Facts that also
concern the compiler project are mirrored in
`docs/new-compiler-plan.md`, "Verified facts".)

- (2026-09-26, item 1) A clean `new-toc` LIBRARY load (no main) exits 134 and its stderr ends with exactly two trailing lines after `*** Loaded <file>`: `*** 'main' function is missing or malformed` and `*** Could not find implementation of 'Container/map' for type 'Agent' with 2 arguments at core: 1453`. Both are the standard missing-main abort path, NOT errors in the loaded file — control `interpreter/intrp-ast.toc` produces byte-identical trailing output. (Already recorded as baseline noise in `docs/new-compiler-plan.md` Verified facts.)
- (2026-09-26, item 2) Ctor names are GLOBAL within a module namespace, not per-deftype: two FIELDed ctors with the same name in different deftypes is a hard load error (`*** A type named 'X' was already defined. Re-defined at <file>: N`). A BARE singleton ctor name in a later multi-ctor list references the existing fielded ctor of that name (order-dependent) — that is how `intrp-ast.toc` coexists with `Expression/Inline [type-expr c-code loc]` and a bare `TopLevel` `Inline`.
- (2026-09-27, item 2) When stuck — a load error, a plan prescription that seems impossible, a "gotcha" with no obvious workaround — CONSULT `docs/toccata-style.md` (and existing code that already solves the same problem, e.g. `intrp-ast.toc`) BEFORE declaring BLOCKED and parking the item for an owner ruling. The style doc is the authoritative statement of the language; a plan file is a prediction about it, and the doc wins. Concrete instance: the item-2 session (2026-09-26) hit `*** A type named 'Inline' was already defined` and treated the collision as a design contradiction, but the style doc's `deftype` section already documents the fix — a bare name in a multi-ctor list references an existing ctor, so ONE fielded ctor can serve multiple types (the defining deftype must come first). It was falsely parked as BLOCKED because it treated the plan's ctor list as more authoritative than the style doc and skipped the check.
- (2026-09-27, item 3) ast-json's byte spans are corrupted by STRING ESCAPE SEQUENCES: `ast-rdr.toc`'s `ignore` combinator advances the reader's `file-pos` by `(count-chars (.value result))` — the RESOLVED value length — not the raw source byte count, so every escape inside a string literal (`\t`, `\"`, `\\`, ...) undercounts the position by 1 byte and the error accumulates to EOF. Repro: a scratch file containing `(def b "t\ty")` dumps a span 1 byte short of the source text; `tools/toc-edit/tests/fixture.toc` (no escapes) dumps exact spans (verified by byte-comparing every node's `[start,end)` against the file). Consequence: `toc_edit.py` is unusable on any `.toc` file that contains string escapes — including `interpreter/intrp-grammar.toc`, whose grammar data inherently contains `"\""` / `"\\\""` literals both before and after the v3 rewrite (so items 3, 6a, 6b are all blocked on this). Owner-side fix: `ast-rdr.toc` must advance `file-pos` by the raw source length of string literals, then `make ast-json` (depends on the `toccata` target, which the loop must not run).
- (2026-09-27, item 3) `toc_edit.py check` fails (exit 1) on files containing a bare forward declaration `(def name)`: new-toc prints `*** declare <name> glblValN` for it, and that line is not in the tool's known-clean set (fail-closed classification). The file loads clean — the exit 1 is a false positive, not a load error. `interpreter/intrp-grammar.toc`'s `sub-expression` forward declaration is the current instance; it goes away with the item-3 rewrite.
- (2026-09-27, item 3) The owner's ast-json span fix (commit ddaa5b3) is VERIFIED WORKING: `ast-json` byte spans are now exact past string escape sequences (verified by byte-comparing every top-level node's span against `interpreter/intrp-grammar.toc`'s source, which contains `"\t"` / `"\""` / `"\\\""` escapes), and `toc_edit.py check` passes (exit 0) on that file. The item-3 BLOCKED state is lifted; the structural editing tool is usable again for grammar edits (items 6a/6b).
- (2026-09-27, item 3) `toc_edit.py` cannot produce a clean FULL-FILE REWRITE: an edit splices exactly the target node's [start, end), and the whitespace between nodes is unowned (docs/toc-edit-spec.md) and never removed — deleting N top-level nodes leaves their trailing newlines behind as orphan blank lines (item 3's deletion of 50 of 67 top-level nodes would have left ~40 orphan blank lines in the committed file). For a rewrite that deletes most of a file's top-level nodes, the clean path is: compose the candidate in scratch, run a stack-based nesting check, run `new-toc` on the candidate (validate-then-write — install only on a clean load), then verify the installed file. Item 3 was done this way (deviation from the tool mandate, reported to the owner in the as-built note).
- (2026-09-27, item 3) new-toc string rendering: a multi-operand string concatenation whose operand is a Vector renders the vector with `[ ]` and space-joined elements, and the spacing is NOT normalized (observed: `"(Ref " name ")"` nested in such a vector prints `(Ref  name )` — an extra space appears). Consequence: `str-vect` output over grammar data is debug-only — never byte-compare it against hand-written expectations.
- (2026-09-27, item 4a) `fold` with a DEFN handler (one defn dispatching on `type-name`) deterministically miscompiles when the fold reaches a ctor whose `recurse` impl maps over a VECTOR (Any / All / Concat in `intrp-grammar.toc`): the generated program aborts with `*** Compiler screwed up. Incomplete result. at runtime3.c:3396` (the Tag SUB safety check) — 5/5 with the same binary; minimal repro is a fold over `(Any [CharRange NotChar])` with a PLAIN no-cond defn handler. The PROTOCOL shape (`defp` + one `extend-type` impl per ctor type — the v1 EBNF emitter's pattern, `interpreter/intrp-ebnf.toc`) runs the same fold over the same value clean. A defn handler is safe when the fold never reaches a map-recurse ctor (leaves + single-child ctors only). Consequence: the plan's Settled "one defn dispatching on type-name" analyze handler is not buildable; item 4a uses the `analyze-node` protocol instead (deviation reported in the as-built note).
- (2026-09-27, item 4a) `pr*` on the RESULT of a String comparison (`str=` / `=` — the inline `Some`) deterministically aborts with no message (3/3; minimal repro `(main [argv] (pr* (str= "a" "a")))`). The same value is fine in a boolean context (a cond test) or via `to-str` / `type-name`. Consequence: never `pr*` a raw comparison result — print a derived string.
- (2026-09-27, item 4b.1) `pr*` output is REVERSED relative to creation order: `(main [argv] (pr* "a") (pr* "b") (pr* "c"))` prints `bac` (minimal control, clean build, malloc diff 0). Consequence: probe output lines appear in the opposite order to the calls — read probe output content-wise, never positionally (a label `pr*`'d before its content lines prints AFTER them).

## As-built notes

One short note per completed item: what was actually built, any
deviation from the plan's prediction (expected none), and how it was
verified. Also the place to record a toolchain-window observation
(see Toolchain policy) when a 5/5 silent abort with balanced parens
and a healthy control is evidence about `new-toc`. Append-only, newest
last, so the final acceptance (item 7a) and the owner have the record
to check against.

- (2026-09-26, item 1) Created `interpreter/intrp-state.toc`: the ten kit pieces extracted byte-verbatim from `interpreter/intrp-rdr.toc` (verified by substring check of each original block against the new file), header comment rewritten for the library/namespaced-by-importer role. One self-inflicted transcription slip (a dropped `)` in `skip-comment`) was caught by a stack-based nesting check before the first successful load and fixed. Verified: `./new-toc interpreter/intrp-state.toc > /dev/null 2>err` prints `*** Loaded interpreter/intrp-state.toc` with no other error lines (trailing missing-main/Agent lines confirmed standard via the `intrp-ast.toc` control); `interpreter/intrp-rdr.toc` unmodified.
- (2026-09-26, item 2, BLOCKED — owner input needed) Wrote `interpreter/intrp-raw-ast.toc` per the Raw AST section (Location; TopLevel 7 ctors; Expression 12 ctors; the 6 auxiliaries; str-vect on every ctor; no `!` annotations). It FAILS to load: `*** A type named 'Inline' was already defined. Re-defined at interpreter/intrp-raw-ast.toc: 109` — the settled section names BOTH `TopLevel/Inline [c-code loc]` and `Expression/Inline [type-expr c-code loc]` as fielded ctors, and the ctor-name-globality hazard (Inherited verified facts, 2026-09-26 item 2 bullet) forbids that. Probe in scratch (TopLevel's renamed to `TopInline`) loads clean, so the collision is the ONLY blocker — the rest of the file (all 26 other ctors, every str-vect) is verified good. The ambiguity also reaches the grammar: `Node "Inline"` tags in the Rule inventory (items 3/6a/6b) and the want files (item 5) cannot disambiguate two same-named ctors. The file is left in the worktree UNCOMMITTED (unverified); box left unchecked. Owner decision needed: which of the two `Inline` ctors is renamed (and the plan's Raw AST section + Rule-inventory `Node` names updated to match) — then item 2 is a one-line fix away from done.
- (2026-09-26, item 2, BLOCK RESOLVED) Owner ruling: there is only ONE `Inline` — `Expression/Inline [type-expr c-code loc]` — added to `TopLevel` as a bare ctor reference (the style-doc mechanism: a bare name in a multi-ctor list references an existing ctor; order-dependent, so `Expression` is defined before `TopLevel` — the same shape as `intrp-ast.toc`). Applied: `Expression` moved above `TopLevel` with a comment; `TopLevel`'s fielded `Inline [c-code loc]` replaced by bare `Inline`. Verified: `./new-toc interpreter/intrp-raw-ast.toc > /dev/null` prints `*** Loaded interpreter/intrp-raw-ast.toc` with no other error lines (exit 134, trailing missing-main/Agent lines standard per the item-1 fact); `interpreter/intrp-ast.toc` unmodified. Consequence: `Node "Inline"` in the grammar unambiguously targets `Expression/Inline`; a top-level inline form parses to that same ctor as a `TopLevel` value.
- (2026-09-27, item 3, BLOCKED — owner input needed) Item 3 (grammar v3 deftype + token/literal port) is blocked on the ast-json span bug (Inherited verified facts, 2026-09-27 item 3 bullet): `toc_edit.py` cannot operate on `interpreter/intrp-grammar.toc` because the grammar data contains string escapes and the dump's byte spans are corrupted from the first escape to EOF. Per AGENTS.md ("stop and report it, don't work around it"), the file was NOT hand-rewritten around the broken dump. Verified progress this run, committed: (1) the header comment's two em-dashes replaced by hyphens — the file was in violation of the style doc's pure-ASCII rule, which independently corrupts spans (the documented UTF-8 hazard); after the fix `./new-toc interpreter/intrp-grammar.toc` still loads clean (`*** Loaded interpreter/intrp-grammar.toc`; the only other non-boilerplate line is the pre-existing `*** declare sub-expression` from the forward declaration, see the check false-positive fact). (2) A controlled scratch probe (a 4-line file, one string escape per suspect form) confirmed the undercount is exactly 1 byte per escape, cumulative to EOF, while the no-escape fixture dumps exact spans. (3) Root cause located: `ast-rdr.toc`'s `ignore` deftype, `rd/parse` impl — `file-pos` advances by `(count-chars (.value result))` (resolved value length) instead of the raw source length. Owner actions to unblock: fix the string position advancement in `ast-rdr.toc` and rebuild `ast-json` (`make ast-json` depends on the `toccata` target). The item-3 rewrite itself is fully determined by the plan's Settled design (v3 deftype: drop `Recur`, add `Ref` / `Node [name parser]` / `Concat`; `symbol` / `int-literal` / `string` as `Node` over `Concat`; `expression`'s recursive positions as `(Ref "expression")`; delete the `sub-expression` trick, the whitespace rules, the unused keyword/delimiter string defs, and the old `(min N)`-style constraint rules) — the next run can execute it with `toc_edit.py` once the dump is trustworthy.
- (2026-09-27, item 3, UNBLOCKED + DONE) The owner's ast-json span fix (ddaa5b3) lifted the block. Built: the v3 `ParserCombinator` deftype in `interpreter/intrp-grammar.toc` — `Recur` dropped; `Ref [name]` (leaf), `Node [name parser]`, `Concat [parsers]` added, each with `recurse` + `str-vect` following the existing ctor pattern; `Ignore` / `AlwaysSucceed` / `Error` kept unchanged. Token/literal port to the all-`Ref` convention: `symbol` = `Node "Symbol"` over `Concat [(Ref "symbol-start") (Many (Ref "rest-of-symbol"))]`; `int-literal` (rule name renamed "integer" → "int-literal" per the Rule inventory) = `Node "IntegerLit"` over `Concat [(Ref "digits") (Many (Ref "digits"))]`; `string` (def + rule renamed from `double-quoted-string`) = `Node "StringLit"` over `All [(Ignore "\"") (Concat [content]) (Ignore "\"")]`; `symbol-start` / `rest-of-symbol` now reference `alpha` / `digits` via `Ref`; `expression`'s recursive positions are `(Ref "expression")` (the call alt kept as `All ["(" (Ref "expression") (Many (Ref "expression")) ")"]` until item 6a restructures it into `group` / `group-head`). Deleted: the `sub-expression` forward-declaration + defn trick, `linear-whitespace` / `whitespace` / `skip-ws`, all keyword / delimiter / quote / special-symbol string defs, `float-literal` (floats are out of scope (a) — item 8), `vector-expression` / `hash-map-expression` (pre-v3 shapes; item 6a adds the Node-tagged `vector` / `hash` rules), and the five `(min N)`-style constraint rules. DEVIATION (reported): the rewrite was NOT done with `toc_edit.py` — the tool splices exact node spans and can never remove the unowned whitespace between nodes, so deleting 50 of 67 top-level nodes would have left ~40 orphan blank lines in the committed file (Inherited verified facts, 2026-09-27 item 3). Instead: composed the candidate in scratch, stack-based nesting check (caught one real error — a `]` mis-nested where the `Concat` vector closes), `new-toc` on the candidate, installed only on a clean load, verified the installed file. Verified: `./new-toc interpreter/intrp-grammar.toc` prints `*** Loaded` with no error lines (exit 134, standard missing-main tail); `toc_edit.py check` exit 0; grep: zero `Recur`, every rule reference in a rule body is a `(Ref "...")` (the only bare values left are `lower-case` / `upper-case`, which are CharRange values, not rules); a temporary interpreter-side probe (deleted before the commit) ran `str-vect` over `expression` / `digits` and a `fold`-with-`identity` round-trip over `expression` — all new ctors' `recurse` / `str-vect` impls execute, the round-trip render is identical, malloc diff 0, remaining nodes 0, exit 0. Note: `interpreter/emit-pred.toc` (the driver) now fails to load — it references the dropped `grammar/Recur` / `grammar/sub-expression` / `grammar/double-quoted-string`; that is the expected mid-transition state until the item-5 driver rewrite.
- (2026-09-27, item 4b.1, DONE) Built (code committed by the previous run as "in progress"; this run completed the verification): the leaf-ctor renders in `interpreter/intrp-emit.toc` — `render-char-range` / `render-not-char` (skip-at-entry via `state/skip-whitespace`, empty-input guard, `char-code` range / not-equal pred, `ParserMatch` the consumed one-char `subs` and `take-char` the skipped state, `ParserError` otherwise — let-free one-line vectors); `render-string` (`(str-prefix? L (.input SV))` + N nested `take-char`s, N=1 and N=3 verified, escapes round-trip: grammar literal `"` emits source `\"`); `render-ref` (`(<name> <sv>)`); `render-always` (`ParserMatch` of the String literal or `empty-vector` — parameter-split dispatch on the value's type name); `render-error` (`ParserError` with the rendered msg). Plus the shared helpers: `escape-char` / `escape-str` / `render-literal` (generated source must re-parse to the same text), `skip-expr`, `take-chars`. Verified: the library loads clean (exit 134, `*** Loaded interpreter/intrp-emit.toc`, standard missing-main tail); a temporary interpreter-side probe (deleted before the commit) ran `analyze` + each render over nine shapes (CharRange, NotChar, 1-char String, 3-char String, escaped String, Ref, AlwaysSucceed-String, AlwaysSucceed-empty-vector, Error) — every emitted line hand-verified against the Ctor table (skip-at-entry, let-free, the N-take-char nesting, the escape round-trip, the `empty-vector` constant); probe compiled and ran clean: malloc diff 0, remaining nodes 0, exit 0. No deviation from the plan (the render functions take the state variable name as a parameter and return `[String]` lines — the plumbing the 4b.2-4b.6 items build on). Note: probe output lines print in REVERSE creation order (new fact recorded) — the verification read the output content-wise.
- (2026-09-27, item 4b.2, DONE) Built: the wrapper-ctor renders in `interpreter/intrp-emit.toc` — `render-ignore` (`(parse-then <child> (fn [_ s2] (state/ParserMatch empty-vector s2)))` — the discard fn binds `_`, threads `s2`, standalone value the unit `empty-vector`); `render-concat` (nested `parse-then` over the children, the value joined into ONE string at the innermost `ParserMatch`); `render-node` (`(parse-then <child> (fn [v s2] (state/ParserMatch (raw/<name> <args> (state/state-line (state/skip-whitespace sv))) s2)))` — the child's value spread as ctor args, the auto-loc the entry state's line). Plus the value-shape/arity helpers: `char-level`, `ir-value-shape` (string/vector classifier — aborts on Ref/Node/Ignore/Error: Ref needs the rule set at 4c.2, the rest are not value sources in scope (a)), `ir-value-arity` (static arity for vector shapes — aborts on Many-slow: dynamic arity not spreadable), `vec-arg-exprs`, `node-value-args`, `concat-fragment` (string value as-is, vector value flatten-joined via `(reduce v "" (fn [acc s] (str acc s)))`), `concat-fragments`, `join-strings`, `build-join`, `gen-vnames`. The render dispatch `render-child` and the wrappers are mutually recursive (the wrappers render their children via the dispatch); new-toc is single-pass, so the dispatch is crutch-declared (`(def render-child)`) before the wrappers and its `defn` after them. Verified: the library loads clean (exit 134, `*** Loaded interpreter/intrp-emit.toc`, standard missing-main tail); a temporary interpreter-side probe (deleted before the commit) rendered `Ignore` over a CharRange, `Concat` of two CharRanges, and `Node "Symbol"` over that Concat — every emitted source hand-verified (the discard-fn shape, the `(str v0 v1)` join in child order, the `(raw/Symbol v (state/state-line (state/skip-whitespace state)))` ctor call with the auto-loc field); probe compiled and ran clean: malloc diff 0, remaining nodes 0, exit 0. Three bugs found and fixed during verification: (1) the helper was first named `vector-arg-exprs` — a Toccata symbol may NOT start with `vector` (the parser splits it: `vectorx` → `vector` + `x`, `vector-arg-exprs` → "Undefined symbol: '-arg-exprs'"); renamed to `vec-arg-exprs`. (2) `.data ir` is a raw Vector, not a Maybe — the wrappers wrongly did `(extract (.data ir))` ("No implementation of 'extract' found for type Vector"); the 4b.1 leaves correctly do `(extract (get (.data ir) i))`; fixed all 8 wrapper/helper sites to use `.data ir` directly. (3) the Concat join came out REVERSED (`(str v1 v0)`) — `conj` appends to the END (verified: `conj (conj (vector "a") "b") "c"` = [a b c]), so the forward 0→n recursion in `concat-fragments` built the fragment vector in reverse; changed it to recurse n→0 (like `vec-arg-exprs`). New facts: a Toccata symbol cannot start with `vector`; `conj` appends to the end; the crutch `(def name)` works in a module loaded via `add-ns`; a probe that `add-ns`es the emitter must live in the SAME directory (same module path string) or the grammar loads twice (two distinct ctor types) and the emitter's `defimpl`s don't match the probe's types — the probe was therefore placed in `interpreter/` (deleted before the commit), a deviation from the scratch/-only probe convention forced by the double-load.
- (2026-09-27, item 4a, DONE) Built: the clean-slate rewrite of `interpreter/intrp-emit.toc` for the analyze phase — `NodeIR [kind char-level? data]` (one deftype: `kind` the ctor name, `char-level?` a Maybe, `data` the raw fields for leaves / the child IRs for containers); the `analyze-node` protocol (defp + one extend-type impl per ctor type — 13 impls); the ir-* builder defns (one per ctor); `analyze [pc]` = `(fold pc analyze-node)`. Classification per the Settled design: CharRange / NotChar / one-char String are `Some None`; Any iff ALL alts are char-level (a reduce over the child flags); Rule / Many delegate to their child; Ref / Node / Concat / All / Ignore / AlwaysSucceed / Error never are. Ref and String are leaves, so the fold terminates structurally. DEVIATION (reported): the Settled section prescribes `h` / `h-dispatch` as one defn dispatching on `type-name` via a flat cond, but that shape deterministically miscompiles (Inherited verified facts, 2026-09-27 item 4a first bullet): with a defn handler, any fold that reaches a map-recurse ctor (Any / All / Concat) aborts the generated program with `Compiler screwed up. Incomplete result` (Tag SUB, runtime3.c:3396) — 5/5 with the same binary, minimal repro a fold over `(Any [CharRange NotChar])` with a plain no-cond defn. The protocol shape — the v1 EBNF emitter's proven pattern — runs the same fold clean, so `analyze-node` is a defp; the entry point keeps the plan's name and body (`analyze [pc]` = `(fold pc analyze-node)`). Verified: the library loads clean (exit 134, `*** Loaded interpreter/intrp-emit.toc`, standard missing-main tail); two temporary interpreter-side probes (deleted before the commit) ran `analyze` over (a) five hand-written rule values covering all 13 ctors (including a self-Ref and a mutual-Ref cycle) and (b) fifteen standalone ctor shapes — every node's kind, char-level? flag, and data (raw fields / child IRs) hand-verified correct; malloc diff 0, remaining nodes 0, exit 0. Note: `make emit-pred` fails to load (the driver references the dropped v1/v2 grammar ctors) — the expected mid-transition state until the item-5 driver rewrite.
