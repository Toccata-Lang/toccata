
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

* Structural edits to `.toc` files are made with the structural editing tool
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
  3. Edit with `insert` (splice a snippet at the node's start with `--before`
     or its end with `--after`), `replace` (splice over the node's span), or
     `delete` (remove the node's span). Snippet text goes in a file passed via
     `--from-file`.
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

* No local symbol may shadow a symbol from the core namespace — new-toc codegen emits colliding C identifiers (see docs/new-compiler-plan.md, Verified facts).

* new-toc diagnostics rules:
  * Always capture and read new-toc's stderr — it often points directly at the problem (e.g. `Undefined symbol: 'x' at file: N`, `Error at file: N; msg`). Never discard it (`2>/dev/null`) when a build fails.
  * Shell capture trap (owner-confirmed bogus-test incident, 2026-09-10): `out=$(cmd > /dev/null 2>&1)` sends stderr to /dev/null TOO (redirections apply left-to-right), so `$out` is always empty — grepping it "proves" nothing and misdiagnoses clean runs as silent crashes. Capture stderr to a file (`cmd > /dev/null 2>/tmp/err.txt`) or pipe it straight into the grep; before drawing ANY conclusion from a capture loop, verify the captured output is non-empty (e.g. print its line count once). Corollary: for new-toc LIBRARY loads (no main), exit 134 is a NORMAL clean-load exit code (the missing-main abort path) — the sole pass/fail signal is the `*** Loaded <file>` line in actually-captured stderr, never the exit code, never an empty capture.
  * If new-toc segfaults/aborts WITHOUT printing an error message, the crash is transient — retry up to 5 times total.
  * If a retry prints an error message and then aborts, stop retrying and fix the error.
