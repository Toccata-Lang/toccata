# New Compiler Plan (from scratch)

Status: in planning (grilling session, 2026-08-26). Settled items are final
until re-opened; open items are queued for the next session.

## Goal

A new Toccata compiler, written in Toccata and built by `new-toc`, that emits
C for the new runtime (`new.c` / `runtime3.c`). The current compiler source
(`compiler.toc`, `base.toc`, `typer.toc`, `codegen.toc`) is ignored — it is
entangled with agents/promises, git-dependency, and a type system the new
runtime doesn't have. `codegen.toc` is kept as a *reference* for phase 4.

`hvm-core.toc` / `runtime3.*` may be extended as needed; decide when we get
there.

## Phase plan

1. **Reader** — a parser that fully reads `hvm-core.toc` (the whole core:
   `defn`, `def`, `defp`, `deftype`, `extend-type`, top-level `inline`, plus
   the expression forms). AST from `interpreter/intrp-ast.toc`; parser =
   direct recursive-descent functions in `interpreter/intrp-rdr.toc` (the
   file's original combinator grammar was the reference; it has been
   replaced). Ends with `parse-program` reading `hvm-core.toc` to a complete
   vector of TopLevel values (source order) with zero parse errors.
2. **Concrete interpreter** — runs pure-Toccata programs (scope below) over
   the phase-1 reader's AST.
3. **Abstract interpreter** — the same evaluator over abstract values
   (types/properties) = the type checker. Operations are designed as
   dispatch points (protocols) from day 1 so this is an extension, not a
   rewrite.
4. **Code generation** — AST → C for the new runtime, borrowing heavily from
   `codegen.toc`. Validated against the interpreter (differential testing on
   the pure subset) and the type checker.

## Phase 1 scope (settled) — reader

Goal: `parse-program` reads `hvm-core.toc` **in full** — every top-level
form, zero parse errors — into a complete vector of TopLevel values
(source order). This is the phase's acceptance bar (checklist item 6g).

Top-level forms the reader must produce (hvm-core.toc usage in parens):

- `defn` (108), `def` (5) — already done (items 4–6)
- `defp` (58) — bare `(defp name [params])` and with `!`/`!returns`
  annotations + an optional body → `TopLevel.Defp`
- `deftype` (5) — multi-ctor and single-ctor forms, with ctor protocol
  impls; the ctor list may also name an existing compiler-defined type
  (the bare `StringBuffer` in `deftype String ...`) → `TopLevel.DefType`
  (+ `Constructor` nodes)
- `extend-type` (16) — `(extend-type T (proto [params] body) ...)` →
  `TopLevel.ExtendType`
- top-level `inline` (2) — global C declaration blocks, nameless →
  `TopLevel.Inline`

Expression forms: everything the current reader already handles (literals,
calls, `fn`, `let`, `cond`, `and`, `or`, `str`, `println`, `->`, `!`
annotations, vectors, hash maps, field getters, quoted syms; expression-level
`inline` currently parses as a plain `Call` — see item 6f for the
Call-vs-`Inline`-node decision).

Still parse errors (none appear in hvm-core.toc): `match`, `defmacro`,
`add-ns`, top-level `|` superposition.

Acceptance: checklist item 6g — `parse-program` on `hvm-core.toc` returns a
`ParserMatch` (not `ParserError`) with all 194 top-level forms accounted for
(108 `defn`, 5 `def`, 58 `defp`, 5 `deftype`, 16 `extend-type`, 2 inline).

## Phase 2 scope (settled) — concrete interpreter

Single-file programs:

- `defn` (named, recursive), `fn` (incl. named local fns), `let`, calls
- literals: int, float, string, symbol; vectors; hash maps
- control flow: `and` `or` `either` `cond` `int-cond`; `->` threading
- protocols and deftypes **from the core only** (Integer, String, Vector,
  Maybe, HashMap, ...)

Deferred (interpretation): user `deftype` / `extend-type` / `defp`, user
inline C, `match`, superposition, `add-ns` / multi-module, hash sets
(`hash-set` not in the new core), `defmacro`. (The reader *parses* the
core's `defp` / `deftype` / `extend-type` in phase 1; the interpreter does
not *interpret* user ones.)

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
- `interpreter-tests/` (20 negative type-error tests) belongs to phase 3
  (the abstract interpreter / type checker).

## AST (settled — 8 points)

Base: `interpreter/intrp-ast.toc` (parses/typechecks clean under
new-toc; `!`
annotations are ignored by the compiler).

| Node | Shape | Notes |
|---|---|---|
| `Location` | `[file line]` | |
| `Expression.Symbol` | `[ns name loc]` | |
| `Expression.IntegerLit` | `[value loc]` | |
| `Expression.FloatLit` | `[value loc]` | |
| `Expression.StringLit` | `[value loc]` | |
| `Expression.Call` | `[operator operands loc]` | `loc` added 2026-09-02 (owner) — call site for the `not callable` / arity errors; the parser records the line at the opening paren |
| `Expression.Fn` | `[name parameter-list body loc]` | name = self-recursion binding; `loc` added |
| `Expression.FieldGetter` | `[field-name loc]` | used as `Call` operator; name sans dot |
| `Expression.Inline` | `[type-expr c-code loc]` | type-expr = `Maybe` of `Expression` (uninterpreted tree); `!` annotations commented out — annotations over `String`/`Location`-kind types break the bare reference from `TopLevel` (see verified facts) |
| `Expression.TypeConstraint` | `[symbol type-expr]` | symbol = String \| `returns` singleton |
| `Expression.Superposition` / `Match` / `BlockComment` | kept | unused in phase 1 |
| `TopLevel.Main` | `[parameter-list body loc]` | separate constructor (special form) |
| `TopLevel.Definition` | `[name value loc]` | `def` = fundamental top-level binding |
| `TopLevel.Inline` | bare name → `Expression/Inline` | ctor names are globally unique, so `TopLevel` reuses the `Expression` ctor via a bare reference (verified working); `TopLevel`'s `BlockComment` is likewise a bare reference to `Expression/BlockComment` |
| `TopLevel.DefType` / `Defp` / `ExtendType` / `AddNs` / `BlockComment` | kept | `DefType` / `Defp` / `ExtendType` are produced by the phase-1 reader (items 6c–6e); `AddNs` / `BlockComment` unused in phase 1 |
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

Reader-scope forms: `defp`, `deftype`, `extend-type`, and top-level
`inline` are parsed in phase 1 (items 6c–6f) so the reader can read
`hvm-core.toc`. Still parse errors (none appear in hvm-core.toc): `add-ns`,
`match`, `|` (top-level superposition), `defmacro` — no parse-and-reject
rules; each form's rule arrives with its phase.

## Parser (settled so far)

- **Direct recursive-descent functions**, not a parser-combinator data
  structure. The combinator approach required building the whole grammar
  structure at every compiler startup — too slow. Compiled parse functions
  are ready at startup with no structure-building.
- The original `intrp-rdr.toc` was the grammar reference (token rules,
  keywords, symbol/number/string syntax). `ParserState [input values]`
  (values = bookkeeping map: file, line, ...) and `ParserResults`
  survive as the threaded state / return convention (shapes may be
  refined); the new parser lives in `interpreter/intrp-rdr.toc`.
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
  **SUPERSEDED for alternatives (2026-08-29, owner) — see the next bullet.**
- **Parser alternatives are superpositions, not `or` tries (2026-08-29,
  owner)**: instead of sequential or-style alternatives, parse alternatives
  with a superposed expression `(| a b ...)` in which a failing branch
  produces `( | )` (the empty superposition), terminating that branch. The
  "first `Match`/`Ignore` wins" rule above is superseded. For now, ignore
  the possibility that a string is ambiguous: assume at most one AST is
  possible from any string — the surviving branch is the parse result.
  **PAUSED (2026-08-29, owner)**: dropped for the item-6b rewrite — the
  rewrite uses sequential `or`-style alternatives (the superseded rule
  above is in force again for now). The superposition approach is under
  further owner consideration; the NUL-eraser probes (`scratch/sup-nul*.toc`)
  show total failure erases the workflow with no error value to carry
  `msg`/`state`.
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
  feature). `interpreter/intrp-ebnf.toc` is kept as its reference (owner,
  2026-08-29); it references the removed combinator machinery and does not
  compile against the current parser.
- **Note (2026-08-29): the `unfold`-based reader was attempted and DROPPED
  (owner, 2026-09-01)** — generating an AST value from a string via the core's
  `unfold` recursion scheme is not expressible over the settled AST (see the
  Verified-facts entry); the parser stays direct recursive descent.
- **Note (2026-08-29): empty-input checks are pervasive** — nearly every
  parse/skip/collect function tests whether the remaining input is empty
  (`(str= input "")`) before dispatching. That is a performance hit; find a
  way to eliminate as many of these checks as possible.
