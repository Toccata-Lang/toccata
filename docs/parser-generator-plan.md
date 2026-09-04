# Parser Generator Plan (grammar data → recursive-descent reader)

Status: in planning (2026-09-04). Settled items are final until
re-opened; open items are queued. Companion to
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
- `interpreter/intrp-emit.toc` — the emitter (library, no main).
- `interpreter/emit-*.toc` — acceptance drivers (committed, same
  convention as the `rdr-*` drivers).
- `interpreter/gen-rdr.toc` — the generated module. Written by the
  driver (inline-C file write), built by a Makefile target, a build
  artifact — NOT committed.
- Throwaway probes: `scratch/` (never committed from there).

## Settled design

### Emitter API (`interpreter/intrp-emit.toc`)

The emitter `add-ns`es the grammar module (for the `ParserCombinator`
values) and the hand-written rdr module (for shared helpers such as
`vect-concat`; define locally instead if the dependency is unwanted —
implementer's choice, but keep module path spellings identical).

```toccata
;; Context threaded through every emit call:
;;   rule   — name of the rule currently being emitted ("" at top
;;            level); a Recur node renders as a call to this name.
;;   prefix — name prefix for generated helpers (the enclosing rule
;;            or helper name + "-").
(deftype EmitCtx [rule prefix])

;; Source lines for an expression which, given the let-bound symbol
;; `state`, yields a ParserResults. One impl per ctor. -> [String]
(defp emit-body [pc ctx])

;; Source of a single-char predicate expression over char string `c`.
;; Implemented ONLY for char-level ctors (CharRange, NotChar, bare
;; String, Any-of-char-level, and Rule/Many delegating per the ctor
;; table); default body aborts with a clear message — calling it on a
;; general parser is an emitter bug, and the abort IS the
;; char-level classification (no yes/no tag needed at most sites).
;; -> [String]
(defp emit-pred [pc ctx])

;; `Many` helper emission by FLIPPED RECEIVER: the receiver is the
;; CHILD combinator (task-1 ruling — no yes/no tag protocol; the impl
;; IS the path). Char-level ctors (CharRange, NotChar, bare String,
;; char-level Any, and Rule/Many delegating per the ctor table) get
;; the run-path impl (the <name>-char predicate defn + read-run
;; wrapper); the default body is the loop path (lift the child if
;; anonymous, emit the site-(b) accumulator defn). `emit-body` for
;; `Many` renders only the call expression. -> [String]
(defp emit-many [child ctx])

;; How to reference this parser in call position. Rule -> its name
;; (the Rule's defn is emitted at module level). Anonymous
;; combinators are never referenced directly: the parent lifts them
;; via emit-fn first. -> String
(defp emit-ref [pc ctx])

;; Lift any combinator to a named parser fn (plain defn, no protocol):
;;   (defn <name> [state] <emit-body pc (EmitCtx <rule> <name>->)>)
(defn emit-fn [pc name ctx])

;; Driver: takes the vector of top-level rules to emit + the entry
;; rule name. Emits, in order: module header (add-ns of the helper
;; layer), one parser defn per Rule (char-level body -> one-char
;; parser defn; see ctor table), generated helpers, then a main
;; template (see Generated module). -> [String]
(defn emit-module [rules entry])
```

### Char-level vs parser-level

A combinator is **char-level** iff it classifies a single character:
`CharRange`, `NotChar`, bare `String` (one char), `Any` of char-level
alts, and `Rule`/`Many` whose child is char-level (they delegate).
Char-level combinators have `emit-pred` and `emit-many` impls;
everything else hits the default abort / the default loop path.
`Many` of a char-level child is the **maximal-run fast path** (one
generated set-predicate defn + one `read-run`), not a loop.

### Ctor → generated code

Generated code assumes the helper layer (from
`interpreter/intrp-rdr.toc`): `ParserState`, `ParserResults`
(`ParserMatch`/`ParserIgnore`/`ParserError` — no `ParserFail`),
`Token`, `make-state`, `take-char`, `skip-whitespace`, `read-run`,
`str-prefix?`, `state-line`, and the `parse-then` / `parse-or` kit
from the Result-discrimination section.

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

Lifting rule: an anonymous combinator is lifted to a named helper iff
it is (a) the child of a `Many` slow path, (b) an alternative of a
parser-level `Any`, or (c) any position where its body would contain a
`let` inside a non-else `cond` clause (the malformed-cond hazard — see
inherited facts). Bare `String` bodies are let-free and inline
everywhere. Consequence: the generated code contains NO `let` in any
non-else `cond` clause, by construction.

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

Self-recursion only (`expression` ↔ its call alternative) → no forward
declarations needed (verified: self-recursion needs none under new-toc).
If the grammar ever grows mutual recursion, the emitter emits the
`(def name)` crutch.

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
a committed driver.

**`Many` fast/slow classification (ruling):** the flipped-receiver
protocol `emit-many` (see Emitter API) — char-level ctors carry the
run-path impls, the default body is the loop path. A yes/no tag
protocol was rejected: it would re-introduce the tag-shaped-impl
pattern this section rejects. Invariant, in the generated code and
in the emitter alike: impls ARE the branches.

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
  exit code is always 134 (abort), useless; pass = `*** Loaded <file>`
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
  `parse-program-acc` pattern).
