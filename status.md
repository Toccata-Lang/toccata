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

Read in order: the calculus defines the rules, the implementation shows how they work, the tests show how to exercise them, and the graph/debug files help diagnose issues.

## Current Work

Currently implementing hash-map functionality. This involves translating C code from `runtime3.c` into higher-level Toccata code, with the low-level pieces extracted into inline C functions.

## Working tests

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
- [x] comment-in-deftype
- [x] test-global-empty-list
- [x] test-ignore-inferred
- [x] cond-expr-1
- [x] cond-expr-2
- [x] cond-expr-3
- [x] cond-expr-4
- [x] cond-expr-5
- [x] free-static-value
- [x] tail-cond-1
- [x] test-inline-namespaced-sym
- [x] string-regressions
- [x] vector-regressions
- [x] test-recursive-map-fn
- [x] test-tail-recur-1
- [x] test-tail-recur-3
- [x] test-trailing-comment
- [x] test-underscore-inline
- [x] test-uni
- [x] test-inlined-result-constraint
- [x] function-regressions
- [x] test-closures

## Tests to be processed

### New functionality to implement

- [ ] agent-regressions — agent system
- [ ] hash-map-regressions — hash-map data structure
- [ ] maybe-regressions — Maybe type
- [ ] state-error1-1 — state-error monad
- [ ] state-error1-2 — state-error monad
- [ ] test-gensym — gensym
- [ ] test-apply-constructor — apply + constructor

### Compiler tests (verify existing compiler features)

- [ ] and-prop — type property inference (blocked: `instance?` not implemented)
- [ ] check-bad-incRef — reference counting (needs hash-map: reduce on collections)
- [ ] or-and-constraints — type constraints in or/and (needs type system completion)
- [ ] test-cond — cond syntax (waiting on: `any?`, `ever?`, `instance?`)
- [ ] test-inline-invoke — inline invoke (needs hash-map)
- [ ] test-inline-sym-literal — symbol literals (blocked: `instance?`)
- [ ] test-or-comment — comments in or/and (needs hash-map: get, map)
- [ ] test-proto-def-constraints — protocol constraints (needs type system)
- [ ] test-tail-recur-2 — tail recursion (blocked: str-vect dispatch on None)
- [ ] types-regressions — type system (blocked: type system)
- [ ] test-for — `for` comprehension (needs more compiler work)
- [ ] test-threading — `->` threading macro (depends on hash-map functionality: assoc, vals, reduce)

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

## Known Issues

**`test-hvm` node leak.** `glblAlloced should be 0, got 1` at `regression-tests/test-hvm.c:1828`. Pre-existing — not caused by any recent changes. Leaving as-is until I want to tackle it. All 49 REG_TESTS pass.

