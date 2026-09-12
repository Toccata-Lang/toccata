#!/usr/bin/env python3
"""toc_edit: structural editor for Toccata .toc files (see docs/toc-edit-spec.md)."""

import argparse
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path

# The repo root is two levels above tools/toc-edit/; the ast-json and
# new-toc binaries live there (see docs/toc-edit-spec.md).
REPO_ROOT = Path(__file__).resolve().parents[2]
AST_JSON = REPO_ROOT / "ast-json"
NEW_TOC = REPO_ROOT / "new-toc"


class TocEditError(Exception):
    """A clean, user-facing error (printed by main; never a traceback)."""


def not_implemented(args):
    print("not implemented")
    sys.exit(2)


def run_ast_json(file):
    """Run `ast-json <file>` and return the parsed JSON AST (a top-level list).

    Raises TocEditError (clear message, no traceback) if the ast-json
    binary is missing, fails, or emits non-JSON output.
    """
    if not AST_JSON.is_file():
        raise TocEditError(f"ast-json binary not found at {AST_JSON}")
    proc = subprocess.run([str(AST_JSON), str(file)], capture_output=True, text=True)
    if proc.returncode != 0:
        detail = proc.stderr.strip() or proc.stdout.strip() or "(no output)"
        raise TocEditError(
            f"ast-json failed on {file} (exit {proc.returncode}): {detail}"
        )
    try:
        ast = json.loads(proc.stdout)
    except json.JSONDecodeError as e:
        raise TocEditError(f"ast-json produced non-JSON output for {file}: {e}")
    if not isinstance(ast, list):
        raise TocEditError(
            f"ast-json produced a non-array top level for {file}: {type(ast).__name__}"
        )
    return ast


def run_new_toc(file):
    """Run `new-toc <file>`; return (exit code, stdout, full stderr).

    stderr is captured in full and never discarded. The exit code is NOT
    the pass/fail signal (a clean library load with no `main` exits 134);
    classify the stderr instead (see classify).
    """
    if not NEW_TOC.is_file():
        raise TocEditError(f"new-toc binary not found at {NEW_TOC}")
    proc = subprocess.run([str(NEW_TOC), str(file)], capture_output=True, text=True)
    return proc.returncode, proc.stdout, proc.stderr


# Known-clean `*** ` lines that a clean new-toc load prints (observed
# 2026-09-12; see docs/toc-edit-plan.md item 0.5). Anything else that
# starts with `*** ` is treated as a rejection (fail closed).
_CLEAN_EXACT_LINES = {
    "*** creating-parser",
    # Printed on every clean library load (no `main`).
    "*** 'main' function is missing or malformed",
    # Pre-existing core warning, printed on every clean load.
    "*** Could not find implementation of 'Container/map' for type 'Agent' "
    "with 2 arguments at core: 1453",
}
_CLEAN_LINE_PREFIXES = (
    "*** parser-created ",
    "*** time-for-core ",
    "*** Loading ",
    "*** Loaded ",
)


def _is_clean_stderr_line(line):
    return line in _CLEAN_EXACT_LINES or line.startswith(_CLEAN_LINE_PREFIXES)


def classify(stderr):
    """Classify new-toc stderr: 'error' if it contains any `*** ` line not
    in the known-clean set, 'silent' if stderr is empty, else 'clean'.
    Pure function.

    Owner decision (2026-09-12, resolves the 0.5 STUCK note): fail closed
    — a clean load prints only the boilerplate in _CLEAN_EXACT_LINES /
    _CLEAN_LINE_PREFIXES, so any other `*** ` line (`*** Error`,
    `*** Undefined symbol`, `***  Conflicting assertions`, or any future
    format) is a rejection. A future new core info line will cause a
    false rejection (exit 3, file untouched, `.rejected` saved) rather
    than a silently accepted broken file; extend the allowlist then.
    This makes item 2.5b (delete a used `defn` → exit 3) work.
    """
    if any(
        line.startswith("*** ") and not _is_clean_stderr_line(line)
        for line in stderr.splitlines()
    ):
        return "error"
    if stderr == "":
        return "silent"
    return "clean"


def resolve_path(ast, path):
    """Resolve a dot-separated integer path to an AST node.

    Path '0' is top-level node 0; '42.3' is child 3 of top-level node 42
    (see docs/toc-edit-spec.md, CLI). Raises TocEditError — a distinct
    error naming the deepest existing prefix — for a non-integer segment
    or an out-of-range index.
    """
    container = ast
    prefix = ""
    node = None
    for part in str(path).split("."):
        if not (part.isascii() and part.isdigit()):
            raise TocEditError(
                f"bad path {path!r}: segment {part!r} is not a non-negative "
                f"integer (deepest existing prefix: {prefix or '<top level>'})"
            )
        idx = int(part)
        if idx >= len(container):
            raise TocEditError(
                f"bad path {path!r}: index {idx} out of range "
                f"({len(container)} children); "
                f"deepest existing prefix: {prefix or '<top level>'}"
            )
        node = container[idx]
        prefix = part if not prefix else prefix + "." + part
        container = node["children"]
    return node


def _node_text(raw, node):
    """The node's verbatim source text: file[start:end] (byte slice)."""
    return raw[node["start"]:node["end"]].decode("utf-8", errors="replace")


def cmd_show(args):
    """`show`: print a node's path, kind, span [start, end), verbatim text
    file[start:end], and its children with their paths (see
    docs/toc-edit-spec.md, Scope (b)).
    """
    ast = run_ast_json(args.file)
    node = resolve_path(ast, args.path)
    raw = Path(args.file).read_bytes()
    print(f"path: {args.path}")
    print(f"kind: {node['kind']}")
    print(f"span: [{node['start']}, {node['end']})")
    print(f"text: {_node_text(raw, node)}")
    for i, child in enumerate(node["children"]):
        print(
            f"  {args.path}.{i} {child['kind']} "
            f"[{child['start']}, {child['end']})"
        )