- A program ending with a SUB or SUP error probably means a function
  called with the wrong number of arguments.

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
  closure captures a FREE VARIABLE leaks term pairs on the lazy
  machine (over 194 nodes it exhausts the 1MB term buffer) — use
  explicit recursion passing the value as a plain parameter (the
  `threading-acc` / `count-class-acc` pattern).
- `str*` over a vector of `str-vect` implementors concatenates to one
  String; `(str* [n])` renders an Integer (via `number-str`). `pr*`
  takes ONE string — `pr*` on a vector aborts silently; build the
  string with `(str* [...])` first.
- `str-append` (hvm-core.toc:608) appends IN PLACE into the dest's
  buffer — the dest must be a pre-allocated StringBuffer with enough
  capacity (the hand-written rdr pre-allocates its acc). Appending to
  a static string literal (e.g. `(str-append "" "x")`) overflows the
  global buffer — segfault (ASan: global-buffer-overflow in strncat)
  (2026-09-04, item 2a).
- `vect-concat` is user-defined (in `interpreter/intrp-rdr.toc`,
  defined BEFORE the grouping deftypes), not core.

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
this file + AGENTS.md. The emitter / grammar / driver sources may use
inline C freely. Generated code follows `docs/toccata-style.md`.

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

- [ ] **3. Emitter skeleton + char-level emission**
  `interpreter/intrp-emit.toc` (library): `add-ns` of the grammar
  module; `EmitCtx [rule prefix]` (no `!` annotations); `emit-pred`
  protocol — impls for `CharRange` (LO/HI evaluated at emit time via
  `char-code` + `str*`), `NotChar` (`(not (str= c CH))`), bare
  `String` (`(str= c S)`), `Any` (map `emit-pred` over alts, join with
  `or` — a non-char-level alt hits the default abort), `Rule` and
  `Many` (delegate per the ctor table); default body
  aborts with a clear message naming the ctor. `emit-module` v1:
  emits a module header + char-level rules' predicates only. Driver
  `interpreter/emit-pred.toc` + Makefile target (`rdr-top` pattern):
  emits the predicates for `digits`, `upper-case`, `lower-case`,
  `alpha`, `symbol-start`, `rest-of-symbol` and asserts the EXACT
  expected source lines (fingerprint via `str*` over the emitted
  vector, the `body-fingerprint` convention from `rdr-defp.toc`).
  - Done when: the driver prints OK for all six predicates with exact
    expected output, zero leaks, 0 remaining nodes, exit 0.

