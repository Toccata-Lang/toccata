# Rationale

The first (almost) usuable version of Toccata is finally nearing completion. By reading
through the commits and the entries in this file, you should be able to watch Toccata
grow from nothing to a usable language. This is not a historical record. The real
development effort (and resulting code) was very ugly with lots of dead ends, painful bugs,
and tears. (Not really on the tears part, but close)

This repo is to capture the organizational structure of the code.

New entries will be added to the end, and so should be read from the top down.

Happy Reading and Pull Requests welcome!

Jim Duey

# Story

## Entry 1:

In the beginning, God created the Heavens and the Earth. Gen. 1:1

Unlike in Genesis, a lot of work has preceded this commit. Until this compiler is
self-hosting, you won't be able to compile and run it. Nor compile the various regression
tests. I'm using the previous version of Toccata to cross-compile this code until that point.

Now, let us begin.

The `regression-tests` directory will contain a collection of Toccata programs that
exercise various parts of the compiler. `main1.toc` is the simplest program possible.
But already, there's a ton of code required. We have to ...
    open the file
    read it
    parse the contents into a nested list structure
    convert the parsed structure to an AST (Abstract Syntax Tree)
    traverse the AST and emit C code to stdout

If the C code has been written to a file, it can be compiled to an executable using
LLVM or GCC.

So `toccata.toc` is what it takes to do the above steps. This is about as simple a
compiler for Toccata as possible. Everything else builds from there.


## Entry 2:

When an expression is emitted, it is converted to a list of data structures. Each data structure
has the following
    A string that holds the C variable the result of the expression is assigned to
    A list of strings of C code to declare static C values
    A list of strings of C code that produces the expressions result
    A number that signifies the type of the result of the expression
    A hash-map of of C variables that are used in the expression to the number of times their used

Also, `inline` expressions can appear at the top level of a file just like the `main` expression

## Entry 3:

Comment blocks are AST nodes also, this allows easy literate programming and code formatting

## Entry 4:

Defining symbols is the next step. Toccata source files are called modules and eventually there will be
a system for compiling multiple modules into a program. The C variable for each symbol in a module
is added to the global context so it can be looked up later.

## Entry 5:

Now we're able to assign numbers to symbols

## Entry 6:

And now string literals to symbols

## Entry 7:

When doing the next commit, I discovered a need to refactor some code

## Entry 8:

And now we're finally getting somewhere. This is the first point that we can define a function and call it. Looking at this commit, you'll see the complexity start to increase. And we've just scratched the surface. This is where `core.c` makes it's appearance, which will hold all of the C code that is the foundation of Toccata. Watch it explode in successive commits.

## Entry 9:

Since we compile expressions that are calls to functions, we need to handle assiging the results to global static symbols. Which means we need to initialize them at run time before `main` runs. So this commit adds that machinery. It also adds the `core.toc` standard library.

## Entry 10:

Now we'll add a couple of low-level functions that we can call. We'll add the functionality to allocate and free a few primitive value types.

## Entry 11:

Add a fn to compare integers for equality. Which means we need to add the Maybe type as well. And add malloc/free for Functions and Lists.

## Entry 12:

Add `let` expressions to bind values to symbols

## Entry 13:

Inline calls to `list` and `vector` so that those functions don't actually get called to create them. And test their destructuring in let and function parameters.

## Entry 14:

Add `do` expressions for side-effecting code

## Entry 15:

Selecting an expression based on some condition is one of the fundamental features a language must have. So we add `and` & `or` expressions. `or` is the only expression that is lazy. Successive expressions in it's body are only executed after previous expressions evaluate to `nothing`. The first expression that evaluates to a `maybe` value produces the result of the `or` expression. Likewise, `and` is the only expression that short circuits when the first expression returns `nothing`. If all expressions return `maybe` values, the final one becomes the value the `and` expression evaluates to.

## Entry 16:

Though they don't do anything yet, 'assert' expressions are going to play a big role later on. The compiler will use them to do all kinds of static analysis.

## Entry 17:

And now we're finally into the good stuff. Toccata lifts/borrows/steals the idea of `protocols` from Clojure. This commit lets protocols be defined. Almost all of the core library is implemented as protocol functions, which has some very nice properties we'll see later. In Toccata, protocol functions can specify a default impleenation that will be executed if no type-specific protocol implementation is provided.

## Entry 18:

The first hints of where I want to take Toccata appear in `core.toc`.

## Entry 19:

A rather large commit to enable extending types with additional protocol function implementations
