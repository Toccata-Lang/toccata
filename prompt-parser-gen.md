# Ralph loop prompt — parser generator (grammar data → recursive-descent reader)

You are one iteration of an automated loop implementing the **parser
generator** project: a Toccata program, built by the existing `new-toc`
compiler, that takes the parser-combinator grammar (data) and emits a
recursive-descent parser module in Toccata source for that grammar.
You have no memory of previous runs — the plan file and the git
history are your memory.

This is a separate loop from the compiler plan's loop (`prompt.md` /
`docs/new-compiler-plan.md`). Work only on
`docs/parser-generator-plan.md`; never check or edit boxes in
`docs/new-compiler-plan.md`.

## Read first (every run, in this order)

1. `AGENTS.md` — project rules. They override everything in this file.
2. `docs/parser-generator-plan.md` — the complete settled design, and
   the **"Ralph loop — task list"** section. The task list is the
   single source of truth for what to do next.
3. `git log --oneline -15` — what previous runs did.

## What to do this run

1. Open the task list and find the **first unchecked item**.
2. If there is no unchecked item: print `ALL ITEMS COMPLETE` and stop.
   Do no other work.
3. If the first unchecked item is **item 1 (OWNER DECISION)**: do not
   implement it. Print `STOP POINT REACHED (item 1)`, then a brief
   state report (what exists, what builds, and the specific open
   questions: the result-discrimination pattern for the generated
   code's site shapes (a)–(d) — options A/B/C as listed in the plan's
   Open items — and the emitter's `Many` fast/slow classification
   ruling), and stop. The owner decides those.
4. Otherwise: implement **exactly that one item**.
   - Follow the settled design in the plan file. Do not redesign. Do
     not edit the task items themselves — only check the box.
   - If you hit a genuine design gap the plan does not cover: stop,
     report the gap precisely, and do not guess a design.
   - Verify the item's "done when" criterion for real — run the
     builds, run the drivers, build and run the generated module, diff
     the outputs. A checked box you have not verified is a lie.
   - Record any durable fact you verified this run — a `new-toc`
     coding technique or hazard, a build/test quirk, a generated-code
     shape that surprised you — in the plan's **"Inherited verified
     facts"** section, date-stamped, so a future run does not
     re-derive it. If the fact also concerns the compiler project
     (e.g. a `new-toc` hazard hit while building the emitter), append
     it to `docs/new-compiler-plan.md`'s **"Verified facts"** as well
     — append-only; never rewrite that file's other content.
   - Check the box in `docs/parser-generator-plan.md`.
   - Commit your work (code + checkbox + any plan notes) with a
     message starting `parser-gen item N: <what changed>`.
5. Do **not** start the next item in the same run.

## Standing constraints

- One item per run. Small, correct, committed steps beat big leaps.
- Never use `sudo`. Never make the `toccata` Makefile target.
- The emitter / grammar / driver sources may use inline C freely.
- `new-toc` is a build crutch — its behavior is not a design
  constraint, but everything you write *and* everything you generate
  must compile and run under it.
- Builds must be clean: zero malloc/free diff, zero remaining nodes.
- Real sources live in `interpreter/`; throwaway probes in `scratch/`
  (never committed from there). `interpreter/gen-rdr.toc` is a build
  artifact written by the driver — never commit it.
- Do not edit the hand-written `interpreter/intrp-rdr.toc` (the
  phase-1 reader) or `hvm-core.toc`. If an item seems to require
  changing either, that is a STUCK/owner matter, not a fix.
- **Persist verified facts.** The plan's **"Inherited verified facts"**
  section is the durable memory for discoveries: `new-toc` coding
  techniques and hazards, build/test quirks, generated-code shapes.
  Append anything you verify this run, date-stamped, before you
  commit. Save only reusable facts — not transient state (current test
  status, work-in-progress). A fact already recorded there is settled:
  trust it rather than re-checking, unless you have reason to doubt it.
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
   `STOP POINT REACHED (item 1)` / `STUCK: <reason>` — sentinels on
   their own line).
2. What you changed (files).
3. How you verified the "done when" criterion — the actual commands
   and the key output lines.
4. What the next run should pick up.
