# Makefile for toccata compiler

# Variables
PROJECT_NAME = toccata
VERSION = 1.0.0
CC = clang
CFLAGS = -g
LDFLAGS = -lpthread -latomic
TOC_FLAGS = -march=native -I/home/jim/toccata -DCHECK_MEM_LEAK=1 -DSAFETY=1 -DSTATS=1 -lm 
TARGET = $(PROJECT_NAME)

# .PHONY: toccata
# toccata: toccata.c core.c
# 	echo "Building toccata"; \
# 	$(CC) -O3 $(CFLAGS) -o toccata toccata.c core.c $(LDFLAGS); \

# Test1 target
.PHONY: test1
test1: test1.c new.c runtime3.c
	$(CC) $(CFLAGS) -o regression-tests/test1 $(TOC_FLAGS) $(LDFLAGS) runtime3.c new.c regression-tests/test1.c
	regression-tests/test1 party-pooper | sort > regression-tests/test1.rslt

test1.c: new-toc regression-tests/test1.toc
	@echo "Building test1"
	./new-toc regression-tests/test1.toc > regression-tests/test1.tmp
	awk '/^#$$/ { printf "#line %d \"%s\"\n", NR+1, "m.c"; next; } { print; }' \
          regression-tests/test1.tmp > regression-tests/test1.c
	rm regression-tests/test1.tmp
	@echo "test1.c generated successfully"

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  test1   - Build and run test1"
	@echo "  help    - Show this help message"

# Clean build files
# .PHONY: clean
# clean:
# 	rm -f toccata

# Default help if no target specified
.DEFAULT_GOAL := help