- Dropped: `ParserCombinator` deftype as engine, `defp parse` evaluator
  protocol, `Recur` + rule registry, `Apply` combinator, `location`
  combinator, str-vect-based EBNF.

## Verified facts (2026-08-26)

- **When `new-toc` does not compile successfully, you MUST check its
  stderr to see whether an error was reported** (e.g. `*** Undefined
  symbol: ... at <file>: <line>`) rather than assuming the failure is
  spurious. If an error is reported, the abort is correct — the bug is
  in the source being compiled, not in the compiler. The `*** Loading
  ...`, `*** Loaded ...`, and `*** declare ...` lines are normal
  progress output, not errors.
- `interpreter/intrp-ast.toc` parses/typechecks clean under `new-toc`
  (aborts only on missing `main`, as a library file would).
  `interpreter/intrp-rdr.toc` compiles clean with zero leak.
- **Compile check for library files**: `./new-toc <file> > /dev/null` —
  the exit code is always 134 (abort), so it is useless; pass = `***
  Loaded <file>` in stderr with no other error lines. `'main' function is
  missing or malformed` + `Could not find implementation of
  'Container/map' for type 'Agent' ... at core: 1453` are baseline noise
  in the abort path for *any* file (verified with trivial inputs).
- The original `intrp-rdr.toc` had grammar rules but **no evaluator**
  and **no AST construction**; its `main` just printed the parser via
  `str-vect` (the file has since been rewritten as the
  recursive-descent parser). `interpreter/intrp-ebnf.toc` is broken
  (Conflicting assertions at line 17, multi-arg `str`).
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
- A **bare `(def name)`** — a `def` with **no value expression** — is a
  **forward reference**, a crutch used only to make new-toc happy (so a
  later value definition of the same name resolves). The new parser
  should **ignore** it: it produces no AST node / map entry (verified
  2026-08-27).
- **`+` is exactly 2-arg** in the new core (`(defn + [x y])`,
  hvm-core.toc:303); `(+ a b c)` is a parse error ("Wrong number of
  args for '+'"). Nest to combine more: `(+ a (+ b c))` (verified
  2026-08-27, item 4).
- **`first`/`rest` on a Vector**: `(first v)` returns `Some element`
  (a Maybe), not the bare element — extract with `(extract (first v))`;
  `(rest v)` returns a Vector (hvm-core.toc:945, 951).
  `interpreter/intrp-rdr.toc`'s `elt0`/`elt1` use `(extract (first ...))`
  (verified 2026-08-27,
  item 4).
- **`add-ns` module paths resolve relative to the importing file's
  directory**, not the CWD; the regression tests work only because
  their modules sit beside them (verified 2026-08-27, item 4).
  Combined with the raw-path-string module-cache key (the `../` bullet
  below): keep a module's references in one directory, spelled
  identically.
- **`char-code`** (hvm-core.toc:645) → first char's integer code
  (0-255); `char` (634) is the inverse.
  `interpreter/intrp-rdr.toc`'s predicates classify via `char-code` +
  `<=` (verified 2026-08-27, item 4).
- **`subs` is 3-arg** (`(defp subs [s start len])`, hvm-core.toc:661);
  the rest of a string is `(subs s 1 (count s))` (len clamps to the
  tail). `count` is O(1) for both `SubString` and `StringBuffer`
  (verified 2026-08-27, item 4).
- **Named / namespace-qualified functions are first-class values**:
  `rdr/read-symbol` can be passed as an argument and called as
  `(reader s)` (verified 2026-08-27, item 4).
- **Lazy-machine side-effect threading (extended)**: a continuation that
  drops its threaded count lets the machine skip the *earlier* `pr*`
  side effects (only the last print appeared). Thread an accumulator
  through every continuation so the final result depends on all of them
  (verified 2026-08-27, item 4; see `scratch/rdr-tokens.toc`
  `read-and-show`).
- **`interpreter/intrp-rdr.toc` is a library (no main)** holding the
  item-4 state + token helpers and the item-5 expression parsers: `make-state`, `state-line`, predicates
  `is-digit`/`is-alpha`/`symbol-start?`/`symbol-continue?`/
  `float-char?`, helpers `peek-char`/`take-char`/`skip-comment`/
  `skip-whitespace`/`run-length`/`read-run`, accessors `elt0`/`elt1`,
  readers `read-string-content`/`read-string`/`read-symbol`/`read-int`/
  `read-float`. Self-recursion needs no forward declaration; the old
  `ParserCombinator` machinery and grammar `(def ...)` rules are
  removed. `read-run` = length-scan (`run-length`) + one `subs`; token
  readers return `[token-text new-state]` pairs (verified 2026-08-27,
  item 4).
- **No local symbol may shadow a core-namespace symbol** (constraint,
  2026-08-27): a local `let` binding named `first` (in
  `read-full-symbol`) makes new-toc emit colliding C identifiers — the
  mangled local name (`first_9`) is also used for an unrelated core
  entity (a parameter port of the `Integer/str-vect` lambda), so the
  generated C fails clang with `use of undeclared identifier 'first_9'`.
  The Toccata-level load check (`*** Loaded`) does not catch it; only a
  clang build of the generated C does. Rename the local while building
  with new-toc (build crutch).
- **A deftype has two forms** (owner, 2026-08-27):
  1. **Single-ctor form**: `(deftype CtorName [field1 field2 ...] <protocol
     impls>)` — one constructor, named the same as the type.
  2. **Multi-ctor form**: `(deftype TypeName (CtorName1 [field1 field2 ...]
     <impls>) (CtorName2 [field1 ...] <impls>) ...)` — in this form the
     `TypeName` **and each `CtorName`** are all types that can be used in
     type expressions.
- **Zero-arg constructors are singleton values, not calls** (rule,
  2026-08-27): when a constructor is specified with no arguments — e.g.
  `(ParserFail [])` in the deftype — it is **not called like a normal
  constructor**. Writing `(ParserFail [])` attempts to pass the empty
  vector as an argument; under new-toc a cross-namespace ctor call of
  this shape generates crashing C (`Invalid APP VAL pair` — minimal
  repro: `scratch/i5-lib.toc` + `scratch/i5-probe.toc`). Instead, as a
  special case, a **singleton value** bearing the constructor's name is
  created, accessed directly without a function call: `ParserFail` (or
  `lib/ParserFail` cross-namespace) *is* the value. Consequence for the
  parser: the `ParserFail` result is the bare symbol, never
  `(ParserFail [])`; the `returns` singleton works the same way.
- **new-toc transient segfaults** (2026-08-27): new-toc sometimes
  segfaults/aborts with NO error message; the crash is transient and a
  retry (up to 5 times total) compiles the same input fine. If a run
  prints an error message before aborting, it is a real error, not a
  transient crash — stop retrying and fix it. Always capture and read
  new-toc's stderr on a failed build (`2>/dev/null` is forbidden): the
  message (`Undefined symbol: 'x' at file: N`, `Error at file: N;
  msg`) usually points directly at the problem.
- **Flat `cond` pairs; multi-binding `let`** (2026-08-27): `cond`
  takes any number of flat (test value) pairs — `(cond t1 v1 t2 v2 ...)`.
  What new-toc limits is *nesting*: 8+ levels of nested `cond` are
  rejected (7 is the limit — cf. the `dispatch-special` /
  `dispatch-special-2` split in `interpreter/intrp-rdr.toc`). `let`
  takes multiple
  bindings in one form — `(let [b1 e1 b2 e2 ...] body)` with sequential
  binding semantics (later initializers see earlier bindings). Prefer a
  single multi-binding `let` over nested `let`s (verified under new-toc
  in `scratch/rdr-exprs.toc`).
