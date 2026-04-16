# Makefile for toccata compiler

# Variables
PROJECT_NAME = toccata
VERSION = 1.0.0
CC = clang
CFLAGS = -g
LDFLAGS = -lpthread -latomic
TOC_FLAGS = -march=native -I/home/jim/toccata -DCHECK_MEM_LEAK=1 -DSAFETY=1 -DSTATS=1 -lm 
TARGET = $(PROJECT_NAME)

# Test targets pattern
TESTS = test1 test2
TEST_SOURCES = new.c runtime3.c

# Generic rule for test targets
.PHONY: $(TESTS)
$(TESTS): %: %.c $(TEST_SOURCES)
	$(CC) $(CFLAGS) -o regression-tests/$@ $(TOC_FLAGS) $(LDFLAGS) $(TEST_SOURCES) regression-tests/$*.c
	regression-tests/$@ party-pooper | sort > regression-tests/$*.rslt

# Generate C files from .toc files using TESTS
.PHONY: $(addsuffix .c, $(TESTS))
$(addsuffix .c, $(TESTS)): %.c: new-toc regression-tests/%.toc
	@echo "Building $@"
	./new-toc regression-tests/$*.toc > regression-tests/$*.tmp
	awk '/^#$$/ { printf "#line %d \"%s\"\n", NR+1, "m.c"; next; } { print; }' \
          regression-tests/$*.tmp > regression-tests/$*.c
	rm regression-tests/$*.tmp
	@echo "$*.c generated successfully"

# Run all tests
.PHONY: tests
tests: $(TESTS)

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  tests   - Build and run all tests"
	@echo "  help    - Show this help message"

# Clean build files
# .PHONY: clean
# clean:
# 	rm -f toccata

# Default help if no target specified
.DEFAULT_GOAL := help
