# intrp-tests — phase 1 interpreter test audit (checklist item 3)

This directory will hold the new positive pure-Toccata test suite for the
phase-1 interpreter (checklist item 11). This README records the item-3
audit: all 51 regression tests (`REG_TESTS` in the Makefile) audited
line-by-line against the phase-1 scope, the suitable list for
differential testing (items 9–10), and the union of core symbols they
reference (feeds item 7's initial env).

Scope reference: `docs/new-compiler-plan.md` — "Phase 1 scope (settled)"
and "Parser (settled so far)". In scope: `defn` (named, recursive), `fn`
(incl. named local fns), `let`, calls; literals (int, float, string,
symbol); vectors; hash maps; `and` `or` `either` `cond` `int-cond`;
`->` threading; `!` / `!returns` annotations; core protocols/deftypes.
Parser desugarings: `let`, `[...]` / `(vector ...)`, `{...}`, `->`,
`cond`, `and`/`or`, `str`/`println`, quoted syms. Parse errors: user
`deftype` / `defp` / `extend-type` / `add-ns` / `match` / `|` /
`defmacro`, top-level `inline`.

## Suitable for differential testing — 19 verified clean

Every form in each file is in the settled phase-1 scope (verified
line-by-line, 2026-08-27). All 19 are live `REG_TESTS` with `.rslt`
goldens.

| test | notes |
|---|---|
| test8 | def chains, `+`, `number-str`, `pr*` |
| test9 | def chains, `+`, `number-str`, `pr*` |
| test13 | `and`/`or`/`cond` over Maybe, `int-=`, `inc`, `abort`, `.x` field getter |
| test15 | `either`, `first`, `fn`, `let` |
| test16 | trivial `pr*` |
| test17 | named local fns (`fn boom2 []`), `and`/`or` over Maybe |
| test-global-empty-list | top-level `def` of `[]`, `println` of a quoted sym |
| cond-expr-1 | `cond` (4 pairs + else), `str` w/ quoted syms, `inc`, `=` |
| cond-expr-2 | same shape, first test false |
| cond-expr-3 | same shape, third test true |
| cond-expr-4 | same shape, fourth test true |
| tail-cond-1 | `defn` + `cond`/`str`/quoted syms |
| test-inlined-result-constraint | `! nums [Integer]`, `! n Integer`, `!returns Some`, `map`, `[1 2 3]` |
| test-recursive-map-fn | top-level recursion via name, `map` over `[]` |
| test-tail-recur-1 | recursive `defn`, `or`/`and`, `<`, `Some`, 30000 iterations |
| test-tail-recur-2 | recursive `defn`, `and`, `<`, `Some` |
| test-tail-recur-3 | recursive `defn`, `either`/`and`, `<`, `Some` |
| test-trailing-comment | comments around body, `str` |
| test-or-comment | `or`/`and` with comments, `get` on `{}`, `map`, `vector` form, single-arg `or` |

## Flagged — pending owner decision (2)

The rough audit (plan, 2026-08-26) listed these as clean; the
line-by-line audit found each uses one form the settled design does not
cover. Core-symbol references are a subset of the 19's union, so the
symbol list below is unaffected by the decision.

### test10 — uses `do`

`(main [_] (do (let ...) ...))` — a `do` sequence. `do` is a real old
reader form (`ast/do-expr`, reader.toc:1142) but has **no constructor in
the settled AST** and is **not in the settled desugaring list**. Natural
desugaring (nested `let` with fresh binding names, last expr as body —
note: `_` names are out, per the verified fact that underscore params
miscompile under new-toc) is a design decision the plan does not make.
Otherwise clean: `def` chains, `let`, `+`, `number-str`, `pr*`.

### test-fusing — bare `(def)` declare + arity-flexible calls

1. `(def rep)` — a bare `def` with no value. The old reader parses this
   as `ast/declare` (reader.toc:1421, a forward declaration). The settled
   AST has no `Declare` constructor and the settled top-level rule covers
   only `def name value` / `defn`. In the settled lazy-top-level world a
   declare is a semantic no-op (the later `(defn rep [xs] ...)` binds the
   name; everything is bound before `main` runs), so the parser could
   simply skip it — but that rule is not settled.
2. Arity-flexible (left-associative) application: `rep-O` calls
   `(rep p (fn [k] (f (f k))) x)` — 3 operands against `rep`'s single
   param — and `insert`/`O`/`I`/`E` build values by partial application
   (e.g. `(O (O (O (O (O (O E))))))` against 4-param `O`). The old
   compiler emits a multi-operand call as one `pushRedex` over a
   left-associative `APP` chain (verified in `regression-tests/
   test-fusing.c:17547-17552`); the new runtime is raw HVM, so a lambda
   binds its params and the leftover chain applies to the result.
   `eval-call` (item 8) must decide operand-count > param-count
   semantics; the plan does not settle it.

## Excluded — 30 tests, one-line reasons

| test | reason |
|---|---|
| test1 | user inline C (main body is one inline) |
| test2 | user inline C (3 inline defns) |
| test3 | user inline C (main body) |
| test4 | user inline C (def value + main body) |
| test5 | user inline C (main body) |
| test6 | user inline C (2 inline defns) |
| test7 | user inline C (4 inline defns) |
| test11 | superposition `\|` |
| test12 | user deftype (`Tree`) |
| test14 | user deftype (`SeedList`) + superposition `\|` |
| test18 | user inline C (`pr-num`) |
| comment-in-deftype | user deftype (`Commentary`) |
| function-regressions | `add-ns` (regression-tester.toc) |
| test-ignore-inferred | user deftype + `defp` + `extend-type` |
| cond-expr-5 | user inline C (`new-nothing`) |
| free-static-value | user deftype (`Bogus`) |
| test-either | user deftype (`AType`) |
| test-comment-in-let | `defp` (`boomaliscious`) |
| test-underscore-inline | user deftype (`Boomer`) |
| test-uni | user inline C + `defp` + user deftype (`Reified`) |
| test-bmi | `add-ns` + user deftypes (`Key1`/`Key2`/`Val`/`CHash`) |
| test-array-node | `add-ns` + user deftypes |
| test-collision-node | `add-ns` + user deftypes (`Key1`/`Val`/`CHash`) |
| test-threading | `add-ns` (regression-tester.toc) |
| test-closures | `add-ns` (regression-tester.toc) |
| test-inline-namespaced-sym | `add-ns` (Boomer.toc) |
| integer-regressions | `add-ns` (regression-tester.toc) |
| string-regressions | `add-ns` (regression-tester.toc) |
| vector-regressions | `add-ns` (regression-tester.toc) |
| hash-map-regressions | `add-ns` + user deftype (`ControlledHash`) |

The 6 `add-ns`-only tests (integer/string/vector-regressions,
test-threading, test-closures, test-inline-namespaced-sym) are otherwise
in scope — verified: no inline/deftype/defp/extend-type/match/`\|` in
any of them. They come in phase 2 with `add-ns`; they will also need
parser support for the `_FILE_` / `_LINE_` magic symbols (old reader
desugars them to file/line literals — reader.toc:912-929) and will
reference core symbols beyond the phase-1 union below (per the plan, the
initial env grows when a test hits an unbound symbol).

## Core-symbol union (feeds item 7's initial env)

Union of core symbols the 21 suitable/flagged candidates reference,
split into direct references and parser-desugaring targets. All verified
present in `hvm-core.toc` (the core new-toc builds against —
`base.toc:30`).

Direct references:

| symbol | hvm-core.toc | kind | referenced by |
|---|---|---|---|
| `+` | 303 | defn | test8, test9, (test10), test-tail-recur-1/2/3 |
| `<` | 105 | defp | test-tail-recur-1/2/3 |
| `=` | 101 | defp | test13, cond-expr-1..4, tail-cond-1 |
| `None` | 97 | Maybe ctor | test13, test-trailing-comment, test-or-comment |
| `Some` | 98 | Maybe ctor | test13, test17, test-inlined-result-constraint, test-tail-recur-1/2/3, test-or-comment |
| `abort` | 254 | defn | test13, (test-fusing) |
| `dec` | 327 | defn | test-tail-recur-1/2/3 |
| `first` | 152 | defp | test15 |
| `get` | 212 | defp | test-or-comment |
| `inc` | 313 | defn | test13, cond-expr-1..4, tail-cond-1, test-or-comment |
| `int-=` | 495 | defn | test13 |
| `map` | 122 | defp | test-inlined-result-constraint, test-recursive-map-fn, test-or-comment |
| `number-str` | 507 | defn | test8, test9, (test10), test13, test15, test17 |
| `pr*` | 269 | defn | test8, test9, (test10), test13, test15, test16, test17, (test-fusing) |
| `either` | 113 | defp | test15, test-tail-recur-3 |

Parser-desugaring targets (introduced by the settled desugarings):

| symbol | hvm-core.toc | kind | introduced by |
|---|---|---|---|
| `and` | 111 | defp | `and` right-nesting (test13, test-tail-recur-1/2) |
| `or` | 112 | defp | `or` right-nesting (test13, test-tail-recur-1) |
| `cond` | 114 | defp | `cond` right-nesting (test13, cond-expr-1..4, tail-cond-1) |
| `empty-vector` | 438 | def | `[...]` / `(vector ...)` / `str` / `println` folds (test-global-empty-list, test-inlined-result-constraint, test-or-comment, all `str`/`println` uses) |
| `vect-conj` | 440 | defn | same folds |
| `emptyBMI` | 203 | def | `{...}` fold (test-or-comment) |
| `assoc` | 206 | defp | `{...}` fold (rule requirement; no candidate has a non-empty map literal) |
| `str*` | 2030 | defn | `str` desugaring (cond-expr-1..4, tail-cond-1, test-trailing-comment, test-or-comment) |
| `println*` | 462 | defn | `println` desugaring (test-global-empty-list, cond-expr-1..4, tail-cond-1, test-inlined-result-constraint, test-recursive-map-fn, test-tail-recur-1/2/3) |

Total: 25 symbols. `int-cond` (hvm-core.toc:521) is in phase-1 scope but
referenced by no candidate; add it when a test needs it.

Notes:

- `str`, `println`, `vector` are **not** core symbols — the new core has
  only `str*`/`println*` (verified: no plain `str`/`println`/`vector`
  def in hvm-core.toc). They are parser-level forms per the settled
  desugarings (`(vector a b c)` ≡ `[a b c]`; verified the old compiler
  emits `(vector ...)` as a `vect-conj` fold over `empty-vector` —
  `regression-tests/test-or-comment.c:17544-17572`).
- `and`/`or`/`either`/`cond` are protocols implemented only for `None`
  / `Some` (hvm-core.toc:444-496) — they operate on Maybe values, which
  matches every candidate's usage.
- `FieldGetter` (`.x` in test13) needs no core symbol — field access is
  a value operation (item 8).

## main's argument vector

No suitable candidate reads main's argument vector. The compiled runtime
passes the **full argv** (program name + args) as main's single
parameter (runtime3.c:3327-3338: `argVect` built from `argv[0..argc]`, then
`pushRedex(callArgs, mainFn)`); the item-9 interpreter will pass a
different vector (`./intrp <file> <args>`), so any future candidate that
reads argv must be flagged here. test-fusing binds `args` but never reads
it.