- **Never use `../` in `add-ns` module paths** (2026-08-28): new-toc's
  module cache keys on the **raw path string** in `(module "...")`, not
  the resolved file. The same `.toc` file referenced under two different
  relative paths (e.g. `../intrp-ast.toc` from `scratch/` and
  `intrp-ast.toc` from the root) is compiled **twice**, and each compile
  assigns fresh type numbers — the same deftype gets two runtime
  identities. Cross-module protocol dispatch then fails at runtime with
  `No implementation of 'X' found for type Y (N)` even though the value
  prints the correct type name (the impl registered under one compile's
  type id; the value carries the other's). Rule: keep every reference
  to a module spelled identically across the whole module graph — no
  `../`, no redundant `./scratch/`-style prefixes. Verified via
  `interpreter/xns-probe.toc` (passes with same-directory bare paths;
  the earlier `scratch/` copy with `../` paths failed).
- **Hash-map literals work in expression position under new-toc**
  (2026-08-28): `{"k" v, "k2" v2}` compiles and runs — `,` is
  whitespace. Cleaner than nested `assoc` over `emptyBMI` (cf.
  `make-state` in `interpreter/intrp-rdr.toc`). The phase-1
  restriction on hash literals is an *interpreted-program* scope
  decision, not a new-toc limitation.
- **Toccata top-levels are PREFIX forms** (2026-08-28, item 6): a
  top-level form is `(def ...)`, `(defn ...)`, `(main ...)` — it starts
  with `(` and the keyword is the first token *inside*. `parse-top-level`
  takes the `(`, reads the keyword, and dispatches (`parse-top-level-form`
  → `parse-top-level-args`). The value of a `def` is a general expression
  followed by a closing `)` that `parse-top-def-value-expr` consumes;
  `defn`/`main` bodies end in the `)` consumed by `parse-body`.
- **new-toc "malformed 'cond' expression" quirk** (2026-08-28, item 6):
  a `cond` whose **non-else clause** is a `let` that contains a nested
  `cond` is rejected ("malformed 'cond' expression"). The same shape as
  the cond's **else** is fine (cf. `parse-threading`). Fix: extract the
  `let`+`cond` into a helper function so the clause is a plain call
  (done for `parse-top-def` → `parse-top-def-value` / `-expr`, and
  `parse-top-level` → `parse-top-level-form` / `-args`).
- **`strSha1` includes the value's TYPE** (2026-08-28, item 6):
  `runtime3.c` `strSha1` does `Sha1Update(&arg0->type, 8)` before the
  bytes, so a `SubString` and a `String` with the same content hash
  **differently**. Consequence: a hash map's keys must be a single type,
  or `get` misses. The parse keys the name → TopLevel map by **full
  Strings**: `sub-to-str` (inline C in `interpreter/intrp-rdr.toc`) copies
  each parsed def/defn name (a `subs` SubString view) to a fresh
  `StringBuffer`; the `main` key is the String literal `"main"`. This
  keeps the map mergeable with the core-symbol env (item 7), whose keys
  are String literals. (Verified probe: a SubString-keyed entry is found
  by a SubString lookup but NOT a same-content String lookup.)
- **`sub-to-str` is CONSUMING; `elt0`/`first` increments the ref count**
  (2026-08-28, item 6): `sub-to-str` `dec_and_free`s its input. This is
  safe because `elt0` (`(extract (first v))`) creates a new reference, so
  the extracted SubString and its `[run state]` pair each hold a ref —
  consuming one leaves the pair's ref valid (no double-free; zero leaks).
  A **non-consuming** copy leaked (the original SubString, still held by
  the pair, was never freed — malloc diff 33). Also: new-runtime
  `dec_and_free(Term, int)` takes a `Term`, not a `Value*` (cast
  `(Term)p`).
- **Map ops for the parse output** (2026-08-28, item 6): `count` IS
  implemented for the map node types (use `(count m)` for the entry
  count); `keys` is **NOT** ("No implementation of 'keys' found for type
  HashMap (11)"). `get`/`assoc` work. To test a `get` result's
  Some/None-ness, a local `(defp maybe-kind [m])` + `extend-type None`
  / `Some` works (instance? is unsupported).
- **Item-6 driver: `interpreter/rdr-top.toc`** (2026-08-28): parses the
  19 suitable tests (item 3) through `parse-program` and checks each map
  has exactly the expected entries (entry count via `count` + every
  expected name present via `get`); checks a top-level `inline` is a
  parse error with `file:line: msg`; checks the 7 out-of-scope forms
  (`deftype`/`defp`/`extend-type`/`add-ns`/`match`/`defmacro`/`|`) are
  parse errors. Reads files via an inline-C `slurp`. All 27 checks pass,
  zero leaks. Run from the repo root: build with `./new-toc
  interpreter/rdr-top.toc` + the awk `#line` step + `clang ... new.c
  runtime3.c graph.c rdr-top.c`, then `./rdr-top`.
- **`( | )` is UNREADABLE under new-toc** (2026-08-29, item 6b probe):
  the embedded reader's `read-super` rule (vendored
  `reader.git@081d8ef` `reader.toc:940`) runs `(map elems .constraint)`
  over the alts vector, so an empty superposition aborts the reader:
  `Insufficient values for 'arg0' ... Needed 1, got 0`. The reader
  source is compiled INTO the new-toc binary (generated C in
  `new-toc.c` carries `#line` refs to the vendored path), so fixing it
  means editing the vendored dependency + rebuilding new-toc (the
  `new-toc` target depends on the `toccata` binary — owner territory).
  Codegen already has an empty-alts fallback: the
  `superposition-ast` CodeGen in `codegen.toc` emits `var = NUL;` when
  `(first alts)` is None (the reader never reaches that path).
  `(| 1)` / `(| 1 2)` etc. read and compile fine.
- **Superpositions with ≥1 element work at runtime** (2026-08-29,
  item 6b probe, `scratch/sup-probe.toc`): `(| 1)` compiles to the bare
  value (no SUP node); `(| a b)` compiles to a SUP linked list
  (`var = a; var = makePair(SUP, lab, b, var);`). They flow through
  `cond`/`fn`/`let`/`str` correctly; zero leaks, zero remaining nodes.
- **A NUL branch is ERASED when the superposition is applied**
  (2026-08-29, item 6b probe, `scratch/sup-nul.toc`):
  `(| (empty-sup) 5)` evaluates to `5`, where `empty-sup` is inline C
  `result = NUL;` (NUL = 0x03, the eraser tag). So an inline-C NUL
  value is a workable stand-in for the unreadable `( | )` failing
  branch — the eraser kills its duplicated workflow when the
  superposed value is used.
- **Total failure erases the WHOLE downstream workflow** (2026-08-29,
  item 6b probe, `scratch/sup-nul2.toc`): applying an all-NUL
  superposition (every branch failed) silently erases the redex chain —
  side effects after the application point are lost, the final result
  is a garbage partial value, `bad result SUP pair` prints to stderr,
  exit code 0. There is no ParserError-like value to carry
  `msg`/`state`. Consequence: the settled error behavior (parse
  functions return `ParserError [msg state]`, the driver formats
  `file:line: message` and aborts; `rdr-top.toc`'s inline-error and
  out-of-scope-form checks read `.msg`/`.state`) is NOT expressible
  with pure superposition alternatives. Owner must decide how error
  messages flow (e.g. failing branches print to stderr as a side
  effect before dying, and how the driver detects total failure).
- **`unfold`/`recurse` work over a user deftype with a vector of child
  sub-nodes** (2026-08-29, item 6b probe, `scratch/unfold-probe.toc`):
  `(deftype Node [label children] (recurse [v f] (Node (.label v)
  (map (.children v) f))) ...)` + `(unfold 2 f)` builds a tree with
  zero leaks / zero remaining nodes. The `recurse` impl must pass ONLY
  the recursive fields to `f` — passing a leaf field (e.g. an Integer
  label) re-unfolds it forever and exhausts the term buffer
  (`Error: Not enough space to allocate pair`).
- **new-toc "malformed 'cond' expression" crutch #2: a field getter inside
  a vector arg to a deftype ctor, inside a let, inside a cond clause**
  (2026-08-29, item 6b): `(cond t (let [x ...] (ast/Call (ast/Fn ""
  [(name x)] ...)) ...))` — i.e. a `.field` getter as an element of a
  vector literal passed to a ctor, in a let that is a cond clause — is
  rejected with `malformed 'cond' expression`. Bisected: the getter is the
  trigger (a fn call like `(elt0 x)` in the same position compiles; a
  getter in a plain `str` call compiles; the getter only fails inside a
  vector arg to a ctor in a cond clause). Workaround: bind the getter to a
  `let` var and use the plain symbol in the vector arg (`(let [nm (name x)]
  ... [nm] ...)`). Applied in `let-desugar-rec` and `hash-fold-acc` in
  `interpreter/intrp-rdr.toc`. (Distinct from the item-6 "let containing a
  nested cond" malformed-cond quirk.)
- **Item 6b as-built (2026-08-29): the concrete style rewrite is DONE and
  verified; the `unfold` sub-part is DROPPED (owner, 2026-09-01).**
  `interpreter/intrp-rdr.toc` now has 11 grouping deftypes with named
  fields + `.field` access (no positional vectors, no `elt0`/`elt1`/`elt2`):
  `Token [text state]`, `FullSymbol [ns name state]`, `TypeExpr [text
  state]`, `ParamList [params state]`, `TypeConstraintPair [tc state]`,
  `LetBinding [name expr]`, `HashPair [key value]`, `LetBindings [bindings
  state]`, `HashPairs [pairs state]`, `Body [constraints exprs state]`,
  `TopLevelEntry [name value]`. Each implements `map`/`flat-map`; the
  structurally-recursive ones (`LetBindings`/`HashPairs`/`Body`) also
  implement `recurse` (over the vector-of-children field only). No `!`
  annotations on the grouping ctors (avoids the multi-field-ctor
  "Conflicting assertions" hazard). `vect-concat` is defined BEFORE the
  grouping deftypes (their `flat-map` impls call it). `rdr-top.toc`'s
  `check-oos` `[source msg]` pairs became an `OosCase [source msg]` deftype
  (its `flat-map` calls `rdr/vect-concat`). `rdr-exprs.toc` is unchanged
  (uses only the stable public API). Both drivers pass with IDENTICAL output
  to the pre-rewrite baseline, zero leaks, 0 remaining nodes. The ONE
  forward declaration (`(def parse-expr)`) and all error messages are
  unchanged.
- **Field getters require a prior type with that field name** (2026-08-29,
  item 6b): new-toc resolves `.field` getters by looking up an already-
  defined type that has a field with that name. If no such type exists yet,
  the getter is an `Undefined symbol`. Workaround: define a dummy type with
  the needed field names before the first use (e.g. `(deftype Dummy [key
  value])` in `intrp-ast.toc` so that `.key` / `.value` resolve in the
  desugar helpers that run before `HashPair` is defined). Build crutch only;
  the new compiler should resolve getters structurally.
- **The `unfold`-based reader is NOT expressible over the settled AST —
  OPEN design gap (2026-08-29, item 6b).** `unfold x f` = `recurse (f x)
  (fn [v] (unfold v f))` threads the node's CHILD VALUES through `f`. A
  parser must thread STATES (string positions): each recursive parse step
  needs the state at the sub-expression's start, not the already-parsed
  sub-expression. The settled AST's recursive fields are sub-`Expression`s
  (e.g. `Call [operator operands]`), not sub-states, so `recurse` over an
  AST node hands `f` sub-Expressions that a parse function cannot consume.
  Making `unfold` drive the parse would require a new state-carrying
  parse-node type (a redesign of the settled AST / a second materialize
  pass), which the loop must not guess. **Owner decision (2026-09-01):
  dropped** — the parser stays direct recursive descent (as was done for
  superposition-alternatives).
- **SUB/SUP end-of-program error ⇒ wrong arity (2026-08-30)**: when a
  program ends with a SUB or SUP error, that probably means a function is
  being called with the wrong number of arguments somewhere. Especially
  when no superpositions are used.
- **Field access on a union-typed deftype value: tag + per-ctor protocols
  (2026-09-01, item 6c)**: a `.field` getter on a value statically typed
  as a multi-ctor deftype (e.g. `TopLevel` from a map `get`) is unsafe —
  several ctors can carry the same field name (`Definition` and `Defp`
  both have `name`), and new-toc resolves getters by name lookup over
  already-defined types. The working pattern (cf. `result-kind` /
  `val-kind`): a local tag protocol extended per ctor (`top-kind`), then
  per-ctor extraction protocols whose impls see the receiver statically
  typed as the concrete ctor (`defp-name`/`defp-params`/`defp-body` over
  `ast/Defp`). Guard the extraction calls behind the tag check so a
  wrong-ctor value prints a FAIL instead of aborting the protocol.
- **`str*` fingerprints for shape checks (2026-09-01, item 6c)**:
  `(str* vec)` over a vector of `str-vect` implementors concatenates them
  into one String; `(str* [])` is `""` (`to-str`/`vect-reduce` handle the
  empty case). Vector's `str-vect` prints `[a b]` (space-interposed). Use
  for expected-shape assertions in drivers (cf. `body-fingerprint` /
  `elem-fingerprint` in `interpreter/rdr-defp.toc`).

- **malformed-cond quirk refined (2026-09-01, item 6d)**: the
  "let containing a nested cond" trigger (item-6 entry) fires when the
  nested cond's clauses are BOTH lets — `parse-ctor`'s
  `(cond (symbol-start? ...) (let [...] (cond (str-prefix? ...) (let ...)
  (let ...))) ...)` was rejected at the OUTER cond's line; the same
  outer shape with a plain-call inner else (`parse-top-defn`,
  `parse-top-defp`, `parse-impl`) compiles fine. Workaround (applied):
  extract the inner cond into a helper (`parse-ctor-body`,
  `parse-top-deftype-body`) so the outer clause is a plain call.

- **A direct `(.field v)` call parses as a `Call` with a `Symbol`
  operator named `".field"` — NOT a `FieldGetter` operator** (2026-09-01,
  item 6e): `parse-call` reads `.field` via `read-full-symbol` and
  `dispatch-special` falls through to the plain-Call branch.
  `FieldGetter` operators arise ONLY from `->` threading steps (the
  `Symbol .f` thread-step branch). Phase-2 eval must therefore resolve a
  leading-dot Symbol operator as a field getter.

- **Item-6d driver: `interpreter/rdr-deftype.toc`** (2026-09-01):
  parses the 5 hvm-core.toc deftypes (Maybe, Leaf, String,
  GetSentinelVal, List) verbatim through `parse-program` and checks
  each map by ctor/field/impl fingerprint (`ctor-fingerprint` /
  `impl-fingerprint` — tag protocols + guarded per-ctor extraction,
  same pattern as `rdr-defp.toc`); 5 OK lines, zero leaks, 0 remaining
  nodes. `rdr-deftype` Makefile target (same pattern as `rdr-top`).
  Fingerprint gotcha: `(str* (str-vect v))` already prints the vector
  WITH its `[`/`]` — the first driver draft added extra brackets and
  printed `[[x]]`.

- **new-toc non-deterministic module symbol loss (2026-09-01, item 6f)**:
  with `parse-program`'s defn LAST in `intrp-rdr.toc` (the natural
  order), new-toc drops `parse-program` from the module's symbol table
  in ~90% of builds — the importing driver fails with `Undefined symbol:
  'rdr/parse-program'` (or whichever symbol the driver references last).
  The SAME file sometimes compiles clean, so it is a race inside new-toc,
  not a source error (bisection over the file's contents changed the
  failure rate but never eliminated it). Workaround (applied, verified):
  forward-declare `parse-program-acc` and put `parse-program`'s defn
  BEFORE `parse-program-acc`'s — 0 undefined-symbol failures in 40+ builds.
  Transient no-message segfaults still occur in ~30% of builds of the
  same file — retry (existing rule).
- **Driver-probe gotchas (2026-09-01, item 6f)**: (a) `pr*` on a VECTOR
  of strings aborts silently (SIGABRT, no output) — `pr*` takes one
  string; use `(str* [...])` to build it. (b) `str-vect` is NOT
  implemented on the `TopLevel` ctors (only the `Expression` ctors) —
  drivers fingerprint TopLevel values with the `top-kind` tag + per-ctor
  extraction, never `str-vect`. (c) scratch binaries must be built with
  `-DCHECK_MEM_LEAK=1` (the Makefile targets do) — without it, lazy
  evaluation hits `BOOM("Make this threadsafe")` in `eraseLazy` (new.c).
- **`symbol-start?` was missing `*` (2026-09-01, item 6g)**: the parser's
  `symbol-start?` (interpreter/intrp-rdr.toc) handled `. _ < > = + - /` but
  NOT `*`, so `(defn * [x y] ...)` (hvm-core.toc:330) failed with
  `expected a name after defn` — and any `*` in expression position was
  silently dropped (the `parse-expr` unknown-char fallback). `symbol-continue?`
  already included `*` (and `?`/`!`); only the start set was incomplete.
  Added `(str-prefix? "*" s)` to `symbol-start?`. hvm-core.toc's operator
  names are `+ - * < <= =`; only `*` was missing. With the fix, `parse-program`
  reads hvm-core.toc with zero parse errors.
- **malformed-cond: a `let` in a NON-ELSE clause is rejected (2026-09-01,
  item 6g)**: the `malformed 'cond' expression` trigger is broader than the
  documented "let containing a nested cond" — a `let` in a NON-ELSE clause
  (even with NO nested cond) is rejected. E.g. `(cond (= k 2) (let [...] (print
  ...)) <else>)` fails at the let's line. Fix: extract the let into a helper
  `defn` so the clause is a plain call (this is why the parse-error handling in
  the drivers lives in a `defn`, e.g. `parse-error-line`, not inline in a cond
  clause). A `let` in the ELSE clause is fine.
- **A `reduce` closure capturing a FREE VARIABLE leaks (2026-09-01, item 6g)**:
  a `reduce` whose closure captures a free variable (e.g. `cls` in
  `count-class`) leaks term pairs on the lazy machine — over hvm-core.toc
  (194 nodes) a single such reduce exhausts the 1MB term buffer (`Error: Not
  enough space to allocate pair`), and 7 of them leave ~11k leaked pairs +
  `bad result SUP pair`. A reduce with a LITERAL (no free-variable capture) is
  clean, as are reduces that only call a protocol dispatch (`top-kind`) with no
  capture. Fix: explicit recursion passing the value as a plain parameter — the
  same workaround as `intrp-rdr.toc`'s `threading-acc` (cf. the threading
  verified fact). `rdr-hvmcore.toc`'s `count-class` uses `count-class-acc`
  (explicit recursion, `cls` as a param). Verified with a staged probe: trivial
  reduce clean, `top-kind`-dispatch reduce clean, `node-class`+literal clean,
  `node-class`+variable-capture crashes.
