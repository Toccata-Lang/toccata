# Implementation Notes

Working notes on the internal workings of Toccata/HVM, gathered while debugging the
BMI / hash-map work (Aug 2026). Complements `docs/implementation.md` — the emphasis
here is on the machinery that actually bit us: term layout, argument passing,
protocol dispatch codegen, and the BMI C API.

Depth marker per section: **[read]** = traced in the source this session;
**[observed]** = seen in generated code / usage, not fully traced.

## 1. Term representation (new.h) [read]

A `Term` is a single 64-bit value:

```
[ Location: 32 bits ][ Label: 28 bits ][ Tag: 4 bits ]
```

Tags:

| Tag | Value | Kind |
|---|---|---|
| VAL | 0x00 | positive native value (no location) |
| VAR | 0x01 | positive variable (indirection) |
| SUB | 0x02 | negative "possible deferred redex" |
| NUL | 0x03 | positive eraser |
| ERA | 0x04 | negative eraser |
| LAM | 0x05 | positive constructor {− +} |
| APP | 0x06 | negative constructor {+ −} |
| REF | 0x07 | positive reference (no location) |
| VL1 | 0x08 | VAL alias (`termTag` maps VL1→VAL) |
| SUP | 0x09 | positive duplicator {+ +} |
| DUP | 0x0a | negative duplicator {− −} |
| OPX / OPY | 0x0b / 0x0c | negative operations {+ −} |
| I60 | 0x0d | positive integer; `newI60(x) = (x << 4) \| I60` (header comment says 56-bit int) |
| F60 | 0x0e | positive float |
| LAZ | 0x0f | positive lazy node {− +} |

Key points:

- **Native terms carry no location** — VAL, NUL, REF, ERA, I60, F60, SUB are
  self-contained in the 64-bit value (`hasLocation` returns 0 for them).
- **Pairs occupy two consecutive buffer cells.** For a pair at location `L`:
  port 1 is `nodeBuff[L]`, port 2 is `nodeBuff[L+1]`.
  `portLoc(n, t) = n + termLoc(t) - 1`.
- The pair's tag/label live in the Term value itself, not in the buffer.

## 2. Buffer, allocation, reference counting (new.c) [read]

- `nodeBuff` is a flat array of 64-bit cells. Pairs are allocated/freed in
  2-cell chunks via an O(1) free list (`allocPair` / `freePair`).
- `glblAlloced` (SAFETY) counts live pairs — **must be 0 at test end**.
- Heap `Value`s (runtime3.c structs) are separately refcounted with
  `incRef` / `dec_and_free`; `malloc_count` / `free_count` (CHECK_MEM_LEAK)
  **must match at test end**. These are two independent leak checks.
- `take(loc)`:
  - SUB / LAZ → returns a VAR indirection to `loc` (location NOT freed).
  - otherwise → frees the cell and returns the term itself; VAR is followed
    (the returned term points at the target).
- `swap(loc, term)`: atomic exchange. If the displaced term is a deferred
  redex (SUB with a location), it is queued into the redex bag and SUB is
  returned.
- `dupeArg` duplicates an argument (refcount bump) — generated for every use
  of a global (`Term glblX_n; glblX = dupeArg(glblX, &glblX_n, 0);`).

## 3. Redex bag and interaction [read]

- Per-thread bag: `pairs.rdxs[LOCAL_PAIRS_SIZE=16000][2]`.
- `pushRedex(neg, pos)` / `popRedex` / `interact(neg, pos)`.
- SAFETY: BOOM on a "bad redex" (positive neg or negative pos).
- Codegen emits either `pushRedex(...)` (STRICT builds) or a LAZ node
  (`swap(termLoc(Rslt), makePair(LAZ, 0, args, fn))`) — the `#ifdef STRICT`
  pairs seen throughout generated C.

## 4. Argument passing: APP chains and strictArgs [read]

- A call `(f a b c)` is an APP chain: `APP(a, APP(b, APP(c, SUB)))`.
  Codegen pushes args **last-first** (each `makePair(APP, 0, arg, prev)`).
- `strictArgs(ref, args, expected, &out)` (new.c:709) collects `expected`
  strict args off the front of the chain and returns the **remaining** APP
  chain:
  - VAL / I60 / F60 / REF → collected directly.
  - LAM → wrapped in a malloc'd `TermVal` holding an allocated pair
    (this is a heap allocation — counted by malloc_count).
  - VAR → `varArg` (lazy indirection handling) [observed].
  - SUP → duplicator: builds a lazy sharing network via `argsNet` [observed].
  - NUL → frees collected args, `count = -1` (chain exhausted).
- This is how every native/inline-C function receives its arguments. The
  `expected` count is the declared arity — a mismatch means args are silently
  left in the remaining chain (see §5 pitfall).

## 5. Protocol dispatch (codegen.toc) [read]

A protocol call `(copyAssoc m k v hash shift)` compiles to two generated
pieces:

