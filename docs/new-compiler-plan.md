# New Compiler Plan (from scratch)

Status: in planning (grilling session, 2026-08-26). Settled items are final
until re-opened; open items are queued for the next session.

## Goal

A new Toccata compiler, written in Toccata and built by `new-toc`, that emits
C for the new runtime (`new.c` / `runtime3.c`). The current compiler source
(`compiler.toc`, `base.toc`, `typer.toc`, `codegen.toc`) is ignored — it is
entangled with agents/promises, git-dependency, and a type system the new
runtime doesn't have. `codegen.toc` is kept as a *reference* for phase 3.

`hvm-core.toc` / `runtime3.*` may be extended as needed; decide when we get
there.

## Phase plan

1. **Concrete interpreter** — runs pure-Toccata programs (scope below).
   AST from `intrp-ast.toc`; parser is new direct recursive-descent
   functions (`intrp-rdr.toc` is the grammar reference only).
2. **Abstract interpreter** — the same evaluator over abstract values
   (types/properties) = the type checker. Operations are designed as
   dispatch points (protocols) from day 1 so this is an extension, not a
   rewrite.
3. **Code generation** — AST → C for the new runtime, borrowing heavily from
   `codegen.toc`. Validated against the interpreter (differential testing on
   the pure subset) and the type checker.

## Phase 1 scope (settled)

Single-file programs:

- `defn` (named, recursive), `fn` (incl. named local fns), `let`, calls
- literals: int, float, string, symbol; vectors; hash maps
- control flow: `and` `or` `either` `cond` `int-cond`; `->` threading
- protocols and deftypes **from the core only** (Integer, String, Vector,
  Maybe, HashMap, ...)

Deferred: user `deftype` / `extend-type` / `defp`, user inline C, `match`,
superposition, `add-ns` / multi-module, hash sets (`hash-set` not in the new
core), `defmacro`.

Interpreter design (settled):

- Concrete values **are** runtime values (I60 terms, `Value*`).
- The core is compiled into the interpreter binary; every core symbol
  resolves to a native call. The interpreter only interprets user
  `defn` / `fn` / `let` / control-flow.
- User inline C → rejected with a clear error (a compiler-phase concern).
- "Ignore inline C" therefore means: core inline C = native primitives;
  user inline C = not interpreted.

Acceptance:

- A new positive test suite (`intrp-tests/`) of pure-Toccata programs with
  golden outputs (run, `sort`, diff — same convention as the regression
  tests).
- **Step: audit the 51 regression tests against the interpreter scope** and
  use the suitable ones as **differential tests** — run each through the
  interpreter and compare its output against the existing `.rslt` golden
  (compiled-binary output), ignoring stats lines (ITERS/TIME/MIPS/node
  counts). Rough audit (2026-08-26): ~20 clean candidates (test8–10, 13,
  15–17, test-global-empty-list, cond-expr-1..4, tail-cond-1,
  test-inlined-result-constraint, test-recursive-map-fn,
  test-tail-recur-1/2/3, test-trailing-comment, test-or-comment,
  test-fusing); 6 more blocked only by `add-ns` (integer-regressions,
  string-regressions, vector-regressions, test-threading, test-closures,
  test-inline-namespaced-sym). The rest use inline C, user deftypes, or
  match/superposition. **Item-3 audit (2026-08-27): 19 verified clean;
  test10 flagged — `do` is not in the settled AST/desugarings; test-fusing
  flagged — bare `(def)` declare + arity-flexible (left-associative)
  calls; both pending owner decision. Full audit + 25-symbol initial-env
  list: `intrp-tests/README.md`.**
- `interpreter-tests/` (20 negative type-error tests) belongs to phase 2.

## AST (settled — 8 points)

Base: `intrp-ast.toc` (parses/typechecks clean under new-toc; `!`
annotations are ignored by the compiler).