- **Item-6g driver: `interpreter/rdr-hvmcore.toc` (2026-09-01)**: the reader
  acceptance driver. `slurp`s hvm-core.toc (inline-C whole-file read, same as
  `rdr-top.toc`), runs `parse-program`, and checks: the result is a
  ParserMatch; the `[TopLevel]` vector has 194 nodes; the per-form counts are
  108 defn / 5 def / 58 defp / 5 deftype / 16 extend-type / 2 top-level inline
  / 0 other; and a spot-check of one defp (`type-name [x]`), one deftype
  (`Maybe`: `None` + `Some [x]`), one extend-type (`None`: 8 methods), and one
  top-level inline (first node, c-code prefix `\n#define _XOPEN_SOURCE 600`).
  The defn/def split is by the `Definition`'s value shape (Fn → defn, Inline →
  def — all 5 hvm-core `def`s bind an inline). `node-class` tags each TopLevel
  (tag protocols `top-kind`/`expr-kind` + per-ctor extraction, the item-6c
  pattern). All checks OK, zero leaks, 0 remaining nodes. The `rdr-hvmcore`
  Makefile target follows the `rdr-top` pattern. The parse-error branch and the
  success path are separate `defn`s (`run-parse`/`run-success`) to keep `let`s
  out of non-else cond clauses (see the malformed-cond fact above).
