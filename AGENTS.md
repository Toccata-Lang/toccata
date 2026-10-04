
Only do what you are explicitly told to and nothing else.

If you are told to show something, then only show it. Do not take any further actions.

If you are told to create a file, then only create it. Do not try to execute it unless told to.

* You are an amazing software developer. Here are some facts you need to know about this project

* Never use "sudo" to run any command. That is explicitly forbiddin. You do not have "sudo" access.

* Never make the toccata target! That is for me to do when needed.

* **NEVER touch `toccata.c` or `core.c`.** Do not edit them, do not compile them
  standalone (no `clang -c core.c`, no `clang -c toccata.c`, no object files,
  no `nm` on them), do not delete, move, or regenerate them, and do not include
  them in any probe or experiment. Full stop. No exceptions, no "just to check"
  invocations. If a task seems to require it, stop and ask the owner.

* Before writing or editing any `.toc` file, read docs/toccata-style.md and follow it.

* All changes to toccata code (`.toc` files) are made with the structural editing tool
  (`tools/toc-edit/toc_edit.py`, run from the repo root; copy-paste examples in
  `tools/toc-edit/README.md`) — not by hand-editing node text. It performs
  whole-node span surgery (untouched bytes stay untouched) and is
  validate-then-write: the candidate is run through `new-toc` and the original
  is atomically replaced only on success, so editing in place is safe and the
  original is never clobbered. Workflow:
  1. `check <file>` first — work only on a file that passes (exit 0).
  2. Find the target: `line <file> <n>` gives the innermost node on a line;
     `show <file> <path>` prints a node's path, kind, span, verbatim text, and
     child paths. Verify the shown text is what you expect before editing.
  3. Before inserting any new snippet of toccata code, validate it: write
     the snippet to its own file and run `check` on that file — only splice
     a snippet that passes (exit 0; the missing-main abort is the normal
     library-load path and is fine). Only then edit with `insert` (splice a
     snippet at the node's start with `--before` or its end with `--after`),
     `replace` (splice over the node's span), or `delete` (remove the node's
     span). Snippet text goes in a file passed via `--from-file`.
  4. There is no in-form token editing: to change part of a form, `replace`
     the whole node with a snippet containing the full new text.
  5. `delete` leaves a preceding header comment behind (comments are
     position-anchored, never attached to forms). `show` the preceding sibling;
     if it describes the deleted form, delete it as a second explicit edit.
  6. `check <file>` after the edit.
  Exit codes: 0 success; 1 `check` failed; 3 edit rejected — original
  untouched, the failed candidate saved as `<name>.rejected` beside it,
  `new-toc`'s stderr printed (read it — it says why); 4 unverified — `new-toc`
  crashed silently after 5 attempts, file NOT modified, re-run the edit.
  If a node's shown text doesn't match its span (or spans look wrong in any
  way), suspect the `ast-json` dump first — stop and report it, don't work
  around it. Files must be pure ASCII (docs/toccata-style.md): multi-byte
  UTF-8 characters corrupt the dump's byte spans from that character to EOF.

* When trying to fix a syntax error in a `.toc` file (unbalanced or
  mis-nested parens/brackets, missing tokens), use the structural
  editing tool (`tools/toc-edit/toc_edit.py`) — not hand-edits of node
  text. Verify with a STACK-BASED nesting check, never a count-based
  one: on 2026-09-26 a hand-written `.toc` probe carried a `)`
  mis-nested inside a `[]` that a count check passed, and the whole
  bisection built on it had to be retracted (docs/parser-generator-
  plan.md, item-10 as-built note UPDATE 8). Note: `check` must pass
  (exit 0) before the tool will work on a file — it cannot repair a
  file that fails `new-toc`; for such files do the minimal hand edit
  and say so.

* Paren/quote-heavy lines: rely on `toc_edit.py` for structural edits
  (it splices verbatim spans — no escaping involved). When the tool
  is unavailable (the file fails `check`) and a hand edit is
  required, do NOT use the edit tool's text matching on lines dense
  with nested quotes and parens — the escaping is error-prone (on
  2026-09-27 two consecutive edit calls on such a line removed the
  wrong paren and added a stray quote, each needing an od-dump
  corrective pass). Instead do a BYTE-EXACT rewrite: a small Python
  script that locates the line (by a unique plain-text substring) and
  replaces the whole line with the desired bytes, then re-run the
  stack-based nesting check.

* Core-API gotchas that produced three consecutive bogus
  "toolchain crash" diagnoses on 2026-09-26 — CHECK THESE BEFORE
  blaming new-toc: (1) `get` on a Vector returns a MAYBE (`Some
  element` / `None`), not the bare element — use `(extract (get v
  i))`; a raw Maybe compared to a bare value is silently `no`/false,
  and `type-name` printing `Some` is CORRECT, not corruption. (2)
  `+` takes EXACTLY 2 args — `(+ a b c)` is invalid source and
  makes new-toc abort (silently, or with `Wrong number of args for
  '+'`). Both are in docs/toccata-style.md (Core API); read that
  file before writing any probe (as the rule above already
  requires).

* No local symbol may shadow a symbol from the core namespace — new-toc codegen emits colliding C identifiers (see docs/new-compiler-plan.md, Verified facts).

* new-toc diagnostics rules:
  * Always capture and read new-toc's stderr — it often points directly at the problem (e.g. `Undefined symbol: 'x' at file: N`, `Error at file: N; msg`). Never discard it (`2>/dev/null`) when a build fails.
  * Shell capture trap (owner-confirmed bogus-test incident, 2026-09-10): `out=$(cmd > /dev/null 2>&1)` sends stderr to /dev/null TOO (redirections apply left-to-right), so `$out` is always empty — grepping it "proves" nothing and misdiagnoses clean runs as silent crashes. Capture stderr to a file (`cmd > /dev/null 2>/tmp/err.txt`) or pipe it straight into the grep; before drawing ANY conclusion from a capture loop, verify the captured output is non-empty (e.g. print its line count once). Corollary: for new-toc LIBRARY loads (no main), exit 134 is a NORMAL clean-load exit code (the missing-main abort path) — the sole pass/fail signal is the `*** Loaded <file>` line in actually-captured stderr, never the exit code, never an empty capture.
  * If new-toc segfaults/aborts WITHOUT printing an error message, the crash is transient — retry up to 5 times total.
  * If a retry prints an error message and then aborts, stop retrying and fix the error.
