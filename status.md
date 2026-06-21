# HVM Implementation Status

## Before fixing a new test

Read these files for context before starting any new interaction:

2. `docs/calculus.md` — Formal calculus: node types, polarities, all 15 interaction rules with before/after descriptions
3. `docs/interactions.dot` — Visual diagrams for each rule
4. `docs/implementation.md` — Architecture reference: term layout, memory, reduction engine
5. `new.h` — Type definitions, tag constants, function declarations
6. `new.c` — Core implementation: `take`, `swap`, `move`, `interact`, existing rules
8. `graph.c` — DOT graph generation for debugging
10. `Makefile`

Read in order: the calculus defines the rules, the implementation shows how they work, the tests show how to exercise them, and the graph/debug files help diagnose issues.

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

## Tests to be processed

- [ ] agent-regressions
- [ ] and-prop
- [ ] Boomer
- [ ] boom
- [ ] check-bad-incRef

- [ ] destruct-test
- [ ] function-regressions
- [ ] future-regressions
- [ ] hash-map-regressions
- [ ] interpreter-test
- [ ] lazy-list-regressions
- [ ] list-regressions
- [ ] maybe-regressions
- [ ] minimal-variadic-params
- [ ] or-and-constraints
- [ ] promise-regressions
- [ ] state-error1-1
- [ ] state-error1-2
- [ ] symbol-regressions
- [ ] test-apply-constructor
- [ ] test-closures
- [ ] test-cond
- [ ] test-for
- [ ] test-gensym
- [ ] test-hlist-cons
- [ ] test-inlined-result-constraint
- [ ] test-inline-invoke
- [ ] test-inline-sym-literal
- [ ] test-or-comment
- [ ] test-parser-efficience
- [ ] test-proto-def-constraints
- [ ] test-proto-impl-destruct
- [ ] test-recursive-map-fn
- [ ] test-regressions
- [ ] test-tail-recur-1
- [ ] test-tail-recur-2
- [ ] test-tail-recur-3
- [ ] test-threading
- [ ] test-trailing-comment
- [ ] test-type-of-tail
- [ ] test-underscore-inline
- [ ] test-uni
- [ ] types-regressions
- [ ] use-before-defined

## string-regressions investigation

**Status:** FIXED. Segfault was caused by `ReifiedVal` struct lacking a `next` field — the free list code cast `ReifiedVal*` to `Value*` to access `next`, which overlapped with `hashVal`. When memory was reused for a different implCount, the stale `next` value persisted and linked to the wrong free list. Fix: added `next` field to `ReifiedVal`, initialized in `malloc_reified` and reset in `decValRef`.

## Lessons Learned

**Cycle detection at encoding time is required.** Simply keeping a list of visited nodes during traversal (e.g. in `isCycle`/`findCycle`) is insufficient — by the time you detect a cycle during traversal, the damage may already be done. Cycles must be detected and handled at encoding time, before the graph is constructed.