- **The generated protocol dispatcher's shape** (2026-09-02, item 7a):
  `emit-proto` (codegen.toc:900) emits per defp: `strictArgs([file line
  receiver])` → `if (termTag(dispVal) == I60) { <Integer impl, else BOOM
  "found for integers"> } else { switch(dispVal->type) { <case per impl>
  default: <UnknownType impl, else BOOM "found for type %s"> } }`. A
  **defp with a body** registers the body under `UnknownType` and the
  `default:` case calls it (verified: the `type-name` dispatcher's
  default → the defp body, `glbltype_name9`). The I60 branch does NOT
  fall through to the default (BOOMs "for integers" even when a body
  exists). There is **no F60 branch** — a Float receiver falls into the
  switch and dereferences the float's bits as a `Value*` → garbage type
  → BOOM. A **REF receiver** reads the function pointer as a `Value*` →
  garbage → BOOM. Dispatch failures print `file:line` ("called from
  %.*s: %ld") but the message is the dispatcher's fixed text.
- **`type-num` BOOMs on REFs** (2026-09-02): hvm-core.toc:86
  `case REF: BOOM("too tire")` — repro `env-test.toc` (`(type-num pr*)`)
  → `too tire at m.c:131`. Fix settled in item 7a (REF → `FunctionType`
  4, `dec_and_free` per the ref contract).
- **`FunctionType` (4) is free in the new runtime** (2026-09-02):
  defined at runtime3.h:159, unused in runtime3.c/new.c/hvm-core.toc.
  Precedent: the old core has a reified `Fn` Type checking
  `checkInstance(FunctionType, ...)` (core.toc:264) and the typer types
  defn globals `'Fn` (typer.toc:404, 739).
- **`dec_and_free` on a REF is a no-op** (2026-09-02): falls to
  `default:` → `pushRedex(ERA, pv)`; `interactions[ERA][REF] = nop`.
  Safe to call for ref-contract consistency.
- **`vectGet` duplicates; DUP is safe on VAL/REF/VAR** (2026-09-02):
  `vectGet` uses `dupeVal` (refcount-safe extraction for `intrp-call`'
  C body); `interactions[DUP]` → `dupLeaf` (VAL/REF) / `negVar` (VAR),
  so a value may be used twice in Toccata source.
- **The new-toc build pipeline** (2026-09-02): the `toccata` target
  builds the OLD compiler binary; the `new-toc` target runs
  `./toccata compiler.toc` (the new compiler's sources:
  compiler/base/typer/codegen.toc) and compiles the generated C with
  `core.c` (old runtime). New-toc-source edits need only `make new-toc`;
  the old binary sees new symbols as data. `c/` in the compiler sources
  = the constraints git-dependency (pinned sha 2f1dce1; has
  `FunctionType` at line 117, verified in the pinned commit).
- **Core-module type-table seeding** (2026-09-02): `b/new-module`
  (base.toc:193) builds the core SymTable's types from
  `c/core-type-constraints` (constraints.toc:3235 — Integer,
  StringBuffer, FnArity, BitmapIndexedNode, ArrayNode,
  HashCollisionNode, Vector, HashMap, Opaque; NO Function/Float).
  `extend-type <sym>` resolves via `b/get-type-info` (typer.toc
  extend-ast pre-check) → current ns's `.types`, then core ns's.

## Settled (continued)

- **Interpreter work lives in `interpreter/`** (2026-08-28): all
  interpreter sources — `intrp-ast.toc`, `intrp-rdr.toc` (moved),
  future `intrp-eval.toc` / `intrp.toc`, plus the new-toc debug probe
  `xns-probe.toc` — live in the `interpreter/` subdirectory. `add-ns`
  module references use bare filenames (resolved relative to the
  importing file's directory); co-location also keeps the module
  cache's raw-path-string keys identical across the graph (see the
  `../` verified fact).
- **Phases 1–2 are strictly single-file.** `add-ns` of local modules is
  deferred to a later phase; the 6 blocked differential tests
  (integer-regressions, string-regressions, vector-regressions,
  test-threading, test-closures, test-inline-namespaced-sym) come in with
  it. The interpreter (phase 2) uses exactly one namespace; the namespace
  *structure* exists from the start (see the Environment bullet below) and
  is populated when `add-ns` lands.
- **`eval-call` has two target kinds**: a user closure (AST body —
  interpreted) or a primitive (a `REF`: core defn / defp dispatcher /
  native). **Settled (2026-09-02, grilling)**: the env holds the raw
  REFs (initial env = compile-time literal hash map of core symbols);
  a primitive is called by the interpreter's inline-C `intrp-call [f
  ops]` (vector-based: APP chain + `VAR` at `portLoc(2)` +
  `pushRedex` — the generated-C call pattern); no core `apply`
  extension. Discrimination is by real protocols — `resolve` (lookup)
  and `interpret` (call target) — enabled by making `Function` a
  first-class compiler type (checklist item 7a); see Phase-2 design
  status.
- **Closure deftype** (`interpreter/intrp-eval.toc`):
  `(deftype Closure [name params body env])` — `name` String (`""` if
  anonymous; call-time self-binding), `params` `[String]`, `body`
  `[Expression]` (leading `TypeConstraint`s skipped by eval), `env` =
  captured environment (persistent sharing).
- **Environment** — namespace structure included from the start (`add-ns`,
  a later phase, populates it; adding the shape later would be a pain):
  `(deftype Env [current-ns namespaces])` — `current-ns` String (phase 2:
  always `""`), `namespaces` String → (String → value). Phase 2: one ns
  (`""`) with core bindings + all top-level defs. Closure captures the
  whole Env. Call-time env = captured env with the current ns's map
  extended by param bindings + self-binding. Anticipated resolution
  (confirmed when `add-ns` lands): unqualified → current ns, fallback to
  global `""` on miss (core stays reachable everywhere, as in compiled
  code); qualified `ns/name` → `namespaces[ns]` exactly. Parser produces
  the `ns` field on `Symbol` from day one.
- **`eval` dispatch**: `(defp eval [expr env])` — protocol over the
  Expression constructors (receiver = the Expression, first arg), one
  `extend-type` impl per constructor. Phase 3's abstract interpreter
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
- **Parse output: a vector of TopLevel values, source order** (2026-09-01,
  owner — supersedes the earlier "hash map of name → AST value" shape).
  `parse-program` returns a plain `[TopLevel]` vector; the name → value map
  is extracted in a later stage, not by the parser. Top-level `defn`/`def`
  → `Definition`; `main` → `Main`; `defp` / `deftype` / `extend-type` →
  their ctors; top-level `inline` → `TopLevel.Inline` (nameless — no key
  needed in a vector); comments skipped. The reader (phase 1) parses all of
  these to read hvm-core.toc; the interpreter (phase 2) interprets only
  `defn`/`def`/`main` and treats the rest as data.
- **Top-levels are LAZY, no memoization (phase 2)**: a top-level name
  binds to its AST (wrapped in a marker deftype so lookup can tell
  "top-level def — evaluate it" from "already a value — return it"
  without dispatching on raw values); the AST is evaluated on first use
  in the environment current at that moment. Consequences: the map needs
  no source order; forward references work (everything is bound before
  `main` runs); cycles → infinite loop at first use; each use re-evaluates
  (pure def values unaffected — all 20 differential candidates are pure;
  side-effecting def values repeat per use — documented sharp edge).
  **Marker settled (2026-09-02)**: `(deftype TopDef [name ast])` —
  `name` String (error messages), `ast` = the def's value Expression;
  no `!` annotations; map/flat-map/recurse impls per the style doc
  (`recurse` over `ast` only). Env binding: `Definition` → name →
  `TopDef`; `Main` → set aside (the driver calls it); `Defp` /
  `DefType` / `ExtendType` / `Inline` / `BlockComment` → skipped
  (top-level `inline` is ignored, not bound).
- **The interpreter is the semantics reference.** new-toc's behavior is
  not a design constraint — it is only the build crutch. (Differential
  tests still validate core behavior on the 20 clean candidates.)
- **Whole-file read: inline-C function named `slurp`** (in
  `interpreter/intrp.toc`).
- **Error reporting (phase 2)**: `file:line: message` to stderr + abort
  (non-zero exit), uniform across parse/structural/runtime errors; no
  Maybe-threading through eval; no backtraces. (Phase 3's typechecker
  gets its own collected-error mechanism.)
- **Makefile**: repurpose the `intrp` target — `intrp.c:
  interpreter/intrp.toc interpreter/intrp-ast.toc
  interpreter/intrp-rdr.toc interpreter/intrp-eval.toc` via
  `./new-toc interpreter/intrp.toc > intrp.c`; compile `new.c
  runtime3.c graph.c intrp.c` with the regression-test flags (`-g
  -march=native -I. -DCHECK_MEM_LEAK=1 -DSAFETY=1 -DSTATS=1 -lm
  -lpthread`).
- **Parser organization**: one function per syntactic form in
  `interpreter/intrp-rdr.toc`, named after the form (`parse-top-level` keyword-
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

## Phase-2 design status

Complete (2026-09-02, grilling session). The two pieces deferred to the
phase-1 → phase-2 boundary are settled:

- **Primitive representation**: the env holds the raw core REFs — the
  initial env is a compile-time literal hash map of the item-3 25
  symbols → the core symbols as values. A primitive is called by a
  single vector-based inline-C `intrp-call [f ops]` in the interpreter
  source (`count` + `vectGet` + APP chain + `VAR` at `portLoc(2, app)`
  + `pushRedex` — exactly the generated-C call pattern, so interpreter
  calls behave like compiled calls). No core `apply` extension; the
  core's `apply` protocol stays unimplemented.
- **Top-level-def marker + discrimination**: `(deftype TopDef [name
  ast])`. Discrimination is by **real protocols** (defp + extend-type),
  enabled by making `Function` a first-class compiler type (item 7a):
  - `resolve [v env]` — defp **with a default body = identity** (`v`);
    one impl: `extend-type TopDef` → `(eval (.ast v) env)`. REFs,
    immediates, Closures, and every other value fall through to the
    default — no identity-impl enumeration (a defp's body registers
    under `UnknownType`; the dispatcher's `default:` case calls it).
  - `interpret [f ops env loc]` — defp **with a default body = the
    clean `file:line: not callable` abort** (`loc` = the `Call`'s loc,
    passed by `eval-call`); impls: `extend-type Function` →
    `(intrp-call f ops)` (item 7); `extend-type Closure` → bind
    params + self, eval body (item 8 — needs `eval`).
  No startup type-ID computation (that belonged to the rejected
  type-num-cond alternative) — the dispatcher switches on type
  internally.
- **Structural loading rules** (owner, 2026-09-02): top-level `inline`
  expressions are ignored (not bound); a `defn` whose body is inline C
  is a structural error unless it appears in the core namespace and its
  name is found in the initial env (phase 2: vacuous — the core is
  compiled in, not loaded; the rule stands for later phases).

## Phase 1 implementation checklist (Ralph loop) — reader

Protocol: work top to bottom, one item per session. Check an item off
only when its "done when" holds, then commit. Context for every item:
this file (the settled design above) + AGENTS.md. The reader's own source
may use inline C freely. Never touch the `toccata` Makefile target. Phase 1
ends at item 6g: a reader that fully reads `hvm-core.toc`.

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

- [x] **2. `interpreter/intrp-ast.toc`: settled AST delta**
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
  - Line-by-line audit of the 51 regression tests against the phase-2 (concrete interpreter)
    scope; record the suitable list (expect the ~20 clean candidates
    from the rough audit above) and the union of core symbols they
    reference (feeds item 7's initial env). Flag any candidate that
    uses `main`'s argument vector (interpreter runs pass a different
    argv than compiled runs).
  - Done when: the suitable list + core-symbol list are written to
    `intrp-tests/README.md`, with a one-line exclusion reason for every
    other test.

- [x] **4. `interpreter/intrp-rdr.toc`: parser — state + tokens**
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

- [x] **5. `interpreter/intrp-rdr.toc`: parser — expressions +
    desugarings**
  - Literals (int/float/string; quoted sym → `StringLit`), calls,
    special forms (`fn`, `let`, `cond`, `and`, `or`, `str`, `println`,
    `->`, `!` markers), vector/hash folds — all the settled
    desugarings.
  - Done when: a scratch driver parses sample expressions and asserts
    the expected shapes via field access + `println*` (let → nested
    immediate Fn apps; cond → right-nested `cond` calls; `[...]` →
    `vect-conj` fold; `{...}` → `assoc` fold; `->` → nested Calls;
    `'sym` → `StringLit`).
  - As-built (2026-08-28): driver `interpreter/rdr-exprs.toc` (14
    samples; all expected fingerprints printed; zero leaks). Found and
    fixed two parser bugs: (1) the `parse-expr` dispatch
    double-consumed the opening `(`/`[`/`{` — it `take-char`'d and the
    sub-parsers (`parse-call`/`parse-vector`/`parse-hash`, documented
    "state at the bracket") did it again; the dispatch now passes the
    state at the bracket. (2) `parse-threading`'s steps branch tested
    `(= sk 2)` (ParserError) instead of `(= sk 0)` (ParserMatch) — on
    success it returned the raw steps vector as the parse value.

