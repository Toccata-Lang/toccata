Read the hash-map Toccata plan (docs/hash-map-plan-toccata.md) and the files it references:
hvm-core.toc, regression-tests/test-bmi.toc, regression-tests/hash-map-regressions.toc,
regression-tests/regression-tester.toc, new.c. Read the BMI section of runtime3.c
(~lines 1920–2460) as the C reference for expected behavior.

You are one iteration of a Ralph loop. Do exactly one thing: implement the next
unchecked test case in the plan's "A. bmiCopyAssoc integration" checklist, in
checklist order. Do not work on the other checklist groups, do not wrap new C
functions in hvm-core.toc, and do not start any other work.

First check `git status`: if a previously interrupted iteration left uncommitted
changes, either finish that test case properly or revert them before starting.

If `CHash` is not yet defined in regression-tests/test-bmi.toc, add the
controlled-hash deftype from the plan's "Prerequisite" section first — include
it in the same commit as the first test case.

Then, for the next unchecked case:
1. Add the test to main() in regression-tests/test-bmi.toc, per the case spec
   in the plan. Use CHash keys with the plan's hash values so bit positions are
   pinned. Follow the existing test style (rt/test with _FILE_ _LINE_).
2. Build and run with `make test-bmi` (regenerates test-bmi.c, builds, runs,
   rewrites test-bmi.rslt). Do not rebuild the compiler (toccata/new-toc); if
   a compiler binary is missing, respond STUCK.
3. Verify, in the test output:
   - "BMI tests are good" is printed (every rt/test passed)
   - `malloc count: N  free count: N  diff: 0` — any non-zero diff is a memory
     leak and a failure
   - `remaining nodes: 0` — any non-zero value is a node leak and a failure
   - `git diff regression-tests/test-bmi.rslt` shows only execution-stat
     changes (ITRS, node/malloc counts) — no changed or missing result lines
4. If the test fails, debug and fix it. A failure exposing a real bug in
   hvm-core.toc is in scope to fix — and if you modify hvm-core.toc, run
   `make tests` first to confirm no other regression test regressed. A failure
   that requires changing runtime3.c (the C reference) is out of scope: STUCK.
5. Mark the case [x] in docs/hash-map-plan-toccata.md.
6. Commit with a message naming the case (e.g. "test: test-bmi bmiCopyAssoc
   same-key-different-value (bmiClone branch)"). Stage only the files you
   changed (test-bmi.toc, test-bmi.rslt, docs/hash-map-plan-toccata.md, and
   hvm-core.toc if you fixed a bug there).

If you hit a failure you cannot fix within the above rules, respond with
"STUCK: <one-line reason>" and stop.

Only when every case in group A is [x], `make test-bmi` passes all checks, and
everything is committed, respond with "All bmiCopyAssoc cases done".
