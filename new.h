#ifndef NEW_H
#define NEW_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Type definitions
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint32_t Location;
typedef uint32_t Lab;
typedef atomic_uint_least64_t a64;

// Bit layout constants
#define TAG_SIZE 4
#define LAB_SIZE 28
#define LOC_SIZE 32
#define TAG_MASK 0xFULL
#define LAB_MASK 0xFFFFFFFULL
#define LOC_MASK 0xFFFFFFFFULL

// Tags for different term types
typedef enum {
    VAR, // Positive variable
    SUB, // Negative variable (aux wire)
    NUL, // Positive eraser node
    ERA, // Negative eraser node
    LAM, // Positive constructor node
    APP, // Negative constructor node
    SUP, // Positive duplicator node
    DUP  // Negative duplicator node
} Tag;

// Term is a 64-bit value:
// - Highest 32 bits: Location
// - Next 28 bits: Label
// - Lowest 4 bits: Tag
typedef u64 Term;

// Function declarations
void hvm_init(u64 size);
void hvm_free(void);
void hvm_reset(void);
const char* tag_to_string(Tag tag);
Term term_new(Tag tag, Lab lab, Location loc);
Tag term_tag(Term term);
Lab term_lab(Term term);
Location term_loc(Term term);
Term swap(Location loc, Term term);
Location port(u64 n, Location x);
Term get(Location loc);
void set(Location loc, Term term);
Term pair_make(Tag tag, Term fst, Term snd);
void link(Term neg, Term pos);
void move(Location neg_loc, Term pos);
void applam(Location neg_loc, Location pos_loc);
void duplam(Location neg_loc, Location pos_loc);

#endif // NEW_H
