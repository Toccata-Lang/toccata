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

## Phase 1 (complete 2026-09-01) — reader

`parse-program` (`interpreter/intrp-rdr.toc`, direct recursive descent)
reads `hvm-core.toc` in full — all 194 top-level forms (108 `defn`,
5 `def`, 58 `defp`, 5 `deftype`, 16 `extend-type`, 2 top-level
`inline`) — to a `[TopLevel]` vector in source order, zero parse
errors, zero leaks. Expression-level `inline` lowers to `Inline`
nodes; all settled desugarings per the AST section. Still parse errors
(none appear in hvm-core.toc): `match`, `defmacro`, `add-ns`,
top-level `|`. Acceptance driver: `interpreter/rdr-hvmcore.toc`
(companion drivers: `rdr-exprs`, `rdr-top`, `rdr-defp`, `rdr-deftype`,
`rdr-extend-type`, `rdr-inline`).

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
  match/superposition. **Test audit (2026-08-27): 19 verified clean;
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
| `TopLevel.DefType` / `Defp` / `ExtendType` / `AddNs` / `BlockComment` | kept | `DefType` / `Defp` / `ExtendType` are produced by the phase-1 reader; `AddNs` / `BlockComment` unused in phase 1 |
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
`inline` are parsed in phase 1 so the reader can read
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
  **PAUSED (2026-08-29, owner)**: dropped for the style rewrite — the
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
- `STR_PREFIX` is **8** in runtime3.h (the original draft said 4, but
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
  consequences (2026-08-26):
  - A constructor named `String` is **unbuildable in any namespace**
    ("A type named 'String' was already defined" — the core `String`
    type). The symbol-ref constructor is `Symbol` per the AST table
    (the pre-rename file compiled clean with it). The 46d6e29 "String
    constructor" adjustment broke the clean-compile property and was
    reverted.
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
  new-toc (verified 2026-08-26).
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
  args for '+'"). Nest to combine more: `(+ a (+ b c))` (verified 2026-08-27).
- **`first`/`rest` on a Vector**: `(first v)` returns `Some element`
  (a Maybe), not the bare element — extract with `(extract (first v))`;
  `(rest v)` returns a Vector (hvm-core.toc:945, 951).
  `interpreter/intrp-rdr.toc`'s `elt0`/`elt1` use `(extract (first ...))`
  (verified 2026-08-27).
- **`add-ns` module paths resolve relative to the importing file's
  directory**, not the CWD; the regression tests work only because
  their modules sit beside them (verified 2026-08-27).
  Combined with the raw-path-string module-cache key (the `../` bullet
  below): keep a module's references in one directory, spelled
  identically.
- **`char-code`** (hvm-core.toc:645) → first char's integer code
  (0-255); `char` (634) is the inverse.
  `interpreter/intrp-rdr.toc`'s predicates classify via `char-code` +
  `<=` (verified 2026-08-27).
- **`subs` is 3-arg** (`(defp subs [s start len])`, hvm-core.toc:661);
  the rest of a string is `(subs s 1 (count s))` (len clamps to the
  tail). `count` is O(1) for both `SubString` and `StringBuffer`
  (verified 2026-08-27).
- **Named / namespace-qualified functions are first-class values**:
  `rdr/read-symbol` can be passed as an argument and called as
  `(reader s)` (verified 2026-08-27).
- **Lazy-machine side-effect threading (extended)**: a continuation that
  drops its threaded count lets the machine skip the *earlier* `pr*`
  side effects (only the last print appeared). Thread an accumulator
  through every continuation so the final result depends on all of them
  (verified 2026-08-27; see `scratch/rdr-tokens.toc`
  `read-and-show`).
- **`interpreter/intrp-rdr.toc` is a library (no main)** holding the
  state + token helpers and the expression parsers: `make-state`, `state-line`, predicates
  `is-digit`/`is-alpha`/`symbol-start?`/`symbol-continue?`/
  `float-char?`, helpers `peek-char`/`take-char`/`skip-comment`/
  `skip-whitespace`/`run-length`/`read-run`, accessors `elt0`/`elt1`,
  readers `read-string-content`/`read-string`/`read-symbol`/`read-int`/
  `read-float`. Self-recursion needs no forward declaration; the old
  `ParserCombinator` machinery and grammar `(def ...)` rules are
  removed. `read-run` = length-scan (`run-length`) + one `subs`; token
  readers return `[token-text new-state]` pairs (verified 2026-08-27).
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
- **Toccata top-levels are PREFIX forms** (2026-08-28): a
  top-level form is `(def ...)`, `(defn ...)`, `(main ...)` — it starts
  with `(` and the keyword is the first token *inside*. `parse-top-level`
  takes the `(`, reads the keyword, and dispatches (`parse-top-level-form`
  → `parse-top-level-args`). The value of a `def` is a general expression
  followed by a closing `)` that `parse-top-def-value-expr` consumes;
  `defn`/`main` bodies end in the `)` consumed by `parse-body`.
- **new-toc "malformed 'cond' expression" quirk** (2026-08-28):
  a `cond` whose **non-else clause** is a `let` that contains a nested
  `cond` is rejected ("malformed 'cond' expression"). The same shape as
  the cond's **else** is fine (cf. `parse-threading`). Fix: extract the
  `let`+`cond` into a helper function so the clause is a plain call
  (done for `parse-top-def` → `parse-top-def-value` / `-expr`, and
  `parse-top-level` → `parse-top-level-form` / `-args`).
- **`strSha1` includes the value's TYPE** (2026-08-28):
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
  (2026-08-28): `sub-to-str` `dec_and_free`s its input. This is
  safe because `elt0` (`(extract (first v))`) creates a new reference, so
  the extracted SubString and its `[run state]` pair each hold a ref —
  consuming one leaves the pair's ref valid (no double-free; zero leaks).
  A **non-consuming** copy leaked (the original SubString, still held by
  the pair, was never freed — malloc diff 33). Also: new-runtime
  `dec_and_free(Term, int)` takes a `Term`, not a `Value*` (cast
  `(Term)p`).
- **Map ops for the parse output** (2026-08-28): `count` IS
  implemented for the map node types (use `(count m)` for the entry
  count); `keys` is **NOT** ("No implementation of 'keys' found for type
  HashMap (11)"). `get`/`assoc` work. To test a `get` result's
  Some/None-ness, a local `(defp maybe-kind [m])` + `extend-type None`
  / `Some` works (instance? is unsupported).
- **Driver: `interpreter/rdr-top.toc`** (2026-08-28): parses the
  19 suitable tests through `parse-program` and checks each map
  has exactly the expected entries (entry count via `count` + every
  expected name present via `get`); checks a top-level `inline` is a
  parse error with `file:line: msg`; checks the 7 out-of-scope forms
  (`deftype`/`defp`/`extend-type`/`add-ns`/`match`/`defmacro`/`|`) are
  parse errors. Reads files via an inline-C `slurp`. All 27 checks pass,
  zero leaks. Run from the repo root: build with `./new-toc
  interpreter/rdr-top.toc` + the awk `#line` step + `clang ... new.c
  runtime3.c graph.c rdr-top.c`, then `./rdr-top`.
- **`( | )` is UNREADABLE under new-toc** (2026-08-29):
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
  `scratch/sup-probe.toc`): `(| 1)` compiles to the bare
  value (no SUP node); `(| a b)` compiles to a SUP linked list
  (`var = a; var = makePair(SUP, lab, b, var);`). They flow through
  `cond`/`fn`/`let`/`str` correctly; zero leaks, zero remaining nodes.
- **A NUL branch is ERASED when the superposition is applied**
  (2026-08-29, `scratch/sup-nul.toc`):
  `(| (empty-sup) 5)` evaluates to `5`, where `empty-sup` is inline C
  `result = NUL;` (NUL = 0x03, the eraser tag). So an inline-C NUL
  value is a workable stand-in for the unreadable `( | )` failing
  branch — the eraser kills its duplicated workflow when the
  superposed value is used.
- **Total failure erases the WHOLE downstream workflow** (2026-08-29,
  `scratch/sup-nul2.toc`): applying an all-NUL
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
  sub-nodes** (2026-08-29, `scratch/unfold-probe.toc`):
  `(deftype Node [label children] (recurse [v f] (Node (.label v)
  (map (.children v) f))) ...)` + `(unfold 2 f)` builds a tree with
  zero leaks / zero remaining nodes. The `recurse` impl must pass ONLY
  the recursive fields to `f` — passing a leaf field (e.g. an Integer
  label) re-unfolds it forever and exhausts the term buffer
  (`Error: Not enough space to allocate pair`).
- **new-toc "malformed 'cond' expression" crutch #2: a field getter inside
  a vector arg to a deftype ctor, inside a let, inside a cond clause**
  (2026-08-29): `(cond t (let [x ...] (ast/Call (ast/Fn ""
  [(name x)] ...)) ...))` — i.e. a `.field` getter as an element of a
  vector literal passed to a ctor, in a let that is a cond clause — is
  rejected with `malformed 'cond' expression`. Bisected: the getter is the
  trigger (a fn call like `(elt0 x)` in the same position compiles; a
  getter in a plain `str` call compiles; the getter only fails inside a
  vector arg to a ctor in a cond clause). Workaround: bind the getter to a
  `let` var and use the plain symbol in the vector arg (`(let [nm (name x)]
  ... [nm] ...)`). Applied in `let-desugar-rec` and `hash-fold-acc` in
  `interpreter/intrp-rdr.toc`. (Distinct from the "let containing a
  nested cond" malformed-cond quirk.)
- **As-built (2026-08-29): the concrete style rewrite is DONE and
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
- **Field getters require a prior type with that field name** (2026-08-29): new-toc resolves `.field` getters by looking up an already-
  defined type that has a field with that name. If no such type exists yet,
  the getter is an `Undefined symbol`. Workaround: define a dummy type with
  the needed field names before the first use (e.g. `(deftype Dummy [key
  value])` in `intrp-ast.toc` so that `.key` / `.value` resolve in the
  desugar helpers that run before `HashPair` is defined). Build crutch only;
  the new compiler should resolve getters structurally.
- **The `unfold`-based reader is NOT expressible over the settled AST —
  OPEN design gap (2026-08-29).** `unfold x f` = `recurse (f x)
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
  (2026-09-01)**: a `.field` getter on a value statically typed
  as a multi-ctor deftype (e.g. `TopLevel` from a map `get`) is unsafe —
  several ctors can carry the same field name (`Definition` and `Defp`
  both have `name`), and new-toc resolves getters by name lookup over
  already-defined types. The working pattern (cf. `result-kind` /
  `val-kind`): a local tag protocol extended per ctor (`top-kind`), then
  per-ctor extraction protocols whose impls see the receiver statically
  typed as the concrete ctor (`defp-name`/`defp-params`/`defp-body` over
  `ast/Defp`). Guard the extraction calls behind the tag check so a
  wrong-ctor value prints a FAIL instead of aborting the protocol.
- **`str*` fingerprints for shape checks (2026-09-01)**:
  `(str* vec)` over a vector of `str-vect` implementors concatenates them
  into one String; `(str* [])` is `""` (`to-str`/`vect-reduce` handle the
  empty case). Vector's `str-vect` prints `[a b]` (space-interposed). Use
  for expected-shape assertions in drivers (cf. `body-fingerprint` /
  `elem-fingerprint` in `interpreter/rdr-defp.toc`).

- **malformed-cond quirk refined (2026-09-01)**: the
  "let containing a nested cond" trigger fires when the
  nested cond's clauses are BOTH lets — `parse-ctor`'s
  `(cond (symbol-start? ...) (let [...] (cond (str-prefix? ...) (let ...)
  (let ...))) ...)` was rejected at the OUTER cond's line; the same
  outer shape with a plain-call inner else (`parse-top-defn`,
  `parse-top-defp`, `parse-impl`) compiles fine. Workaround (applied):
  extract the inner cond into a helper (`parse-ctor-body`,
  `parse-top-deftype-body`) so the outer clause is a plain call.

- **A direct `(.field v)` call parses as a `Call` with a `Symbol`
  operator named `".field"` — NOT a `FieldGetter` operator** (2026-09-01): `parse-call` reads `.field` via `read-full-symbol` and
  `dispatch-special` falls through to the plain-Call branch.
  `FieldGetter` operators arise ONLY from `->` threading steps (the
  `Symbol .f` thread-step branch). Phase-2 eval must therefore resolve a
  leading-dot Symbol operator as a field getter.

- **Driver: `interpreter/rdr-deftype.toc`** (2026-09-01):
  parses the 5 hvm-core.toc deftypes (Maybe, Leaf, String,
  GetSentinelVal, List) verbatim through `parse-program` and checks
  each map by ctor/field/impl fingerprint (`ctor-fingerprint` /
  `impl-fingerprint` — tag protocols + guarded per-ctor extraction,
  same pattern as `rdr-defp.toc`); 5 OK lines, zero leaks, 0 remaining
  nodes. `rdr-deftype` Makefile target (same pattern as `rdr-top`).
  Fingerprint gotcha: `(str* (str-vect v))` already prints the vector
  WITH its `[`/`]` — the first driver draft added extra brackets and
  printed `[[x]]`.

- **new-toc non-deterministic module symbol loss (2026-09-01)**:
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
- **Driver-probe gotchas (2026-09-01)**: (a) `pr*` on a VECTOR
  of strings aborts silently (SIGABRT, no output) — `pr*` takes one
  string; use `(str* [...])` to build it. (b) `str-vect` is NOT
  implemented on the `TopLevel` ctors (only the `Expression` ctors) —
  drivers fingerprint TopLevel values with the `top-kind` tag + per-ctor
  extraction, never `str-vect`. (c) scratch binaries must be built with
  `-DCHECK_MEM_LEAK=1` (the Makefile targets do) — without it, lazy
  evaluation hits `BOOM("Make this threadsafe")` in `eraseLazy` (new.c).
- **`symbol-start?` was missing `*` (2026-09-01)**: the parser's
  `symbol-start?` (interpreter/intrp-rdr.toc) handled `. _ < > = + - /` but
  NOT `*`, so `(defn * [x y] ...)` (hvm-core.toc:330) failed with
  `expected a name after defn` — and any `*` in expression position was
  silently dropped (the `parse-expr` unknown-char fallback). `symbol-continue?`
  already included `*` (and `?`/`!`); only the start set was incomplete.
  Added `(str-prefix? "*" s)` to `symbol-start?`. hvm-core.toc's operator
  names are `+ - * < <= =`; only `*` was missing. With the fix, `parse-program`
  reads hvm-core.toc with zero parse errors.
- **malformed-cond: a `let` in a NON-ELSE clause is rejected (2026-09-01)**: the `malformed 'cond' expression` trigger is broader than the
  documented "let containing a nested cond" — a `let` in a NON-ELSE clause
  (even with NO nested cond) is rejected. E.g. `(cond (= k 2) (let [...] (print
  ...)) <else>)` fails at the let's line. Fix: extract the let into a helper
  `defn` so the clause is a plain call (this is why the parse-error handling in
  the drivers lives in a `defn`, e.g. `parse-error-line`, not inline in a cond
  clause). A `let` in the ELSE clause is fine.
- **A `reduce` closure capturing a FREE VARIABLE leaks (2026-09-01)**:
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
- **Driver: `interpreter/rdr-hvmcore.toc` (2026-09-01)**: the reader
  acceptance driver. `slurp`s hvm-core.toc (inline-C whole-file read, same as
  `rdr-top.toc`), runs `parse-program`, and checks: the result is a
  ParserMatch; the `[TopLevel]` vector has 194 nodes; the per-form counts are
  108 defn / 5 def / 58 defp / 5 deftype / 16 extend-type / 2 top-level inline
  / 0 other; and a spot-check of one defp (`type-name [x]`), one deftype
  (`Maybe`: `None` + `Some [x]`), one extend-type (`None`: 8 methods), and one
  top-level inline (first node, c-code prefix `\n#define _XOPEN_SOURCE 600`).
  The defn/def split is by the `Definition`'s value shape (Fn → defn, Inline →
  def — all 5 hvm-core `def`s bind an inline). `node-class` tags each TopLevel
  (tag protocols `top-kind`/`expr-kind` + per-ctor extraction). All checks OK, zero leaks, 0 remaining nodes. The `rdr-hvmcore`
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
- **Item-7a static verification of the three edits (2026-09-02)** —
  verified against the sources; the `make new-toc` rebuild +
  `scratch/func-dispatch.toc` run are the owner's step (item 7a
  protocol): (1) `c/ReifiedConstraint` field order is `[type-number
  type-symbol field-consts fields path sym]` (constraints.toc:570,
  pinned sha 2f1dce1); its asserts accept `{}`/`empty-list`/
  `c/no-symbol`; `(c/type-num <ReifiedConstraint>)` returns
  `.type-number` (the `extend-type ReifiedConstraint Constraint` impl).
  (2) The extend-ast pre-check's singleton test passes a fieldless
  ReifiedConstraint: `(get type-c .static/.min/.max)` all miss → the
  defaults (`c/no-int`, `int-min`, `int-max`) make every clause true
  (typer.toc:1096–1116). (3) The typer records an extend-type impl as
  `(b/ProtoArity (c/type-num type-info) ...)` (typer.toc:1133–1148), so
  `extend-type Function` lands the impl under `.impls[c/FunctionType]`
  — exactly the key the new REF branch looks up. (4) User deftype type
  numbers start at `c/TypeCount` = 45 (runtime3.h:196; base.toc:33
  `(def type-counter (int-generator c/TypeCount))`), so `FunctionType`
  (4) can never collide with a user type. (5) The OLD runtime header
  core.h:103 also defines `FunctionType` = 4 — the new-toc binary links
  `core.c` (includes core.h), so `c/FunctionType`'s inline C compiles in
  the compiler too; `FunctionType` already appears 1591× in the current
  new-toc.c.