**1. Call-site thunk** (`wrap-proto`, codegen.toc:238):

```c
args = strictArgs(ref, args, 1, &arityArgs);   // collect receiver
if (arityArgs.count == 1) {
  swap(portLoc(1, args), arityArgs.args[0]);   // put receiver back at port 1
  args = makePair(APP, 0, newI60(<line>), args);
  args = makePair(APP, 0, <fileStr>, args);
  interact(args, <protoRef>);
}
```

The chain is logically intact after the swap — nothing is dropped here.

**2. Protocol dispatcher** (`emit-proto`, codegen.toc:905):

```c
args = strictArgs(ref, args, 3, &arityArgs);   // [fileStr, line, dispVal]
if (arityArgs.count == 3) {
  swap(termLoc(args), (Term)dispVal);          // receiver back in front of remaining args
  if (termTag(dispVal) == I60) { ...integer impl or BOOM... }
  else switch (dispVal->type) {
    case <typeNum>: pushRedex(args, <implArity>); break;
    default: BOOM("No implementation of 'X' found for type Y ... file:line");
  }
}
```

The impl function receives `[receiver, ...method-args]` — the full original
arg list.

**Pitfall:** the protocol's param list must match the call-site arity.
Codegen does not check this. A 5-arg call against a 3-param protocol
generated code that dropped the extra args (the `bmiCopyAssoc` incident —
fixed in `9f5c40c` by making `copyAssoc` a 5-param protocol).

## 6. Function codegen shape [observed]

- A `defn` body compiles to a LAM chain (one LAM per param; innermost LAM =
  last param), then the body.
- "link args to body" pattern: the body result is swapped into the LAM's
  result port, then the node is decomposed into `apps`/`lams` and re-wrapped
  in a LAZ before `pushRedex(args, seq)`.
- Protocol impls are ordinary functions (`glbl<name><n>`) wrapped in a REF
  (`Term glbl<name><n+1> = newRef(...)`).

## 7. BMI (BitmapIndexedNode) [read]

Clojure-style persistent trie node: 32-bit bitmap + packed child array;
`shift` advances by 5 per level.

`bmiCopyAssoc` (runtime3.c:2270) — the reference the Toccata port mirrors:

```
bit = bitpos(hash, shift)
if bitmap & bit:
  child = bmiChild(node, bit)
  if child != 0 (sub-node):
    return bmiUpdate(node, bit, copyAssoc(child, k, v, hash, shift+5))
  else (leaf):
    if key == currKey:
      if val == currVal: return node
      else: return bmiClone(node, bit, key, val)
    else: return bmiReplaceCopied(node, k, v, hash, shift, currKey, currVal)
else:
  return addCopiedBMI(node, k, v, hash, shift)
```

C-API hazards (both caused real bugs, see status.md):

- `bmiKey` / `bmiVal` return **borrowed** pointers into the node's array —
  the Toccata wrappers must `incRef` (double-free fixed in `9ac6c7d`).
- `bmiChild` returns 0 for leaves — the Toccata wrapper maps that to a
  Maybe (`Some`/`None`), and `bmiCopyAssoc` branches with
  `(either (map (bmiChild m bit) <recurse>) <leaf-case>)`.
- The sub-node recurse branch only executes when a position holds a sub-node
  (a hash collision at that level) — single-entry tests never reach it.

## 8. sha1 [read]

- **Truncated SHA-1**: `SHA1_HASH` is 8 bytes (64-bit digest), not the full
  160 bits (runtime3.c:1631). Standard Steve Reid public-domain core.
- `integerSha1` (runtime3.c:1829) hashes the type field then the value — but
  reads **8 bytes from a 4-byte `unsigned type`**, pulling in 4 bytes of
  adjacent stack memory. Hash may not be reproducible outside the exact
  runtime binary. Open issue — see status.md Known Issues.
- Dispatched per-type through the `sha1` function pointer
  (`Term (*sha1)(FnArity *, Term)`, runtime3.c:3225).

## 9. Build & test pipeline (Makefile) [read]

- Compiler build: `./toccata compiler.toc > new-toc.tmp` → sed hack
  (`FnArity`→`Vector`) → awk `#line` markers → clang-format → link with
  `core.c` → `new-toc` binary.
- Regression test: `./new-toc regression-tests/X.toc > X.c` (same
  post-processing) → compile with `new.c runtime3.c graph.c` →
  `./X party-pooper | sort > X.rslt`.
- `.rslt` files are golden outputs; `git diff` after a run should show only
  execution stats (ITERS, node counts).
- `REG_TESTS` list in the Makefile; `hash-map-regressions` is currently
  commented out of the list.
- Special targets: `test-hvm`, `test-hash-map`, `test-runtime`
  (all with `-DTESTING_HVM=1`).
- Flags: `-g -DCHECK_MEM_LEAK=1 -DSAFETY=1 -DSTATS=1`; ASan available via a
  commented-out CFLAGS flag.