- [x] **6. `interpreter/intrp-rdr.toc`: parser — top-level + map output**
  - Top-level rule: `def`/`defn` → map entry under the name; `main` →
    `"main"` entry; top-level `inline` → parse error; comments
    skipped. Output: name → AST map. Out-of-scope forms (`deftype`,
    `defp`, `extend-type`, `add-ns`, `match`, `|`, `defmacro`) → parse
    errors.
  - Done when: every suitable test from item 3 parses to a map with
    the expected entries; a program with top-level `inline` fails with
    a clear `file:line` error.
  - As-built (2026-08-28): top-levels are prefix forms — `parse-top-level`
    takes `(`, reads the keyword, dispatches (`parse-top-level-form` /
    `-args`). `parse-top-def`/`-defn`/`-main` build the `Definition` /
    `Main` nodes; a bare `(def name)` is ignored (no entry). `parse-program`
    loops to EOF building the name → TopLevel map (keyed by full Strings —
    see `sub-to-str`). Driver `interpreter/rdr-top.toc`: 19 tests +
    inline-error + 7 out-of-scope forms all pass, zero leaks.

- [x] **6b. `interpreter/intrp-rdr.toc`: style rewrite (docs/toccata-style.md)**
  - Rewrite the parser to the settled style: every positionally-meaningful
    grouping becomes a deftype ctor with named fields, accessed via `.field`
    getters — no positional vectors, no `elt0`/`elt1`/`elt2`. Implement
    `map`/`flat-map` (and `recurse` where structurally recursive) on the
    grouping types per the style doc's Data section.
  - The `unfold`-based reader sub-part is **DROPPED** (owner, 2026-09-01) —
    not expressible over the settled AST (see the Verified-facts entry); the
    parser stays the direct recursive-descent one. Parser alternatives are
    sequential `or`-style tries (first `Match`/`Ignore` wins, else aggregate
    the best `Error`) — the superposition-alternatives approach is
    **dropped for this rewrite** (owner, 2026-08-29), paused pending further
    owner thought (see the Parser section bullet).
  - All settled behavior (parse results, desugarings, error messages, the
    name -> TopLevel map) stays identical. Update the drivers
    (`rdr-exprs.toc`, `rdr-top.toc`) where they touch the changed shapes —
    including `rdr-top.toc`'s `check-oos` positional `[source msg]` pairs.
  - Done when: the file compiles clean under new-toc (`*** Loaded`, no error
    lines); no positional vector grouping remains in `interpreter/*.toc`;
    both drivers pass with zero leaks (the 19-test map check unchanged).