- **Dispatcher C shape AFTER the item-7a edits (2026-09-02)** —
  supersedes the pre-edit shape fact above for rebuilt new-toc: per defp,
  `strictArgs([file line receiver])` → `swap(termLoc(args), (Term)dispVal)`
  (moves the receiver into the args slot for EVERY receiver kind) →
  `if (termTag(dispVal) == I60) { Integer impl → default impl → BOOM
  "for integers" } else if (F60) { default impl → BOOM "for floats" }
  else if (REF) { Function impl → default impl → BOOM "for Function" }
  else { switch(dispVal->type) { <case per impl, excluding 0 / IntegerType
  / FunctionType> default: { default impl → BOOM "for type %s" } } }`.
  "Default impl" = the defp body registered under `UnknownType`. The impl
  receives the receiver as its first arg; the trailing dispatcher args
  (line, file) are ignored by the impl (same as the pre-edit I60 path).
  Emitted by `emit-proto` + helpers `proto-impl-c` / `proto-boom-c` /
  `proto-branch-c` (codegen.toc, before `emit-proto`).
- **Old-compiler (toccata binary) source facts for compiler-source edits
  (2026-09-02, item 7a)**: (1) `reduce` is a protocol with signature
  `(reduce coll init f)` (core.toc:470 List, 1773 Vector) — the existing
  `emit-proto` threads `(reduce [] f)` via `->`. (2) `either` is a
  SPECIAL FORM in the old compiler (`ast/either-ast`, toccata.toc:5657):
  `(either x y)` = extract x if not nothing, else y; works inside
  closures. (3) `get-in` (core.toc:2250) is an ordinary fn — a variable
  may sit in the path vector: `(get-in ptype [.impls impl-key])`. (4)
  `.field` in argument position is a first-class field-getter tag;
  `(get x .field)` = has-field + extract (Associative get, toccata.toc
  6597) — this is how the typer's singleton test probes constraints for
  optional fields.
