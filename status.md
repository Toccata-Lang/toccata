# HVM Implementation Status

Read these files for context before starting any:

1. `docs/calculus.md` — Formal calculus: node types, polarities, all 15 interaction rules with before/after descriptions
2. `docs/interactions.dot` — Visual diagrams for each rule
3. `docs/implementation.md` — Architecture reference: term layout, memory, reduction engine
4. `skills/editing-runtime3.md` — Pitfalls and learnings from uncommenting runtime3.c stubs
5. `runtime3.h` — Value type system, struct definitions, type constants, function declarations
6. `runtime3.c` — Runtime: memory management, free lists, vector/hash-map operations, native HVM functions, main()
7. `new.h` — Type definitions, tag constants, function declarations
8. `new.c` — Core implementation: `take`, `swap`, `move`, `interact`, existing rules
9. `hvm-core.toc` — Core type definitions and structure
10. `graph.c` — DOT graph generation for debugging
11. `Makefile`
12. `implementation-notes.md` — working notes on internal mechanics: term/bit layout, strictArgs, protocol dispatch codegen, BMI C-API hazards, sha1, build/test pipeline

Read in order: the calculus defines the rules, the implementation shows how they work, the tests show how to exercise them, and the graph/debug files help diagnose issues.

## Debugging Tools

**lldb is available** (`/usr/bin/lldb`, v18.1.3). Use it for live debugging of the C runtime — breakpoints, conditional breakpoints on refcount failure predicates, watchpoints, and catching double-frees at the first over-decrement. See `skills/memory-leak-hunting.md` ("lldb Workflow") for the concrete commands.

## Current Work

Hash-map functionality is complete: `hash-map-regressions` passes clean (the 25-key dissoc regression is enabled), and the `integerSha1` over-read is fixed (hashes the 8-byte `TYPE_SIZE` type tag; commit `e6b6c91`).