- [x] **6c. `interpreter/intrp-rdr.toc`: reader — `defp`**
  - Parse `(defp name [params])` (bare protocol declaration, empty body)
    and `(defp name [params] !annos body)` (with `!`/`!returns`
    annotations and an optional body expression) → `TopLevel.Defp
    [name parameter-list body]`. The AST ctor already exists.
  - Done when: a scratch driver parses a bare defp, a defp with
    `!returns`, and a defp with a body to the expected `Defp` shapes
    (field access + `println*`), zero leaks.
  - As-built (2026-09-01): `parse-top-defp` reuses `read-param-list` +
    `parse-body` (leading `!` constraints + optional body until `)`);
    `parse-top-level-args` dispatches `defp` → `parse-top-defp`.
    Driver `interpreter/rdr-defp.toc` (4 samples: bare, comment +
    `!returns`, `!returns` + body, special-symbol name `=`) — all pass,
    zero leaks; `rdr-defp` Makefile target follows the `rdr-top`
    pattern. `rdr-top.toc`'s `defp` OosCase removed (6 OOS forms left).

- [x] **6d. `interpreter/intrp-rdr.toc`: reader — `deftype`**
  - Parse both deftype forms → `TopLevel.DefType [type-name constructors]`
    with `Constructor [name field-list implementations]` nodes:
    multi-ctor `(deftype T (C1 [f...]) (C2 [f...] (impl ...)))` and
    single-ctor `(deftype Ctor [f...] (impl ...))`. The ctor list may also
    name an existing compiler-defined type (the bare `StringBuffer` in
    `deftype String StringBuffer (SubString ...)`) — record it as a
    name-only constructor entry; no special handling. Ctor protocol impls
    are named fns.
  - Done when: a scratch driver parses the 5 hvm-core deftypes (Maybe,
    Leaf, GetSentinelVal, String, List) to the expected `DefType` /
    `Constructor` shapes, zero leaks.
  - As-built (2026-09-01): `parse-top-deftype` (+ `parse-top-deftype-body`)
    handles both forms; `parse-ctor` (+ `parse-ctor-body`) parses
    `(Ctor [fields] <impls>)` / `(Ctor <impls>)` (no field list → no
    fields); `parse-ctor-list-acc` takes ctor definitions and bare
    existing-ctor names (name-only `Constructor`); `parse-impl` /
    `parse-ctor-impls` parse the named-fn impls. The `-body` helpers
    exist because new-toc rejects the let+cond clause shape inline
    (refined malformed-cond fact below). Driver
    `interpreter/rdr-deftype.toc` (the 5 hvm-core deftypes verbatim —
    Maybe, Leaf, String, GetSentinelVal, List — checked by
    ctor/field/impl fingerprint) — all 5 OK, zero leaks; the
    `rdr-deftype` Makefile target follows the `rdr-top` pattern.
    `rdr-top.toc`'s `deftype` OosCase removed (5 OOS forms left; 25
    checks pass).

- [x] **6e. `interpreter/intrp-rdr.toc`: reader — `extend-type`**
  - Parse `(extend-type T (proto [params] body) ...)` → `TopLevel.ExtendType
    [type-name methods]`. Nail down the method shape (likely `Fn` nodes:
    name = protocol, params, body). The AST ctor already exists.
  - Done when: a scratch driver parses the `None`/`Some` extend-type blocks
    (and one with a `let`/`->` body) to the expected shape, zero leaks.
  - As-built (2026-09-01): method shape = `Fn` nodes (name = protocol,
    params, body) — parsed by `parse-impl` / `parse-impls` (renamed from
    `parse-ctor-impls`; now shared by deftype ctor impls and extend-type
    methods). `parse-top-extend-type` reads the type name + method list;
    the entry is keyed under the type name in the map. Driver
    `interpreter/rdr-extend-type.toc` (the hvm-core `None` block — 8
    methods — the `Some` block — 10 methods incl. a `->` body — and a
    synthetic `let`-body method; checked by type name + methods
    fingerprint: per-method name/params/body-elem fingerprints) — all 3
    OK, zero leaks; the `rdr-extend-type` Makefile target follows the
    `rdr-top` pattern. `rdr-top.toc`'s `extend-type` OosCase removed
    (4 OOS forms left; 24 checks pass).

- [x] **6f. `interpreter/intrp-rdr.toc`: reader — top-level + expression
    `inline` + vector parse output**
  - Top-level `inline` (the 2 global C declaration blocks in hvm-core.toc):
    parse `(inline "...")` and `(inline TypeName "...")` → `TopLevel.Inline`
    (the `Expression/Inline [type-expr c-code loc]` node, via the bare
    reference — the dedicated-ctor decision is superseded, see below); a
    nameless entry in the vector, no key needed.
  - **Dedicated `TopLevel.Inline` ctor — SUPERSEDED (2026-09-01, owner)**:
    a dedicated `TopLevel.Inline` constructor was decided earlier today,
    but it cannot coexist with `Expression.Inline` under new-toc's global
    ctor-name uniqueness (verified fact); the bare reference to
    `Expression/Inline` stands. Top-level vs. expression inline is told
    apart by position (top of the program vector), not by node type.
  - Expression-level `inline` (69 in hvm-core.toc) **lowers to
    `Expression.Inline` nodes** (decided 2026-09-01, owner) — not a plain
    `Call` with operator `inline`. `type-expr` = `Some` of one ordinary
    expression (a bare type name in all hvm-core uses) or `None` when
    absent.
  - **Parse output is a plain `[TopLevel]` vector in source order**
    (decided 2026-09-01, owner) — `parse-program` no longer builds the
    name → TopLevel map; the map is extracted in a later stage. Ripples:
    the `sub-to-str` keying leaves the parser; the three drivers
    (`rdr-top.toc`, `rdr-defp.toc`, `rdr-deftype.toc`) rework their map
    checks (count + `get` by name) into vector checks (count + find-by-name
    via the `top-kind` tag + per-ctor extraction).
  - Done when: a scratch driver parses a top-level inline block and an
    expression inline to the settled shapes; `parse-program` returns the
    vector; all three existing drivers pass (reworked to vector lookups),
    zero leaks.
  - As-built (2026-09-01): `parse-inline` (+ `parse-inline-typed` /
    `parse-inline-code`) parses `(inline <type-expr> "<c-code>")` /
    `(inline "<c-code>")` → `ast/Inline [type-expr c-code loc]` (type-expr
    = `None` or `Some` of one ordinary expression; the c-code is the
    `read-string` SubString view); shared by the top-level form
    (`parse-top-level-args`) and the expression level (`dispatch-special`
    `inline` branch) — the `TopLevel.Inline` entry is the bare reference
    to `Expression/Inline`. Vector output: `parse-program` returns the
    `[TopLevel]` vector in source order (`parse-program-acc` accumulator);
    the `TopLevelEntry` wrapper and `sub-to-str` keying are removed from
    the parser. Build crutch: `parse-program` precedes
    `parse-program-acc` in the file (new-toc symbol-loss race — see
    Verified facts). Drivers: `rdr-top` (23 OK), `rdr-defp` (4 OK),
    `rdr-deftype` (5 OK), `rdr-extend-type` (3 OK) all reworked to vector
    lookups (`top-kind` tag + find-by-name) and passing; new
    `interpreter/rdr-inline.toc` (5 samples: top-level nameless/typed,
    `def`-bound nameless/typed, and a mixed `def`/`inline`/`defn` file —
    checked by `top-kind` + per-ctor fingerprint) — all OK, zero leaks,
    0 remaining nodes; the `rdr-inline` Makefile target follows the
    `rdr-top` pattern.

- [x] **6g. Reader acceptance: fully read `hvm-core.toc`**
  - A driver runs `parse-program` over `hvm-core.toc` and produces a
    complete vector of TopLevel values with **zero parse errors**. Every
    top-level form is accounted for: 108 `defn`, 5 `def`, 58 `defp`,
    5 `deftype`, 16 `extend-type`, 2 top-level `inline` (194 total).
  - Done when: `parse-program` on `hvm-core.toc` returns a `ParserMatch`
    (not `ParserError`), the vector has the expected count (194), and a
    spot-check of one `defp`, one `deftype`, one `extend-type`, and one
    `inline` shows the correct shapes. Zero leaks.
  - As-built (2026-09-01): driver `interpreter/rdr-hvmcore.toc` (the
    `rdr-hvmcore` Makefile target follows the `rdr-top` pattern). Two parser
    fixes were required to reach zero parse errors / zero leaks (both recorded
    in Verified facts): (1) `symbol-start?` was missing `*` — `(defn * [x y]`)
    failed with `expected a name after defn`; added `*` to the start set. (2)
    the acceptance driver's `count-class` originally used a `reduce` whose
    closure captured the free variable `cls` — that leaks term pairs on the
    lazy machine (buffer exhaustion over hvm-core.toc); rewritten as explicit
    recursion (`count-class-acc`) passing `cls` as a plain parameter. Result:
    `parse-program` returns a ParserMatch, 194 nodes, per-form counts
    108/5/58/5/16/2/0, all four spot-checks OK, zero leaks, 0 remaining nodes.
    The existing drivers (`rdr-exprs`/`rdr-top`/`rdr-defp`/`rdr-deftype`/
    `rdr-extend-type`/`rdr-inline`) still pass (the `*` change is additive).

