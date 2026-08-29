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
   AST from `interpreter/intrp-ast.toc`; parser = direct
   recursive-descent functions in `interpreter/intrp-rdr.toc` (the
   file's original combinator grammar was the reference; it has been
   replaced).
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
- **Note (2026-08-29): attempt implementing the reader with the core's
  `unfold` recursion scheme** — generating an AST value from a string
  (recursion schemes are a key Toccata feature; see the style doc's Data
  section).
- Dropped: `ParserCombinator` deftype as engine, `defp parse` evaluator
  protocol, `Recur` + rule registry, `Apply` combinator, `location`
  combinator, str-vect-based EBNF.

## Verified facts (2026-08-26)

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

## Settled (continued)

- **Interpreter work lives in `interpreter/`** (2026-08-28): all
  interpreter sources — `intrp-ast.toc`, `intrp-rdr.toc` (moved),
  future `intrp-eval.toc` / `intrp.toc`, plus the new-toc debug probe
  `xns-probe.toc` — live in the `interpreter/` subdirectory. `add-ns`
  module references use bare filenames (resolved relative to the
  importing file's directory); co-location also keeps the module
  cache's raw-path-string keys identical across the graph (see the
  `../` verified fact).
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
- **Closure deftype** (`interpreter/intrp-eval.toc`):
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
- **Whole-file read: inline-C function named `slurp`** (in
  `interpreter/intrp.toc`).
- **Error reporting (phase 1)**: `file:line: message` to stderr + abort
  (non-zero exit), uniform across parse/structural/runtime errors; no
  Maybe-threading through eval; no backtraces. (Phase 2's typechecker
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
  - Line-by-line audit of the 51 regression tests against the phase-1
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

- [ ] **6b. `interpreter/intrp-rdr.toc`: style rewrite (docs/toccata-style.md)**
  - Rewrite the parser to the settled style: every positionally-meaningful
    grouping becomes a deftype ctor with named fields, accessed via `.field`
    getters — no positional vectors, no `elt0`/`elt1`/`elt2`. Implement
    `map`/`flat-map` (and `recurse` where structurally recursive) on the
    grouping types per the style doc's Data section.
  - The rewrite attempts the reader built on the core's `unfold` recursion
    scheme (an AST value generated from a string), with parser alternatives
    as superpositions — `(| ...)` with `( | )` as the failing branch (Parser
    section, 2026-08-29). **If the unfold/superposition approach becomes too
    complicated, STOP and report to the owner** (the owner will help sort it
    out) — do not silently fall back to the old mechanism.
  - All settled behavior (parse results, desugarings, error messages, the
    name -> TopLevel map) stays identical. Update the drivers
    (`rdr-exprs.toc`, `rdr-top.toc`) where they touch the changed shapes —
    including `rdr-top.toc`'s `check-oos` positional `[source msg]` pairs.
  - Done when: the file compiles clean under new-toc (`*** Loaded`, no error
    lines); no positional vector grouping remains in `interpreter/*.toc`;
    both drivers pass with zero leaks (the 19-test map check unchanged).

- [ ] **7. `interpreter/intrp-eval.toc`: interpreter — data + environment — STOP
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

- [ ] **8. `interpreter/intrp-eval.toc`: interpreter — eval**
  - `(defp eval [expr env])` over `String`, `IntegerLit`, `FloatLit`,
    `StringLit`, `Call`, `Fn`, `FieldGetter`, `TypeConstraint` (skip).
    `eval-call`: closure → interpret (param bindings + self-binding);
    primitive → native call (the item-7 mechanism). Errors:
    `file:line: message` + abort.
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

Phases 2–3: to be specified when we get there.
