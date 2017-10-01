Toccata
=======

> "Pithy quote."
> - Unknown

An incomplete, buggy, undocumented,  Clojure-inspired Lisp dialect that compiles to native executable using the Clang compiler

# Quick start (for macOS, Linux should be similar)

Make sure you have `git` and `clang` installed. `clang` is part of the LLVM project and also installed as part of Xcode. Make sure that `/usr/bin/git` exists and points to the correct `git` executable.

* Clone this repo and switch to the cloned directory
* Compile `core.c`

      clang -O3 -g -fno-objc-arc -std=c99 -c core.c

* Compile the Toccata compiler itself

      clang -O3 -g -fno-objc-arc -o toccata -std=c99 core.o toccata.c -lpthread
    
* Set the TOCCATA_DIR environment variable to the directory that contains `core.toc`

      export TOCCATA_DIR=/Users/jduey/toccata
    
* Add that same directory to the C_INCLUDE_PATH environment variable

      export C_INCLUDE_PATH=$C_INCLUDE_PATH:$TOCCATA_DIR

* For convenience, make sure the `toccata` executable is on the $PATH.

      export PATH=$PATH:$TOCCATA_DIR
    
# Compile your first program

* Paste this text to a file named `hw.toc`

      (main [_]
        (println "Howdy, folks"))
        
* Compile it to C code

      toccata hw.toc > hw.c

* Compile the C code using `clang` and link with `core.o`

      clang -g -fno-objc-arc -o hw -std=c99 $TOCCATA_DIR/core.o hw.c -lpthread
     
* Run it

      ./hw
      
# But wait!

## READ THIS SECTION! It will save you hours of frustration.

See that phrase at the very top? I'll put it here just to make sure

      "inspired by Clojure"

Toccata is not a copy or a port of Clojure. There are some very key differences. I'm going to list a few here, but there are others as well.

* `for` is not just for sequences. It works on any data type that implements the `flat-map` protocol function

* `map` is a protocol function. That means it can be implemented for any data type. Not just sequences. It also means the value that is being mapped over comes first and the mapping function comes second. This is the opposite order that Clojure uses.

* There is no Boolean data type, no `true` or `false` values, no `if`, `cond` or `when` forms. This was a very speculative idea and I'm really happy how it worked. There will be a series of blog posts very soon explaining this in detail.

* Comments are nodes in the AST. If you have an S expression that won't compile and it contains comments, try deleting the comments. I've got some rough edges to polish there.

* Right now, documentation consists of this README, the comments and source code in `core.toc` and the programs in the `regression-tests`. Yes, that's pitiful. I'm working on getting blog posts out as quickly as possible.

* Code is added to all C files that track the memory allocations and frees. The stats are printed at the end of each run. If there's a discrepancy, the return code will indicate failure. If you write a Toccata program that consistently fails, I'd be very interested in it. Also, any program that fails with an `incRef` error or a `dec_and_free` error. Those should definitely not happen.

# And now ...

This is just the beginning of a long road to make Toccata into a useful programming language. I deeply appreciate your patience and assistance in making that happen.

Check the HISTORY.md file for a detailed description.

You can learn more about Toccata by following the [blog here](http://toccata.io)