`defprotocol` has been eliminated from the language — protocol functions are declared as individual `defp`s (see `hvm-core.toc`). `check-bad-incRef` is deferred until the very end. The remaining defprotocol-era tests (test-inline-invoke, state-error1-1/2) still need the same treatment. `test-apply-constructor` is deferred until the end (unclear if it's really needed).

## Working tests (51)

**Memory leaks are a failure condition.** Every test must leave `malloc_count == free_count` (all Value allocations freed). Any non-zero diff at the end of a test is a failure.

**Node leaks are a failure condition.** Every test in test-hvm.c must leave `glblAlloced == 0` (all HVM node buffer pairs freed). Any non-zero value at the end of a test is a failure.

**Side effect.** An expression that is not the last expression in a function body. It exists only to execute a side effect such as printing a string.

**Regression test output files.** Each regression test has a corresponding `*.rslt` file. After a test is run, this file should be mostly unchanged according to `git diff`. The exception is execution statistics like ITERS count and amount of nodes used.

- [x] test1
- [x] test2
- [x] test3
- [x] test4
- [x] test5
- [x] test6
- [x] test7
- [x] test8
- [x] test9
- [x] test10
- [x] test11
- [x] test12
- [x] test13
- [x] test14
- [x] test15
- [x] test16
- [x] test17
- [x] test18
- [x] comment-in-deftype
- [x] test-global-empty-list
- [x] function-regressions
- [x] test-ignore-inferred
- [x] cond-expr-1
- [x] cond-expr-2
- [x] cond-expr-3
- [x] cond-expr-4
- [x] cond-expr-5
- [x] free-static-value
- [x] tail-cond-1
- [x] test-inline-namespaced-sym
- [x] integer-regressions
- [x] string-regressions
- [x] test-inlined-result-constraint
- [x] vector-regressions
- [x] test-either
- [x] test-comment-in-let
- [x] test-recursive-map-fn
- [x] test-tail-recur-1
- [x] test-tail-recur-2
- [x] test-tail-recur-3
- [x] test-trailing-comment
- [x] test-underscore-inline
- [x] test-uni
- [x] test-closures
- [x] test-fusing — known deferred 239-pair node leak (baseline committed with the leak)
- [x] test-bmi
- [x] test-array-node
- [x] test-collision-node
- [x] test-threading
- [x] test-or-comment
- [x] hash-map-regressions

## Tests to be processed

### New functionality to implement

- [ ] agent-regressions — agent system (needs agents/promises — ignored features)
- [ ] maybe-regressions — Maybe type (blocked: `type-args` not implemented; also uses `list`)
- [ ] state-error1-1 — state-error monad (needs defp rewrite like check-bad-incRef; plus lists, `instance?`, destructuring)
- [ ] state-error1-2 — state-error monad (needs defp rewrite like check-bad-incRef; plus lists, `instance?`, destructuring)
- [ ] test-gensym — gensym (needs `future` — ignored; gensym not wired up)
- [ ] test-apply-constructor — apply + constructor (deferred until the end — unclear if it's really needed; compiles, runtime: "No implementation of 'apply' found for type")

### Compiler tests (verify existing compiler features)

- [ ] and-prop — type property inference (deferred until the very end)
- [ ] check-bad-incRef — reference counting (deferred until the very end)
- [ ] or-and-constraints — type constraints in or/and (needs type system completion)
- [ ] test-cond — cond syntax (deferred until the very end)
- [ ] test-inline-invoke — inline invoke (blocked: field access — "Undefined symbol: 'invoke'" for the `invoke-fn` deftype field)
- [ ] test-inline-sym-literal — symbol literals (deferred until the very end)
- [ ] test-proto-def-constraints — protocol constraints (needs type system)
- [ ] types-regressions — type system (blocked: type system)
- [ ] test-for — `for` comprehension (needs more compiler work)

## Ignored for now

These features won't be in the new version (lists might be added eventually):

- [ ] future-regressions — futures
- [ ] lazy-list-regressions — lazy lists
- [ ] list-regressions — lists (might be added eventually)
- [ ] minimal-variadic-params — variadic params
- [ ] promise-regressions — promises
- [ ] destruct-test — destructuring
- [ ] test-proto-impl-destruct — protocol destructuring (obsolete)
- [ ] symbol-regressions — symbols
- [ ] test-hlist-cons — hlist + cons
- [ ] use-before-defined — definition ordering
- [ ] test-type-of-tail — tail expression types (obsolete)
- [ ] test-regressions — meta-test runner (obsolete)
- [ ] test-parser-efficience — parser performance/debug test (obsolete, git deps)

## Lessons Learned

**Inline C constraint:** Inline C expressions may only appear at the top level of a Toccata file, or as the only code expression in a `defn` body. Comments and type annotations may appear in the `defn` body alongside them, but no other code expressions.

**Cycle detection at encoding time is required.** Simply keeping a list of visited nodes during traversal (e.g. in `isCycle`/`findCycle`) is insufficient — by the time you detect a cycle during traversal, the damage may already be done. Cycles must be detected and handled at encoding time, before the graph is constructed.

**Segfault on large function bodies.** When a function gets too many expressions, the current compiler segfaults. If that happens, factor out some of the side-effecting expressions to their own functions. Done in `regression-tests/test-bmi.toc` (`a93b2f2`): `main` was split into `test-copy-assoc` and `test-mutate-assoc`.

## Known Issues

**`test-hvm` node leak.** `glblAlloced should be 0, got 1` at `regression-tests/test-hvm.c:1828`. Pre-existing — not caused by any recent changes. Leaving as-is until I want to tackle it. All REG_TESTS pass except the deferred `test-fusing` leak below.

**`test-fusing` node leak (deferred).** Leaves 239 node pairs allocated ("Leaked pairs!! 239"); the committed `.rslt` baseline includes the leak. Known since `e6b6c91`; left in REG_TESTS with the baseline as-is until the leak is hunted (lldb workflow in `skills/memory-leak-hunting.md`).

**`new-toc` typer crash: fn literal as a direct deftype constructor argument.** `(MyType (fn [x] x))` crashes the compiler ("Value 'arN' of type 'AllValues' does not have field '.param-consts' at typer.toc: 223"). Binding the fn first — `(let [f (fn [x] x)] (MyType f))` or a top-level `def` — compiles fine. Worked around in `check-bad-incRef` (`se-nop-fn` def). Will also affect state-error1-1/2 and test-inline-invoke when they get rewritten.

**`new-toc` codegen is nondeterministic in global numbering.** Repeated runs of `./new-toc` on the same `.toc` produce different `glbl*` numbering (and occasionally a different number of globals), so regenerated `.c` files differ even with no source change. Behavior has been identical across variants so far (same ITRS/results), but a transient `.toc` state on 2026-08-25 did produce a hash-map variant that leaked 242 pairs — so a leak can be variant-dependent. Treat `.c` files as non-diffable across builds, and be suspicious of leak reports that don't reproduce on a fresh regeneration.

