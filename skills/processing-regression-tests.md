# Process Regression Tests

## ⛔ CRITICAL: Commit after every change

**Commit after every meaningful change.** Do not batch changes. Each commit should be a single, focused change that builds and passes tests. This keeps history clean and makes rollback easy.

## ⛔ CRITICAL: Never use 42 as test data

**Never use 42 as a test integer value** — it's cliche and irritating. Pick something else.

## Overview

This project uses a custom compiler (toccata) that transforms `.toc` source files into C code, which is then compiled and executed. Each regression test consists of:

- **`.toc`** — source file (toccata language)
- **`.c`** — generated C code (compiled from `.toc`)
- **`.rslt`** — expected output (regression baseline)

Many tests in the backlog don't have `.rslt` files yet. When running new tests, the `.rslt` is generated on first run and serves as the baseline. Subsequent runs should produce matching output (excluding stats).

## Files to Read for Context

Before processing any regression test, read these files in order:

| # | File | Purpose |
|---|---|---|
| 1 | `skills/processing-regression-tests.md` | This skill — how to process regression tests |
| 2 | `new.h` | Type definitions, tag constants, function declarations |
| 3 | `runtime3.h` | Runtime header — function declarations, type definitions |
| 4 | `new.c` | Core implementation — `take`, `swap`, `move`, `interact`, existing rules |
| 5 | `runtime3.c` | Runtime support — reference counting, memory management, native functions |
| 6 | `graph.c` | DOT graph generation for debugging |
| 7 | `Makefile` | Build targets and flags |

Read in order: the skill defines the workflow, and the runtime/graph files help diagnose issues.

## Build Pipeline

```
.toc  →  (toccata compiler)  →  .c  →  (clang)  →  binary  →  (run)  →  output vs .rslt
```

### Makefile targets

```bash
# Build and run a single test (compiles .toc → .c → binary, then runs)
make test1          # runs regression-tests/test1 party-pooper | sort > regression-tests/test1.rslt

# Build and run all regression tests
make tests          # includes test-hvm + all REG_TESTS

# Build test-hvm (HVM unit tests, not .toc-based)
make test-hvm

# Clean generated .c files
make clean-c
```

### How the Makefile works (for each REG_TEST)

```makefile
regression-tests/%.c: new-toc regression-tests/%.toc regression-tests/regression-tester.toc hvm-core.toc
	./new-toc regression-tests/$*.toc > regression-tests/$*.tmp
	awk '/^#$$/ { printf "#line %d \"%s\"\n", NR+1, "m.c"; next; } { print; }' \
          regression-tests/$*.tmp > regression-tests/$*.c
	clang-format -i regression-tests/$*.c
	rm regression-tests/$*.tmp

$(REG_TESTS): %: regression-tests/%.c $(TEST_SOURCES)
	$(CC) $(CFLAGS) -o regression-tests/$@ $(TOC_FLAGS) $(LDFLAGS) $(TEST_SOURCES) regression-tests/$*.c
	regression-tests/$@ party-pooper | sort > regression-tests/$*.rslt
```

1. **`new-toc`** — the toccata compiler (must be built first)
2. **`.toc` → `.c`** — compilation with line-number directives for debugging
3. **`.c` + `$(TEST_SOURCES)`** — compiled with `new.c`, `runtime3.c`, `graph.c`
4. **Run** — executes the binary with `party-pooper` as argument, output piped through `sort`, written to `.rslt`

### Required flags (from `TOC_FLAGS`)

- `-DCHECK_MEM_LEAK=1` — malloc/free counting
- `-DSAFETY=1` — runtime safety checks (polarity validation, etc.)
- `-DSTATS=1` — redex counting
- `-lm` — math library
- `-I.` — include path

## Processing Workflow

### Step 1: Understand the test

Read the `.toc` source to understand what the test does:

```bash
cat regression-tests/<name>.toc
```

**⚠️ Important:** These tests were written for a previous version of Toccata. Inline C code is likely outdated and will need updating. You may need to ask for help with the inline C — see `cond-expr-5.toc` for an example of the new style.

The `.toc` language uses S-expression-like syntax:
- `(main [_] ...)` — entry point
- `(defn name [args] ...)` — function definitions
- `(inline "C code")` — inline C code (may need updating)
- `(println expr)` — print expression result
- `(cond clause1 result1 clause2 result2 ...)` — conditional
- `(either ...)` — conditional/exhaustive matching

**⚠️ API note:** `subs` requires 3 arguments: `(subs string start end)`. If a call has only 2 args, add `(count <string>)` as the third argument.

**⚠️ API note:** Replace `(maybe <expr>)` with `(Some <expr>)`. `maybe` is obsolete.

**⚠️ API note:** Replace `(list ...)` with `(vector ...)`. `list` is obsolete.

**⚠️ API note:** Replace `(string-list ...)` with `(str-vect ...)`. `string-list` is obsolete.

**⚠️ API note:** Replace `(print-err ...)` with `(println ...)`. `print-err` is obsolete.

**⚠️ API note:** `partial` no longer exists — partial application happens automatically when you don't supply enough arguments in a call. Remove any explicit `partial` usage.

**⚠️ API note:** `some` is being renamed to `any?` (not yet implemented). `every` is being renamed to `ever?` (not yet implemented).

**⚠️ API note:** `abort` requires a string message argument: `(abort "error")`.

**⚠️ API note:** Replace `nothing` with `None`.

**⚠️ API note:** Assert syntax changed: `(assert (instance? Type x))` → `! x Type`. Return annotations: `(assert-result r (instance? Type r))` → `!returns Type`. **⛔ NEVER wrap `!` or `!returns` in parentheses** — they are not value-level expressions, they are special forms at the statement level. Type expressions: simple names like `Some`, `None`; vectors like `[Integer]`.