- **`scratch/str-prefix.toc` and `env-test.toc` are NOT in the repo
  (2026-09-02)**: `scratch/str-prefix.toc` (the STR_PREFIX verification
  program) was DELETED (not moved) in a5b3c7b "Move work to
  interpreter directory" — item 12 must recreate it (its content is in
  git: `git show 1367f7b:scratch/str-prefix.toc`). `env-test.toc` (the
  `type-num` REF repro) was ad hoc and never committed; its check
  (`(type-num pr*)` → 4) is folded into `scratch/func-dispatch.toc`.
- **Item-7a scratch driver: `scratch/func-dispatch.toc` (2026-09-02)**:
  `defp disp [x]` with `!returns Integer` + body `(99)` + `(extend-type
  Function (disp [f] (42)))`; `main` checks, in order: `(type-num pr*)`
  → 4, `(disp pr*)` (REF → Function impl) → 42, `(disp 1)` (I60 →
  default) → 99, `(disp 1.5)` (F60 → default) → 99, `(disp (Some 1))`
  (VAL of unlisted type → default) → 99. Side effects threaded through
  the continuation chain with `+` (lazy-machine crutch, the
  str-prefix.toc pattern). Build recipe is in the file header (new-toc +
  awk `#line` + clang with `-DCHECK_MEM_LEAK=1 -DSAFETY=1 -DSTATS=1`,
  link `new.c runtime3.c graph.c`). **Status (2026-09-02)**: the
  `extend-type Function` blocker is GONE — the owner rebuilt new-toc
  (dc1eaae) and fixed the constraints git-dependency (Function in
  `core-type-constraints`), so a minimal `extend-type Function` probe
  compiles clean under the current binary (`scratch/ext-fn-probe.toc`
  — exit 0, no error lines). The file itself still fails — in the
  TYPER, on its own `(99)` / `(42)` self-expr integer-literal bodies
  (`Conflicting assertions (571)` — the gap-4 latent typer bug); with
  those rewritten to bare literals it is the item-7a verification
  program again.
