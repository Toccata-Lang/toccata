#!/usr/bin/env python3
"""toc_edit: structural editor for Toccata .toc files (see docs/toc-edit-spec.md)."""

import argparse
import json
import subprocess
import sys
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


def build_parser():
    parser = argparse.ArgumentParser(
        prog="toc_edit.py",
        description="Structural editor for Toccata .toc files, backed by ast-json.",
    )
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("check")
    p.add_argument("file")
    p.set_defaults(func=not_implemented)

    p = sub.add_parser("show")
    p.add_argument("file")
    p.add_argument("path")
    p.set_defaults(func=not_implemented)

    p = sub.add_parser("line")
    p.add_argument("file")
    p.add_argument("n")
    p.set_defaults(func=not_implemented)

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
    args.func(args)


if __name__ == "__main__":
    main()