- [ ] **4. Leaf body ctors + end-to-end pipeline**
  `emit-body` impls: `CharRange` (one-char parser), `NotChar`, bare
  `String` (let-free via `str-prefix?`), `AlwaysSucceed`, `Error`;
  `emit-ref` (Rule → name; default aborts); `emit-fn` (defn wrapper,
  prefix = name + "-"). The driver gains an inline-C file write: it
  writes the generated module to `interpreter/gen-rdr.toc` (build
  artifact, not committed). Makefile target `gen-rdr` builds
  `interpreter/gen-rdr.toc` (`rdr-top` pattern, depends on
  `interpreter/intrp-rdr.toc` for the helper layer). Generated `main`
  template per the Settled section (slurp argv file, parse a sequence
  of expressions, one result line each, `file:line: msg` on error,
  non-zero exit on error).
  - Done when: a synthetic one-rule grammar (e.g. `(Rule "digit"
    (CharRange "0" "9"))`) generates a module that builds under
    `make gen-rdr` and, run on a sample file, prints the expected
    match lines / `file:line: msg` error lines by hand-verification;
    zero leaks, 0 remaining nodes.

- [ ] **5. `All` + `Ignore`**
  `emit-body` impls: `All` (nested `parse-then` per the site-(c)
  template, state threaded through the continuation params, result =
  vector of sub-values), `Ignore` (site-(c) shape: `parse-or` over
  `parse-then` with `identity` as the else-fn). Anonymous children
  lifted per the Lifting rule.
  - Done when: a synthetic grammar exercising `All` of mixed
    named/anonymous/bare-string children (and `Ignore` of an
    `All`) generates a module that builds and parses a sample file
    with hand-verified output; zero leaks.

- [ ] **6. `Any` (parser-level)**
  `emit-body` impl: nested `parse-or` per the site-(a) template;
  anonymous alts lifted to `N-i` helpers; the char-level `Any`
  behavior (via `emit-pred`) unchanged.
  - Done when: a synthetic grammar with a parser-level `Any` mixing
    bare strings, an anonymous `All`, and a named Rule generates a
    module that builds and parses a sample file where each
    alternative wins at least once and a failure case yields the
    expected error; zero leaks.

- [ ] **7. `Many` + `Rule`/`Recur` + full `emit-module`**
  `Many` both paths (fast: `-char` predicate defn + `read-run`
  wrapper with the `Token` → `ParserMatch` wrap; slow: accumulator
  loop per the ctor table — acc-recursion, NOT a reduce); `Recur` →
  call to `(.rule ctx)`; `emit-module` final form: explicit rule
  vector, every Rule → parser defn, helpers in index-path names, main
  template.
  - Done when: a synthetic SELF-RECURSIVE grammar (mini S-expression:
    `Any [symbol-ish (All ["(" (Many (Recur self)) ")"])]`) generates
    a module that builds and parses nested input to the correct
    vector-of-text values, with the loop terminating on non-matching
    input; zero leaks.

- [ ] **8. The real grammar + corpus**
  `emit-module` over the rules reachable from `expression` in
  `interpreter/intrp-grammar.toc`: `expression`, `symbol`,
  `symbol-start`, `rest-of-symbol`, `alpha`, `upper-case`,
  `lower-case`, `digits`, `int-literal` (Rule name `"integer"`),
  `float-literal`, `double-quoted-string`, `escaped-char`. Generate
  `interpreter/gen-rdr.toc`, build. Corpus: a driver-held pair of
  files (input lines / expected result lines) covering: ints, floats
  (incl. multi-digit both sides), strings with each escape
  (`\\` `\"` `\n` `\r` `\t`), symbols incl. operator names (`+`, `*`,
  `->`, `!x`), nested calls, empty input, and malformed lines
  (unterminated string, bare `)`, trailing garbage after a complete
  expression). The `gen-rdr` binary runs over the corpus input and the
  output is diffed against the expected file.
  - Done when: every corpus case matches, zero leaks, 0 remaining
    nodes.

- [ ] **9. Final verification**
  Zero leaks (malloc/free diff 0, remaining nodes 0) across: the
  grammar library load, the emitter library load, every `emit-*`
  driver, and the generated module over the full corpus. Every item
  above checked. This file updated with as-built notes (deviations,
  new hazards hit).
  - Done when: all items checked.
