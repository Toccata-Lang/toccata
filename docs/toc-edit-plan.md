# toc-edit Implementation Plan

Build plan for the structural editor specified in `docs/toc-edit-spec.md`.
The spec is the design; this document is the ordered build checklist.

Protocol: work top to bottom, **one item per Ralph iteration**; tick an
item off only when its *Verify* step passes, then commit. Context for every
item: this file + `docs/toc-edit-spec.md` + AGENTS.md. 22 items, each sized
for a single iteration.

Role split: all structure comes from `ast-json`. `new-toc` has no bearing
on the editor except as the verification gate — edited files must compile
correctly using `new-toc` (and `check` is that same gate run on an
unmodified file). Nothing about path addressing, spans, or edit mechanics
depends on `new-toc`.

Ground truth verified before planning (2026-09-12):

* `./ast-json <file.toc>` exists and emits the JSON AST with byte spans on
  every node, matching the spec's contract (comments included as
  first-class nodes).
* `./new-toc <file.toc>` (verification gate only) prints C to stdout and
  diagnostics to stderr. Error case: exit 134 with
  `*** Error at <file>: <line>; <msg>` on stderr. Clean library load
  (no `main`): also exit 134, but with `*** Loaded <file>` and no error
  line. Per the new-toc diagnostics rules, the pass/fail signal is the
  stderr content, never the exit code.
* `tools/` does not exist yet; nothing here touches `toccata.c`/`core.c` or
  the toccata target.

Conventions used in the checklist below:

* `T` = `tools/toc-edit/toc_edit.py` (run from the repo root as `./T`).
* `F` = `tools/toc-edit/tests/fixture.toc` (created in item 0.3).
* "one-liner" = a `python3 -c` command that imports `toc_edit`
  (`sys.path.insert(0, 'tools/toc-edit')`) and asserts on its functions.
* "a copy of F" = `cp F` to a scratch name; never mutate `F` itself.

## Checklist

### Phase 0 — skeleton and plumbing

- [x] **0.1 Skeleton file.** Create `tools/toc-edit/toc_edit.py`:
      `#!/usr/bin/env python3`, executable, pure stdlib, with argparse
      subcommands `check` / `show` / `line` / `insert` / `replace` /
      `delete` that each print `not implemented` and exit 2 (distinguished
      from argparse usage errors by the message).
      *Verify:* `ls -l` shows the exec bit; `./T` prints usage and exits
      nonzero; `./T check` prints `not implemented` and exits 2.
- [x] **0.2 CLI argument validation.** Enforce: `insert` requires exactly
      one of `--before`/`--after`; `insert` and `replace` require
      `--from-file`; `show`/`line`/`delete` take exactly their positional
      args.
      *Verify:* `./T insert F 0` (no --before/--after) and
      `./T insert F 0 --before --after` both fail with a usage error
      (argparse exit 2, no traceback); `./T replace F 0` (no --from-file)
      fails the same way.
- [x] **0.3 Test fixtures.** First read `docs/toccata-style.md` (project
      rule for `.toc` files). Create `tools/toc-edit/tests/fixture.toc`
      covering: a top-level comment, a comment inside a form, nested
      exprs, a vector, a hash map, a string, a multi-line form, a form
      immediately preceded by a header comment, and a `defn` that is used
      by another top-level form (so deleting the `defn` is a rejection
      case). Plus `tests/bad.toc` that `new-toc` rejects.
      *Verify:* `./ast-json F` emits a JSON array whose node kinds include
      `comment`, `expr`, `vect`, `hash-map`, `string`;
      `./new-toc F 2>&1 >/dev/null` contains `*** Loaded` and no
      `*** Error` line; `./new-toc tools/toc-edit/tests/bad.toc
      2>&1 >/dev/null` contains an `*** Error` line.
      Note (verified 2026-09-12): `bad.toc` is a `cond` with no
      default — that produces the `*** Error at <file>: <line>; <msg>`
      format. But other rejections use other formats: an undefined
      symbol prints `*** Undefined symbol: ... at <file>: <n>` (no
      `*** Error`), a type clash prints `***  Conflicting assertions
      ...`. Item 0.5's `classify` as specified (matches only
      `*** Error`) will classify those as `clean` — a design gap that
      breaks item 2.5b (delete the used `defn` → exit 3, whose stderr
      is `*** Undefined symbol`). Item 0.5 must report this gap
      (STUCK) rather than guess a broader match.