- **Runtime field access is generic and index-based (2026-09-02,
  gap 2)**: `accessFieldFn` (runtime3.c:3085) reads
  `value->impls[fldIdx]` given a field index — the runtime has no
  name→index table; compiled code gets the index from the typer. The
  concrete interpreter resolves (type, field-name) → index with a
  hand-rolled literal table keyed by runtime type id (`SomeType 43`,
  `NoneType 42`, `SubStringType 44` — runtime3.h:193–195), and the
  read is inline C that reuses the `accessField` REF with the exact
  generated-C pattern (codegen.toc:336–340). The receiver's type id
  comes from `type-num` (total after item 7a: I60→1, F60→19, REF→4,
  VAL→type id).
- **A call whose target is an Integer or String is invalid; new-toc's
  typer misreports it (2026-09-02, gap 4)**: Integer and String values
  can never be called as functions — `(99)` is a zero-argument call
  with the integer 99 as the target, totally invalid. In source the
  only way to write an Integer/String target is a literal in operator
  position. new-toc's typer produces no "not callable" error for it —
  it aborts with `Conflicting assertions (571)` (the call-site bottom,
  typer.toc:571). Verified with `(defn f [x] (98))` /
  `(defn f [x] (99))` (repros: `scratch/7a-probe-2cd5e2.toc`,
  `7a-probe-e7a389.toc`, `7a-probe1–3.toc`); symbol-target call bodies
  (`(str "hi")`, `(vector 1 2)`) and bare literal bodies are fine.
  Avoidance rule for new-toc-compiled source (items 7–8): never put
  an Integer or String in operator position. The concrete interpreter
  implements the semantics cleanly — `interpret`'s default body
  aborts `file:line: not callable`. The typer fix (a proper
  "not callable" type error) is owner territory (new-toc rebuild).
