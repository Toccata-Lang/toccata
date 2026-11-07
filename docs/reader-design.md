# Toccata Reader — Design Doc

Living design doc for the hand-built Toccata reader. Written so the agent
can read it and pick up work at any time. Supersedes
`docs/parser-generator-plan.md` for the reader's purpose (that plan's
data-driven-grammar + generator approach is abandoned — see §2).

Repo root is the directory containing `interpreter/` and `docs/`. All
paths below are relative to it.

## Background docs (read for context)

The reader work sits on these reference docs; consult them before
building, not after hitting a wall.

- **`docs/toccata-style.md`** — the authoritative statement of the Toccata
  language: forms, naming, data, Core API, dispatch, evaluation, and
  superposition/`collapse` semantics. **Read before any `.toc` edit.**
  (Already cited in §3.3 and §7.)
- **`docs/implementation.md`** — the HVM runtime (`new.c`): the `Term`
  representation (location | label | tag), the 16 tags and their
  positive/negative polarity, memory (`nodeBuff`, free list), and the
  reduction engine (per-thread redex stack). Relevant because the reader
  runs on this runtime and superposition/`collapse` are runtime features.
- **`docs/implementation-notes.md`** — practical/observed notes. The parts
  that matter most for reader work: the evaluation & refcounting model
  (§12), debugging tools (§11 — the `prefs` probe, lldb, `graph.c` → dot,
  and that `pr*` output prints in reverse creation order), the C API
  (`Term` vs `Value*`, §14), REF values (§15), and the known new-toc bug
  (§16 — nested self-application segfaults, which blocks a pure-Toccata
  `fix`).

## 1. The goal

Build a **reader for Toccata** — it converts raw `.toc` source into an AST —
that is **as performant as possible**. It is built **directly in
`interpreter/intrp-rdr.toc`, piece by piece** (no code generation).

## 2. What was abandoned

The parser-generator plan (`docs/parser-generator-plan.md`) tried to express
the grammar as data (a `ParserCombinator` deftype) and emit a reader module
from it. That is **going by the wayside** — the owner has iterated on it
several ways and it is no longer accurate. The reader is now written by hand.

The generator-era artifacts still in the tree are **historical / not the
active path** (do not treat them as the current design):
- `interpreter/intrp-grammar.toc` (the grammar-as-data)
- `interpreter/intrp-emit.toc` (the emitter)
- `interpreter/gen-rdr.toc`, `interpreter/emit-pred.toc`,
  `interpreter/emit-want/*` (generated module + driver + want files)
- `interpreter/probe-*.toc`, `interpreter/emit-bisect*.toc` (probes)
- Makefile targets `gen-rdr`, `gen-corpus`, `emit-pred`

## 3. Settled design decisions

### 3.1 Superposition is the performant way to handle alternatives

This is the central decision. Alternatives are modeled as a **superposition**
(`(| a b c ...)`), and a **single top-level `collapse`** folds it.

- Superposition is *fast* at handling alternatives. It does **not** pay a
  per-branch workflow cost in any expensive sense.
- `collapse` walks the superposition tree **lazily, exactly like a
  recursive-descent parser**: it descends into the leftmost branch, and
  **branches to the right of the first complete match are never forced**
  (true short-circuit).
- Consequence: the superposition + single-`collapse` shape is the performant
  shape. Do **not** try to "optimize it away" into something else; build on
  it. (This corrects an earlier, wrong assumption that superposition
  duplicates workflow per branch.)

### 3.2 The monad model (as implemented in `intrp-rdr.toc`)

A parser is `fn [state] -> superposition of ParserResults`.

- **unit** = a bare `(ParserMatch v s)`
- **bind** = the `flat-map` protocol (threads the continuation over the
  value and the state)
- **alt** = the `|` literal
- **fail** = `(|)` — bare NUL; propagates through `flat-map` and `|` up to
  the top-level `collapse`, which returns its seed
- **One `collapse` at the top** folds the whole superposition with the
  `pick-result` protocol:
  - a `ParserMatch` **truncates** the fold (first complete match wins);
  - `ParserError` leaves **merge**, keeping the error with the **larger
    line** (a line tie keeps the **left** leaf — the collapse runs the right
    leaf first, then the left against the accumulator);
  - `ParserError` is produced **only when there is a real error to report**;
    a plain non-match is a NUL, not an error.

