# Editing runtime3.c — Key Learnings

This skill documents the process and pitfalls of uncommenting and adapting stub functions in `runtime3.c` for the hash-map implementation.

## ⛔ CRITICAL: Never use 42 as test data

**Never use 42 as a test integer value** — it's cliche and irritating. Pick something else.

## ⛔ CRITICAL: Git Workflow

**When committing, ONLY stage and commit the files that were actually changed.**
Never run `git add -A` or `git add .` — it will pick up untracked files (backup files, generated files, dependencies, build artifacts) that are not part of your changes. Instead, explicitly list only the files you modified:

```bash
git add runtime3.c runtime3.h
```

Then commit. If you see untracked files (`??`) in `git status`, they must NOT be included in the commit.

**Wait for explicit instruction before committing.** Do not commit changes until told to do so.

## Runtime3.c Type System — The Core Issue

**runtime3.c uses `Term` (unsigned long) for most operations, NOT `Value *`.**

This is the fundamental difference from core.c and the source of all type adaptation work.

### Key Function Signatures

| Function | Takes | Returns |
|---|---|---|
| `incRef` | `Term val` | `Term` |
| `dec_and_free` | `Term pv` | void |
| `integerSha1` | `Term arg0` | `Term` |
| `strSha1` | `Value *arg0` | `int64_t` |
| `getI60` | `Term val` | `i64` |
| `newI60` | `i64 x` | `Term` |
| `termVal` | `Term val` | `Term` (no-op macro) |
| `termTag` | `Term val` | `Tag` |

### Type Casting Rules

When calling runtime3.c functions from code that has `Value *`:

```c
// incRef/dec_and_free take Term — cast Value* to Term
incRef((Term)(Value *)ptr, 1);
dec_and_free(termVal((Term)(Value *)ptr), 1);

// Functions taking Value* need (Value *) cast around incRef result
copyAssoc((Value *)incRef((Term)(Value *)ptr, 1), ...);
equal((Value *)incRef((Term)(Value *)key, 1), (Value *)incRef((Term)(Value *)keyOrNull, 1));

// mutateVectConj takes Term — use termVal()
mutateVectConj(vec, termVal((Term)(Value *)ptr));

// ArrayNode->array is Term*, BitmapIndexedNode->array is Value**
ArrayNode *an = ...;
an->array[i] = (Term)(Value *)someValue;  // ArrayNode uses Term*

BitmapIndexedNode *bm = ...;
bm->array[i] = someValue;  // BMI uses Value**
```

## Types That DON'T Exist in runtime3.c

These types exist in core.c but NOT in runtime3.c:

| Type | Why it matters | Replacement |
|---|---|---|
| `Integer` struct | Used for integer values with `numVal` field | Integers are I60 terms — use `newI60(x)` and `getI60(x)` |
| `ListType` | Used in switch cases | Not defined — remove cases |
| `HashedValue` struct | Used for hash caching | Not defined — remove caching logic |
| `new_num` / `new_i24` | Creates integer values | Use `(Value *)newI60(x)` instead |

## Protocol Functions NOT Wired Up

These extern function pointers are declared but not assigned:

| Function | Purpose | Status |
|---|---|---|
| `count` | Returns count of sequence | Not wired |
| `sha1` | Computes SHA1 hash | Not wired |
| `get` | Polymorphic get dispatch | Not wired (renamed to `mapGet` in some contexts) |
| `baseDissoc` | Polymorphic dissoc dispatch | Not wired |
| `hashSeq` | Flatten to sequence | Not wired |
| `vals` | Get all values | Not wired |

When a function uses these, it cannot work until they are wired up. Simplify the function to return a placeholder (e.g., 0) for now.

## Process for Uncommenting a Stub

### Step 1: Identify the stub pattern

```c
Value *funcName(...) {
  fprintf(stderr, "Boom %s:%d\n", __FILE__, __LINE__);
  abort();
  return ((Value *)NULL);
  /*
  <real implementation>
  // */
}
```

### Step 2: Remove stub lines

Delete: `fprintf`, `abort()`, `return ((Value *)NULL);`, `/*`

Delete: `// */`

### Step 3: Apply type adaptations

Check every function call and variable usage against the rules above:

1. `incRef(Value*, n)` → `incRef((Term)(Value *)Value*, n)`
2. `dec_and_free(Value*, n)` → `dec_and_free(termVal((Term)(Value *)Value*), n)`
3. `integerSha1(Value*)` → `integerSha1(termVal((Term)(Value *)Value*))`
4. `new_num(new_i24(x))` → `(Value *)newI60(x)`
5. `HashedValue` references → remove caching logic
6. `Integer` struct usage → replace with I60 term handling
7. `ListType` cases → remove
8. `value->array[i]` assignments where array is `Term*` → cast to `(Term)(Value *)`
9. `value->array[i]` assignments where array is `Value**` → no cast needed

### Step 4: Check for protocol dependencies

If the function calls `count()`, `sha1()`, `get()`, etc., note that these are not wired up. Simplify the function to handle what it can and return a placeholder for the rest.

### Step 5: Verify compilation

```bash
clang -c -g -march=native -I. -DCHECK_MEM_LEAK=1 -DSAFETY=1 -DSTATS=1 runtime3.c -o /tmp/runtime3.o 2>&1
```

No errors = good. Fix any remaining type mismatches.

### Step 6: Update status.md

Mark the function as done in the hash-map checklist.

## Lessons Learned

1. **Never assume `Value *` works where `Term` is expected** — runtime3.c is built around `Term` (unsigned long). Every `incRef`, `dec_and_free`, `integerSha1` call needs a cast.

2. **`newI60()` creates I60 terms, not `Value *`** — when returning an integer value from a function that returns `Value *`, cast: `(Value *)newI60(x)`.

3. **`ArrayNode->array` is `Term*`, not `Value**`** — assignments to ArrayNode arrays need `(Term)(Value *)` casts. BMI arrays are `Value**` and don't need casts.

4. **`termVal()` is a no-op macro** — it exists but does nothing. The cast `(Term)(Value *)` is what actually does the conversion. `termVal()` is only needed when the compiler needs to see a `Term` type explicitly.

5. **Protocol functions are extern pointers** — `count`, `sha1`, `get`, etc. are declared as `extern Value *(*)...` but not assigned. They will segfault if called before wiring. Simplify functions to avoid calling them until wired.

6. **`HashedValue` caching doesn't exist** — the original core.c code caches hashes in a `HashedValue` struct. This type doesn't exist in runtime3.c. Remove all caching logic and compute hashes fresh.

7. **`Integer` struct doesn't exist** — integers are represented as I60 terms, not as `Integer` structs. Use `newI60(x)` to create and `getI60(x)` to extract.

8. **`ListType` is not defined** — remove any switch cases for `ListType`. It doesn't exist in runtime3.h.

9. **`nakedSha1` signature changed** — it now takes `Term trm` instead of `Value *v1`. Use `termTag()` to dispatch on I60 vs VAL.

10. **Always verify compilation after each change** — don't wait until all functions are done. Compile frequently to catch type errors early.

11. **Don't commit until told** — the user explicitly asked to wait for instruction before committing. This is a strict rule.

12. **Be careful with automated scripts** — Python scripts that do bulk replacements can match unintended patterns. Always verify the diff before committing.

13. **Function signatures matter** — when changing a function signature (like `nakedSha1` taking `Term`), update both the implementation AND the declaration in runtime3.h.

14. **Simplify when types are missing** — when a function depends on types that don't exist (Integer, HashedValue), simplify the function to handle what it can and return a placeholder for the rest. Don't try to force-fit core.c patterns.
