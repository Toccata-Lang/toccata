# Superposition Reader (design)

Status: version (a) committed (7e697ae); version (b) in progress -
`fix` landed, first grammar parser (`symbol`) verified. Replaces
the recursive-descent reader, preserved at
`interpreter/intrp-rdr-old.toc`.

## Current state - session handoff (2026-10-23)

Committed so far: 7e697ae (version a), 2d33d43 (this doc), dcb494e
(implementation-notes: nested-self-app segfault repro), 7bd0cd2
(`fix` in hvm-core.toc), 26c2538 (`symbol` parser + driver t6-t13).

Pending commit at handoff: `interpreter/intrp-rdr.toc` (owner's
Maybe-driven char-walk rewrite of `take-char`/`drop-char`/
`run-length`/`symbol`), `interpreter/rdr-sup.toc` (owner's
`parse-failed` global + bare-sequence `main`),
`docs/toccata-style.md` (and/or desugaring entry + String
Maybe-walk entry).

Done since version (a):

- `fix` is in `hvm-core.toc` (above `fold`). The pure-Toccata
  Y-combinator is blocked by the nested-self-app compiler segfault
  (implementation-notes section 16 has the repro + bisect table).
- First grammar parser: `symbol` in `intrp-rdr.toc`, with the char
  predicates (`is-alpha`, `is-digit`, `symbol-start?`,
  `symbol-continue?`) and `run-length`. Verified by driver checks
  t6-t13.
- Owner's char-walk idiom (now in the style sheet, Core API):
  Maybe-driven - `first`/`rest`, `None` IS end-of-string, the
  binders (`flat-map`, `and`) propagate it; no `str= c ""` guards,
  no `(subs s 1 (count s))` steps; "no char" and "char didn't
  match" are one default branch.
- Driver: 17 checks. `parse-failed` is the named global seed
  (defined once). `main`'s body is a bare sequence - DELIBERATE
  (owner): the checks are parallel side-effecting workflows,
  execution order is not source order, pass = no check aborts.

Next, in order (version b):

1. Remaining leaf parsers in `intrp-rdr.toc`: `string` (escape
   unescaping - see `parse-string-content` in `intrp-rdr-old.toc`),
   `integer`, `full-symbol`. Note for `full-symbol`: `/` is a
   symbol-START char but NOT a continue char, so the `symbol` run
   stops at it; the full-symbol layer reads the second side and
   builds the `Symbol [ns name loc]` AST node (intrp-ast.toc).
2. The recursive core under ONE `fix` over the grammar family
   (a bundle of parser fns carrying their own reference; `many` is
   a member, not special): `call`, `fn`, `let`, `hash`, `vector`,
   `superposition` - building `intrp-ast.toc` nodes as values.
