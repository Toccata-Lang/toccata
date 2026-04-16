# Makefile for toccata compiler

# Variables
PROJECT_NAME = toccata
VERSION = 1.0.0
CC = clang
CFLAGS = -g -std=c99
LDFLAGS = -lpthread -latomic
TARGET = $(PROJECT_NAME)

# Default target
.PHONY: toccata
toccata: toccata.c core.c
	@echo "Building toccata"
	$(CC) -O3 $(CFLAGS) -o toccata toccata.c core.c $(LDFLAGS)

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  toccata - Build the base toccata compiler"
	@echo "  clean   - Remove build files"
	@echo "  help    - Show this help message"

# Clean build files
.PHONY: clean
clean:
	rm -f toccata

# Default help if no target specified
.DEFAULT_GOAL := help