### Step 2: Compile and run

```bash
make <test-name>
```

This will:
1. Compile the `.toc` to `.c` (via `new-toc`)
2. Compile the `.c` with the HVM runtime sources
3. Run the binary and write output to `.rslt`

### Step 3: Check the output

```bash
cat regression-tests/<name>.rslt
```

Expected output format:
```
<program output>
- ITRS: <number>
malloc count: <n>  free count: <n>  diff: 0
remaining nodes: <n> (<n>)
result: <exit code> bashResult: <exit code>
- Threads: <n>
```

**Key assertions:**
- `malloc count == free count` (diff: 0) — **no memory leaks**
- `remaining nodes: 0` — **no node leaks**
- `result: <code> bashResult: <code>` — matching exit codes

### Step 4: Diagnose failures

If the test fails:

1. **Compilation failure** — the `.toc` source has a syntax error or references undefined symbols. Check the `.c` output for clues.

2. **Runtime crash** — the program aborts during execution. Check:
   - `graphs.dot` for visual term graph dump
   - `graphs/` directory for individual interaction dumps
   - Error messages in stderr

3. **Wrong output** — for existing tests with `.rslt`:
   ```bash
   git diff regression-tests/<name>.rslt
   ```
   The `.rslt` should be **mostly unchanged** — only execution stats (ITRS, node counts) may differ.

   For new tests without `.rslt`: verify the output makes sense based on the `.toc` source. The first run's output becomes the baseline.

4. **Memory leak** — `malloc count != free count` or `remaining nodes > 0`:
   - Look for unfreed terms in the interaction handlers
   - Check that `take()` is used to clean up ports
   - Verify `freePair()` is called for pair bodies when needed

5. **Bad interaction / badrdx** — unimplemented interaction rule:
   - The interaction jump table entry is `&badrdx`
   - Refer to `skills/interaction-recipe.md` to implement the rule

### Step 5: Fix and re-verify

After fixing:
1. Re-run: `make <test-name>`
2. Verify output matches expected
3. Verify no memory leaks
4. Check `git diff` — only the `.rslt` should have changed (and possibly stats)

### Step 6: Finalize (after approval)

Once you confirm the test is processed correctly:

1. **Add to REG_TESTS** in the Makefile:
   ```bash
   # Add the test name to the REG_TESTS line in the Makefile
   ```

2. **Add `.rslt` to git** (force-add if in `.gitignore`):
   ```bash
   git add -f regression-tests/<test-name>.rslt
   ```

3. **Update status.md** — move the test from "Tests to be processed" to "Working tests" and mark it checked.

## Common Patterns

### Inline C code

Tests often use `(inline "C code")` to inject C directly:

```lisp
(inline "
  Vector *ve = ((Vector *)((u64)arg_1 & ~VAL_MASK));
  if (ve->count < 2)
    printf(\"Howdy from HVM 1\\n\");
  pushRedex(ERA, arg_1);
")
```

### Program structure

A typical `.toc` test:

```lisp
(main [_]
  (inline "
    // Setup: create terms, set up global functions
    Term x = makePair(LAM, 0, SUB, newI60(42));
    Term y = makePair(APP, 0, newI60(7), SUB);
  "))

(defn some-fn [args]
  (inline "
    // Function body
    pushRedex(args, someTerm);
  "))
```

### Using the compiler

The `new-toc` compiler must exist:

```bash
# Build the compiler first (if not already built)
make new-toc
```

## Error Handling

### Compilation errors

```bash
# Check the generated .c for clues
cat regression-tests/<name>.c | head -100
```

### Runtime errors

The `BOOM(msg)` macro prints file/line and aborts. Check stderr output.

### Graph debugging

After a crash, `graphs.dot` contains the term graph visualization:

```bash
dot -Tsvg graphs.dot > graphs.svg
# Open graphs.svg in a browser
```

Individual interactions are also dumped to `graphs/` directory.

## Checklist

### During processing
- [ ] `.toc` source read and understood
- [ ] `make <test-name>` succeeds (compilation + execution)
- [ ] Output matches expected `.rslt` (excluding stats), or makes sense if no `.rslt` yet
- [ ] `malloc count == free count` (no memory leaks)
- [ ] `remaining nodes == 0` (no node leaks)
- [ ] `git diff` shows only `.rslt` changes (not `.c` or other files)
- [ ] `make test-hvm` still passes (regression check)

### After approval
- [ ] Test added to REG_TESTS in Makefile
- [ ] `.rslt` added to git (`git add -f`)
- [ ] Moved to "Working tests" in status.md and marked checked

## Lessons Learned

1. **Always use `make <test-name>`** — not manual compilation. Ensures correct flags and source files.
2. **`.rslt` should be mostly unchanged** — only ITRS count and node usage stats may differ between runs. For new tests, the first run's output becomes the baseline.
3. **Memory leaks are failures** — every test must leave `malloc_count == free_count` and `glblAlloced == 0`.
4. **Node leaks are failures** — every test must leave `remaining nodes: 0`.
5. **Check `git diff` after running** — untracked files (`.tmp`, backup files, etc.) should NOT be committed.
6. **The compiler (`new-toc`) must be built first** — if it's missing, build it with `make new-toc`.
7. **Inline C code accesses HVM terms directly** — arguments arrive as `Term` variables; results are produced via `printf` or by manipulating the term graph.
8. **`pushRedex(ERA, arg)` erases a term** — common pattern in inline C to clean up after use.
9. **`party-pooper` is a standard argument** — all regression tests are run with this argument; it's consumed by the runtime.
10. **When a test fails, check the generated `.c`** — the compiler output can reveal issues with the `.toc` source.