- **proto-branch-c dispatch bug in the item-7a codegen edit — found,
  fixed, owner rebuild pending (2026-09-03, item 7)**: the generated
  defp dispatcher's I60 / REF branches pushed the default body even
  when a specific impl was registered — harness check 4 (`interpret`
  a core REF) aborted `file:line: not callable` instead of calling the
  Function impl. Diagnosis: the generated C's symbol table names the
  ProtoArity globals `"<fn> <type-num>"` — `"interpret 4"` (the
  Function impl, recorded under type 4 = FunctionType) existed but was
  unreferenced by the dispatcher, while every branch pushed
  `"interpret 0"` (the body, type 0 = UnknownType). The typer recorded
  the impl correctly (`.impls[4]`); the bug was `proto-branch-c`
  (codegen.toc): its reduce was LAST-key-wins —
  `(reduce impl-keys boom (fn [fb key] (either (impl-c key) fb)))` —
  so the last key (UnknownType) overwrote the earlier specific impl.
  Fix (applied): reduce over `(reverse impl-keys)` — the specific keys
  are visited last and overwrite the default / BOOM fallback, giving
  the settled first-found-wins branch order. Verified end-to-end with a
  THROWAWAY compiler `scratch/new-toc-fb` (built from the existing
  `./toccata` binary — not rebuilt — via the Makefile new-toc recipe:
  `./toccata compiler.toc` + the `maybe((FnArity`→`maybe((Vector` sed
  + the awk `#line` step + `clang -march=native -I. -lm
  -DWAIT_FOR_LINGERING=1 -std=c99 core.c <gen>.c -lpthread -latomic`):
  the REF branch now pushes the Function impl; harness checks 1–5 pass,
  zero leaks. Blast radius: the REF-branch bug is observable (the
  interpreter's `interpret` is the first defp with a Function impl +
  body); the I60-branch bug is currently MASKED — the only core defp
  with an Integer impl + body is `type-name`, and its body
  (`default-type-name`) special-cases I60 → `"Integer"`, the same
  result as the Integer impl (probe `scratch/i7-typename-probe.toc`
  prints `Integer` under both binaries). The official new-toc (the
  2026-09-02 19:37 build) does NOT contain the fix — the owner must
  run `make new-toc` before item 7's harness passes under the official
  binary.
