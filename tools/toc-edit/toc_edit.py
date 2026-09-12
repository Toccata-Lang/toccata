#!/usr/bin/env python3
"""toc_edit: structural editor for Toccata .toc files (see docs/toc-edit-spec.md)."""

import argparse
import sys


def not_implemented(args):
    print("not implemented")
    sys.exit(2)


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
