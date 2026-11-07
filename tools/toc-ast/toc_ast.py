#!/usr/bin/env python3
"""Count-prefixed s-expression authoring format for Toccata.

`serialize` reads a count-prefixed form (docs/toc-ast-design.md) and writes
plain Toccata, verified by the toc-edit `parse` gate before anything is
written.

Format: a form is an atom, a double-quoted string literal, or an opening
bracket immediately followed by the number it contains. Closing brackets
are ignored wherever they appear; comments do not exist in this
format.

Exit codes: 0 success (output written); 1 grammar error in the prefixed
input (line:col on stderr, nothing written); 2 parse gate rejected the
serialized output (candidate saved as <out>.rejected).
"""

import argparse
import os
import subprocess
import sys
import tempfile
from pathlib import Path

CLOSE = {"(": ")", "[": "]", "{": "}"}
OPEN = set(CLOSE)
SPECIAL = OPEN | set(CLOSE.values()) | {";", '"'}
WS = {" ", "\t", "\r", "\n"}
DIGITS = set("0123456789")


class GrammarError(Exception):
    pass


class Tok:
    __slots__ = ("kind", "text", "count", "line", "col", "nl_before")

    def __init__(self, kind, text, count, line, col, nl_before):
        self.kind = kind          # 'open' | 'atom' | 'string'
        self.text = text          # bracket char / atom text / full literal
        self.count = count        # int, opens only
        self.line = line          # 1-based, at the token's first char
        self.col = col
        self.nl_before = nl_before  # newline in the whitespace just before


def tokenize(src):
    toks = []
    i, n = 0, len(src)
    line, col = 1, 1
    pending_nl = False

    def fail(l, c, msg):
        raise GrammarError("line %d col %d: %s" % (l, c, msg))

    while i < n:
        ch = src[i]
        if ch in WS:
            if ch == "\n":
                pending_nl = True
                line += 1
                col = 1
            else:
                col += 1
            i += 1
            continue
        if ord(ch) > 127:
            fail(line, col, "non-ASCII byte %#04x (input must be pure ASCII)"
                 % ord(ch))
        if ch in ")]}":
            # closing brackets are ignored (docs/toc-ast-design.md, rule 4)
            col += 1
            i += 1
            continue
        if ch == ";":
            fail(line, col, "comments are not allowed in the prefixed format")
        if ch in OPEN:
            if i + 1 >= n or src[i + 1] not in DIGITS:
                got = ("end of input" if i + 1 >= n else "'%s'" % src[i + 1])
                fail(line, col, "expected a count (digits) immediately "
                     "after '%s', got %s" % (ch, got))
            j = i + 1
            while j < n and src[j] in DIGITS:
                j += 1
            toks.append(Tok("open", ch, int(src[i + 1:j]), line, col,
                            pending_nl))
            col += j - i
            i = j
            pending_nl = False
            continue
        if ch == '"':
            j = i + 1
            closed = False
            while j < n:
                c = src[j]
                if c == "\\":
                    if j + 1 >= n:
                        fail(line, col, "unterminated string (backslash at "
                             "end of input)")
                    if ord(src[j + 1]) > 127:
                        fail(line, col, "non-ASCII byte %#04x in string "
                             "(input must be pure ASCII)" % ord(src[j + 1]))
                    j += 2
                    continue
                if ord(c) > 127:
                    fail(line, col, "non-ASCII byte %#04x (input must be "
                         "pure ASCII)" % ord(c))
                if c == '"':
                    closed = True
                    break
                if c == "\n":
                    fail(line, col, "unterminated string (newline before "
                         "closing quote)")
                j += 1
            if not closed:
                fail(line, col, "unterminated string (end of input before "
                     "closing quote)")
            toks.append(Tok("string", src[i:j + 1], 0, line, col,
                            pending_nl))
            col += j + 1 - i
            i = j + 1
            pending_nl = False
            continue
        # atom: maximal run of non-whitespace, non-special ASCII chars
        j = i
        while (j < n and src[j] not in WS and src[j] not in SPECIAL
               and ord(src[j]) <= 127):
            j += 1
        toks.append(Tok("atom", src[i:j], 0, line, col, pending_nl))
        col += j - i
        i = j
        pending_nl = False
    return toks


