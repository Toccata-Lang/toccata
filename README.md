# Toccata

Toccata is a Clojure-inspired Lisp dialect that compiles to a native executable.
This repository holds the current HVM (interaction-net) based implementation: the
`new-toc` compiler (written in Toccata, built from `compiler.toc`), the runtime
(`new.c`, `runtime3.c`), the core library (`hvm-core.toc`), and a regression-test
suite.

## Layout

- `compiler.toc`, `base.toc`, `typer.toc`, `codegen.toc` — the `new-toc` compiler
- `hvm-core.toc` — core type definitions and structure
- `new.c` / `new.h` — interaction-net runtime core
- `runtime3.c` / `runtime3.h` — runtime support: memory, native functions
- `regression-tests/` — the regression-test suite
- `docs/` — the calculus, implementation reference, and working notes
- `skills/` — agent skills for common workflows

## Building

Requires `git`, `clang`, and `dot` (Graphviz).

    make new-toc     # build the compiler
    make tests       # build and run the full regression suite

`make test-hvm` and `make test-hash-map` run the C-level unit tests. `make help`
lists the available targets.

## Documentation

- `status.md` — current phase, working tests, backlog, known issues
- `docs/calculus.md` — the formal interaction calculus
- `docs/implementation.md` — runtime architecture reference
- `docs/implementation-notes.md` — working notes on the internal mechanics
- `HISTORY.md` — history of the first (C-based) compiler
- `spec.md` — language specification (in progress)
- `ebnf.md` — grammar reference

## How Toccata differs from Clojure

Toccata is not a copy or a port of Clojure. A few key differences:

- `for` works on any data type that implements the `flat-map` protocol function,
  not just sequences.
- `map` is a protocol function: it can be implemented for any data type, and the
  value being mapped over comes first (the opposite of Clojure's order).
- There is no Boolean type, no `true`/`false`, and no `if`/`when` forms.
- Comments are nodes in the AST.
- Every C file tracks memory allocations and frees; the stats are printed at the
  end of each run, and a discrepancy (or an `incRef`/`dec_and_free` error) is a
  failure.

## License

See `LICENSE`.

You can learn more about Toccata by following the [blog here](http://toccata.io)
