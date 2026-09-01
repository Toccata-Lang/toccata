# Makefile for toccata compiler

# Variables
PROJECT_NAME = toccata
VERSION = 1.0.0
CC = clang
CFLAGS = -g # -fsanitize=address
LDFLAGS = -lpthread -latomic
TOC_FLAGS = -march=native -I. -DCHECK_MEM_LEAK=1 -DSAFETY=1 -DSTATS=1 -lm 
TARGET = $(PROJECT_NAME)

# Test targets pattern

REG_TESTS = test1 test2 test3 test4 test5 test6 test7 test8 test9 test10 test11 test12 test13 \
	test14 test15 test16 test17 test18 comment-in-deftype test-global-empty-list function-regressions \
	test-ignore-inferred cond-expr-1 cond-expr-2 cond-expr-3 cond-expr-4 cond-expr-5 \
	free-static-value tail-cond-1 test-inline-namespaced-sym integer-regressions string-regressions \
	test-inlined-result-constraint vector-regressions test-either test-comment-in-let \
	test-recursive-map-fn test-tail-recur-1 test-tail-recur-2 test-tail-recur-3 test-trailing-comment \
	test-underscore-inline test-uni test-closures test-fusing test-bmi test-array-node test-collision-node \
	test-threading test-or-comment hash-map-regressions closure-capture-sup
TEST_SOURCES = new.c runtime3.c graph.c

new-toc: compiler.toc base.toc typer.toc codegen.toc toccata
	./toccata compiler.toc > new-toc.tmp
	sed -i 's/maybe((FnArity/maybe((Vector/' new-toc.tmp
	awk '/^#$$/ { printf "#line %d \"%s\"\n", NR+1, "new-toc.c"; next; } { print; }' new-toc.tmp > new-toc.c
	clang-format -i new-toc.c
	$(CC) -march=native -I. -lm  -DWAIT_FOR_LINGERING=1 -o new-toc -std=c99 core.c new-toc.c $(LDFLAGS)

# Generate C files from .toc files using pattern rules
regression-tests/%.c: new-toc regression-tests/%.toc regression-tests/regression-tester.toc hvm-core.toc
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

# Interpeter
intrp.c: interpreter/intrp-ast.toc interpreter/intrp-rdr.toc hvm-core.toc new-toc
	./new-toc interpreter/intrp-rdr.toc > $*.tmp
	awk '/^#$$/ { printf "#line %d \"%s\"\n", NR+1, "m.c"; next; } { print; }' $*.tmp > $*.c
	rm $*.tmp

.PHONY: intrp
intrp: intrp.c
	$(CC) $(CFLAGS) -o $* $(TOC_FLAGS) $(LDFLAGS) $*.c
	./intrp

# Reader drivers
.PHONY: rdr-exprs
rdr-exprs: new-toc interpreter/intrp-ast.toc interpreter/intrp-rdr.toc interpreter/rdr-exprs.toc hvm-core.toc
	./new-toc interpreter/rdr-exprs.toc > rdr-exprs.tmp
	awk '/^#$$/ { printf "#line %d \"m.c\"\n", NR+1, "rdr-exprs.c"; next; } { print; }' rdr-exprs.tmp > rdr-exprs.c
	clang-format -i rdr-exprs.c
	rm rdr-exprs.tmp
	$(CC) $(CFLAGS) -o rdr-exprs $(TOC_FLAGS) $(LDFLAGS) rdr-exprs.c new.c runtime3.c graph.c
	./rdr-exprs

.PHONY: rdr-defp
rdr-defp: new-toc interpreter/intrp-ast.toc interpreter/intrp-rdr.toc interpreter/rdr-defp.toc hvm-core.toc
	./new-toc interpreter/rdr-defp.toc > rdr-defp.tmp
	awk '/^#$$/ { printf "#line %d \"m.c\"\n", NR+1, "rdr-defp.c"; next; } { print; }' rdr-defp.tmp > rdr-defp.c
	clang-format -i rdr-defp.c
	rm rdr-defp.tmp
	$(CC) $(CFLAGS) -o rdr-defp $(TOC_FLAGS) $(LDFLAGS) rdr-defp.c new.c runtime3.c graph.c
	./rdr-defp

.PHONY: rdr-deftype
rdr-deftype: new-toc interpreter/intrp-ast.toc interpreter/intrp-rdr.toc interpreter/rdr-deftype.toc hvm-core.toc
	./new-toc interpreter/rdr-deftype.toc > rdr-deftype.tmp
	awk '/^#$$/ { printf "#line %d \"m.c\"\n", NR+1, "rdr-deftype.c"; next; } { print; }' rdr-deftype.tmp > rdr-deftype.c
	clang-format -i rdr-deftype.c
	rm rdr-deftype.tmp
	$(CC) $(CFLAGS) -o rdr-deftype $(TOC_FLAGS) $(LDFLAGS) rdr-deftype.c new.c runtime3.c graph.c
	./rdr-deftype

