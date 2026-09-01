# Writing Toccata

Rules for writing Toccata in this repo. These state the language as it is being
built (the new compiler and runtime) — they apply now and going forward.
Temporary new-toc build workarounds are NOT here; they live in
`docs/new-compiler-plan.md` (Verified facts) and expire when the new compiler
builds the code.

## Forms

* Top-level forms are prefix forms: `(keyword ...)` with the keyword as the
  first token inside the form.

* `deftype` has two forms:
  - *Single-ctor:* `(deftype Name [f1 f2 ...] <protocol impls>)` — one
    constructor, named the same as the type.
  - *Multi-ctor:* `(deftype Type (Ctor1 [fields] <impls>) (Ctor2 [fields]
    <impls>) ...)` — the type name **and each ctor name** are types usable in
    type expressions.

  Ctors are namespaced like any other symbol: bare name in their own module,
  `ns/` prefix when accessed through an imported namespace.

  A bare name in a multi-ctor list references an existing ctor (works for
  core ctors too), rather than defining a new one.

  A constructor declared with no fields is a **singleton value**, referred to
  by its name (the preferred form). The no-arg call form `(CtorName)` is valid
  but not yet supported.

* `cond` takes flat (test value) pairs and must end with a default expression
  that catches the case where every test fails: `(cond t1 v1 t2 v2 ... e)`.
  Separate each (test value) clause pair with an empty line. Exception: if
  every clause fits on a single line and the test halves are all about the
  same length, keep them on consecutive lines with no blank lines between.

* `let` takes multiple bindings in one form: `(let [b1 e1 b2 e2 ...] body)` —
  sequential binding semantics: later initializers see earlier bindings.
  Bind a name only if its value is used more than once in the rest of the
  `let`; otherwise write the value inline. Exception: bind a very long value
  expression even when used once, if inlining it would be awkward.

* Hash-map literals are available in expression position: `{"k" v, "k2" v2}` —
  commas between pairs are whitespace. The literal desugars to core calls, as
  `[...]` does.

* Never use `emptyBMI` directly — write the `{}` literal for an empty hash map.

* There are no forward declarations. Named `defn`s may be mutually recursive —
  the compiler parses the whole source before code generation and resolves
  concrete call targets itself.

## Naming

* By convention, types and ctors start with a capital and are camel-cased
  (`ParserState`, `IntegerLit`). Other top-level `def`'d symbols are all
  lowercase, kebab-cased (`parse-expr`, `read-run`).

* Local bindings (`let`, `fn` args) may shadow any name **except core names** —
  core names may not be shadowed. Top-level symbols (`def`/`defn`/`defp`,
  types, ctors) must not collide within the same namespace.

## Modules

* `add-ns` module paths are relative to the importing file's directory and are
  bare relative paths only — no `../`, no absolute paths, no leading `./`. A
  module reference may not point outside the project directory.

## Data

* A vector is a homogeneous sequence — never a record. Grouping related
  values whose meaning is determined by position (including same-typed groups
  like `[start end]`) is done only with a deftype ctor with appropriately
  named fields, accessed via getters (`.field`). Multi-value returns from a
  function are grouped this way; there is no other mechanism.

* Grouping deftypes are meant to be sequenced and traversed via protocol
  implementations:
  - Define `map` and `flat-map` for a grouping deftype to sequence
    operations that produce or consume it.
  - For structurally recursive types, also implement `recurse` — this
    enables the core's recursion schemes (`fold`, `unfold`) over the
    structure.

  Recursion schemes are a key feature of Toccata. Few, if any, are
  implemented at present, and using them may cause node leaks; short-term
  issues are worked around as they come up.

* A hand-rolled accumulator recursion that walks a vector left-to-right is
  `reduce` — write the fold, not the `-acc` function. Building right-nested
  structure (e.g. the `cond`/`and` desugarings) is a `reduce` over
  `(reverse ...)` with the innermost element as the init.

## Core API

* `+` is exactly 2-arg — nest to combine more: `(+ a (+ b c))`.

* `first` on a Vector returns `Some element`, not the bare element — extract
  with `(extract (first v))`. `rest` returns a Vector.

* `reduce` is a left fold: `(reduce coll init f)` applies `(f acc elem)` per
  element and returns `init` for an empty collection. `reverse`, `last`, and
  `butlast` are implemented for Vector; `last` returns `Some element` like
  `first`, `butlast` returns a Vector (empty for a single-element vector).

* `subs` is 3-arg: `(subs s start len)`. The rest of a string is
  `(subs s 1 (count s))`. `count` is O(1) for SubString and StringBuffer.

* `char-code` gives the first char's integer code (0–255); `char` is the
  inverse.

* Keep a hash map's keys a single type — a `get` with a differently-typed key
  will miss.

* Named / namespace-qualified functions are first-class values — passable as
  arguments, callable as `(f x)`.

## Dispatch

* There is no `instance?` — dispatch is via protocols. To branch on which ctor
  of a deftype a value carries, use a `match` expression (not yet
  implemented). Until then, the stopgap is a tag protocol extended per ctor —
  it aborts if called on a type it isn't extended for, so only use it on
  values known to be one of your own ctors.

* A protocol implementation may not contain an inline C body. When the
  implementation needs C, the protocol impl makes an immediate call to a
  plain `defn` whose body is the `(inline ...)` expression (cf. `type-name`
  calling `default-type-name` in `hvm-core.toc`).

## Evaluation

Evaluation is data-dependent workflows, not sequential execution. A function
body's main branch produces the output value (the datum passed to the next
stage — not a "return value"); side-effecting expressions spawn parallel
workflows that run independently of it.

* Every side effect the workflow reaches runs at least once.
* Plain `defn`: side effects run on every call.
* Closures: a side effect whose arguments don't depend on the closure's call
  args runs once, at closure creation; one that does runs on every call.

## Superposition

`|` is a first-class language feature. `(| a b c ...)` creates a
**superposed value** — a value that is simultaneously all of `a`, `b`, `c`.

A superposed value **duplicates any workflow it is given to, once per
superposed value**. The duplicates execute in parallel, all lazily, and
produce a single superposed result. Each duplicated workflow runs its own
side effects — an effect in the duplicated region fires once per branch
(at least once each).

`( | )` — no values — creates an **empty superposition**, which terminates
the branch it flows into. If all branches of a superposition terminate, the
result is a single empty superposition.

Planned primitives (not yet implemented, unnamed): superposing all values of
a vector over a single fn, and converting a superposed value to a vector.