class Atom:
    __slots__ = ("text", "nl_before")

    def __init__(self, text, nl_before):
        self.text = text
        self.nl_before = nl_before


class Form:
    __slots__ = ("bracket", "children", "nl_before")

    def __init__(self, bracket, children, nl_before):
        self.bracket = bracket
        self.children = children
        self.nl_before = nl_before


def parse_forms(toks):
    pos = 0

    def parse_one():
        nonlocal pos
        t = toks[pos]
        pos += 1
        if t.kind != "open":
            return Atom(t.text, t.nl_before)
        children = []
        for k in range(t.count):
            if pos >= len(toks):
                raise GrammarError(
                    "line %d col %d: declared %d forms after '%s', found %d "
                    "(end of input)" % (t.line, t.col, t.count, t.text, k))
            children.append(parse_one())
        return Form(t.text, children, t.nl_before)

    forms = []
    while pos < len(toks):
        forms.append(parse_one())
    return forms


def ser(node):
    if isinstance(node, Atom):
        return node.text
    parts = [node.bracket]  # first child follows the bracket directly;
    # the count that sat between them is stripped, so no separator
    for idx, ch in enumerate(node.children):
        if idx > 0:
            parts.append("\n" if ch.nl_before else " ")
        parts.append(ser(ch))
    parts.append(CLOSE[node.bracket])
    return "".join(parts)


def serialize(forms):
    if not forms:
        return ""
    return "\n".join(ser(f) for f in forms) + "\n"


def toc_edit_path():
    return (Path(__file__).resolve().parent.parent.parent
            / "tools" / "toc-edit" / "toc_edit.py")


def run_parse_gate(text):
    """Run the toc-edit `parse` gate on the serialized text.
    Returns (ok, stderr_text)."""
    tmp = None
    try:
        with tempfile.NamedTemporaryFile("w", suffix=".toc",
                                         delete=False) as f:
            f.write(text)
            tmp = f.name
        r = subprocess.run([sys.executable, str(toc_edit_path()), "parse",
                            tmp], capture_output=True, text=True)
        return r.returncode == 0, r.stderr
    finally:
        if tmp is not None:
            os.unlink(tmp)


def cmd_serialize(args):
    try:
        # latin-1 never fails; the tokenizer reports the first non-ASCII
        # byte with line:col.
        src = Path(args.in_file).read_bytes().decode("latin-1")
    except OSError as e:
        print("toc_ast: cannot read %s: %s" % (args.in_file, e),
              file=sys.stderr)
        return 1
    try:
        toks = tokenize(src)
        forms = parse_forms(toks)
    except GrammarError as e:
        print("toc_ast: %s: %s" % (args.in_file, e), file=sys.stderr)
        return 1
    text = serialize(forms)
    out = Path(args.out_file)
    rejected = Path(str(out) + ".rejected")
    if text:
        ok, stderr = run_parse_gate(text)
        if not ok:
            rejected.write_text(text)
            print("toc_ast: parse gate rejected the serialized output; "
                  "candidate saved as %s" % rejected, file=sys.stderr)
            if stderr:
                sys.stderr.write(stderr)
            return 2
    out.write_text(text)
    if rejected.exists():
        rejected.unlink()
    return 0


def main():
    ap = argparse.ArgumentParser(
        prog="toc_ast.py",
        description="Count-prefixed s-expression authoring format for "
                    "Toccata (docs/toc-ast-design.md).")
    sub = ap.add_subparsers(dest="command", required=True)
    p = sub.add_parser("serialize",
                       help="prefixed form -> plain Toccata (parse-gated)")
    p.add_argument("in_file")
    p.add_argument("out_file")
    p.set_defaults(func=cmd_serialize)
    args = ap.parse_args()
    sys.exit(args.func(args))


if __name__ == "__main__":
    main()