| Node | Shape | Notes |
|---|---|---|
| `Location` | `[file line]` | |
| `Expression.Symbol` | `[ns name loc]` | |
| `Expression.IntegerLit` | `[value loc]` | |
| `Expression.FloatLit` | `[value loc]` | |
| `Expression.StringLit` | `[value loc]` | |
| `Expression.Call` | `[operator operands]` | |
| `Expression.Fn` | `[name parameter-list body loc]` | name = self-recursion binding; `loc` added |
| `Expression.FieldGetter` | `[field-name loc]` | used as `Call` operator; name sans dot |
| `Expression.Inline` | `[type-expr c-code loc]` | type-expr = `Maybe` of `Expression` (uninterpreted tree); `!` annotations commented out — annotations over `String`/`Location`-kind types break the bare reference from `TopLevel` (see verified facts) |
| `Expression.TypeConstraint` | `[symbol type-expr]` | symbol = String \| `returns` singleton |
| `Expression.Superposition` / `Match` / `BlockComment` | kept | unused in phase 1 |
| `TopLevel.Main` | `[parameter-list body loc]` | separate constructor (special form) |
| `TopLevel.Definition` | `[name value loc]` | `def` = fundamental top-level binding |
| `TopLevel.Inline` | bare name → `Expression/Inline` | ctor names are globally unique, so `TopLevel` reuses the `Expression` ctor via a bare reference (verified working); `TopLevel`'s `BlockComment` is likewise a bare reference to `Expression/BlockComment` |
| `TopLevel.DefType` / `Defp` / `ExtendType` / `AddNs` / `BlockComment` | kept | unused in phase 1 |
| `Constructor` / `Module` | kept | auxiliaries for later phases |

Dropped: `BodyExpressions` (bodies are `[Expression]` vectors).

Parser-owned desugarings:

- `defn name [p] body` → `Definition [name (Fn name [p] body) loc]`
  (the `Fn` name field is the call-time self-binding for recursion)
- `let [b1 e1 ... bn en] body` → nested immediate anonymous-fn applications,
  one `Fn` per binding; sequential binding semantics (later initializers see
  earlier bindings); no 9-param arity limit
- `[a b c]` / `(vector a b c)` → left-fold `vect-conj` over `empty-vector`
- `{k v ...}` → left-fold `assoc` over `emptyBMI`
- `->` → nested `Call`s; steps: bare symbol, call (threaded value prepended),
  `.field` (→ `FieldGetter` operator), block comments dropped
- `! sym T` / `!returns T` → `TypeConstraint` entries at the front of fn
  bodies. Positions: fn/defn body front; between let bindings → front of
  that binding's fn body; let body front → innermost fn body.