### 3.3 `for` is built-in compiler sugar

`for` is not a library combinator — the compiler expands it. It is syntax
sugar for **nested `flat-map` calls** (monad comprehension / do-notation):
each binding line is a `flat-map` over the bound value, threading the state;
the final expression is the result. `_` is a throwaway binding (value
discarded, state still threaded). A binding slot may hold any `flat-map`-
compatible value, including a `(| ...)` superposition. (Recorded in
`docs/toccata-style.md`, Forms.)

### 3.4 The output seam (TBD)

The **parsing structure is the stable part; what a parse *produces* is a
seam.** Today a successful parse yields a value carried in
`ParserMatch.value` (currently an `ast/` node or a string). Later, to reuse
the same parsing machinery for a different target (e.g. **generate EBNF**),
the owner will re-point what a parse produces — by changing the definition of
`Parser` or some related seam.

**This seam is not designed yet (TBD).** Do not bake a particular output-shape
assumption into the pieces built now. Keep the reader's output re-pointable.

### 3.5 Superposition belongs at the result level, never at the parser value level

Owner-confirmed runtime semantics: **a superposition that is DUPed (its
value used more than once) takes the *same branch* in every copy —
duplication is *correlated*, not independent.** This is intended HVM
semantics, not a bug.

Evidence (deterministic minimal repro):

- `((fn [s] (pr* (number-str (+ s s)))) (| 1 2))` → 2 leaves: `2` and `4`
  — the diagonal only; the cross terms `1+2=3` and `2+1=3` are missing.
- Decisive contrast: `(let [a (| 1 2) b (| 3 4)] (+ a b))` → 4 leaves
  `4 5 5 6` (two **distinct** sups, each used once → independent), while
  `(let [s (| 1 2)] (+ s s))` → 2 leaves `2 4` (one sup used twice →
  correlated).

Consequences for the reader:

- A **single `Parser` value is DUP-safe**: reusing it (passing it to two
  call sites, applying it at several positions) just refcounts the same
  value; each application of its `f` is independent.
- A **sup of Parser values is not DUP-safe**: `(| p q)` creates a sup of
  *values*. If that value is reused — e.g. a `many`-style loop applies
  `p` at every position — every use is forced onto the same branch, and
  the parse dead-ends at the first position where that branch doesn't
  match.
