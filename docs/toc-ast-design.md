# toc-ast Design

A count-prefixed s-expression format for *authoring* Toccata forms, plus a
serializer that turns it into plain Toccata. The format exists to remove a
specific failure class: hand-emitting balanced parens/brackets/braces across
many lines. In the prefixed format, every open form carries its own
completion condition (the count), so bracket matching is unnecessary — a
form ends when its count
is satisfied. Closing brackets may be written for readability but are
ignored.

Every error the format can produce is local (at or within one line of the
mistake), immediate (caught by the tool, not at EOF), and single-token
(a one-character fix). The one residual error — "declared 3, meant 4" —
serializes to *valid* structure (the 4th form becomes a sibling) and is
uncatchable mechanically; it stays a human-review item.

## The format

A prefixed form is an opening bracket, a count, and exactly that many
sub-forms. Nothing else.

```
form   := atom
        | string
        | open count form*

open   := '(' | '[' | '{'
count  := one or more decimal digits, immediately after `open` (no space)
string := a Toccata double-quoted string literal, lexed by the reader's
          rules (a single token, may contain spaces)
atom   := any run of characters that is not whitespace and does not begin
          with '(', '[', '{', ')', ']', '}', ';', or '"'
```

Rules, as decided:

1. **Uniform count semantics.** The count is the number of *forms* that
   follow, for all three bracket kinds. `{4` takes 4 forms (k1 v1 k2 v2);
   it is not a pair count. One parsing rule, one error message, no
   special case.
2. **Strict adjacency.** The first digit of the count is the very next
   character after the opening bracket. `( 3` is a grammar error
   ("expected count after '(', got ' '").
3. **Zero counts are legal.** `(0` → `()`, `[0` → `[]`, `{0` → `{}`.
   Required to represent `(|)`, `[]`, and `{}`.
4. **Closing brackets are ignored.** `)`, `]`, and `}` are skipped
   wherever they appear: no matching is checked, they carry no
   structure, and they never become part of an atom. Authors habitually
   write them, so they are tolerated rather than rejected.
5. **No comments.** `;` anywhere is a grammar error. Comments are
   position-anchored in Toccata and belong to the file, not to the form —
   they are handled by the existing toc-edit workflow.
6. **Brackets are always structural.** `(`, `[`, `{` start a form even
   mid-token (`foo(` lexes as atom `foo` plus a new form), matching
   Toccata's reader, so the tokenizer never re-tokenizes silently.
7. **ASCII only.** Non-ASCII input is a grammar error (`.toc` files must
   be pure ASCII; the prefixed input is a form of `.toc` text).

Examples:

```
(3 fn [1 x] (3 let [2 y (3 + x) 1]) y])
  → (fn [x] (let [y (+ x 1)] y))

{4 "a" 1 "b" 2}
  →  {"a" 1 "b" 2}

(1 |)          →  (|)
[0]            →  []
```

## Serialization

The serializer emits plain Toccata:

* **Newline-preserving.** Where the prefixed input has a newline between
  two elements, the output has a newline there; all other element
  separation is a single space. The author keeps control of line breaks in
  the source they write.
* **No map commas.** Map entries are separated like any other elements
  (space or newline), never commas.
* **Top-level forms** each end on their own line. Multiple top-level forms
  in one input file are all serialized.
* **Empty input** → empty output, success.

## Verification

The serializer runs the existing structure-only gate on its output before
writing anything: the candidate is written to a temp file and run through
`./tools/toc-edit/toc_edit.py parse` (s-expression structure plus the
leftover-bytes coverage gate; no new-toc compile, so context-dependent
snippets pass). The output file is written only if the candidate passes.

This closes the hole the count check can't: any atom text the prefixed
grammar accepts but the Toccata reader would treat structurally would
otherwise travel to the splice step, where the error points at the
wrong file instead of the prefixed input.

## CLI

```
./tools/toc-ast/toc_ast.py serialize <in.prefixed> <out.toc>
```

Run from the repo root, script at `tools/toc-ast/toc_ast.py`, following
the toc-edit conventions (executable, pure stdlib, file args,
explicit exit codes).

| Exit | Meaning |
|------|---------|
| 0 | Success: output written. Any stale `<out>.rejected` is deleted. |
| 1 | Prefixed input failed the grammar/count check. Line:col diagnostics on stderr. No output written. |
| 2 | Serialization succeeded but the `parse` gate rejected the candidate. Diagnostics on stderr. The failed candidate is saved as `<out>.rejected` (overwriting any earlier one), mirroring toc-edit's convention. |

`.rejected` lifetime: a successful `serialize` deletes `<out>.rejected` if
one exists, so a rejected file only ever lives between a failure and the
next run. No background cleanup, no TTL.

## Out of scope (deliberately)
* **`annotate`** (plain Toccata → prefixed form, the read-modify-write
  direction). Deferred; the subcommand name is reserved. When built, it
  needs a lexical-only Toccata tokenizer (no semantics, no byte-span
  claims) or an `ast-json`-based reconstruction — decide then.
* **Toccata-level corrections.** The serializer preserves the opening
  bracket exactly as written; it does not try to correct Toccata-level
  problems (e.g. it will not turn `(1 x)` into `[1 x]`). Bracket choice
  is the author's job, not the serializer's.
* Comment handling, reformatting of existing `.toc` files, any semantic
  (new-toc) validation — those belong to toc-edit.

## Implementation notes

* The tokenizer's string handling must mirror Toccata's reader exactly
  (`interpreter/intrp-rdr.toc` is the reference) so a quoted atom is
  always lexed the way new-toc would lex it.
* The grammar check is a single pass: on an `open`, consume exactly
  `count` forms; EOF in the middle of a form is the underflow error
  ("declared N, found M, at line X col Y"). Over-count is undetectable by
  design (see residual error above).
* Nothing here touches `toccata.c`/`core.c` or the toccata target.
