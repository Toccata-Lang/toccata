# Parser Generator Plan (full grammar → generated Toccata reader)

Status: clean slate (2026-09-26). Supersedes
`docs/parser-generator-plan-bad.md` in its entirety — the v1 and v2
emitters are garbage (owner ruling); their source lives in git
history. Carried forward from the old plan: the `emit-want` byte-
exact test targets (re-blessed once, see item 5) and the
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
in an acceptance driver (item 7) — not want-files (want-files test the
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
- `interpreter/emit-want/*` — the expected files (14 after item 5
  adds the `nd` grammar), RE-BLESSED once from the v3 emitter (item
  5, owner review); byte-exact net thereafter.
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
  ctors loc]`, `ExtendType [type-name methods loc]`, `Inline [c-code
  loc]`, `Main [param-list body loc]`
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

**Phase 1 — analyze.** `analyze [pc]` = `(fold pc h)` — the core
`fold`, no new walker. `h` is one function dispatching on
`type-name`; to respect the proven let-wrapping-cond miscompile
shape, the dispatch is the parameter split: `(defn h [v]
(h-dispatch (type-name v) v))` with a flat-cond `h-dispatch [k v]`.
After `recurse` reassembles a node, container ctors' fields hold
children's IR values; leaves hold raw fields. `Ref` and `String`
are leaves, so the fold terminates structurally even though the
generated reader is recursive. The IR carries classification
(`char-level?` — CharRange / NotChar / one-char String; Any iff all
alts; Rule / Many iff child) plus the structure the render needs.

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
  item 5) are regenerated from the v3 emitter over the same
  synthetic grammars (the `rc` grammar rewritten to `Ref`; a new `nd`
  grammar exercising `Node` + auto-loc + a mutual-Ref cycle added
  to the driver) and re-blessed ONCE after owner review (item 5). The
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

- [ ] **2. Raw AST: `interpreter/intrp-raw-ast.toc`**
  The deftypes per the Raw AST section: `Location`, `TopLevel`
  (7 ctors), `Expression` (12 ctors), the auxiliaries
  (`TypeConstraint`, `LetBinding`, `Clause`, `HashPair`,
  `DefTypeCtor`, `Method`); `str-vect` impl on every ctor; no `!`
  annotations on multi-field ctors.
  - Done when: the library loads clean (same check as item 1);
    `interpreter/intrp-ast.toc` unmodified.

- [ ] **3. Grammar v3 deftype + token/literal port**
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

- [ ] **4a. Emitter v3: analyze (the fold + IR)**
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

- [ ] **4b. Emitter v3: render (per-ctor source emission)**
  Extend `interpreter/intrp-emit.toc` with the render phase: a plain
  `defn` walking the IR with context (enclosing rule name +
  helper-name prefix) that emits source per ctor exactly per the
  Ctor table — skip-at-entry in every parser entry; grouped-literal
  Any; fast/slow Many; `Ref` → named call; `Concat` → flatten-join;
  `Node` → `raw/<name>` ctor call + auto-loc; `Ignore` →
  parse-and-discard; `AlwaysSucceed` / `Error`. No module assembly
  yet — the render functions return source lines for their node.
  - Done when: the library loads clean; a temporary probe (deleted
    before the commit) renders a hand-written 3-rule grammar (one
    char-level, one `Node`-tagged with a self-`Ref`, one mutual-`Ref`
    pair) and each node's emitted source is hand-verified against
    the Ctor table (skip-at-entry, the auto-loc field, the
    grouped-literal Any, the fast/slow Many split, `Ref` as a named
    call).

- [ ] **4c. Emitter v3: module assembly + driver API**
  Extend `interpreter/intrp-emit.toc` with module assembly and the
  driver-facing API: the module header (add-ns state + raw; the
  `parse-then` / `parse-or` kit; the parse-error kit); `(def
  <rule>)` crutches — only for rules in a cycle (computed over the
  `Ref` name graph); rule defns in topological order (dependencies
  first); lifted helpers; `parse-program` + thin `main` per the
  template; and the `emit-pred` / `emit-module` API per the Settled
  section.
  - Done when: the library loads clean; a temporary probe (deleted
    before the commit) runs `emit-module` over the same 3-rule
    grammar and the emitted module builds clean under `new-toc`
    (loads with no error lines), with crutches exactly on the cycle
    rules, topological defn order, skip-at-entry, and the auto-loc
    field present.

- [ ] **5. Driver rewrite + want-file re-blessing**
  Rewrite `interpreter/emit-pred.toc` for the v3 contract: the
  same synthetic-grammar families (`ig` / `an` / `mn` / `fn` — `ig`
  now exercising the discard-and-continue `Ignore`), the `rc`
  grammar rewritten `Recur` → `Ref`, plus a new `nd` grammar
  exercising `Node` + auto-loc + `Concat` + a mutual-Ref cycle;
  regenerate all `emit-want` files from the v3 emitter; **owner
  reviews and blesses the new expected files** (the one-time re-
  blessing — record the review in the as-built note).
  - Done when: `make emit-pred` passes every diff byte-identical
    against the blessed files (the six predicate diffs expected
    unchanged from the old files); the generated `rc` module builds
    and its success/failure sample paths print the expected lines
    with the expected exits; zero leaks, 0 remaining nodes.

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
  - Done when: the grammar library loads clean; `make emit-pred`
    passes (all diffs byte-identical, `module-real` against a file
    re-blessed by the owner to reflect the new expression rules).

- [ ] **6b. Grow the grammar to scope (a) — top-level forms**
  Add to `interpreter/intrp-grammar.toc` per the Rule inventory, the
  top-level rules: `deftype-ctor` (with the `AlwaysSucceed`
  empty-field-list alt) / `method-form`, and the seven top-level
  form rules + `top-level-form` (`def-form` last in its prefix
  family). All `Ref`, every form rule `Node`-tagged with its raw-AST
  ctor name, every `Node` value vector matching its ctor's field
  list per the Value-shape discipline. Update the driver's
  real-grammar emission to the full rule set with `top-level-form`
  as the entry; regenerate `module-real` (blessed by the owner as
  part of this item's diff review).
  - Done when: the grammar library loads clean; `make emit-pred`
    passes (all diffs byte-identical, `module-real` against the
    re-blessed file); `make gen-rdr` builds the generated full-
    grammar reader module.

- [ ] **7. Acceptance: `rdr-accept.toc` + `gen-accept` + error corpus**
  Write `interpreter/rdr-accept.toc` (add-ns the generated module;
  the file-fact assertions per the Test strategy — counts,
  per-form tallies, spot-checks; parse-error branch and success
  path as separate defns, lets out of non-else cond clauses);
  write the `gen-bad*.toc` corpus + expected outputs; add the
  `gen-accept` Makefile target (build the generated module, run
  the acceptance driver, run the corpus with expected non-zero
  exits); delete the retired `gen-corpus*.toc` / `-want` files and
  the `gen-corpus` target.
  - Done when: `make gen-accept` passes end to end — both
    acceptance files read with zero parse errors and every
    assertion OK, every corpus case matches its expected output +
    exit, zero leaks / 0 remaining nodes on the success path.

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

## As-built notes

One short note per completed item: what was actually built, any
deviation from the plan's prediction (expected none), and how it was
verified. Also the place to record a toolchain-window observation
(see Toolchain policy) when a 5/5 silent abort with balanced parens
and a healthy control is evidence about `new-toc`. Append-only, newest
last, so the final acceptance (item 7) and the owner have the record
to check against.

- (2026-09-26, item 1) Created `interpreter/intrp-state.toc`: the ten kit pieces extracted byte-verbatim from `interpreter/intrp-rdr.toc` (verified by substring check of each original block against the new file), header comment rewritten for the library/namespaced-by-importer role. One self-inflicted transcription slip (a dropped `)` in `skip-comment`) was caught by a stack-based nesting check before the first successful load and fixed. Verified: `./new-toc interpreter/intrp-state.toc > /dev/null 2>err` prints `*** Loaded interpreter/intrp-state.toc` with no other error lines (trailing missing-main/Agent lines confirmed standard via the `intrp-ast.toc` control); `interpreter/intrp-rdr.toc` unmodified.
- (2026-09-26, item 2, BLOCKED — owner input needed) Wrote `interpreter/intrp-raw-ast.toc` per the Raw AST section (Location; TopLevel 7 ctors; Expression 12 ctors; the 6 auxiliaries; str-vect on every ctor; no `!` annotations). It FAILS to load: `*** A type named 'Inline' was already defined. Re-defined at interpreter/intrp-raw-ast.toc: 109` — the settled section names BOTH `TopLevel/Inline [c-code loc]` and `Expression/Inline [type-expr c-code loc]` as fielded ctors, and the ctor-name-globality hazard (Inherited verified facts, 2026-09-26 item 2 bullet) forbids that. Probe in scratch (TopLevel's renamed to `TopInline`) loads clean, so the collision is the ONLY blocker — the rest of the file (all 26 other ctors, every str-vect) is verified good. The ambiguity also reaches the grammar: `Node "Inline"` tags in the Rule inventory (items 3/6a/6b) and the want files (item 5) cannot disambiguate two same-named ctors. The file is left in the worktree UNCOMMITTED (unverified); box left unchecked. Owner decision needed: which of the two `Inline` ctors is renamed (and the plan's Raw AST section + Rule-inventory `Node` names updated to match) — then item 2 is a one-line fix away from done.