- **Result-level superposition is the safe level**: a sup of
  `ParserResults` distributes correctly through the `flat-map`/`collapse`
  machinery (verified directly: a 2-leaf sup of matches stepped through
  the reader's `next-step` produces all four combinations).

Design rule: a combinator that offers alternatives must return **one
`Parser`** whose `f` builds a fresh sup of *results* per application —
never a sup of parsers. Shape identified during the t47 investigation
(then reverted — not in the tree):

    (defn alt [pa pb]
      (Parser (fn [s]
                (| ((.f pa) s)
                   ((.f pb) s)))))

## 4. Current state of the reader (`interpreter/intrp-rdr.toc`)

Self-contained library (no `main`). Its only import is
`(add-ns ast (module "intrp-ast.toc"))`. It does **not** import
`intrp-state.toc` (that file is a generator-era leftover — see §5).

### 4.1 State machinery
- `deftype ParserState [input values]` — `input` = unconsumed source suffix
  (String); `values` = a hash map with `'file` and `'line`.
- `make-state [input file line]`, `state-line [state]` (reads `'line`,
  defaulting to 0 via `either`).
- `take-char [state]` — consume one char, bump the line counter on `"\n"`.
- `drop-char [state]` — advance one char without the line-bump bookkeeping
  (used by the comment/whitespace scanners).
- `skip-comment [state]` — state is just after `;`; skip to end of line
  (the newline itself is not consumed).
- `skip-whitespace [state]` — spaces, tabs, newlines, carriage returns, and
  comments; bumps the line counter on `"\n"`.

### 4.2 Results + pick
- `defp pick-result [r acc]` (the fold for the top-level collapse).
- `deftype ParserResults` with two ctors, each carrying `str-vect`,
  `flat-map`, `map`, and `pick-result` impls:
  - `ParserMatch [value state]` — `flat-map` applies the continuation;
    `pick-result` returns the match (truncates).
  - `ParserError [msg state]` — `flat-map short-circuits` (continuation
    never runs); `pick-result` merges, keeping the larger line.

### 4.3 Character predicates
- `is-alpha [c]`, `is-digit [c]` (via `char-code`).
- `symbol-start? [s]` — string **starts** with a symbol-start char
  (alpha, or the specials `.` `_` `<` `>` `=` `+` `-` `*` `/`).
- `symbol-continue? [c]` — start chars plus digits `?` `!`.
- `run-length [s pred]` — how many leading chars satisfy `pred`
  (self-recursive scan of data, not a parse — no `fix`).

### 4.4 The `Parser` type + helpers
- `deftype Parser [f]` where `f` is `state -> ParserResults`; carries
  `str-vect`, `map`, `flat-map`.
- `error [msg]` → a parser that always yields `ParserError`.
- `always [v]` → a parser that yields `ParserMatch v` without consuming.
- `optional [p]` → `( | p (always ""))` — **latent §3.5 violation** (a
  sup of parser values); dormant only because it is used once per call
  site today.

### 4.5 Leaf parsers built so far
- `comment` — match a `;` comment, return its raw text.
- `whitespace` — skip whitespace+comments, return `""`.
- `literal [text]` — match an exact string prefix; value = the literal;
  no match → `(|)`.
- `location` — `ast/Location` for the state's file and line.
- `symbol` — skip whitespace, consume a run of symbol-continue chars;
  value = the raw text.
- `full-symbol` — a `symbol` run then an optional `"/name"` side; value =
  `ast/Symbol [ns name loc]`. Built with a **`for`** comprehension. The
  ns/bare branch is a p-branch (not a p-alt): the bare form is a prefix of
  the ns form, and the match-truncating fold would let the shorter match
  win. A slash with no name side is a real `ParserError`.
- `text-to-int [s]` — the integer value of a string of digits (no sign),
  via `reduce`.
- `digit-run` — consume a run of digits; value = the raw text. No leading
  digit → `(|)`.
- `int-lit` — an optional `"-"`, then a `digit-run`; value =
  `ast/IntegerLit`. A `"-"` with no digits is a NUL (falls through to the
  symbol parser — `"-foo"` is a symbol, not a number). Built with a
  **`for`** comprehension.
- `string-end [state]` — data scan (self-recursive, like `skip-comment`),
  not a parse: from just past the opening quote, a `"` closes, a `\`
  skips the following char as an escaped pair (kept intact — `\"` does
  not close, `\\"` does), newline or EOF → `None`. Returns `Some`
  of the state just past the closing quote.
- `string-body [loc]` — parser wrapping `string-end`; value = the raw
  text (escapes kept intact); unterminated (newline or EOF before the
  closing quote) → `ParserError`.
- `string-lit` — whitespace, location, opening `"`, `string-body`; value
  = `ast/StringLit [text loc]`. Built with a **`for`** comprehension.

### 4.6 Not built yet (the remaining pieces)
For a complete reader, still to add (the expression parsers are next):
- expression parser: calls, `let`, `cond`, `fn`, `->` threading (the
  recursive core — everything else composes through it), `.field`
  getters, vectors, hash maps
- top-level form parsers: `def` / `defn` / `defp` / `deftype` /
  `extend-type` / `inline` / `main`
- the `parse-program` entry point (file → `[TopLevel]` vector) + a thin
  `main` CLI

## 5. Related files

- **`interpreter/intrp-ast.toc`** — the AST shapes the reader currently
  produces. `Location [file line]`; `Expression` (ctors incl. `no-symbol`,
  `Symbol [ns sym-name loc]`, `IntegerLit`, `FloatLit`, `StringLit`,
  `Call`, `Fn`, `FieldGetter`, `TypeConstraint`, `Superposition`, `Match`,
  `BlockComment`, `Inline`); `TopLevel` (`Definition`, `DefType`, `Defp`,
  `ExtendType`, `AddNs`, `Inline`, `BlockComment`, `Main`); a `body-elem`
  protocol for mixed body vectors. NOTE: its header comment still describes
  `intrp-rdr.toc` as "a single-pass recursive descent" — **stale** (the
  reader is now superposition-based).
