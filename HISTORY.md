Rationale
========

The first (almost) usuable version of Toccata is finally nearing completion. By reading
through the commits and the entries in this file, you should be able to watch Toccata
grow from nothing to a usable language. This is not a historical record. The real
development effort (and resulting code) was very ugly with lots of dead ends, painful bugs,
and tears. (Not really on the tears part, but close)

This repo is to capture the organizational structure of the code.

New entries will be added to the end, and so should be read from the top down.

Happy Reading and Pull Requests welcome!

Jim Duey

Story
=====

Entry 1:

In the beginning, God created the Heavens and the Earth. Gen. 1:1

Unlike in Genesis, a lot of work has preceded this commit. Until this compiler is
self-hosting, you won't be able to compile and run it. Nor compile the various regression
tests. I'm using the previous version of Toccata to cross-compile this code until that point.

Now, let us begin.

The 'regression-tests' directory will contain a collection of Toccata programs that
exercise various parts of the compiler. 'main1.toc' is the simplest program possible.
But already, there's a ton of code required. We have to ...
    open the file
    read it
    parse the contents into a nested list structure
    convert the parsed structure to an AST (Abstract Syntax Tree)
    traverse the AST and emit C code to stdout

If the C code has been written to a file, it can be compiled to an executable using
LLVM or GCC.

So 'toccata.toc' is what it takes to do the above steps. This is about as simple a
compiler for Toccata as possible. Everything else builds from there.