## Phase 2 implementation checklist (Ralph loop) — concrete interpreter

Items 7–13 are the former phase-1 interpreter work, moved here unchanged.
Same protocol as phase 1. The item-7 design (primitive representation +
top-level-def marker + discrimination) was settled with the owner
2026-09-02 (grilling) — see Phase-2 design status. **Item 7a is a
prerequisite** (compiler changes, agent edits + owner build); item 7
depends on it.

- [ ] **7a. Compiler prerequisite: `Function` type + REF/F60 protocol
    dispatch (agent edits, owner builds + verifies)**
  The interpreter's discrimination protocols dispatch over REFs (core
  symbols) and immediates; the generated dispatcher cannot do that
  today (see the dispatcher-shape verified fact). Three source edits,
  then the owner runs `make new-toc` and verifies — the agent stops
  after the edits.
  - **hvm-core.toc — `type-num` (lines 74–89)**: replace
    `case REF: BOOM(\"too tire\");` with
    `case REF: result = newI60(FunctionType); dec_and_free(x_1, 1); break;`
    (`FunctionType` = 4, runtime3.h:159, unused in the new runtime;
    `dec_and_free` on a REF is ERA→nop). `type-num` becomes total over
    strict args: I60→1, F60→19, REF→4, VAL→its type id. Acceptance:
    `env-test.toc` (`(type-num pr*)`) prints `4` instead of
    `too tire at m.c:131`.
  - **base.toc — `new-module` (~line 193)**: register `Function` in the
    core module's type table so `extend-type Function` resolves
    (resolution path: typer extend-ast pre-check → `b/get-type-info` →
    current ns's `.types`, then core ns's, seeded from
    `c/core-type-constraints`):
    `(assoc c/core-type-constraints (c/tag 'Function)
           (c/ReifiedConstraint c/FunctionType (c/tag 'Function) {} {}
                                empty-list c/no-symbol))`
    (field order per `create-type`, typer.toc:1567; `c/FunctionType`
    verified present in the pinned constraints sha 2f1dce1). The
    constraints git-dependency itself is untouched.
  - **codegen.toc — `emit-proto` (~lines 900–940)**:
    - I60 branch: Integer impl → **else default impl** → else BOOM
      "for integers" (currently: no fall-through to the default).
    - **New F60 branch**: default impl → else BOOM "for floats" (no
      impl lookup — Float is not a registered type). Today a Float
      receiver dereferences the float's bits as a `Value*` → garbage →
      BOOM.
    - **New REF branch**: Function impl (`.impls[c/FunctionType]`) →
      else default impl → else BOOM "for Function".
    - Switch-case reduce: exclude `c/FunctionType` alongside 0 and
      `c/IntegerType` (a VAL of type 4 cannot exist; the impl is
      handled by the REF branch).
    - "Default impl" = the defp's body registered under `UnknownType`
      (the dispatcher's `default:` case already calls it when present).
  - **Build + verify (owner)**: `make new-toc` (the old `toccata`
    binary is unchanged — new symbols are data to it). Verify with a
    scratch program: a defp-with-body + `extend-type Function`, called
    over a REF receiver (Function impl), an I60 receiver (fall-through
    to default), an F60 receiver (fall-through to default), and a
    VAL-of-unlisted-type receiver (default); plus `env-test.toc`.
  - Done when: the owner has built new-toc and the verification
    program passes.

- [ ] **7. `interpreter/intrp-eval.toc`: interpreter — data + environment**
  Design settled 2026-09-02 (grilling) — see Phase-2 design status.
  - `(deftype Closure [name params body env])`,
    `(deftype Env [current-ns namespaces])`,
    `(deftype TopDef [name ast])` — no `!` annotations; map/flat-map/
    recurse impls per the style doc (`TopDef.recurse` over `ast` only).
  - Initial env: a compile-time literal hash map of the item-3 25
    symbols → the core symbols as values (raw REFs).
  - `intrp-call [f ops]` — inline-C vector-based apply (`count` +
    `vectGet` + APP chain + `VAR` at `portLoc(2, app)` + `pushRedex`).
    [Open implementation detail: the zero-operand case — no candidate
    calls a zero-arg fn; decide at implementation (clean error vs.
    probe the generated zero-arg pattern).]
  - **AST/parser delta**: add `loc` to `Expression.Call`
    (`intrp-ast.toc` + `parse-call` in `intrp-rdr.toc` + `Call`'s
    `str-vect`) — the call site for the `not callable` / arity errors
    (settled 2026-09-02, owner).
  - **Env extension**: `env-bind [env name value]` (one `assoc` into
    the current-ns map) + `env-bind-all [env pairs]` (explicit
    recursion over a flat `[k1 v1 k2 v2 ...]` vector — no `reduce`,
    the free-variable-capture leak); the `interpret` Closure impl
    walks `params`/`ops` in lockstep to build the pairs, appends
    `[name closure]` when `name ≠ ""` (settled 2026-09-02, owner).
  - **Protocol split** (settled 2026-09-02, owner): item 7 ships
    `resolve [v env]` (default body = identity only) +
    `interpret [f ops env loc]` (default body = the `file:line:
    not callable` abort + the `Function` impl → `(intrp-call f ops)`).
    The `TopDef` `resolve` impl and the `Closure` `interpret` impl
    both need `eval` — they land in item 8 (extend-type blocks added
    to the existing defps). `loc` flows from `eval-call`'s `Call` loc;
    `resolve` stays `[v env]` (its only error path is a dispatch
    failure, which carries its own file:line).
  - **Harness** (scratch driver): (1) initial env builds — 25
    entries, count + spot lookups; (2) `env-bind` a value → lookup
    returns it (I60 fall-through to `resolve`'s default); (3) lookup a
    core symbol → returns the REF (REF fall-through); (4) `interpret`
    a core REF with ops (e.g. `+` over `[1 2]`) → `3` (Function impl +
    `intrp-call`); (5) separate sub-run: `interpret` an Integer →
    clean `file:line: not callable` abort, non-zero exit.
  - Done when: the file compiles under the rebuilt new-toc and the
    harness checks 1–5 pass, zero leaks.

- [ ] **8. `interpreter/intrp-eval.toc`: interpreter — eval**
  - `(defp eval [expr env])` over `String`, `IntegerLit`, `FloatLit`,
    `StringLit`, `Call`, `Fn`, `FieldGetter`, `TypeConstraint` (skip).
    `eval-call`: `(interpret op ops env (.loc call))` — the dispatch
    does the rest. Errors: `file:line: message` + abort.
  - Also adds the item-7-deferred impls: `extend-type TopDef` for
    `resolve` → `(eval (.ast v) env)`; `extend-type Closure` for
    `interpret` → arity check (operand count vs param count →
    `file:line: wrong number of args` at the `Call` loc),
    `env-bind-all` over the param/ops pairs + self-binding, then eval
    the body (skipping leading `TypeConstraint`s).
  - Done when: a scratch program with defn recursion, fn/closures,
    let, cond/and/or/either, vectors, hash maps, threading, string/int
    ops interprets with hand-verified output.

- [ ] **9. `interpreter/intrp.toc`: driver + Makefile**
  - `main`: argv element 1 = file (missing → usage + abort); `slurp`
    (inline-C whole-file read); parse; exactly-one-`main` structural
    check; evaluate top-levels (lazy bindings); call `main` with the
    argv vector. Makefile: repurpose the `intrp` target — `intrp.c`
    from `interpreter/intrp.toc` (same awk `#line` step as the
    regression rule),
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

- [ ] **12. Move the `strCmp` prefix scratch test into `regression-tests/`**
  - `scratch/str-prefix.toc` (item 1's verification program) is a proper
    regression test — it has a `main` and exercises `strCmp` STR_PREFIX for
    the equal / prefix / superstring / different / empty cases. Move it to
    `regression-tests/str-prefix.toc`, add `str-prefix` to `REG_TESTS` in the
    Makefile, and let the standard rule generate
    `regression-tests/str-prefix.rslt` (`regression-tests/str-prefix
    party-pooper | sort`). Remove `scratch/str-prefix.toc`.
  - The `.rslt` golden is gitignored (`regression-tests/*.rslt`); the
    existing 51 are tracked only via force-add, so the new one must be
    staged with `git add -f regression-tests/str-prefix.rslt` or it will be
    missing from the commit.
  - Done when: `make str-prefix` builds and runs with the expected
    Some/None result per case, `str-prefix` is in `REG_TESTS`, the
    `str-prefix.rslt` is force-staged, and the scratch file is gone.

- [ ] **13. Final verification**
  - Zero leaks (malloc/free diff 0, remaining nodes 0) for the
    interpreter binary across the full test set; every item above
    checked.
  - Done when: all items checked.

Phases 3–4 (abstract interpreter, code generation): to be specified when we
get there.