- [x] **0.4 `run_ast_json(file)`.** Returns the parsed JSON AST (top-level
      list). Non-JSON output or a missing `ast-json` binary → clear error
      message, no traceback.
      *Verify:* one-liner: `run_ast_json(F)` returns a list; recursive
      walk asserts every node has `start < end` and every child's span is
      contained in its parent's span; the first top-level node's
      `src[start:end]` equals the fixture's first comment text (spot-check
      against the file's bytes); `run_ast_json('/nonexistent.toc')`
      produces a clean error, not a traceback.
- [ ] **0.5 `run_new_toc(file)` + `classify(stderr)`.** `run_new_toc`
      returns (exit code, stdout, full stderr — never discarded).
      `classify` is a pure function: `error` if stderr contains an
      `*** Error` line, `silent` if stderr is empty, else `clean`.
      *Verify:* one-liner: `classify('*** Error at x: 1; m') == 'error'`;
      `classify('*** Loaded x') == 'clean'`; `classify('') == 'silent'`;
      `run_new_toc(F)` → `classify(stderr) == 'clean'`;
      `run_new_toc('tests/bad.toc')` → `'error'`.
- [ ] **0.6 `resolve_path(ast, path)`.** Dot-separated integer indices
      into `children`. Bad path → distinct error naming the deepest
      existing prefix.
      *Verify:* one-liner on `run_ast_json(F)`: `'0'` resolves to the
      first node (a comment); a path to a known nested node resolves to a
      node whose span text matches the expected source slice; `'0.5'` (or
      any too-long index) raises the distinct error mentioning the
      deepest prefix.
- [ ] **0.7 `check` subcommand.** Runs `./new-toc` on the file, prints
      stderr, exits `1` on `error`, `0` otherwise.
      *Verify:* `./T check F` exits 0; `./T check tests/bad.toc` exits 1
      and the printed output contains the `*** Error` line.

### Phase 1 — read-only navigation

- [ ] **1.1 `show` subcommand.** Prints the node's path, kind, span
      `[start, end)`, verbatim text `file[start:end]`, and its children
      with their paths.
      *Verify:* `./T show F <path-to-nested-expr>` prints that path, the
      correct kind, a span whose byte slice (checked with a `python3 -c`
      slice of the fixture) matches the printed text, and one line per
      child with a path that round-trips: feeding each printed child path
      back to `show` prints the same span.
- [ ] **1.2 `line` subcommand.** Finds the innermost node whose span
      contains the byte offset of line `n` (1-based, from source bytes);
      prints path, kind, span, text. Clean error (nonzero exit, message,
      no traceback) for out-of-range lines or lines in unowned
      whitespace.
      *Verify:* `./T line F <line-inside-nested-form>` prints the inner
      node (path longer than the top-level one at that line);
      `./T line F 9999` and `./T line F <blank-line>` both fail cleanly.

### Phase 2 — mutation with validate-then-write

- [ ] **2.1 `apply_edit(file, new_bytes)` success path.** Writes
      `new_bytes` to a temp file **in the same directory as the original**
      (so the rename is atomic), runs `new-toc` on it, and on `clean`
      atomically renames over the original.
      *Verify:* one-liner on a copy of `F`: `apply_edit` with the
      identical bytes returns success; the copy is byte-identical
      (sha256 before/after); no `.rejected` file appears.
- [ ] **2.2 Rejection path.** On `error`: save the candidate as
      `<name>.rejected` beside the original (overwriting any earlier
      one), print `new-toc`'s stderr to the tool's stderr (do not save
      it), return the rejection result, original untouched.
      *Verify:* one-liner on a copy of `F`: an edit `new-toc` rejects —
      use a parse-error snippet such as `(add 1` as the candidate (a
      guaranteed rejection) → copy's sha256 unchanged; `<name>.rejected`
      exists and its contents equal the candidate bytes; a second
      rejection overwrites the `.rejected` file; the captured stderr
      contains the `*** Error` line.
- [ ] **2.3 Silent-crash retry loop.** A pure function taking an
      attempt function: up to 5 attempts; an attempt returning `error`
      stops immediately (normal rejection); all-silent → `unverified`.
      *Verify:* one-liner with a fake attempt fn: always-`silent` →
      exactly 5 calls, result `unverified`; `silent, silent, error` →
      exactly 3 calls, result `error`; `clean` on first call → 1 call.