def _line_range(raw, n):
    """Byte range [start, end) of 1-based line n in the source bytes.

    end is one past the line's newline (or the file end). Raises
    TocEditError for out-of-range lines.
    """
    if n < 1:
        raise TocEditError(f"line {n} is out of range (lines are 1-based)")
    start = 0
    for i in range(1, n):
        start = raw.find(b"\n", start)
        if start < 0:
            raise TocEditError(f"line {n} is out of range (file has {i} lines)")
        start += 1
    end = raw.find(b"\n", start)
    return start, (len(raw) if end < 0 else end + 1)


def _innermost_on_line(ast, lo, hi):
    """The innermost node containing a byte of [lo, hi): (path, node), or
    None if the line is entirely unowned whitespace. Deepest node wins;
    ties break to the earliest node in the line.
    """
    best = None  # ((depth, -start), path, node)

    def walk(nodes, pre):
        nonlocal best
        for i, node in enumerate(nodes):
            path = f"{pre}.{i}" if pre else str(i)
            s, e = node["start"], node["end"]
            if s < hi and e > lo:
                key = (path.count(".") + 1, -s)
                if best is None or key > best[0]:
                    best = (key, path, node)
            walk(node["children"], path)

    walk(ast, "")
    return None if best is None else (best[1], best[2])


def cmd_line(args):
    """`line`: find the innermost node whose span contains the line (1-based)
    and print its path, kind, span, and text (see docs/toc-edit-spec.md,
    Scope (b)). Clean error for out-of-range lines or lines in unowned
    whitespace.
    """
    ast = run_ast_json(args.file)  # clean error for a missing file
    raw = Path(args.file).read_bytes()
    lo, hi = _line_range(raw, args.n)
    hit = _innermost_on_line(ast, lo, hi)
    if hit is None:
        raise TocEditError(f"line {args.n} is in unowned whitespace")
    path, node = hit
    print(f"path: {path}")
    print(f"kind: {node['kind']}")
    print(f"span: [{node['start']}, {node['end']})")
    print(f"text: {_node_text(raw, node)}")


def _rejected_path(path):
    """The `.rejected` name beside the original: hvm-core.toc ->
    hvm-core.rejected (docs/toc-edit-spec.md, Failure handling)."""
    if path.suffix == ".toc":
        return path.with_suffix(".rejected")
    return path.with_name(path.name + ".rejected")


def apply_edit(file, new_bytes):
    """Validate-then-write (docs/toc-edit-spec.md, Failure handling).

    Writes new_bytes to a temp file in the SAME directory as `file` (so
    the rename over the original is atomic), runs new-toc on the temp,
    and on 'clean' atomically renames it over the original. The original
    is never clobbered by a rejected or unverified edit.

    Returns ('ok', None) on success. On 'error' (rejection) the candidate
    is saved as `<name>.rejected` beside the original (overwriting any
    earlier one), new-toc's stderr is printed to the tool's stderr (not
    saved to a file), and ('error', stderr) is returned; the original is
    untouched. On 'silent' the temp file is removed, the original is
    untouched, and ('silent', stderr) is returned for the retry loop
    (plan item 2.3). stderr is returned in full, never discarded.
    """
    path = Path(file).resolve()
    fd, tmp_name = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent
    )
    tmp = Path(tmp_name)
    try:
        with os.fdopen(fd, "wb") as f:
            f.write(new_bytes)
        _code, _stdout, stderr = run_new_toc(tmp)
    except BaseException:
        tmp.unlink(missing_ok=True)
        raise
    verdict = classify(stderr)
    if verdict == "clean":
        os.replace(tmp, path)
        return ("ok", None)
    tmp.unlink()
    if verdict == "error":
        _rejected_path(path).write_bytes(new_bytes)
        sys.stderr.write(stderr)
    return (verdict, stderr)


def cmd_check(args):
    """`check`: run new-toc on the file, print its stderr, exit 1 on
    'error', 0 otherwise (see docs/toc-edit-spec.md, Failure handling).
    """
    _code, _stdout, stderr = run_new_toc(args.file)
    if stderr:
        sys.stderr.write(stderr)
    sys.exit(1 if classify(stderr) == "error" else 0)


def build_parser():
    parser = argparse.ArgumentParser(
        prog="toc_edit.py",
        description="Structural editor for Toccata .toc files, backed by ast-json.",
    )
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("check")
    p.add_argument("file")
    p.set_defaults(func=cmd_check)

    p = sub.add_parser("show")
    p.add_argument("file")
    p.add_argument("path")
    p.set_defaults(func=cmd_show)

    p = sub.add_parser("line")
    p.add_argument("file")
    p.add_argument("n", type=int)
    p.set_defaults(func=cmd_line)

    p = sub.add_parser("insert")
    p.add_argument("file")
    p.add_argument("path")
    p.add_argument("--before", action="store_true")
    p.add_argument("--after", action="store_true")
    p.add_argument("--from-file", required=True)
    p.set_defaults(func=not_implemented)

    p = sub.add_parser("replace")
    p.add_argument("file")
    p.add_argument("path")
    p.add_argument("--from-file", required=True)
    p.set_defaults(func=not_implemented)

    p = sub.add_parser("delete")
    p.add_argument("file")
    p.add_argument("path")
    p.set_defaults(func=not_implemented)

    return parser


def main():
    parser = build_parser()
    args = parser.parse_args()
    if args.command == "insert" and args.before == args.after:
        parser.error("insert requires exactly one of --before/--after")
    try:
        args.func(args)
    except TocEditError as e:
        print(f"toc_edit: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