- **`interpreter/intrp-state.toc`** — generator-era extracted helper kit
  (`ParserState`, a 3-ctor `ParserResults` incl. `ParserIgnore`,
  `parse-then`/`parse-or`, `Token`, `read-run`, `take-chars`). **Not used by
  the current hand-built reader** (which inlines its own versions). Leftover
  from the abandoned generator.
- **`interpreter/rdr-sup.toc`** — the **verification driver** for the reader.
  Imports `intrp-rdr.toc` as `rdr` and `intrp-ast.toc` as `ast`. Provides a
  result-inspection protocol kit (`res-kind`/`match-val`/`match-input`/
  `err-msg`/`err-input`), trivial hand-rolled parsers (`parse-a`/`parse-b`),
  check helpers, and the `run`/`run-full`/`parse-result` wrappers that apply
  the **single top-level `collapse rdr/pick-result`** with a `parse-failed`
  seed. Test cases t1–t19 cover the monad mechanics (unit/bind/alt/error/
  pick-result) and the `symbol`/`full-symbol` parsers.
- **Repo-root copies** `intrp-ast.toc`, `intrp-rdr.toc`, `intrp-ebnf.toc`
  (in the repo root, not `interpreter/`) are **older duplicates** — the
  canonical reader files live in `interpreter/`.

## 6. How to build / verify

- **Run the driver:** `make rdr-sup` — builds `interpreter/rdr-sup.toc`
  through `new-toc` + clang and runs it. The run passes iff **every check
  prints `OK`** (a mismatch prints a `FAIL` line). `main`'s body is a bare
  sequence of side-effecting checks; the machine runs every reachable side
  effect at least once, not in source order.
- **Load the library standalone:** `./new-toc interpreter/intrp-rdr.toc`
  should print `*** Loaded interpreter/intrp-rdr.toc` and no other error
  lines. For a library (no `main`), **exit 134 is the normal clean-load
  exit** (the missing-main abort path) — the pass/fail signal is the
  `*** Loaded ...` line in actually-captured stderr, never the exit code.
- Other per-form drivers from the earlier hand-written era also exist as
  Makefile targets (`rdr-exprs`, `rdr-defp`, `rdr-deftype`,
  `rdr-extend-type`, `rdr-inline`, `rdr-top`, `rdr-hvmcore`) — they build on
  `intrp-rdr.toc` + `intrp-ast.toc`.

## 7. Constraints (from AGENTS.md + docs/toccata-style.md)