.PHONY: rdr-extend-type
rdr-extend-type: new-toc interpreter/intrp-ast.toc interpreter/intrp-rdr.toc interpreter/rdr-extend-type.toc hvm-core.toc
	./new-toc interpreter/rdr-extend-type.toc > rdr-extend-type.tmp
	awk '/^#$$/ { printf "#line %d \"m.c\"\n", NR+1, "rdr-extend-type.c"; next; } { print; }' rdr-extend-type.tmp > rdr-extend-type.c
	clang-format -i rdr-extend-type.c
	rm rdr-extend-type.tmp
	$(CC) $(CFLAGS) -o rdr-extend-type $(TOC_FLAGS) $(LDFLAGS) rdr-extend-type.c new.c runtime3.c graph.c
	./rdr-extend-type

.PHONY: rdr-top
rdr-top: new-toc interpreter/intrp-ast.toc interpreter/intrp-rdr.toc interpreter/rdr-top.toc hvm-core.toc
	./new-toc interpreter/rdr-top.toc > rdr-top.tmp
	awk '/^#$$/ { printf "#line %d \"m.c\"\n", NR+1, "rdr-top.c"; next; } { print; }' rdr-top.tmp > rdr-top.c
	clang-format -i rdr-top.c
	rm rdr-top.tmp
	$(CC) $(CFLAGS) -o rdr-top $(TOC_FLAGS) $(LDFLAGS) rdr-top.c new.c runtime3.c graph.c
	./rdr-top

# Sidequest
sidequest.c: new-toc sidequest.toc hvm-core.toc new.h
	./new-toc sidequest.toc > sidequest.tmp
	awk '/^#$$/ { printf "#line %d \"%s\"\n", NR+1, "m.c"; next; } { print; }' \
          sidequest.tmp > sidequest.c
	clang-format -i sidequest.c
	rm sidequest.tmp

sidequest: sidequest.c $(TEST_SOURCES)
	$(CC) $(CFLAGS) -o sidequest $(TOC_FLAGS) $(LDFLAGS) $(TEST_SOURCES) sidequest.c
	./sidequest || dot -Tsvg graphs.dot > graphs.svg

# Cross-namespace extend-type probe (interpreter/xns-probe.toc)
interpreter/xns-probe.c: new-toc interpreter/xns-probe.toc interpreter/intrp-ast.toc interpreter/intrp-rdr.toc hvm-core.toc
	./new-toc interpreter/xns-probe.toc > interpreter/xns-probe.tmp
	awk '/^#$$/ { printf "#line %d \"%s\"\n", NR+1, "m.c"; next; } { print; }' \
          interpreter/xns-probe.tmp > interpreter/xns-probe.c
	rm interpreter/xns-probe.tmp
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

# Test HVM
.PHONY: test-hvm
test-hvm:
	$(CC) $(CFLAGS) -o regression-tests/test-hvm $(TOC_FLAGS) $(LDFLAGS) -DTESTING_HVM=1 new.c regression-tests/test-hvm.c graph.c runtime3.c
	./regression-tests/test-hvm
	dot -Tsvg graphs.dot > graphs.svg

.PHONY: test-hash-map
test-hash-map:
	$(CC) $(CFLAGS) -o regression-tests/test-hash-map $(TOC_FLAGS) $(LDFLAGS) -DTESTING_HVM=1 new.c regression-tests/test-hash-map.c runtime3.c
	./regression-tests/test-hash-map

.PHONY: test-runtime
test-runtime:
	$(CC) $(CFLAGS) -o regression-tests/test-runtime $(TOC_FLAGS) $(LDFLAGS) -DTESTING_HVM=1 new.c regression-tests/test-runtime.c graph.c runtime3.c
	./regression-tests/test-runtime

# Default help if no target specified
.DEFAULT_GOAL := help

# Graphs
DOT_FILES := $(wildcard graphs/*.dot)
SVG_FILES := $(DOT_FILES:.dot=.svg)

.PHONY: graphs
graphs: $(SVG_FILES)

graphs/%.svg: graphs/%.dot
	dot -Tsvg $< > $@

# Clean C files
.PHONY: clean-c
clean-c:
	rm -f $(addprefix regression-tests/, $(addsuffix .c, $(REG_TESTS))) $(addprefix regression-tests/, $(REG_TESTS))
