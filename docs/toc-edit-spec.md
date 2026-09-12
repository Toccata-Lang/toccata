# toc-edit: structural editing tool for .toc files

Spec for a Python CLI that performs structural edits on Toccata source files,
backed by the `ast-json` AST dump. Decided by design review; this document is
the complete spec — no further design conversation is required to build it.

## Architecture

* There is **no Python parser**. `ast-json <file-name>` (no other arguments)
  emits a JSON AST with
  **byte spans (start/end offsets) on every node** — forms and comments alike.
  All structure comes from the real C parser; nothing is re-derived.

* Edits are **span surgery**: the tool splices raw text at the target node's
  byte span. Untouched bytes are untouched by construction — no
  re-serialization, no reformatting, no normalization. Comments, blank lines,
  and inline C strings survive verbatim.

* The AST dump C code is owned by the compiler work. There are **no golden
  files**. Insurance against schema drift: if `toc_edit` dies with a Python
  traceback on a file that `new-toc` accepts, suspect the AST dump schema
  first.

## AST dump contract

The `ast-json <file-name>` JSON the tool consumes. Hard requirements —
deviating from any of them is a schema-drift bug:

1. **Top level is a JSON array of nodes in file order.** Index *i* is path
   `i`. Comments are in this array — first-class nodes, ordered by position.
2. **Every node has `start` and `end`: byte offsets into the source file,
   half-open `[start, end)`** (`end` is one past the node's last byte; no
   zero-length or inverted spans). `file[start:end]` is exactly the node's
   text. Form parentheses are part of the `expr` span, not separate nodes.
   Siblings need not be contiguous — whitespace between them is unowned.
3. **Every node has `children`: an array of child nodes in file order**
   (`[]` for leaves). Path `a.b` = child `b` of node `a`; that is the entire
   addressing scheme.
4. **Every node has `kind`: a string, one of `expr`, `symbol`, `integer`,
   `string`, `comment`, `vect`, `hash-map`.** `expr` covers every prefix form,
   including top level. The tool treats `kind` as opaque (display only) and
   never branches on it, so kinds may be renamed or extended freely.

Not required (and not consumed): line/column numbers, embedded source text,
token streams, whitespace positions, attached-comment concepts. A comment
inside a form is a child of the innermost node containing it, in file order.

Worked example. Source:

```
// add two numbers
(add (vect-map (range 3) (fn [x] x)))
```

AST:

```json
[
  { "kind": "comment", "start": 0, "end": 22, "children": [] },
  {
    "kind": "expr", "start": 22, "end": 60,
    "children": [
      { "kind": "symbol", "start": 23, "end": 26, "children": [] },
      {
        "kind": "expr", "start": 27, "end": 59,
        "children": [
          { "kind": "symbol", "start": 28, "end": 38, "children": [] },
          {
            "kind": "expr", "start": 39, "end": 48,
            "children": [
              { "kind": "symbol", "start": 40, "end": 45, "children": [] },
              { "kind": "integer", "start": 46, "end": 47, "children": [] }
            ]
          },
          {
            "kind": "expr", "start": 49, "end": 58,
            "children": [
              { "kind": "symbol", "start": 50, "end": 52, "children": [] },
              { "kind": "vect", "start": 53, "end": 56, "children": [
                { "kind": "symbol", "start": 54, "end": 55, "children": [] }
              ] },
              { "kind": "symbol", "start": 57, "end": 58, "children": [] }
            ]
          }
        ]
      }
    ]
  }
]
```

(`file[22:60]` = `(add (vect-map (range 3) (fn [x] x)))`; `file[0:22]` = the
comment including its newline; `file[53:56]` = `[x]`.)

## Scope

* **(a) check** — parse/validate a file.
* **(b) navigate** — `show` a node by path; find the node containing a line.
* **(c) whole-node edit** — `insert` / `replace` / `delete` a node by path;
  replacement text is supplied via `--from-file`.
* **(d) is out of scope** — no in-form token editing. In-form changes are made
  by writing a replacement snippet file and using `replace`.

## CLI

```
tools/toc-edit/toc_edit.py check <file.toc>
tools/toc-edit/toc_edit.py show <file.toc> <path>
tools/toc-edit/toc_edit.py line <file.toc> <n>
tools/toc-edit/toc_edit.py insert <file.toc> <path> --before|--after --from-file <snippet>
tools/toc-edit/toc_edit.py replace <file.toc> <path> --from-file <snippet>
tools/toc-edit/toc_edit.py delete <file.toc> <path>
```

* **Paths** are integer indices into the AST dump's child order,
  dot-separated: top-level nodes are `0`, `1`, `2`, … in file order
  (comments included, since they are nodes); `42.3` is the 4th child of
  top-level node 42. The tool prints paths in its output (`line`, `show`) so
  paths are never guessed.
* `insert` requires `--before` or `--after`.
* Replacement/insertion text always comes from a file — never inline on the
  command line (shell-quoting C strings is the original failure mode this
  tool exists to kill).

## Semantics

* **Comments are position-anchored, never attached to forms.** Replacing a
  form leaves any comment above it in place, now above the new form.
* **Deleting a form does not delete its comment.** An orphaned header comment
  must be deleted as a separate, explicit edit. (In `hvm-core.toc` those
  comments are long and load-bearing; the README must warn: when deleting a
  form, `show` the preceding sibling first, and if it is a comment describing
  the deleted form, delete it as a second edit.)
* Deleting a comment is an ordinary explicit `delete` — comments are first-class
  path targets like any other node.

## Failure handling

* **Validate-then-write.** Mutating operations edit a temp file, run
  `new-toc` on the temp, and atomically rename over the original only on
  success. The original is never clobbered.
* After every mutating operation the tool shells out to `new-toc` and reads
  stderr fully — never discards it.
* Failure modes and exit codes:
  * `0` — success.
  * `1` — `check` failed: `new-toc` reported an error (message shown).
  * `3` — edit rejected: `new-toc` reported an error on the edited file.
    The original is untouched; the failed candidate is saved as
    `<name>.rejected` beside the original (`hvm-core.toc` →
    `hvm-core.rejected`), overwriting any earlier `.rejected`; `new-toc`'s
    stderr is **printed** to the tool's stderr but **not saved** to a file
    (it is cheap to recreate).
  * `4` — silent crash: `new-toc` aborted with no stderr output, 5 attempts
    total. The edit is **unverified**; the tool says so loudly. (Per the
    new-toc diagnostics rules: a retry that produces an error message is
    treated as a normal rejection, not retried further.)
* Distinct exit codes per failure mode so a non-interactive caller can report
  the exact kind of intervention needed. The tool never depends on a TTY and
  never prompts; intervention is the artifact (`.rejected`) plus the exit
  code plus the printed diagnostics.

## Placement and build

* New `tools/` directory in the repo.
* `tools/toc-edit/toc_edit.py` — the main tool file, **executable**
  (`#!/usr/bin/env python3`, `chmod +x`), invoked directly.
* Pure Python 3.14 stdlib. No pip dependencies, no venv.
* No Makefile involvement; nothing the toccata build knows about.
* Never touches `toccata.c`, `core.c`, or the toccata target.

## README requirements (tools/toc-edit/README.md)

* The schema-drift line: "If `toc_edit` crashes with a Python traceback on a
  file that `new-toc` accepts, suspect the AST dump schema first."
* The orphaned-comment warning for `delete` (see Semantics).
* The exit-code table.