3. Restore the old drivers (`rdr-exprs.toc`, `rdr-defp.toc`,
   `rdr-top.toc`, ...) against the new reader - they are the
   verification corpus; broken until this lands (they import the
   old reader's API).

Working notes for the new session:

- `.toc` edits go through `tools/toc-edit/toc_edit.py` (check first,
  parse the snippet, insert/replace, check after). The edit tool
  mangles long newText (word-per-line) - use it only for small
  changes and verify with git diff; byte-exact Python splices are
  the reliable repair path.
- new-toc diagnostics: capture stderr to a file, never into a shell
  variable with 2>/dev/null; a library load exits 134 normally -
  the pass signal is the `*** Loaded <file>` line in actually-
  captured stderr.
- Build (see the Makefile `rdr-sup` target): `./new-toc <file> >
  out.tmp`, the awk `#line` pass, clang with `new.c runtime3.c
  graph.c` and `-DCHECK_MEM_LEAK=1 -DSAFETY=1 -DSTATS=1`.

## Goal

A reader for Toccata in which **parse alternatives are HVM
superpositions**. The reader is a monad whose bind is the machine
itself: `flat-map` over a superposition duplicates the call per leaf,
so alternatives are explored in parallel by the runtime instead of by
backtracking control flow. The reader builds the `intrp-ast.toc` AST
as it parses (the match's `value` field carries the AST node), reusing
the existing AST, emitter, and verification corpus.

## The monad is the machine

There are no named combinators (`p-unit`, `p-bind`, `p-alt`). The
monad operations are the bare language constructs:

- **unit** = a bare `(ParserMatch v s)`
- **bind** = the `flat-map` protocol (the machine duplicates the call
  per sup leaf)
- **alt** = the `|` literal
- **fail** = `(|)` — bare NUL

A parser is `fn [state] -> sup of ParserResults`.

## Result leaves

`ParserResults` has exactly two ctors:

- `ParserMatch [value state]` — `flat-map` impl:
  `((f (.value v)) (.state v))` — applies the continuation (a
  `value -> parser` function) to the value, threads the state.
- `ParserError [msg state]` — `flat-map` impl: `r` — short-circuit;
  the continuation never runs on this branch.

No `Alt` deftype (alternatives ARE the superposition), no
`ParserIgnore` (owner ruling). No `!` field annotations on the
multi-field ctors (new-toc mis-applies them).

## Failure semantics

A failing parser returns `(|)` (bare NUL). NUL propagates for free
through `flat-map` and `|` up to the top-level `collapse`, which
returns the seed. `ParserError` is produced **only when there is a
real error to report** (owner ruling, Q17-A). This matters because a
totally-NUL superposition applied downstream erases the workflow
(verified fact, docs/new-compiler-plan.md) — so the NUL must die at
the single top-level collapse, never flow into a continuation.

## Top level: one collapse

Exactly one `collapse` in the whole parser, at the top, with the
`pick-result` protocol as the fold and a line-0 `ParserError` seed:

- `ParserMatch` impl: `m` — a match **truncates the fold**.
- `ParserError` impl: merge, keeping the error with the **larger
  line**; a tie keeps the **left leaf**.

No leaf counting, no ambiguity checks.

**Fold order (empirically verified by probe):** the collapse fold
runs the **right leaf first** (against the seed), then the left leaf
against the accumulator; `pick-result [r acc]` receives `r` = leaf,
`acc` = accumulator-so-far. Hence the tie-break lands on the left
leaf.

## ParserState

`ParserState [input values]` (values: `{'file file, 'line line}`),
with `make-state`, `state-line`, and two consumers:

- `take-char` — consume one char, bump the line counter on `"\n"`.
- `drop-char` — consume one char without the line-count lookup.

`skip-comment` and `skip-whitespace` use `drop-char` for ordinary
whitespace and `take-char` only for `"\n"`.

## Recursion: the `fix` combinator

No explicit recursion in `defn` (style rule) — with one sanctioned
exception: `fix` itself, a named self-recursive defn in
`hvm-core.toc` (right above `fold`, which is self-recursive too):

```toc
(defn fix [f]
  (f (fix f)))
```

The self-reference goes through the defn's global REF, not a
source-level `(a a)` self-application, so it sidesteps the
nested-self-app compiler segfault (implementation-notes §16). The
pure-Toccata Y-combinator was tried first and is blocked by that
bug. Verified: compiles, runs a 3-deep recursive count-down, zero
node/malloc leaks (probe deleted after verification).

Usage: the continuation must return a function that *uses* `self`
— `(fix (fn [self] (fn [n] ... self ...)))`, not
`(fix (fn [self] some-fn))` (the fixed point of the latter is just
`some-fn`, shifting the args).

`fix` is needed for the **expression family as a whole**, not just
`many` — the grammar is directly self-recursive through single
(non-repeating) positions: `fn` body, `let` body and binding values,
call head, vector/hash elements, superposition alternatives. Plan:
one `fix` over the whole grammar family (a bundle of parser fns
carrying their own reference); `many` falls out as just another
member.

## Version (a) — done

- `interpreter/intrp-rdr.toc` (library): `ParserState` + helpers,
  `ParserMatch`/`ParserError` + `flat-map` impls, `pick-result`.
- `interpreter/rdr-sup.toc` (driver): 9 checks through the single
  top-level collapse - now 17 (t6-t13 are the version-(b) symbol
  cases) — unit pass-through; bind threads state; alt
  unions / both-fail to seed; error short-circuits the flat-map
  chain; pick-result (match truncates, deeper error wins, tie keeps
  left, empty sup to seed). All pass, zero leaks (`make rdr-sup`).
- Trivial hand-rolled parsers (raw `str-prefix?` + consume) — the
  real grammar is version (b).

## Version (b) — pending

1. ~~`fix` in `hvm-core.toc`~~ - DONE (7bd0cd2).
2. Port the expression grammar from `intrp-rdr-old.toc` into
   superposition parsers building `intrp-ast.toc` nodes - IN
   PROGRESS: `symbol` done (26c2538); remaining leaf parsers
   (`string`, `integer`, `full-symbol`), then the recursive core
   (`call`, `fn`, `let`, `hash`, `vector`, `superposition`) under
   the single `fix`.
3. Verification driver against the old corpus — the old drivers
   (`rdr-exprs.toc` etc.) are the test suite; they are broken until
   this lands (they import the old reader's API).

## Verified facts about the machinery

Learned while building version (a); check these before debugging the
reader:

- **`collapse` fold order**: right leaf first, then left against the
  accumulator (probe: `E:L acc:ACC`).
- **No `true`/`false`** — booleans are Maybe values; `and`/`or` are
  Maybe short-circuit protocols (`(and a b)` = `a` if `a` is None,
  else `b`).
- **`defp` default syntax**: `(defp name [params] "default")` — the
  `]` closes before the default.
- **Malformed-cond hazard**: a `cond` inside a `let` initializer is
  rejected (extract the predicate into its own `defn`); also a `let`
  in a non-else `cond` clause (docs/new-compiler-plan.md).
- **The flat-map continuation must return a parser** (a function):
  the `ParserMatch` impl applies the continuation's result to the
  state; a non-function result dies with `Invalid APP VAL pair`.
- **`'x` is quoted-string shorthand** — Toccata has no symbols;
  `'file` is `"file"`.
- **`(|)` is readable** (the stale 2026-08-29 fact is superseded).