- **Item 7 as-built (2026-09-03)**: `interpreter/intrp-eval.toc`
  (library, no main): `Closure [name params body env]` / `Env
  [current-ns namespaces]` / `TopDef [name ast]` deftypes — no `!`
  annotations, `recurse` per the style doc, no map/flat-map (the style
  doc now says they need not be implemented); `initial-ns-map` = a
  compile-time literal hash of the audit's symbols → the core symbols
  as values (24 entries — the audit README's `Total: 25` is a
  miscount, its tables list 24 distinct symbols: 15 direct + 9
  desugar targets); `initial-env` = `(Env "" {"" initial-ns-map})`;
  `intrp-call [f ops]` inline C (args are `f_1` / `ops_2`; `arrayFor`
  is defined in runtime3.c but NOT declared in a header — the inline
  C extern-declares it; VAL elements are incRef'd for the APP chain,
  I60/F60/REF used directly; zero-operand call = clean abort — the
  item's open detail, decided clean-error); `env-bind` / `env-bind-all`
  (explicit recursion over the flat `[k1 v1 ...]` vector, no reduce —
  the free-variable-capture leak) / `env-lookup`; `resolve [v env]`
  (default body = identity); `interpret [f ops env loc]` (default body
  = `interpret-not-callable` — inline C in a plain defn, since a
  protocol impl may not contain an inline C body — aborting
  `file:line: not callable`; `extend-type Function` → `(intrp-call f
  ops)`). `test-loc [file line]` helper: the scratch harness cannot
  import `ast` directly without double-compiling intrp-ast.toc under a
  second raw path spelling, so it builds Locations through it. Harness:
  `intrp-tests/i7-harness.toc` (checks 1–4) + `intrp-tests/i7-not-callable.toc`
  (check 5, a separate program because it aborts). The harness imports
  via `(add-ns eval (module "../interpreter/intrp-eval.toc"))` — the
  `../` is SAFE here: intrp-eval.toc appears under exactly one spelling
  in the harness module graph (intrp-ast.toc loads exactly once); the
  `../` hazard is double-spelling the same file. Verified under
  scratch/new-toc-fb: checks 1–4 print `OK check-1 count=24 sp+=1
  sppr*=1` / `OK check-2 bound=42` / `OK check-3 ref-type=4` /
  `OK check-4 plus=3`, zero leaks, 0 remaining nodes, exit 0; check 5
  aborts `i7-not-callable.toc:5: not callable`, exit 134. The
  Call-loc delta (`Expression.Call [operator operands loc]` —
  intrp-ast.toc str-vect + recurse; `parse-call` captures `call-loc`
  at the opening paren and threads it through `dispatch-special`; all
  desugar helpers pass loc) is verified: all seven reader drivers pass
  (rdr-exprs clean; rdr-top 23, rdr-defp 4, rdr-deftype 5,
  rdr-extend-type 3, rdr-inline 5, rdr-hvmcore 5 OK; zero leaks) — the
  drivers fingerprint via tag protocols + field extraction, not Call's
  str-vect, so the loc delta does not change their output.
- **Item 7 verified under the OFFICIAL new-toc (2026-09-03)**: the owner
  rebuilt new-toc (binary timestamp 2026-09-03 07:32; the 2026-09-02 19:37
  build predated the `proto-branch-c` fix). The rebuilt binary contains the
  fix — `intrp-tests/i7-harness.toc` checks 1–4 pass (`OK check-1 count=24
  sp+=1 sppr*=1` / `OK check-2 bound=42` / `OK check-3 ref-type=4` /
  `OK check-4 plus=3`; zero leaks, 0 remaining nodes, exit 0) and
  `intrp-tests/i7-not-callable.toc` aborts `i7-not-callable.toc:5: not
  callable`, exit 134. The throwaway `scratch/new-toc-fb` is no longer
  needed for item 7 (left in `scratch/`). Two build-behavior notes:
  (a) a new-toc build of a program **with** `main` exits **0** on success
  (the "exit code is always 134" fact applies to library files without
  main); (b) blank lines in new-toc's stderr are normal — when checking
  for error lines, filter `^\*\*\* ` AND blank lines, or a blank line
  masquerades as an "error message" and stops a transient-crash retry
  loop early.
- **Runtime toolchain broken (2026-09-04, found during parser-gen
  item 2)**: the git-HEAD runtime sources (`new.c` / `runtime3.c` /
  `graph.c`) cannot run ANY program generated by the current `new-toc`
  binary (2026-09-03 07:32) — even `(main [_] 0)` crashes the SAFETY
  check `Error: LAZ pair requires positive term in port 2 / Port 2
  term tag: SUB at new.c:516` (segfault without `-DSAFETY=1`).
  Rebuilding the old generated C (`rdr-exprs.c`) that produced the
  working prebuilt binary ALSO crashes against the current runtime
  sources; the prebuilt drivers (e.g. `./rdr-exprs`, 2026-09-04
  08:45) still work but were linked against an older runtime that no
  longer exists on disk. Additionally: the `new-toc` binary is
  non-deterministic (6 runs of the same input → 6 different
  generated-C outputs, plus occasional segfaults), and `new-toc.c` is
  missing from disk (only `new-toc.c~`) — new-toc cannot be rebuilt
  from the Makefile. All driver builds/runs are blocked until a
  consistent runtime/new-toc pair is restored.
- **Bare `(def name)` is a forward declaration (2026-09-04, found
  during parser-gen item 2)**: a bare `(def name)` with no value
  declares the symbol (new-toc prints `*** declare <name> <global>`) and
  makes it usable before the defining `def`/`defn` appears later in
  the file. All other top-level ordering is strict use-after-
  definition: referencing a top-level symbol defined LATER in the file
  (from a `def` initializer, `defn` body, or `fn` body) fails
  `Undefined symbol: '<name>'` — 100% reproducible, not the symbol-loss
  race. This makes def↔defn reference cycles expressible (declare
  first, define in either order after).
- **Runtime toolchain restored (2026-09-04, parser-gen item 2a)**:
  the RUNTIME TOOLCHAIN BROKEN fact above is SUPERSEDED — a fresh
  `new-toc` build of a trivial program (`(main [_] 0)`) compiles and
  runs clean against the current `new.c` / `runtime3.c` / `graph.c`
  (exit 0, malloc diff 0, remaining nodes 0), as does a 2000-
  iteration driver; deterministic across reruns and a rebuild. The
  owner restored a consistent runtime/new-toc pair. Driver builds/runs
  are unblocked. The `new-toc.c`-missing / binary non-determinism
  caveats in that fact still stand.
- **`inline` C body convention (2026-09-04, parser-gen item 2a)**:
  the inline body must set `result = <Term>` — codegen emits the body
  inside a `void` fn and appends `move(portLoc(2, args), result);`
  itself; a `return` in the inline body is a clang error ("void
  function should not return a value"). The `(inline TypeExpr "...")`
  type annotation does NOT change the generated C fn signature.
  `inline` is not allowed in a `cond` CLAUSE ("'inline' expressions
  not allowed here" — defn-body position is fine). Fn args are
  visible in the C as `<arg>_1`, `<arg>_2`, ... (1-based).
- **`str-append` requires a pre-allocated StringBuffer dest
  (2026-09-04, parser-gen item 2a)**: `str-append` (hvm-core.toc:608)
  `strncat`s into the dest's buffer IN PLACE — the dest must have
  enough allocated capacity (the hand-written rdr pre-allocates its
  acc). Appending to a static string literal (e.g.
  `(str-append "" "x")`) overflows the global buffer — segfault
  (ASan: global-buffer-overflow in strncat).
- **defp default body runtime-verified (2026-09-04, parser-gen
  item 3)**: a `defp` WITH a body runs that body at runtime for a VAL
  receiver with no `extend-type` impl — a default body of
  `(abort <message>)` fired when the protocol was called on a value
  whose ctor had no impl (message printed, exit 134). Confirms the
  "only VAL receivers reach the default" note above.
- **`new-toc` exit code on a clean library load is not stable
  (2026-09-04, parser-gen item 3)**: the same clean load exited 134
  in one run and 0 in another — the `*** Loaded <file>` stderr line
  (with no other error lines) is the only criterion; the "exit code
  is always 134" note above is not universal.
- **`escape-chars` (2026-09-04, parser-gen item 3)**:
  `escape-chars` (hvm-core.toc:652) escapes `\` `"` `\n` `\r` `\f`
  `\b` `\t` to two-char sequences — exactly a Toccata string-literal
  body; usable by any source-emitting tool to splice runtime strings
  into generated `.toc` source. `type-name` over a deftype ctor value
  returns the BARE ctor name (no namespace prefix).
- **Bare `(def name)` forward decl fixes the symbol-loss race, and
  the runtime binding is verified (2026-09-09, parser-gen item 6)**:
  with `emit-module` as the last defn of `intrp-emit.toc`, the
  symbol-loss race became DETERMINISTIC (10/10 builds: `Undefined
  symbol: 'emit/emit-module'`). A bare `(def emit-module)` placed at
  the TOP of the file (separated from the defn) prints `*** declare
  emit-module` and makes the build deterministic. A bare def placed
  IMMEDIATELY before the same-named defn (no forms between) prints no
  declare line and does not register. Runtime binding verified with a
  probe: `(def f)` + later `(defn f [x] (+ x 1))` + `(f 41)` → 42
  (declared global and defn binding are the same global) — resolves
  the "runtime binding UNVERIFIED" caveat on the forward-decl fact
  above for same-file def + defn.
- **let-wrapping-cond miscompiles (2026-09-09, parser-gen item 6)**:
  a defn body shaped `(let [n ...] (cond t (let ...) (let ...)))` — a
  let wrapping a cond whose clauses are both lets — LOADS CLEAN under
  new-toc but miscompiles: the program aborts at runtime with `bad
  incRef value: (nil)` even when the function is NEVER CALLED (the
  crash fires at module load / registration). Fix: inline the
  let-bound value so the cond is the defn body directly (the inverse
  shape — let INSIDE cond clauses, as in `all_body-acc` — is safe).
  This is the runtime face of the documented "a let containing a
  nested cond whose clauses are both lets" load-time quirk.
- **first/rest iteration over a bare string walks it CHAR BY CHAR
  (2026-09-09, parser-gen item 6)**: a helper that iterates a
  collection arg via `count` / `first` / `rest` (e.g. an
  append-accumulator) given a bare STRING arg yields one-char-string
  elements (later rendered by `to-str` as space-joined characters).
  The collection arg must be a vector — wrap with `(conj [] s)`. Also
  verified: `(vect-conj [l] xs)` appends `xs` as ONE nested element;
  splice a recursive vector result with the append-accumulator
  instead (a nested vector in an emitted-lines vector renders under
  `to-str` with stray `[` brackets).
- **RUNTIME TOOLCHAIN BROKEN AGAIN (2026-09-09, parser-gen item 6)**:
  after a machine reboot, the `new-toc` binary (2026-09-05 20:40)
  crashes (exit 134, NO error message) on ALL non-trivial inputs —
  0/30+ on `interpreter/intrp-grammar.toc` (which loaded clean earlier
  the same session), also `intrp-rdr.toc` and the emitter; trivial
  programs (`(main [_] 0)`, 50 flat defns, a single simple deftype)
  still compile. Rebuilding is BLOCKED: `make -B new-toc` triggers a
  `toccata` rebuild whose link fails (`undefined reference to
  emptyBMI`). The owner must restore/rebuild the toolchain. Earlier
  the same session the same binary built the parser-gen drivers
  intermittently (retry loops of 5–12 attempts; the symbol-loss race
  and truncated-C link failures were the failure modes).

## Settled (continued)

- **Temporary files live in `scratch/`** (2026-09-03): all temporary files (probes, harnesses, throwaway drivers) are created in `scratch/`. A scratch file that proves useful long term is moved to `interpreter/` and committed from there — never committed from `scratch/`.
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
  `TypeConstraint` (skip), `Inline` (eval-time error — see the
  Inline-handling bullet above).
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
  initial env is a compile-time literal hash map of the audit's 25
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
- **Inline handling** (owner, 2026-09-02 — supersedes the earlier
  "structural loading rules"): top-level `inline` expressions are
  ignored (not bound — they never reach eval). Every other `Inline`
  node — a `def` value, a `defn` body, or nested anywhere in an
  interpreted expression — is an **eval-time error**: one
  `extend-type Inline` impl on `eval` aborts `file:line: inline C is
  not interpreted`. No load-time scan — the eval backstop alone
  enforces the rule (a never-used inline def never fires; lazy-
  consistent). The interpreter never reads the core namespace (the
  env is pre-seeded with the core REFs), so there is no core-ns /
  symbol-table exception.
- **Field getters — future design (owner, deferred; not phase 2)**:
  field getters (symbols starting with `.`) are protocol functions,
  automatically created at first appearance or when a deftype names a
  never-before-seen field. Where the proto dispatcher lives is
  undecided.

## Phase 2 implementation checklist (Ralph loop) — concrete interpreter

Items 7–13 are the former phase-1 interpreter work, moved here unchanged.
Protocol: work top to bottom, one item per session; check an item off
only when its "done when" holds, then commit. Context for every item:
this file (the settled design above) + AGENTS.md. The interpreter
sources may use inline C freely. Never touch the `toccata` Makefile
target. The item-7 design (primitive representation +
top-level-def marker + discrimination) was settled with the owner
2026-09-02 (grilling) — see Phase-2 design status. **Item 7a is a
prerequisite** (compiler changes, agent edits + owner build); item 7
depends on it. **Hazard (items 7–8): if new-toc aborts with
`Conflicting assertions (571)`, check for an Integer or String in
operator position first — a latent typer bug, see Verified facts.**

- [x] **7a. Compiler prerequisite: `Function` type + REF/F60 protocol
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
  - As-built (2026-09-02): the owner rebuilt new-toc (commit dc1eaae).
    `scratch/func-dispatch.toc` does NOT yet pass: it fails in the
    TYPER with `Conflicting assertions (571)` (the call-site bottom in
    typer.toc), which reproduces with a trivial user `defn` whose body
    is a bare Integer literal (e.g. `(defn f [x] (99))`) — no
    Function / extend-type involvement; string/vector literal bodies
    are fine. Owner ruling: a latent new-toc typer bug, not a 7a
    regression — 7a checked off (the bug is characterized in the
    gap-4 Verified-facts entry: a call whose target is an Integer or
    String is invalid; the typer aborts instead of producing a
    "not callable" error). Also noted: the dc1eaae commit
    message claims the `base.toc` Function-registration edit, but the
    diff does not touch `base.toc`. **Resolved (2026-09-02, owner)**:
    the constraints git-dependency was fixed so that the `base.toc`
    Function-registration edit is no longer needed — `extend-type
    Function` resolves via the dependency. Probe files left in
    `scratch/7a-probe*`.

- [x] **7. `interpreter/intrp-eval.toc`: interpreter — data + environment**
  Design settled 2026-09-02 (grilling) — see Phase-2 design status.
  - `(deftype Closure [name params body env])`,
    `(deftype Env [current-ns namespaces])`,
    `(deftype TopDef [name ast])` — no `!` annotations; map/flat-map/
    recurse impls per the style doc (`TopDef.recurse` over `ast` only).
  - Initial env: a compile-time literal hash map of the audit's 25
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
  - As-built (2026-09-03): `interpreter/intrp-eval.toc` + the
    Call-loc delta are written and verified (see the Verified-facts
    entry): the harness checks 1–5 pass, zero leaks — but under the
    THROWAWAY fixed compiler `scratch/new-toc-fb`, because verification
    exposed a dispatch bug in the item-7a codegen edit (`proto-branch-c`
    last-key-wins reduce — the default body shadowed the Function impl
    in the REF branch). The fix is applied to `codegen.toc` (reduce
    over the reversed keys). **PENDING the owner's `make new-toc`
    rebuild**; then rerun `intrp-tests/i7-harness.toc` +
    `intrp-tests/i7-not-callable.toc` under the official binary (recipes in
    their headers) and check the box.

- [x] **8. `interpreter/intrp-eval.toc`: interpreter — eval**
  - `(defp eval [expr env])` over `String`, `IntegerLit`, `FloatLit`,
    `StringLit`, `Call`, `Fn`, `FieldGetter`, `TypeConstraint` (skip),
    `Inline` (error — abort `file:line: inline C is not interpreted`).
    `eval-call`: `(interpret op ops env (.loc call))` — the dispatch
    does the rest. Errors: `file:line: message` + abort.
  - **Field access** (settled 2026-09-02, grilling): `eval-call`
    intercepts two shapes — a `FieldGetter` operator (from `->`
    threading) and a `Symbol` operator whose name starts with `.`
    (a direct `(.f v)` call) — and routes
    both to one field-read path: eval the single operand →
    `(type-num v)` → look up a literal `{type-id {field-name idx}}`
    table → inline C reusing the runtime's `accessField` REF with the
    generated-C pattern (`makePair(APP, 0, v, SUB)` →
    `makePair(APP, 0, newI60(idx), …)` → `pushRedex`). Table seed:
    `43 {"x" 0}` (`Some.x` — test13); grow on demand when a test hits
    a new field (same policy as the initial-env symbol list). Table
    miss (unknown type or field; non-VAL receivers map to 1/19/4 via
    `type-num` and have no entry) → clean `file:line: …` abort. The
    parser stays frozen (phase 1) — the interception lives in
    `eval-call`, not the reader.
  - Also adds the item-7-deferred impls: `extend-type TopDef` for
    `resolve` → `(eval (.ast v) env)`; `extend-type Closure` for
    `interpret` → arity check (operand count vs param count →
    `file:line: wrong number of args` at the `Call` loc),
    `env-bind-all` over the param/ops pairs + self-binding, then eval
    the body (skipping leading `TypeConstraint`s).
  - Done when: a program with defn recursion, fn/closures, let,
    cond/and/or/either, vectors, hash maps, threading, field access
    (`.x` over `Some`), string/int ops interprets with hand-verified
    output, and a def with an inline-C value aborts with the clean
    eval-time error.
  - As-built (2026-09-03): `eval` + the lazy flow-control handlers
    (cond/either/and/or) + the field-read path + the item-7-deferred
    `TopDef`/`Closure` impls are in `interpreter/intrp-eval.toc`.
    Verified by `intrp-tests/i8-harness.toc` (slurp → parse → bind →
    find-main → eval-main) run on `intrp-tests/i8-prog.toc` (12
    hand-verified lines — fact-5=120, adder=15, let=15, and=42,
    or=99, either=7, vec=1, map=1, map-miss=999, thread=1, field=8,
    str=hello; zero leaks, 0 remaining nodes, exit 0) and
    `intrp-tests/i8-inline.toc` (a def with an inline-C value aborts
    `i8-inline.toc:11: inline C is not interpreted`, exit 134).

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
  - Runner (script or Makefile target): for each suitable test listed in the audit
    (`intrp-tests/README.md`), `./intrp regression-tests/$t.toc party-pooper | sort`,
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
  - `scratch/str-prefix.toc` (the STR_PREFIX verification program) is a proper
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
