#ifndef NEW_H
#define NEW_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

// Type definitions
typedef uint64_t u64;
typedef int64_t i64;
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

// Global VM state
extern u64 RNOD_END;
extern u64 RBAG_INI;
extern u64 RBAG_END;
extern bool stop_reducing;
extern Location FREE_LIST; // Head of free list
extern pthread_mutex_t redex_mutex; // Mutex for thread-safe redex operations
extern pthread_cond_t redex_cond; // Condition variable for signaling when redex is available
extern a64* get_buff(void);  // For testing only
void print_free_list(void);   // For debugging

// Tags for different term types
#define VAL 0x00 // positive native value
#define VAR 0x01 // positive variable
#define SUB 0x02 // negative possible deferred redex
#define NUL 0x03 // positive eraser
#define ERA 0x04 // negative eraser
#define LAM 0x05 // positive constructor
#define APP 0x06 // negative constructor
#define REF 0x07 // positive reference
#define VL1 0x08 // positive native value alieas
#define SUP 0x09 // positive duplicator
#define DUP 0x0a // negative duplicator
#define OPX 0x0b // negative operation
#define OPY 0x0c // negative operation
#define I56 0x0d // positive 56 bit int
#define F56 0x0e // positive 56 bit float
#define LAZ 0x0f // positive lazy node
typedef u32 Tag; // Tag is now just an unsigned integer

// Operators
#define OP_ADD 0x00
#define OP_SUB 0x01
#define OP_MUL 0x02
#define OP_DIV 0x03
#define OP_MOD 0x04
#define OP_EQ  0x05
#define OP_NE  0x06
#define OP_LT  0x07
#define OP_GT  0x08
#define OP_LTE 0x09
#define OP_GTE 0x0A
#define OP_AND 0x0B
#define OP_OR  0x0C
#define OP_XOR 0x0D
#define OP_LSH 0x0E
#define OP_RSH 0x0F

// Term is a 64-bit value:
// - Highest 32 bits: Location
// - Next 28 bits: Label
// - Lowest 4 bits: Tag
typedef u64 Term;

// creating number terms
#define new_i56(x) (((u64)x << TAG_SIZE) | I56)
#define get_i56(x) (i64)((u64)x >> TAG_SIZE)
#define get_u64(x) (i64)((u64)x >> TAG_SIZE)
#define new_num(type, x) (((u64)x << TAG_SIZE) | type)

// Function declarations
void hvm_init(u64 size);
void hvm_free(void);
void hvm_reset(void);
void init_free_list(u64 start, u64 end);
Location pair_alloc(void);
void pair_free(Location loc);
const char* tag_to_string(Tag tag);
Term term_new(Tag tag, Lab lab, Location loc);
Tag term_tag(Term term);
Lab term_lab(Term term);
Location term_loc(Term term);
Term swap(Location loc, Term term);
Location port(u64 n, Location x);
// Check term polarity
bool is_positive(Term term);
bool is_negative(Term term);

// Get term at location
Term get(Location loc);
void set(Location loc, Term term);
Term pair_make(Tag tag, Lab lab, Term fst, Term snd);
void term_link(Term neg, Term pos);
void move(Location neg_loc, Term pos);
bool interact(Term neg, Term pos);
void push_redex(Term neg, Term pos);
bool pop_redex(Term* neg, Term* pos);

// Perform interactions until the redex stack is empty
// Returns the number of interactions performed
void normalize(void);

typedef bool (*interactionFn)(Term a, Term b);

void *boom(char *msg, char *file, int line);
#define BOOM(msg) boom(msg, __FILE__, __LINE__)

#endif // NEW_H
