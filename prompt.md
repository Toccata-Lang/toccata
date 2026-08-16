Read the hash map plans (docs/hash-map-plan-c.md and docs/hash-map-plan-toccata.md) and the files they reference,
including skills/convert-test-to-strings.md.

You are one iteration of a Ralph loop. Only do the test-conversion task:
the plan's checklist, in order of appearance in main(). Do not start the
protocol wiring or any other work described in the plan.

First check `git status`: if a previously interrupted iteration left
uncommitted changes, either finish that conversion properly or revert them
before starting.

Convert the next unchecked test, following the skill's workflow exactly
(one key/value at a time, verified after each). When it passes its 1000-run
final regression check, mark it [x] in the plan and commit (you may commit
despite what the skill says; stage only the files you changed).

If you hit a failure you cannot fix (e.g. a pre-existing failure the skill
says to stop on), respond with "STUCK: <one-line reason>" and stop.

Only when every checklist item is [x], all tests are uncommented in main(),
the full 1000-run regression passes with all tests enabled, and everything
is committed, respond with "All tests converted".
