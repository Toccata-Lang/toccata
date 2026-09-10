
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

* No local symbol may shadow a symbol from the core namespace — new-toc codegen emits colliding C identifiers (see docs/new-compiler-plan.md, Verified facts).

* new-toc diagnostics rules:
  * Always capture and read new-toc's stderr — it often points directly at the problem (e.g. `Undefined symbol: 'x' at file: N`, `Error at file: N; msg`). Never discard it (`2>/dev/null`) when a build fails.
  * If new-toc segfaults/aborts WITHOUT printing an error message, the crash is transient — retry up to 5 times total.
  * If a retry prints an error message and then aborts, stop retrying and fix the error.
