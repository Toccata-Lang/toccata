# HVM Implementation Status

## Before fixing a new test

Read these files for context before starting any new interaction:

2. `docs/calculus.md` — Formal calculus: node types, polarities, all 15 interaction rules with before/after descriptions
3. `docs/interactions.dot` — Visual diagrams for each rule
4. `docs/implementation.md` — Architecture reference: term layout, memory, reduction engine
5. `new.h` — Type definitions, tag constants, function declarations
6. `new.c` — Core implementation: `take`, `swap`, `move`, `interact`, existing rules
8. `graph.c` — DOT graph generation for debugging
10. `Makefile` — Build command for `test-hvm`

Read in order: the calculus defines the rules, the implementation shows how they work, the tests show how to exercise them, and the graph/debug files help diagnose issues.

## Working tests

**Memory leaks are a failure condition.** Every test must leave `malloc_count == free_count` (all Value allocations freed). Any non-zero diff at the end of a test is a failure.

**Node leaks are a failure condition.** Every test must leave `glblAlloced == 0` (all HVM node buffer pairs freed). Any non-zero value at the end of a test is a failure.

**Regression test output files.** Each regression test has a corresponding `*.rslt` file. After a test is run, this file should be mostly unchanged according to `git diff`. The exception is execution statistics like ITERS count and amount of nodes used.

- [ ] test1
- [ ] test2
- [ ] test3
- [ ] test4
- [ ] test5
- [ ] test6
- [ ] test7
- [ ] test8
- [ ] test9
- [ ] test10
- [ ] test11
- [ ] test12
- [ ] test13
- [ ] test14
- [ ] test15
- [ ] test16
- [ ] test17
- [ ] comment-in-deftype
- [ ] test-global-empty-list
- [ ] test-ignore-inferred
- [ ] cond-expr-1
- [ ] cond-expr-2
- [ ] cond-expr-3
- [ ] cond-expr-4
- [ ] free-static-value
- [ ] tail-cond-1
- [ ] test-inline-namespaced-sym
- [ ] string-regressions
- [ ] vector-regressions

