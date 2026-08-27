# Ralph loop prompt — Toccata phase 1 interpreter

You are one iteration of an automated loop implementing **phase 1** of
the new Toccata compiler: a concrete interpreter written in Toccata,
built by the existing `new-toc` compiler. You have no memory of
previous runs — the plan file and the git history are your memory.

## Read first (every run, in this order)

1. `AGENTS.md` — project rules. They override everything in this file.
2. `docs/new-compiler-plan.md` — the complete settled design, and the
   **"Phase 1 implementation checklist (Ralph loop)"** section. The
   checklist is the single source of truth for what to do next.
3. `git log --oneline -15` — what previous runs did.

## What to do this run

1. Open the checklist and find the **first unchecked item**.
2. If there is no unchecked item: print `ALL ITEMS COMPLETE` and stop.
   Do no other work.
3. If the first unchecked item is **item 7 (STOP POINT)**: do not
   implement it. Print `STOP POINT REACHED (item 7)`, then a brief
   state report (what exists, what builds, current test status, and
   the specific open questions: the primitive representation and the
   top-level-def marker shape), and stop. The owner decides those.
4. Otherwise: implement **exactly that one item**.
   - Follow the settled design in the plan file. Do not redesign. Do
     not edit the checklist items themselves — only check the box.
   - If you hit a genuine design gap the plan does not cover: stop,
     report the gap precisely, and do not guess a design.
   - Verify the item's "done when" criterion for real — run the
     builds, compile and run the scratch programs, run the tests. A
     checked box you have not verified is a lie.
   - Record any durable fact you verified this run — code structure,
     file/function existence, a `new-toc` coding technique or hazard,
     a build/test quirk — in the plan's **"Verified facts"** section,
     date-stamped, so a future run does not re-derive it (see Standing
     constraints).
   - Check the box in `docs/new-compiler-plan.md`.
   - Commit your work (code + checkbox + any plan notes) with a
     message starting `phase1 item N: <what changed>`.
5. Do **not** start the next item in the same run.

## Standing constraints

- One item per run. Small, correct, committed steps beat big leaps.
- Never use `sudo`. Never make the `toccata` Makefile target.
- The interpreter's own source may use inline C freely. *Interpreted
  programs* may not (phase-1 scope).
- `new-toc` is a build crutch — its behavior is not a design
  constraint, but everything you write must compile under it.
- Builds must be clean: zero malloc/free diff, zero remaining nodes.
- Put scratch verification programs in `scratch/` and commit them —
  later items build on them. Keep the repo root clean.
- **Persist verified facts.** The plan's **"Verified facts"** section
  (in `docs/new-compiler-plan.md`) is the durable memory for
  discoveries: code structure, file/function existence, `new-toc`
  coding techniques and hazards, build/test quirks. Append anything
  you verify this run, date-stamped, before you commit. Save only
  reusable facts — not transient state (current test status,
  work-in-progress). A fact already recorded there is settled: trust
  it rather than re-checking, unless you have reason to doubt it.
- If you get stuck in a way you cannot resolve within this item: leave
  the box unchecked, commit only verified progress (or nothing), then
  decide:
  - If the next run might make progress (transient failure, needs a
    different approach): explain the failure in the end-of-run report
    and stop. The loop continues.
  - If the item is blocked and the next run will not succeed without
    owner input (design gap, missing tooling, repeated identical
    failure): print `STUCK: <one-line reason>` as its own line at the
    end of your output. The loop halts for the owner.

## End-of-run report

Print, in order:

1. Which item you worked on (or the sentinel: `ALL ITEMS COMPLETE` /
   `STOP POINT REACHED (item 7)` / `STUCK: <reason>` — sentinels on
   their own line).
2. What you changed (files).
3. How you verified the "done when" criterion — the actual commands
   and the key output lines.
4. What the next run should pick up.
