Read the hash-map Toccata plan (docs/hash-map-plan-toccata.md) — especially
the "Roadmap: Remaining Toccata-Level Work (Ralph loop tasks)" section and
its Conventions, plus the Lessons section — and the files it references:
hvm-core.toc, regression-tests/test-bmi.toc (and test-array-node.toc /
test-collision-node.toc once they exist), regression-tests/regression-tester.toc,
the Makefile (REG_TESTS). Before wrapping anything, read the C reference
function the task cites (line numbers are in the task) in runtime3.c.

You are one iteration of a Ralph loop. Do exactly one thing: implement the
next unchecked task in the plan's "Roadmap: Remaining Toccata-Level Work"
section, in list order. Do not skip ahead, do not work on other tasks, and do
not start any other work. Do not run or modify regression-tests/
hash-map-regressions.toc — it is the owner's final integration suite (reading
it as a spec is fine where a task says so).

First check `git status`: if a previously interrupted iteration left
uncommitted changes, either finish that task properly or revert them before
starting.

Then, for the next unchecked task:
1. Do what the task says. Mirror the bmiCopyAssoc/bmiMutateAssoc pair: C for
   the mechanical parts, protocol =/sha1 for the semantic parts, owned refs
   from wrappers (see Lessons).
2. Build and run every suite you touched or added — `make test-bmi`,
   `make test-array-node`, `make test-collision-node` (as applicable; each
   regenerates its .c, builds, runs, rewrites its .rslt). Do not rebuild the
   compiler (toccata/new-toc); if a compiler binary is missing, respond STUCK.
3. Verify, in each suite's output:
   - the suite's success line is printed (every rt/test passed)
   - `malloc count: N  free count: N  diff: 0` — any non-zero diff is a memory
     leak and a failure
   - `remaining nodes: 0` — any non-zero value is a node leak and a failure
   - `git diff regression-tests/<suite>.rslt` shows only execution-stat
     changes (ITRS, node/malloc counts) — no changed or missing result lines
   - you modified hvm-core.toc → run `make tests` to confirm no other
     regression test regressed
   - you modified runtime3.c → additionally run `make test-hash-map` (the C
     suite)
4. If a test fails, debug and fix it. A memory error — a leak (non-zero diff
   or remaining nodes) or a double free (abort "failure in decRefs, refs too
   small" from dec_and_free) — use skills/memory-leak-hunting.md to hunt it
   down. A failure exposing a real bug in hvm-core.toc is in scope to fix. A
   failure that requires changing runtime3.c is out of scope unless the task
   explicitly says so (or it is a leak fix): STUCK.
5. Mark the task [x] in docs/hash-map-plan-toccata.md; update the BMI Surface
   / State lines if the exposed surface changed.
6. Commit with a message of the form `task N: <short name> — <what/why>`.
   Stage only the files you changed.

If you hit a failure you cannot fix within the above rules, respond with
"STUCK: <one-line reason>" and stop.

Only when every task in the roadmap is [x], `make tests` passes all checks,
and everything is committed, respond with "Roadmap complete".