- `hvm-core.toc` is loaded at runtime by the `new-toc` binary ("*** Loaded
  core") — editing it needs no compiler rebuild; the test rules list it as a
  prerequisite, so `make test-X` regenerates the `.c` automatically.

## 10. Toccata language notes (as observed) [observed]

- `defn` / `defp` (protocol) / `deftype` / `extend-type`.
- `!` type-annotation comments; `(inline Type "C code")` with `_1.._N`
  arg placeholders and a `result` output.
- Maybe: `None` / `Some [x]` with protocols `map`, `either`, `flat-map`,
  `cond`, `and`, `or`, `recurse`, `extract` (hvm-core.toc:439+).
- Control: `int-cond`, `cond`, `and`, `or`, `either`.
- `->` threading; `_FILE_` / `_LINE_` macros; `rt/test` from
  `regression-tester.toc`.
- Named-defn recursion works; tail recursion tested to 30000 iterations
  (test-tail-recur-1/3).
- Inline-C constraint: top level of a file, or the only code expression in a
  `defn` body (see status.md Lessons Learned).
- `let` is syntax sugar for an immediate application to an anonymous
  function: `(let [x (some expression)] ...)` is
  `((fn [x] ...) (some expression))`.
- A `let` binding cannot reference an earlier binding in the same `let`
  (`(let [m2 ... c (count m2)] ...)` is "Invalid expression") — nest the
  lets.

## 11. Debugging [observed]

- lldb v18.1.3 at /usr/bin/lldb — concrete workflow in
  `skills/memory-leak-hunting.md`.
- `graph.c` → `graphs.dot` → `dot -Tsvg graphs.dot > graphs.svg`.
- Print helpers in new.c: `printTerm`, `printRawTerm`, `pb`, `pr`,
  `checkBuff`, `printBuff`.
- SAFETY mode gives: bad-redex BOOMs, strictArgs arity BOOMs (with
  `printTerm` dump), `glblAlloced` tracking, `swap(VOID)` BOOM.
- `prefs(char *tag, Term v)` (runtime3.c:32) is the C-level probe: prints
  `tag: I60`, `tag: <ptr> <refs>`, or `tag: <ptr>` (for null) to stderr.
  Read-only — it never touches refs. As a C statement it runs exactly where
  written, so it can be dropped at any point in a C function.
- `(prefs "tag" x)` (hvm-core.toc:36, `! tag String`) is the Toccata-level
  wrapper: copies the tag (≤63 chars) to a stack buffer — pooled String
  buffers are not reliably NUL-terminated past `len` — calls the C `prefs`,
  consumes the tag, and **returns `x` unchanged** (pass-through; the
  return-exception means `x`'s ref is NOT decremented). Because Toccata
  execution order is not lexical order (see §12), use it as a wrap —
  `(prefs "tag" <expr>)` — so the print is tied to the moment `<expr>`'s
  value is produced.
- `rt/test` aborts on failure — the process dies before the malloc/free
  counts print, leaving an empty `.rslt`. For leak hunting, evaluate the
  bare expression instead of asserting.

## 12. Evaluation and refcounting: what the prefs probes showed [observed]

From leak-hunting the BMI createNode branch (Aug 2026), with `(prefs "tag" x)`
probes at each stage of a two-assoc expression:

- **Every appearance of a value in the code is a use, and each use has its
  own ref.** The compiler arranges (dupeArg / redex construction) that each
  appearance can consume exactly one ref. Consequence: a `(prefs "tag" x)`
  probe's printed count *includes* the ref for `x`'s appearance in the call
  (the current wrapper passes `x` through rather than consuming it, but the
  appearance still carries its ref at print time), so background count =
  printed − 1.
- **Refs for future uses exist before those uses run.** Right after the let
  bindings (before any assoc executed), the probed values already carried
  refs for uses still ahead: k1 (5 appearances total) showed background 3,
  k2 (5) showed 2, v1 (4) showed 3, v2 (4) showed 2. The compiler
  pre-creates refs for upcoming uses; the exact schedule depends on code
  position, not just appearance count — k1 and k2 had the same number of
  appearances, but k1 (a first-assoc arg) carried one more eager ref than
  k2 (a second-assoc arg).
- **Unforced lazy redexes are freed with their held refs.** A `cond` branch
  that never runs (e.g. the `bmiClone` redex when the `bmiReplaceCopied`
  branch is taken) holds refs to its args in its APP/LAZ pairs, but those
  refs are released when the unforced redex is discarded. Evidence: the
  same-key-different-value test (bmiClone branch) ends diff 0 even though
  the unforced bmiReplaceCopied redex holds m/k/v/currKey/currVal refs.
- **Dataflow, not source order.** The generated code builds redex pairs
  (`makePair` APP/LAZ) and forces them as interaction demands; a value's
  refcount at any point reflects which redexes have been built and forced,
  not which source lines have "executed". Let bindings whose value is a VAR
  indirection (e.g. `m1` passed to a later call) carry no heap ref of their
  own.
