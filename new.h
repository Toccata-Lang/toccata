#ifndef NEW_H
#define NEW_H

// #define CHECK_MEM_LEAK 1
// #define STATS 1
// #define NON_ATOMIC 1
// #define SAFETY 1
// #define STRICT 1

#define HEIGHT 20
extern unsigned hght;

#include <stdatomic.h>

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
#define VAL_MASK 0x7ULL
#define VOID 0

// Term is a 64-bit value:
// - Highest 32 bits: Location
// - Next 28 bits: Label
// - Lowest 4 bits: Tag
typedef u64 Term;

// Define empty free list marker
#define EMPTY_FREE_LIST 0xFFFFFFFE
#define LOCK_FREE_LIST 0xFFFFFFFF

extern a64 RBAG_END;
extern Term* RBAG_BUFF;
#define LOCK_REDEX_STACK 0xFFFFFFFF
extern a64 waiting;

// Global VM state
extern a64 glblAlloced;
extern a64 rdxCount;
extern a64 RNOD_END;
extern int threadCount;
extern pthread_mutex_t redex_mutex; // Mutex for thread-safe redex operations
extern pthread_cond_t redex_cond; // Condition variable for signaling when redex is available
extern Term* get_rbag_buff(void);  // For testing only
void print_free_list(void);   // For debugging
void print_raw_term(Term t);
void print_term(const char* prefix, Term term);
void print_buff(Location start, Location end);
void pb();
void check_buff();
char *refName(Term ref);

// Tags for different term types
#define VAL 0x00 // positive native value
#define VAR 0x01 // positive variable
#define SUB 0x02 // negative possible deferred redex {- +}
#define NUL 0x03 // positive eraser
#define ERA 0x04 // negative eraser
#define LAM 0x05 // positive constructor {- +}
#define APP 0x06 // negative constructor {+ -}
#define REF 0x07 // positive reference
#define VL1 0x08 // positive native value alias
#define SUP 0x09 // positive duplicator {+ +}
#define DUP 0x0a // negative duplicator {- -}
#define OPX 0x0b // negative operation {+ -}
#define OPY 0x0c // negative operation {+ -}
#define I60 0x0d // positive 56 bit int
#define F60 0x0e // positive 56 bit float
#define LAZ 0x0f // positive lazy node {- +}
typedef u32 Tag; // Tag is now just an unsigned integer

#define sideEffects 0x10 + ERA

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


#define MAX_ARGS 15
typedef struct {
  int count;
  Term args[MAX_ARGS + 2];
} NativeArgs;

#define LOCAL_PAIRS_SIZE 4000
typedef struct {
  unsigned count;
  Term rdxs[LOCAL_PAIRS_SIZE][2];
} Pairs;

// creating number terms
#define new_i60(x) (((i64)(x) << TAG_SIZE) | I60)
#define get_i60(x) (i64)((i64)(x) >> TAG_SIZE)
#define get_u64(x) (i64)((u64)(x) >> TAG_SIZE)
#define new_num(type, x) (((u64)(x) << TAG_SIZE) | type)

// Function declarations
void hvm_init(u64 size);
void hvm_free(void);
void hvm_reset(void);
void init_free_list(u64 start, u64 end);
Location pair_alloc(void);
void pair_free(Location loc);
// Term pair_make(Tag tag, Lab lab, Term fst, Term snd);
Term maker(int line, Tag tag, Lab lab, Term fst, Term snd);
const char* tag_to_str(Tag tag);
Term term_new(Tag tag, Lab lab, Location loc);
Term term_val(Term val);
Tag term_tag(Term term);
Lab term_lab(Term term);
Location term_loc(Term term);
Location port(u64 n, Location x);
// Check term polarity
bool is_positive(Term term);
bool is_negative(Term term);

// this abuses the compile time functionality
// to create a Port value
#define term_new_(tag, x) (((u64)x + tag))

// Get term at location
Term get(Location loc);
Term take(Location loc);
void freeLoc(Location loc);
void interact(Term neg, Term pos);
void push_redex(Term neg, Term pos);
bool pop_redex(Term* neg, Term* pos);
void store_redex(Term neg, Term pos);
Term swapStore(Location loc, Term term);
void moveStore(Location neg_loc, Term pos);
Term strictArgs(Term ref, Term args, int expected, NativeArgs *argsStruct);
void link_redexes();
void store_redex(Term neg, Term pos);
Term dupeArg(Term arg, Term *dupedArg, unsigned dupLabel);
Term make_op(Lab op, Term x, Term y);

// Perform interactions until the redex stack is empty
// Returns the number of interactions performed
void *normalize(void *v);
void forceLazy(Term z);
void spawn_threads();
extern pthread_t threads[];

typedef void (*interactionFn)(Term a, Term b);
#define new_ref(x) (((u64)x + REF))

extern unsigned refsCount;
typedef struct {
  interactionFn fn;
  char *name;} refMap;
extern refMap refNames[];

// Create a REF term with a specific interaction function
Term ref_make(interactionFn fn);

u64 time64();
void *boom(char *msg, char *file, int line);
#define BOOM(msg) boom(msg, __FILE__, __LINE__)
#endif // NEW_H

extern pthread_mutex_t buff_mutex;

#define pair_make(tag, lab, fst, snd) maker(__LINE__, tag, lab, fst, snd)

unsigned graphDown(char *title, Term root);
void graphUp(char *title, Term root);