- `cond` → **right-nested ternary core `cond` calls** (core: `(defp cond
  [test clause else])`): `(cond t1 v1 ... tN vN e)` →
  `(cond t1 v1 (cond t2 v2 (... (cond tN vN e))))`. Requires ≥1 (test
  value) pair + a trailing else (odd arity); even arity = parse error.
  (Matches the old reader's reduce over reversed clauses.)
- `and` / `or` → **right-nested binary core calls** (core: `(defp and
  [x y])`, `(defp or [x y])`): `(and a1 ... aN)` →
  `(and a1 (and a2 ... aN))`; single arg folds to the arg itself. (The
  new AST has no And/Or constructors — the old reader's And/Or-expr
  shape is dropped.)
- `str` → `str*` over a left-folded `vect-conj` vector; `println` →
  `println*` over the same (old reader's read-str/read-println pattern —
  the only two variadic print/str forms).
- `int-cond` → **plain call, no special handling** — it is a C-level REF
  value (`(def int-cond (inline "newRef(intCond);"))`) that acts like a
  function; it parses as an ordinary symbol call and resolves through the
  environment like any other core symbol.
- `either` → plain binary call (core: `(defp either [test-clause else])`),
  no special handling.
- `returns` singleton: `(deftype returns [])` — verified working under
  new-toc (declaration generates a global value; the type name is
  referenceable as a value).

Constraints: single arity, 0–9 params; `FieldGetter` calls take exactly one
operand; `(-> x)` with zero steps is a parse error.

Out-of-scope forms (`deftype`, `defp`, `extend-type`, `add-ns`, `match`,
`|`, `defmacro`) are parse errors in phase 1 — no parse-and-reject rules;
each form's rule arrives with its phase.

## Parser (settled so far)

- **Direct recursive-descent functions**, not a parser-combinator data
  structure. The combinator approach required building the whole grammar
  structure at every compiler startup — too slow. Compiled parse functions
  are ready at startup with no structure-building.
- `intrp-rdr.toc` is the grammar reference (token rules, keywords,
  symbol/number/string syntax). `ParserState [input values]` (values =
  bookkeeping map: file, line, ...) and `ParserResults` survive as the
  threaded state / return convention (shapes may be refined).
- AST construction + all desugarings happen inside the parse functions.
- Threaded state: **remaining-string** — `ParserState [input values]` with
  `input` = unconsumed suffix. `subs` returns O(1) `SubString` views, so
  per-character cost is a small pooled allocation; no core changes needed.
  Maximal-run reads (symbols, digits, string contents) accumulate into a
  `StringBuffer` or extract via one `subs` after a length scan — never
  repeated `str` concatenation (O(n²)).
- Result convention: **`ParserResults` as-is** — `ParserMatch [value state]`,
  `ParserIgnore [state]` (consumed, no value), `ParserFail []` (try next
  alternative), `ParserError [msg state]` (definitive, with message).
  Alternatives: try each sub-parser on the same state; first
  `Match`/`Ignore` wins, else aggregate the best `Error`.
- Error format: parse functions produce messages only; the driver formats
  `file:line: message` from the failing state's `values` bookkeeping
  (matching the current compiler's `Error at file: N; 'msg'` style).
- Core extension (decided): a new `strCmp` code path/flag returning true
  when one string is a prefix of the other — for keyword/operator matching.
- Mutual recursion via forward declarations: `(def f)` then
  `(defn f [...] ...)` — verified working under new-toc (2026-08-26).
  **Crutch only**: the new compiler will not have forward definitions,
  so this is available while we build with new-toc, but the parser's
  mutual recursion is a self-hosting issue for later. Candidate fix:
  pass element/operand parsers as function arguments so only
  self-recursion is needed (named defns can call themselves without
  forward declarations).
- EBNF / grammar tooling: a special interpreter that reads the parser
  source file and extracts structure (later, optional — not a runtime
  feature).
- Dropped: `ParserCombinator` deftype as engine, `defp parse` evaluator
  protocol, `Recur` + rule registry, `Apply` combinator, `location`
  combinator, str-vect-based EBNF.

## Verified facts (2026-08-26)

- `intrp-ast.toc` parses/typechecks clean under `new-toc` (aborts only on
  missing `main`, as a library file would). `intrp-rdr.toc` compiles clean
  with zero leak.
- **Compile check for library files**: `./new-toc <file> > /dev/null` —
  the exit code is always 134 (abort), so it is useless; pass = `***
  Loaded <file>` in stderr with no other error lines. `'main' function is
  missing or malformed` + `Could not find implementation of
  'Container/map' for type 'Agent' ... at core: 1453` are baseline noise
  in the abort path for *any* file (verified with trivial inputs).
- `intrp-rdr.toc` has grammar rules but **no evaluator** and **no AST
  construction**; its `main` just prints the parser via `str-vect`.
- `intrp-ebnf.toc` is broken (Conflicting assertions at line 17, multi-arg
  `str`).
- `instance?` is unsupported in new-toc (all uses in passing tests are
  commented out) — dispatch is via protocols.
- `file-io.toc` doesn't load under new-toc (its own git-dependency line);
  `lazy-list` is not in the new core → the driver needs a whole-file read
  (single inline-C `file-read → String` in the interpreter source; the
  interpreter itself is compiled by new-toc and may use inline C freely).
- Desugaring targets exist in the new core: `empty-vector` (hvm-core.toc:438),
  `vect-conj` (440), `emptyBMI` (203), `assoc` (206, protocol). `hash-set`
  does not.
- The old reader (`reader.git@081d8ef`) is the reference for desugaring
  shapes: `read-vector`, `read-hash-map`, `read-threading` /
  `nest-thread-exprs`, `read-defn` / `read-def`, `read-single-arity`
  (params + doc + body), `read-fn` (optional name).
- `STR_PREFIX` is **8** in runtime3.h (item 1's draft said 4, but
  `STR_GT` already occupies 4).
- The current runtime is **lazy**: a `let` initializer (or any
  sub-redex) runs only when the result chain demands it — a
  continuation that ignores its param drops the remaining side
  effects. Scratch programs that print must thread side-effect
  results through a strict combination (e.g. `+`) into the result
  chain (see `scratch/str-prefix.toc`). Also: `fn` literals with
  underscore params miscompile (duplicate C variable) — use named
  params.
- **new-toc deftype constructor names are globally unique** (per
  namespace; the core's names are global too). Two verified
  consequences (2026-08-26, item 2):
  - A constructor named `String` is **unbuildable in any namespace**
    ("A type named 'String' was already defined" — the core `String`
    type). The symbol-ref constructor is `Symbol` per the AST table
    (the pre-rename file compiled clean with it). The 46d6e29 "String
    constructor" adjustment broke the clean-compile property and was
    reverted in item 2.
  - `Expression.Inline` and a same-named `TopLevel.Inline` **cannot
    coexist** as separate ctors in one namespace. **Resolved
    (2026-08-26, owner)**: a bare name in a deftype's ctor list is a
    **reference to an existing ctor** (type-info lookup; works for core
    ctors too — `Some`, `Leaf`, `GetSentinelVal` all verified).
    `TopLevel` therefore reuses `Expression/Inline` (and
    `Expression/BlockComment`) via bare references. Restriction found by
    bisection: a bare reference to a *user-file* ctor fails when that
    ctor carries `!` annotations over scalar `String` / `StringLiteral` /
    `Location` / user types; `Integer`, vector types (`[T]`), or no
    annotations work — so `Expression.Inline`'s annotations are
    commented out. The failure is a misleading `Undefined symbol: 'x' at
    core: 98` / `'start' at core: 665` (location points at a core ctor
    param — `Some`'s `x`, `SubString`'s `start` — not the cause).
- `(deftype returns [])` (zero constructors) compiles clean under
  new-toc (verified 2026-08-26, item 2).
- `!` annotations are parsed/validated by new-toc even though they
  don't affect codegen: `(maybe-of Expression)` is rejected ("Missing
  type assertion in type constraint expression"); bare `Maybe` is
  accepted (cf. new-core.toc's `! mv Maybe`).

## Settled (continued)

- **Phase 1 is strictly single-file.** `add-ns` of local modules is the
  **first step of phase 2**; the 6 blocked differential tests
  (integer-regressions, string-regressions, vector-regressions,
  test-threading, test-closures, test-inline-namespaced-sym) come in with
  it. Phase 1 uses exactly one namespace; the namespace *structure*
  exists from the start (see the Environment bullet below) and is
  populated in phase 2.
- **`eval-call` has two target kinds**: a user closure (AST body —
  interpreted) or a primitive (a `REF`: core defn / defp dispatcher /
  native). **How primitives are represented in the environment and called
  is DEFERRED** (owner will decide when implementing; likely an `apply`-
  like core function — cf. status.md `test-apply-constructor`: the `apply`
  protocol exists but has no fn-value implementation). Verified: a call
  whose operator is a local value compiles to a generic apply
  (`pushRedex(args, <local-fn-value>)`), so source-level calls of function
  values work; the open problem is dynamic arity + safe
  closure-vs-REF discrimination.
- **Closure deftype** (`intrp-eval.toc`):
  `(deftype Closure [name params body env])` — `name` String (`""` if
  anonymous; call-time self-binding), `params` `[String]`, `body`
  `[Expression]` (leading `TypeConstraint`s skipped by eval), `env` =
  captured environment (persistent sharing).
- **Environment** — namespace structure included from the start (phase 2's
  `add-ns` populates it; adding the shape later would be a pain):
  `(deftype Env [current-ns namespaces])` — `current-ns` String (phase 1:
  always `""`), `namespaces` String → (String → value). Phase 1: one ns
  (`""`) with core bindings + all top-level defs. Closure captures the
  whole Env. Call-time env = captured env with the current ns's map
  extended by param bindings + self-binding. Anticipated resolution
  (confirmed in phase 2): unqualified → current ns, fallback to global
  `""` on miss (core stays reachable everywhere, as in compiled code);
  qualified `ns/name` → `namespaces[ns]` exactly. Parser produces the
  `ns` field on `Symbol` from day one.
- **`eval` dispatch**: `(defp eval [expr env])` — protocol over the
  Expression constructors (receiver = the Expression, first arg), one
  `extend-type` impl per constructor. Phase 2's abstract interpreter
  mirrors this with its own protocol over the same constructors.
  Constructor set eval must handle (the actual `Expression` constructors
  the phase-1 parser produces): `Symbol [ns name loc]` (the symbol ref),
  `IntegerLit`, `FloatLit`, `StringLit`, `Call`, `Fn`, `FieldGetter`,
  `TypeConstraint` (skip).
  **Never produced**: `Superposition`, `Match`, `BlockComment`
  (out-of-scope forms — parse errors). There are no And/Or/Cond/IntCond/
  VectorLit/HashLit/Threading constructors in the AST — those forms are
  desugared to `Call`s (see desugarings). **Quoted symbols `'sym` desugar
  to `StringLit` at parse time** (verified: new-toc compiles `'yeppers`
  in cond-expr-1 to a plain String) — eval never sees a symbol literal.
- **Parse output: a hash map of name → AST value** (not a list).
  Top-level `defn`/`def` → entry under the name; `main` → entry under
  `"main"`; top-level `inline` → **parse error** (no name to enter the
  map — enforces the phase-1 user-inline-C rejection at parse time);
  comments skipped.
- **Top-levels are LAZY, no memoization (phase 1)**: a top-level name
  binds to its AST (wrapped in a marker deftype so lookup can tell
  "top-level def — evaluate it" from "already a value — return it"
  without dispatching on raw values); the AST is evaluated on first use
  in the environment current at that moment. Consequences: the map needs
  no source order; forward references work (everything is bound before
  `main` runs); cycles → infinite loop at first use; each use re-evaluates
  (pure def values unaffected — all 20 differential candidates are pure;
  side-effecting def values repeat per use — documented sharp edge).
  The marker's exact shape is settled together with the deferred
  primitive representation.
- **The interpreter is the semantics reference.** new-toc's behavior is
  not a design constraint — it is only the build crutch. (Differential
  tests still validate core behavior on the 20 clean candidates.)
- **Whole-file read: inline-C function named `slurp`** (in `intrp.toc`).
- **Error reporting (phase 1)**: `file:line: message` to stderr + abort
  (non-zero exit), uniform across parse/structural/runtime errors; no
  Maybe-threading through eval; no backtraces. (Phase 2's typechecker
  gets its own collected-error mechanism.)
- **Makefile**: repurpose the `intrp` target — `intrp.c: intrp.toc
  intrp-ast.toc intrp-rdr.toc intrp-eval.toc` via `./new-toc intrp.toc >
  intrp.c`; compile `new.c runtime3.c graph.c intrp.c` with the
  regression-test flags (`-g -march=native -I. -DCHECK_MEM_LEAK=1
  -DSAFETY=1 -DSTATS=1 -lm -lpthread`).
- **Parser organization**: one function per syntactic form in
  `intrp-rdr.toc`, named after the form (`parse-top-level` keyword-
  dispatches on the first token; `parse-expr` dispatches on the first
  char; `parse-call`, `parse-vector`, `parse-hash`, `parse-fn`,
  `parse-let`, `parse-cond`, `parse-threading`, `parse-number`,
  `parse-string`, `parse-symbol`, `parse-quoted-sym`, token helpers
  `skip-whitespace`/`peek-char`/`take-char`/`read-run`). Desugaring
  happens inside these functions as they build the AST. No state beyond
  the threaded `ParserState`.
- **Initial-env core symbol list**: hand-written in the interpreter
  source (no reflection in the subset); start from the union of what the
  20 differential candidates + `intrp-tests` reference (the audit
  produces the list); grow when a test hits an unbound core symbol.

## Phase-1 design status

Complete except two deferred pieces that settle together at the
**checklist item 7 stop point** (joint decision with the owner — the
loop stops there rather than guessing): the **primitive representation**
(how core REFs are represented in the environment and called — likely an
`apply`-like core function) and the **top-level-def marker shape** (the
wrapper that lets symbol lookup tell "top-level def — evaluate" from
"value — return" without dispatching on raw values).

## Phase 1 implementation checklist (Ralph loop)

Protocol: work top to bottom, one item per session. Check an item off
only when its "done when" holds, then commit. Context for every item:
this file (the settled design above) + AGENTS.md. The interpreter's own
source may use inline C freely; *interpreted programs* may not. Never
touch the `toccata` Makefile target. **Item 7 is a STOP POINT** — the
primitive representation is a joint decision (owner + agent); when the
loop reaches it, stop and report the state. Do not choose a design
unilaterally.

- [x] **1. Core: `strCmp` prefix mode**
  - `runtime3.h`: add `#define STR_PREFIX 4` alongside `STR_EQ`/`STR_LT`
    (~line 284). `runtime3.c` `strCmp` (~line 1521): add the prefix
    code path — success iff one string is a prefix of the other
    (compare the shorter length). Preserve the ref-consumption contract
    (every path consumes both argument refs; non-string input returns
    `nothing()`).
  - Done when: a scratch program with an inline-C wrapper calling
    `strCmp(s, t, STR_PREFIX)` compiles under new-toc and answers
    correctly for equal / prefix / superstring / different cases; zero
    leaks.

- [x] **2. `intrp-ast.toc`: settled AST delta**
  - `TopLevel.Main [parameter-list body loc]` (separate constructor);
    `TopLevel.Definition [name value loc]`; `Fn` gains `loc` and its
    `body` becomes `[Expression]` (drop `BodyExpressions`);
    `Expression.Inline [type-expr c-code loc]` (+ a top-level Inline);
    add the `(deftype returns [])` singleton.
  - Done when: it compiles clean under new-toc (the same check that
    passed before the edits).
  - As-built (2026-08-26): top-level `Inline` is a bare reference to
    `Expression/Inline` (not a separate ctor — names are globally
    unique); `Expression.Inline`'s `!` annotations are commented out
    (annotations over `String`/`Location`-kind types break the bare
    reference — see verified facts).

- [x] **3. Test audit**
  - Line-by-line audit of the 51 regression tests against the phase-1
    scope; record the suitable list (expect the ~20 clean candidates
    from the rough audit above) and the union of core symbols they
    reference (feeds item 7's initial env). Flag any candidate that
    uses `main`'s argument vector (interpreter runs pass a different
    argv than compiled runs).
  - Done when: the suitable list + core-symbol list are written to
    `intrp-tests/README.md`, with a one-line exclusion reason for every
    other test.

- [ ] **4. `intrp-rdr.toc`: parser — state + tokens**
  - Replace the combinator machinery with direct recursive-descent
    functions. `ParserState [input values]` (remaining-string model;
    `values` = `{"file" ..., "line" 1}`, line bumped on `"\n"`). Result
    convention: `ParserMatch`/`ParserIgnore`/`ParserFail`/`ParserError`
    as-is. Token helpers: `skip-whitespace`, `peek-char`, `take-char`,
    `read-run` (`StringBuffer` accumulation — never repeated `str`
    concat).
  - Done when: the file compiles; a scratch main tokenizes a sample
    string (whitespace, symbols, ints, floats, strings) and the
    results print correctly.

- [ ] **5. `intrp-rdr.toc`: parser — expressions + desugarings**
  - Literals (int/float/string; quoted sym → `StringLit`), calls,
    special forms (`fn`, `let`, `cond`, `and`, `or`, `str`, `println`,
    `->`, `!` markers), vector/hash folds — all the settled
    desugarings.
  - Done when: a scratch driver parses sample expressions and asserts
    the expected shapes via field access + `println*` (let → nested
    immediate Fn apps; cond → right-nested `cond` calls; `[...]` →
    `vect-conj` fold; `{...}` → `assoc` fold; `->` → nested Calls;
    `'sym` → `StringLit`).

- [ ] **6. `intrp-rdr.toc`: parser — top-level + map output**
  - Top-level rule: `def`/`defn` → map entry under the name; `main` →
    `"main"` entry; top-level `inline` → parse error; comments
    skipped. Output: name → AST map. Out-of-scope forms (`deftype`,
    `defp`, `extend-type`, `add-ns`, `match`, `|`, `defmacro`) → parse
    errors.
  - Done when: every suitable test from item 3 parses to a map with
    the expected entries; a program with top-level `inline` fails with
    a clear `file:line` error.

- [ ] **7. `intrp-eval.toc`: interpreter — data + environment — STOP
    POINT**
  - `(deftype Closure [name params body env])`,
    `(deftype Env [current-ns namespaces])`, initial env built from
    item 3's core-symbol list. The **primitive representation** (how
    core REFs are represented in the env and called) and the
    top-level-def marker shape are decided HERE, jointly, when the
    loop stops at this item — do not guess a design.
  - Done when: the representation + marker are settled with the owner,
    the file compiles, and a scratch harness builds the initial env,
    extends it, and looks up values correctly.

- [ ] **8. `intrp-eval.toc`: interpreter — eval**
  - `(defp eval [expr env])` over `String`, `IntegerLit`, `FloatLit`,
    `StringLit`, `Call`, `Fn`, `FieldGetter`, `TypeConstraint` (skip).
    `eval-call`: closure → interpret (param bindings + self-binding);
    primitive → native call (the item-7 mechanism). Errors:
    `file:line: message` + abort.
  - Done when: a scratch program with defn recursion, fn/closures,
    let, cond/and/or/either, vectors, hash maps, threading, string/int
    ops interprets with hand-verified output.

- [ ] **9. `intrp.toc`: driver + Makefile**
  - `main`: argv element 1 = file (missing → usage + abort); `slurp`
    (inline-C whole-file read); parse; exactly-one-`main` structural
    check; evaluate top-levels (lazy bindings); call `main` with the
    argv vector. Makefile: repurpose the `intrp` target — `intrp.c`
    from `intrp.toc` (same awk `#line` step as the regression rule),
    compile `$(TEST_SOURCES) intrp.c` to `./intrp`.
  - Done when: `make intrp` builds; `./intrp regression-tests/test8.toc
    party-pooper | sort` matches `test8.rslt` after stripping the
    stats lines from both sides (patterns: `^- ITRS: `, `^malloc
    count: `, `^remaining nodes: `, `^result: 0x`, `^- Threads: `).

- [ ] **10. Differential tests**
  - Runner (script or Makefile target): for each suitable test from
    item 3, `./intrp regression-tests/$t.toc party-pooper | sort`,
    strip the stats-line patterns from both sides, diff against
    `regression-tests/$t.rslt`.
  - Done when: every suitable test passes.

- [ ] **11. `intrp-tests/` suite**
  - New positive pure-Toccata tests + `.rslt` goldens (same
    run/`sort` convention), covering: def chains, defn recursion,
    fn/closures, let, cond/and/or/either, vectors, hash maps,
    threading, string/int/float ops, quoted symbols.
  - Done when: the suite passes through the item-10 runner.

- [ ] **12. Final verification**
  - Zero leaks (malloc/free diff 0, remaining nodes 0) for the
    interpreter binary across the full test set; every item above
    checked.
  - Done when: all items checked.

Phases 2–3: to be specified when we get there.
