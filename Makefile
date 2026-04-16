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

# Generate C files from .toc files using pattern rules
regression-tests/%.c: new-toc regression-tests/%.toc
	@echo "Building $*.c"
	./new-toc regression-tests/$*.toc > regression-tests/$*.tmp
	awk '/^#$$/ { printf "#line %d \"%s\"\n", NR+1, "m.c"; next; } { print; }' \
          regression-tests/$*.tmp > regression-tests/$*.c
	rm regression-tests/$*.tmp
	@echo "$*.c generated successfully"

# Generic rule for test targets
.PHONY: $(TESTS)
$(TESTS): %: regression-tests/%.c $(TEST_SOURCES)
	$(CC) $(CFLAGS) -o regression-tests/$@ $(TOC_FLAGS) $(LDFLAGS) $(TEST_SOURCES) regression-tests/$*.c
	regression-tests/$@ party-pooper | sort > regression-tests/$*.rslt

# Run all tests
.PHONY: tests
tests: $(TESTS)

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
	rm -f $(addprefix regression-tests/, $(addsuffix .c, $(TESTS)))