- **Edit `.toc` files only with the structural editing tool**
  (`tools/toc-edit/toc_edit.py`, run from the repo root). Workflow:
  `check` (must pass, exit 0) → `line`/`show` to locate → validate any new
  snippet with `parse` → `insert`/`replace`/`delete` → `check` again.
  Exit 3 = rejected (original untouched, candidate saved as
  `<name>.rejected`, read `new-toc`'s stderr). Exit 4 = unverified (re-run).
- **Read `docs/toccata-style.md` before writing/editing any `.toc` file.**
- **No local symbol may shadow a core-namespace symbol** (codegen emits
  colliding C identifiers).
- **`+` is exactly 2-arg** — nest to combine more.
- **`get`/`first` on a collection return a Maybe** — never `extract`;
  branch via `(either (map <maybe> <fn>) <default>)`.
- **`map` is value-first**: `(map v (fn [x] ...))`.
- **No `[...]` / `{...}` literals** — use `(vector ...)` / `(hash-map ...)`.
- **`subs` is 3-arg**: `(subs s start len)`.
- **Pure ASCII** in `.toc` files (multi-byte UTF-8 corrupts the AST dumper's
  byte spans).
- **new-toc diagnostics:** always capture stderr (never `2>/dev/null` when a
  build fails); a silent segfault/abort with no message is transient — retry
  up to 5 times; if a retry prints an error, stop and fix it.
- **Do not** touch `toccata.c`/`core.c`; do not `make toccata`; no `sudo`.

## 8. Target scope (what the reader must parse)

Inherited from the (now-abandoned) plan's "scope (a)" — **to be confirmed
with the owner**, since a "reader for Toccata" may mean the full language:

- Top-level: `def` / `defn` / `defp` / `deftype` / `extend-type` /
  `inline` / `main`.
- Expressions: calls, `let`, `cond`, `fn`, `->` threading, `.field` getters,
  vectors, hash maps, strings, ints, symbols.
- `!` type constraints as body elements (`! sym Type`, `!returns Type`).

Note: `intrp-ast.toc` already carries shapes for `FloatLit`, `Match`,
`BlockComment`, `Superposition`, `AddNs` — so the actual target may be
broader than scope (a). Confirm before assuming.

## 9. Open questions / TBD

- The **output seam** (§3.4): how the reader's parse output is re-pointed to
  a different target (EBNF, etc.). Not designed.
- Whether to keep producing `intrp-ast.toc` shapes or define new ones.
- The exact **scope** (§8).
- A **performance measurement** approach (what "as performant as possible"
  is measured against; the plan's acceptance files were `hvm-core.toc` and
  `regression-tests/test11.toc`).

## 10. Running log (append newest last)

- (this doc created) Goal restated: hand-built, performant reader in
  `interpreter/intrp-rdr.toc`; generator abandoned; superposition is the
  performant alternative mechanism; output seam TBD. Current reader has the
  state machinery, char predicates, `Parser` type, and the
  `comment`/`whitespace`/`literal`/`location`/`symbol`/`full-symbol` leaf
  parsers; verified by `make rdr-sup` (t1–t19).
- Integer-literal family added to `intrp-rdr.toc`: `text-to-int`,
  `digit-run`, `int-lit` (see §4.5); verified by `make rdr-sup`
  (t20–t29: basic, negative, whitespace, zero, stop-at-non-digit, multi-
  digit, negative-with-ws, no-digit-after-dash, non-numeric).
- String-literal family added to `intrp-rdr.toc`: `string-end` (data
  scan), `string-body`, `string-lit` (see §4.5); backslash escapes kept
  intact in the value; a newline or EOF before the closing quote is a
  `ParserError` ("unterminated string at <loc>"). Verified by
  `make rdr-sup` (t30–t41: basic, empty, ws, escaped quote, escaped
  backslash, escaped-backslash-then-escaped-quote, embedded space,
  unterminated-at-EOF, unterminated-at-newline, backslash-newline,
  non-quote, empty input). First run caught an off-by-one (text span
  included the closing quote) — fixed in `string-body`. Next: the
  expression parsers (calls first).
- t47 (`many` over an alternation) investigated; root cause found and
  recorded as §3.5; the identified fix was **reverted — not in the
  tree**. Symptom: `(many (| (literal 'a) (literal 'b)))` over `"abab"`
  returns `[a] "bab"` instead of `[a b a b]` (input-dependent: `"ba"`
  parses, `"ab"` doesn't; t42–t46 pass, t47 fails). Not a `many-step`
  logic error: the raw sup tree is already corrupt before `collapse`
  runs, and the corruption is *correct* correlated duplication (§3.5) of
  a sup of Parser *values* — `many-step` reuses its `p` at every
  recursion level (`x p` and `(many-step p)`), forcing every parse
  position onto the same branch. t44 passes because its `p` is a single
  Parser (DUP-safe VAL); t47's is a sup of Parsers. The full reader
  showed more severe symptoms on top (non-determinism, buffer
  exhaustion, lost side effects) — the same correlation compounded by
  recursion and state threading. The identified fix (an `alt`
  combinator per §3.5, with t47 calling it) was reverted; `intrp-rdr.toc`
  and `rdr-sup.toc` are back to their committed state and t47 still
  fails. Open: (a) re-apply the `alt` fix when ready; (b) `optional` has
  the same latent pattern (§4.4); (c) a secondary `probe-many-sup`
  symptom (1974 leaked pairs, non-deterministic loss of `println` side
  effects) was never minimized — likely the same root cause.