- [ ] **2.4 `replace` end-to-end.** Splices the snippet file's bytes at
      the target's `[start, end)` and wires it through `apply_edit` + the
      retry loop: success → 0; rejection → 3; all-silent → 4 with a loud
      "edit UNVERIFIED" message and no rename.
      *Verify:* on a copy of `F`: (a) replace a form preceded by a header
      comment with a valid one-line snippet → exit 0, result equals
      `src[:start] + snippet + src[end:]` byte-for-byte (one-liner), the
      header comment is still present above the new form, `./T check`
      exits 0; (b) a parse-error snippet → exit 3, original byte-identical,
      `.rejected` present, `*** Error` line printed; (c) one-liner that
      monkeypatches the module's `run_new_toc` to always return
      `silent` → exit 4, "UNVERIFIED" printed, original byte-identical.
- [ ] **2.5 `delete` end-to-end.** Removes the target's `[start, end)`,
      wired through `apply_edit` + retry.
      *Verify:* on a copy of `F`: (a) delete a form preceded by a header
      comment → exit 0, result equals `src[:start] + src[end:]`
      byte-for-byte, the header comment survives (orphaned), `./T check`
      exits 0; (b) delete the `defn` that another top-level form uses →
      exit 3, original byte-identical, `.rejected` present.
- [ ] **2.6 `insert` end-to-end.** Splices the snippet at the target's
      `start` (`--before`) or `end` (`--after`), wired through
      `apply_edit` + retry.
      *Verify:* on copies of `F`: `--before` yields `src[:start] +
      snippet + src[start:]` and `--after` yields `src[:end] + snippet +
      src[end:]` byte-for-byte, each exit 0 and still passing `check`; a
      parse-error snippet → exit 3, original byte-identical.

### Phase 3 — cross-cutting verification

- [ ] **3.1 Untouched-byte guarantee.** For each of replace/delete/insert,
      confirm only the target span changed.
      *Verify:* one-liner per subcommand: sha256 of the prefix before the
      edit point and the suffix after it are identical before/after the
      edit.
- [ ] **3.2 Orphaned-comment end-to-end.** Delete a form, then delete its
      now-orphaned header comment as a second explicit edit.
      *Verify:* on a copy of `F`: first `delete` exits 0 and the comment
      remains; `show` the preceding sibling (a comment); second `delete`
      on that comment's path exits 0; final bytes equal the expected
      slice; `check` exits 0.
- [ ] **3.3 Idempotence.** Replacing a node with its own text is a no-op.
      *Verify:* on a copy of `F`: write a node's own span text to a
      snippet file, `replace` with it, exit 0, copy byte-identical
      (sha256).
- [ ] **3.4 No TTY dependence.** Every subcommand works with stdin closed.
      *Verify:* run one invocation of each of the six subcommands with
      `</dev/null`; all behave identically to interactive runs (same
      exit codes as in items 0.7/1.1/1.2/2.4).

### Phase 4 — README and finish

- [ ] **4.1 README: required content.** `tools/toc-edit/README.md` with
      the schema-drift line, the orphaned-comment `delete` warning, and
      the exit-code table.
      *Verify:* `grep -F "suspect the AST dump schema first" README.md`;
      `grep -i "orphan" README.md` (warning mentions showing the
      preceding sibling and a second edit); the table lists 0, 1, 3, 4
      with the spec's meanings (`.rejected` for 3, unverified for 4).
- [ ] **4.2 README: usage examples.** One example per subcommand
      (check/show/line/insert/replace/delete); every example must be
      copy-paste runnable from the repo root against a copy of `F`, with
      the expected exit code stated.
      *Verify:* run each of the six example commands exactly as written;
      each produces the exit code stated in the example.
- [ ] **4.3 Final pass.** Exec bit set; nothing outside
      `tools/toc-edit/` added or modified; no Makefile references; no
      `toccata.c`/`core.c` involvement.
      *Verify:* `git status --porcelain` shows changes only under
      `tools/toc-edit/` (and this plan doc); `grep -rn toc-edit Makefile`
      is empty; `ls -l tools/toc-edit/toc_edit.py` shows the exec bit.

## Out of scope (per spec)

* No in-form token editing — write a snippet file and use `replace`.
* No Python parser, no golden files, no Makefile involvement.
