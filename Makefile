# Makefile for toccata compiler

# Variables
PROJECT_NAME = toccata
VERSION = 1.0.0
CC = clang
CFLAGS = -g
LDFLAGS = -lpthread -latomic
TOC_FLAGS = -march=native -I. -DCHECK_MEM_LEAK=1 -DSAFETY=1 -DSTATS=1 -lm 
TARGET = $(PROJECT_NAME)

# Test targets pattern
REG_TESTS = test1 test2 test3 test4 test5 test6 test7 test8 test9 test10 test11 test12 test13 \
        test14 test15 test16 test17 test18 comment-in-deftype test-global-empty-list \
        test-ignore-inferred cond-expr-1 cond-expr-2 cond-expr-3 cond-expr-4 \
        free-static-value tail-cond-1 test-inline-namespaced-sym test-closures # vector-regressions
TEST_SOURCES = new.c runtime3.c

new-toc: compiler.toc base.toc typer.toc codegen.toc toccata
	./toccata compiler.toc > new-toc.tmp
	sed -i 's/maybe((FnArity/maybe((Vector/' new-toc.tmp
	awk '/^#$$/ { printf "#line %d \"%s\"\n", NR+1, "new-toc.c"; next; } { print; }' new-toc.tmp > new-toc.c
	clang-format -i new-toc.c
	$(CC) $(TOC_FLAGS) -DWAIT_FOR_LINGERING=1 -o new-toc -std=c99 core.c new-toc.c $(LDFLAGS)

# Generate C files from .toc files using pattern rules
regression-tests/%.c: new-toc regression-tests/%.toc hvm-core.toc
	./new-toc regression-tests/$*.toc > regression-tests/$*.tmp
	awk '/^#$$/ { printf "#line %d \"%s\"\n", NR+1, "m.c"; next; } { print; }' \
          regression-tests/$*.tmp > regression-tests/$*.c
	clang-format -i regression-tests/$*.c
	rm regression-tests/$*.tmp

# Generic rule for test targets
.PHONY: $(REG_TESTS)
$(REG_TESTS): %: regression-tests/%.c $(TEST_SOURCES)
	$(CC) $(CFLAGS) -o regression-tests/$@ $(TOC_FLAGS) $(LDFLAGS) $(TEST_SOURCES) regression-tests/$*.c
	regression-tests/$@ party-pooper | sort > regression-tests/$*.rslt

# Run all tests
.PHONY: tests
tests: $(REG_TESTS)

intrp.c: intrp-ast.toc intrp-rdr.toc hvm-core.toc new-toc
	./new-toc intrp-rdr.toc > $*.tmp
	awk '/^#$$/ { printf "#line %d \"%s\"\n", NR+1, "m.c"; next; } { print; }' $*.tmp > $*.c
	rm $*.tmp

.PHONY: intrp
intrp: intrp.c
	$(CC) $(CFLAGS) -o $* $(TOC_FLAGS) $(LDFLAGS) $*.c
	./intrp

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  tests     - Build and run all tests"
	@echo "  clean-c   - Remove all test C files"
	@echo "  help      - Show this help message"

# Clean build files
# .PHONY: clean
# clean:
# 	rm -f toccata

# Default help if no target specified
.DEFAULT_GOAL := help

# Clean C files
.PHONY: clean-c
clean-c:
	rm -f $(addprefix regression-tests/, $(addsuffix .c, $(REG_TESTS))) $(addprefix regression-tests/, $(REG_TESTS))
