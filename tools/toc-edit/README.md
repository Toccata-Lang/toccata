# toc-edit

Structural editor for Toccata `.toc` files. `toc_edit.py` performs
whole-node edits (`insert` / `replace` / `delete`) and navigation
(`check` / `show` / `line`) on Toccata source, backed by the `ast-json`
AST dump (byte spans on every node). Edits are span surgery: the tool
splices raw text at the target node's byte span, so untouched bytes are
untouched by construction — no re-serialization, no reformatting.

Every mutating operation is validate-then-write: the candidate is
written to a temp file, run through `new-toc`, and atomically renamed
over the original only on success. The original is never clobbered.

## Schema drift

If `toc_edit` crashes with a Python traceback on a file that `new-toc`
accepts, suspect the AST dump schema first. There is no Python parser
and no golden file to blame — all structure comes from `ast-json`; a
traceback on a valid file means the dump no longer matches the contract
in `docs/toc-edit-spec.md`.

## Warning: `delete` leaves comments behind

Comments are position-anchored, never attached to forms. **Deleting a
form does not delete its comment.** An orphaned header comment must be
deleted as a separate, explicit edit. When deleting a form, `show` the
preceding sibling first, and if it is a comment describing the deleted
form, delete it as a second edit. In `hvm-core.toc` those comments are
long and load-bearing — an orphaned one is easy to miss.

## Exit codes

| Code | Meaning |
|------|---------|
| 0 | Success. |
| 1 | `check` failed: `new-toc` reported an error (message shown). |
| 3 | Edit rejected: `new-toc` reported an error on the edited file. The original is untouched; the failed candidate is saved as `<name>.rejected` beside the original (e.g. `hvm-core.toc` → `hvm-core.rejected`), overwriting any earlier `.rejected`; `new-toc`'s stderr is printed, not saved. |
| 4 | Silent crash: `new-toc` aborted with no stderr output, 5 attempts total. The edit is **unverified** — the tool says so loudly. The file was not modified. |
